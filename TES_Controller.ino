#include <StaticSerialCommands.h>

#include "src/drivers/LTC4302.h"
#include "src/drivers/MCP4728.h"
#include "src/drivers/INA219.h" // Include the INA219 header
#include "src/routers/Router.h" // Include the Router header
#include "src/devices/LNADriver.h" // Include the LNADriver header
#include "src/devices/TESDriver.h" // Include the TESDriver header


// Define I2C addresses for the devices
#define BASE_HUB_LTC4302_ADDR 0x7E // Address for the base hub LTC4302
#define BASE_HUB_MCP4728_ADDR 0x61 // Address for the main MCP4728

// Make TES/LNA counts configurable in one place
#define NUM_TES 12
#define NUM_LNA 2

// Default address lists (easy to edit / override)
const uint8_t DEFAULT_TES_ADDRESSES[NUM_TES] = {
        0x72, 0x62, 0x63, 0x64, 0x65, 0x66,
        0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C
};

const uint8_t DEFAULT_LNA_ADDRESSES[NUM_LNA] = {
        0x6D, 0x6E
};

// Base hub (unchanged)
LTC4302 baseHub(BASE_HUB_LTC4302_ADDR);

// Router uses baseHub
Router router(&baseHub);

// Route for main MCP4728 (Base Hub -> MCP4728)
I2CRoute routeToMainMCP4728 = { &baseHub, nullptr };
MCP4728 mainDac(BASE_HUB_MCP4728_ADDR);

// Use pointer arrays so the active count can vary at runtime/compile-time
LTC4302* tesLTC[NUM_TES];
TESDriver* tesDriver[NUM_TES];

LTC4302* lnaLTC[NUM_LNA];
LNADriver* lnaDriver[NUM_LNA];

// ----- Command definitions -----------------------------------------------------------
constexpr auto lnaChanArg =
    ARG(ArgType::Int, 1, NUM_LNA, "CHANNEL");

constexpr auto tesChanArg =
    ARG(ArgType::Int, 1, NUM_TES, "CHANNEL");

constexpr auto lnaDrainGate = 
    ARG(ArgType::String, "DRAIN|GATE");

constexpr auto dacValueArg =
    ARG(ArgType::Int, 0, 4095, "VALUE");

constexpr auto tesTCAArg = 
    ARG(ArgType::Int, 0, 0xFFFFF, "BITS"); // 20 bits for TES TCA

constexpr auto tesTCAHexArg = 
    ARG(ArgType::String, "HEXSTRING"); // Hex string for TES TCA

constexpr auto tcaCurrentArg = 
    ARG(ArgType::Float, 0, 20, "CURRENT");



void cmdLNA(SerialCommands& sender, Args& args);
void cmdTES(SerialCommands& sender, Args& args);

void cmdDAC(SerialCommands& sender, Args& args);
void cmdDACSet(SerialCommands& sender, Args& args);
void cmdDACGet(SerialCommands& sender, Args& args);

void cmdLNAGetAll(SerialCommands& sender, Args& args);
void cmdLNASet(SerialCommands& sender, Args& args);
void cmdLNAShunt(SerialCommands& sender, Args& args);
void cmdLNABus(SerialCommands& sender, Args& args);
void cmdLNACurrent(SerialCommands& sender, Args& args);
void cmdLNAPower(SerialCommands& sender, Args& args);
void cmdLNAEnable(SerialCommands& sender, Args& args);
void cmdLNADisable(SerialCommands& sender, Args& args);

void cmdTESGetAll(SerialCommands& sender, Args& args);
void cmdTESSet(SerialCommands& sender, Args& args);
void cmdTESSetInt(SerialCommands& sender, Args& args);
void cmdTESSetHex(SerialCommands& sender, Args& args);
void cmdTESBits(SerialCommands& sender, Args& args);
void cmdTESEnable(SerialCommands& sender, Args& args);
void cmdTESDisable(SerialCommands& sender, Args& args);
void cmdTESInc(SerialCommands& sender, Args& args);
void cmdTESDec(SerialCommands& sender, Args& args);
void cmdTESShunt(SerialCommands& sender, Args& args);
void cmdTESBus(SerialCommands& sender, Args& args);
void cmdTESCurrent(SerialCommands& sender, Args& args);
void cmdTESPower(SerialCommands& sender, Args& args);

void cmdHelp(SerialCommands& sender, Args& args);

Command lnaCommands[] = {
    COMMAND(cmdLNAGetAll, "GET", nullptr, "Get All LNA Parameters for Gate/Drain"),
    COMMAND(cmdLNAEnable, "ENABLE", nullptr, "Enable Gate/Drain"),
    COMMAND(cmdLNADisable, "DISABLE", nullptr, "Disable Gate/Drain"),
    COMMAND(cmdLNASet, "SET", dacValueArg, nullptr, "Set Gate/Drain DAC Value"),
    COMMAND(cmdLNAShunt, "SHUNT", nullptr, "Get Gate/Drain Shunt Voltage (mV)"),
    COMMAND(cmdLNABus, "BUS", nullptr, "Get Gate/Drain Bus Voltage (V)"),
    COMMAND(cmdLNACurrent, "CURRENT", nullptr, "Get Gate/Drain Current (mA)"),
    COMMAND(cmdLNAPower, "POWER", nullptr, "Get Gate/Drain Power (mW)"),
};

Command tesCommands[] = {
    COMMAND(cmdTESGetAll, "GET", nullptr, "Get All TES Parameters"),
    COMMAND(cmdTESEnable, "ENABLE", nullptr, "Enable TES Outputs"),
    COMMAND(cmdTESDisable, "DISABLE", nullptr, "Disable TES Outputs"),
    COMMAND(cmdTESSet, "SET", tcaCurrentArg, nullptr, "Search and set TES Output Current (mA)"),
    COMMAND(cmdTESSetInt, "SETINT", tesTCAArg, nullptr, "Set TES TCA Output Bits (Integer)"),
    COMMAND(cmdTESSetHex, "SETHEX", tesTCAHexArg, nullptr, "Set TES TCA Output Bits (Hex)"),
    COMMAND(cmdTESBits, "BIT", nullptr, "Get TES TCA Output Bits (Hex)"),
    COMMAND(cmdTESInc, "INC", tesTCAArg, nullptr, "Increase TES TCA Output Bits (Integer)"),
    COMMAND(cmdTESDec, "DEC", tesTCAArg, nullptr, "Decrease TES TCA Output Bits (Integer)"),
    COMMAND(cmdTESShunt, "SHUNT", nullptr, "Get TES Shunt Voltage (mV)"),
    COMMAND(cmdTESBus, "BUS", nullptr, "Get TES Bus Voltage (V)"),
    COMMAND(cmdTESCurrent, "CURRENT", nullptr, "Get TES Current (mA)"),
    COMMAND(cmdTESPower, "POWER", nullptr, "Get TES Power (mW)"),
};

Command dacCommands[] = {
    COMMAND(cmdDACSet, "SET", dacValueArg, nullptr, "Set Main DAC Value"),
    COMMAND(cmdDACGet, "GET", nullptr, "Get Main DAC Value"),
};

Command commands[] = {
    COMMAND(cmdLNA, "LNA", lnaChanArg, lnaDrainGate, lnaCommands, "LNA Commands"),
    COMMAND(cmdTES, "TES", tesChanArg, tesCommands, "TES Commands"),
    COMMAND(cmdDAC, "DAC", dacCommands, "DAC Commands"),
    COMMAND(cmdHelp, "HELP", nullptr, "List All Commands"),
};

SerialCommands serialCommands(Serial, commands, sizeof(commands) / sizeof(Command));


// Helper to initialize devices (call early in setup before begin() calls)
void initDeviceArrays() {
    // Initialize TES LTC4302s and TESDrivers
    for (int i = 0; i < NUM_TES; ++i) {
            uint8_t addr = DEFAULT_TES_ADDRESSES[i];
            tesLTC[i] = new LTC4302(addr);
            tesDriver[i] = new TESDriver(tesLTC[i], &router);
    }

    // Initialize LNA LTC4302s and LNADrivers
    for (int i = 0; i < NUM_LNA; ++i) {
            uint8_t addr = DEFAULT_LNA_ADDRESSES[i];
            lnaLTC[i] = new LTC4302(addr);
            lnaDriver[i] = new LNADriver(lnaLTC[i], &router);
    }
}

// Helper to print out 0 padded hex values
String toPaddedHex(uint32_t value, uint8_t width) {
    String hexStr = String(value, HEX);
    while (hexStr.length() < width) {
        hexStr = "0" + hexStr;
    }
    hexStr.toUpperCase();
    return hexStr;
}

// --- YAML response helpers -------------------------------------------------
String yamlEscape(const String &s) {
    String out = s;
    out.replace("\\", "\\\\");
    out.replace("\"", "\\\"");
    return out;
}

void printIndent(Stream &s, int indent) {
    for (int i = 0; i < indent; ++i) s.print(' ');
}

void printYAMLKeyValue(Stream &s, const char *key, const String &value, int indent = 2, bool quote = true) {
    printIndent(s, indent);
    s.print(key);
    s.print(": ");
    if (quote) {
        s.print('"');
        s.print(yamlEscape(value));
        s.print('"');
        s.println();
    } else {
        s.println(value);
    }
}

void printYAMLHeader(Stream &s, const char *status) {
    s.println("---");
    s.print("status: ");
    s.println(status);
    s.println("result:");
}

// convenience to finish a response with an optional human message key
void printYAMLMessage(Stream &s, const String &message) {
    if (message.length()) {
        printYAMLKeyValue(s, "message", message, 2, true);
    }
    s.println();
}

bool reportIfError(SerialCommands& sender, uint8_t status, const char* errKey, const char* message) {
    if (status) {
        Stream &out = sender.getSerial();
        printYAMLHeader(out, "error");
        printYAMLKeyValue(out, "error", String(errKey), 2, true);
        // include numeric status code for easier debugging
        printYAMLKeyValue(out, "code", String(status), 2, false);
        printYAMLMessage(out, String(message));
        return true;
    }
    return false;
}

void reportError(SerialCommands& sender, const char* errKey, const char* message) {
    Stream &out = sender.getSerial();
    printYAMLHeader(out, "error");
    printYAMLKeyValue(out, "error", String(errKey), 2, true);
    printYAMLMessage(out, String(message));
}

// ---------------------------------------------------------------------------

void setup() {
    Serial.begin(115200);
    Serial.println("TES Controller Starting...");

    initDeviceArrays();
    uint8_t status;

    status = router.begin();
    if (status) {
        Serial.println("Error initializing Base Hub LTC4302");
    }
    
    status = baseHub.enableBus();
    if (status) {
        Serial.println("Error enabling I2C bus on Base Hub LTC4302");
    }

    status = mainDac.begin();
    if (status) {
        Serial.println("Error initializing Main MCP4728 DAC");
    }

    // Initialize TESs
    for (int i = 0; i < NUM_TES; ++i) {
        status = tesDriver[i]->begin();
        if (status) {
            Serial.print("Error initializing TES Driver for channel "); Serial.println(i + 1);
        }
    }
    // Initialize LNAs
    for (int i = 0; i < NUM_LNA; ++i) {
        status = lnaDriver[i]->begin();
        if (status) {
            Serial.print("Error initializing LNA Driver for channel "); Serial.println(i + 1);
        }
    }

    // mainDac.writeDAC(MCP4728_CHANNEL_A, 1024); // Set channel A to ~1/4-scale (4095 max)
    Serial.println("Initialization complete.");
}

void loop() {
    serialCommands.readSerial();
}

void cmdHelp(SerialCommands& sender, Args& args) {
    sender.listAllCommands(commands, sizeof(commands) / sizeof(Command));
}

void cmdLNA(SerialCommands& sender, Args& args) {
    sender.listAllCommands(lnaCommands, sizeof(lnaCommands) / sizeof(Command));
}

void cmdTES(SerialCommands& sender, Args& args) {
    sender.listAllCommands(tesCommands, sizeof(tesCommands) / sizeof(Command));
}

void cmdDAC(SerialCommands& sender, Args& args) {
    sender.listAllCommands(dacCommands, sizeof(dacCommands) / sizeof(Command));
}

void cmdDACSet(SerialCommands& sender, Args& args) {
    uint16_t value = args[0].getInt();
    uint8_t status;
    // Implement setting main DAC value
    status = mainDac.writeDAC(MCP4728_CHANNEL_A, value);
    if (reportIfError(sender, status, "DAC_SET_ERROR", "Failed to set main DAC value.")) {
        return;
    }
    Stream &out = sender.getSerial();
    printYAMLHeader(out, "ok");
    printYAMLKeyValue(out, "command", "DAC_SET", 2, true);
    printYAMLKeyValue(out, "value", String(value), 2, false);
    printYAMLMessage(out, "Main DAC value set");
}

void cmdDACGet(SerialCommands& sender, Args& args) {
    uint16_t value;
    uint8_t status;
    // Implement getting main DAC value
    status = mainDac.readDAC(MCP4728_CHANNEL_A, value);
    if (reportIfError(sender, status, "DAC_GET_ERROR", "Failed to get main DAC value.")) {
        return;
    }
    Stream &out = sender.getSerial();
    printYAMLHeader(out, "ok");
    printYAMLKeyValue(out, "command", "DAC_GET", 2, true);
    printYAMLKeyValue(out, "value", String(value), 2, false);
    printYAMLMessage(out, "Main DAC value retrieved");
}

void cmdLNASet(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    const char* target = args[1].getString();
    uint16_t value = args[2].getInt();
    uint8_t status;
    if (strcmp(target, "DRAIN") == 0) {
        status = lnaDriver[channel]->writeDrain(value);
        if (reportIfError(sender, status, "LNA_SET_ERROR", "Failed to set Drain DAC value.")) {
            return;
        }
        Stream &out = sender.getSerial();
        printYAMLHeader(out, "ok");
        printYAMLKeyValue(out, "command", "LNA_SET", 2, true);
        printYAMLKeyValue(out, "channel", String(channel + 1), 2, false);
        printYAMLKeyValue(out, "target", "DRAIN", 2, true);
        printYAMLKeyValue(out, "value", String(value), 2, false);
        printYAMLMessage(out, "LNA DRAIN DAC value set");
    } else if (strcmp(target, "GATE") == 0) {
        status = lnaDriver[channel]->writeGate(value);
        if (reportIfError(sender, status, "LNA_SET_ERROR", "Failed to set Gate DAC value.")) {
            return;
        }
        Stream &out = sender.getSerial();
        printYAMLHeader(out, "ok");
        printYAMLKeyValue(out, "command", "LNA_SET", 2, true);
        printYAMLKeyValue(out, "channel", String(channel + 1), 2, false);
        printYAMLKeyValue(out, "target", "GATE", 2, true);
        printYAMLKeyValue(out, "value", String(value), 2, false);
        printYAMLMessage(out, "LNA GATE DAC value set");
    } else {
        reportError(sender, "Invalid target. Use DRAIN or GATE.", "Invalid target");
    }
}

void cmdLNAShunt(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    const char* target = args[1].getString();
    float shuntVoltage;
    uint8_t status;
    if (strcmp(target, "DRAIN") == 0) {
        status = lnaDriver[channel]->getDrainShuntVoltage_mV(shuntVoltage);
        if (reportIfError(sender, status, "LNA_SHUNT_READ_ERROR", "Failed to read Drain shunt voltage.")) {
            return;
        }
        Stream &out = sender.getSerial();
        printYAMLHeader(out, "ok");
        printYAMLKeyValue(out, "command", "LNA_SHUNT", 2, true);
        printYAMLKeyValue(out, "channel", String(channel + 1), 2, false);
        printYAMLKeyValue(out, "target", "DRAIN", 2, true);
        printYAMLKeyValue(out, "shunt_mV", String(shuntVoltage), 2, false);
        printYAMLMessage(out, "Drain shunt voltage (mV)");
    } else if (strcmp(target, "GATE") == 0) {
        status = lnaDriver[channel]->getGateShuntVoltage_mV(shuntVoltage);
        if (reportIfError(sender, status, "LNA_SHUNT_READ_ERROR", "Failed to read Gate shunt voltage.")) {
            return;
        }
        Stream &out = sender.getSerial();
        printYAMLHeader(out, "ok");
        printYAMLKeyValue(out, "command", "LNA_SHUNT", 2, true);
        printYAMLKeyValue(out, "channel", String(channel + 1), 2, false);
        printYAMLKeyValue(out, "target", "GATE", 2, true);
        printYAMLKeyValue(out, "shunt_mV", String(shuntVoltage), 2, false);
        printYAMLMessage(out, "Gate shunt voltage (mV)");
    } else {
        reportError(sender, "Invalid target. Use DRAIN or GATE.", "Invalid target");
    }
}

void cmdLNABus(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    const char* target = args[1].getString();
    float busVoltage;
    uint8_t status;
    if (strcmp(target, "DRAIN") == 0) {
        status = lnaDriver[channel]->getDrainBusVoltage_V(busVoltage);
        if (reportIfError(sender, status, "LNA_BUS_READ_ERROR", "Failed to read Drain bus voltage.")) {
            return;
        }
        Stream &out = sender.getSerial();
        printYAMLHeader(out, "ok");
        printYAMLKeyValue(out, "command", "LNA_BUS", 2, true);
        printYAMLKeyValue(out, "channel", String(channel + 1), 2, false);
        printYAMLKeyValue(out, "target", "DRAIN", 2, true);
        printYAMLKeyValue(out, "bus_V", String(busVoltage), 2, false);
        printYAMLMessage(out, "Drain bus voltage (V)");
    } else if (strcmp(target, "GATE") == 0) {
        status = lnaDriver[channel]->getGateBusVoltage_V(busVoltage);
        if (reportIfError(sender, status, "LNA_BUS_READ_ERROR", "Failed to read Gate bus voltage.")) {
            return;
        }
        Stream &out = sender.getSerial();
        printYAMLHeader(out, "ok");
        printYAMLKeyValue(out, "command", "LNA_BUS", 2, true);
        printYAMLKeyValue(out, "channel", String(channel + 1), 2, false);
        printYAMLKeyValue(out, "target", "GATE", 2, true);
        printYAMLKeyValue(out, "bus_V", String(busVoltage), 2, false);
        printYAMLMessage(out, "Gate bus voltage (V)");
    } else {
        reportError(sender, "Invalid target. Use DRAIN or GATE.", "Invalid target");
    }
}

void cmdLNACurrent(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    const char* target = args[1].getString();
    float current;
    uint8_t status;
    if (strcmp(target, "DRAIN") == 0) {
        status = lnaDriver[channel]->getDrainCurrent_mA(current);
        if (reportIfError(sender, status, "LNA_CURRENT_READ_ERROR", "Failed to read Drain current.")) {
            return;
        }
        Stream &out = sender.getSerial();
        printYAMLHeader(out, "ok");
        printYAMLKeyValue(out, "command", "LNA_CURRENT", 2, true);
        printYAMLKeyValue(out, "channel", String(channel + 1), 2, false);
        printYAMLKeyValue(out, "target", "DRAIN", 2, true);
        printYAMLKeyValue(out, "current_mA", String(current), 2, false);
        printYAMLMessage(out, "Drain current (mA)");
    } else if (strcmp(target, "GATE") == 0) {
        status = lnaDriver[channel]->getGateCurrent_mA(current);
        if (reportIfError(sender, status, "LNA_CURRENT_READ_ERROR", "Failed to read Gate current.")) {
            return;
        }
        Stream &out = sender.getSerial();
        printYAMLHeader(out, "ok");
        printYAMLKeyValue(out, "command", "LNA_CURRENT", 2, true);
        printYAMLKeyValue(out, "channel", String(channel + 1), 2, false);
        printYAMLKeyValue(out, "target", "GATE", 2, true);
        printYAMLKeyValue(out, "current_mA", String(current), 2, false);
        printYAMLMessage(out, "Gate current (mA)");
    } else {
        reportError(sender, "Invalid target. Use DRAIN or GATE.", "Invalid target");

    }
}

void cmdLNAPower(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    const char* target = args[1].getString();
    float power;
    uint8_t status;
    if (strcmp(target, "DRAIN") == 0) {
        status = lnaDriver[channel]->getDrainPower_mW(power);
        if (reportIfError(sender, status, "LNA_POWER_READ_ERROR", "Failed to read Drain power.")) {
            return;
        }
        Stream &out = sender.getSerial();
        printYAMLHeader(out, "ok");
        printYAMLKeyValue(out, "command", "LNA_POWER", 2, true);
        printYAMLKeyValue(out, "channel", String(channel + 1), 2, false);
        printYAMLKeyValue(out, "target", "DRAIN", 2, true);
        printYAMLKeyValue(out, "power_mW", String(power), 2, false);
        printYAMLMessage(out, "Drain power (mW)");
    } else if (strcmp(target, "GATE") == 0) {
        status = lnaDriver[channel]->getGatePower_mW(power);
        if (reportIfError(sender, status, "LNA_POWER_READ_ERROR", "Failed to read Gate power.")) {
            return;
        }
        Stream &out = sender.getSerial();
        printYAMLHeader(out, "ok");
        printYAMLKeyValue(out, "command", "LNA_POWER", 2, true);
        printYAMLKeyValue(out, "channel", String(channel + 1), 2, false);
        printYAMLKeyValue(out, "target", "GATE", 2, true);
        printYAMLKeyValue(out, "power_mW", String(power), 2, false);
        printYAMLMessage(out, "Gate power (mW)");
    } else {
        reportError(sender, "Invalid target. Use DRAIN or GATE.", "Invalid target");
    }
}

void cmdLNAEnable(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    const char* target = args[1].getString();
    bool enable = true;
    uint8_t status;
    if (strcmp(target, "DRAIN") == 0) {
        status = lnaDriver[channel]->setDrainEnable(enable);
        if (reportIfError(sender, status, "LNA_DRAIN_ENABLE_ERROR", "Failed to enable Drain.")) {
            return;
        }
        Stream &out = sender.getSerial();
        printYAMLHeader(out, "ok");
        printYAMLKeyValue(out, "command", "LNA_ENABLE", 2, true);
        printYAMLKeyValue(out, "channel", String(channel + 1), 2, false);
        printYAMLKeyValue(out, "target", "DRAIN", 2, true);
        printYAMLKeyValue(out, "enabled", String("true"), 2, false);
        printYAMLMessage(out, "Drain enabled");
    } else if (strcmp(target, "GATE") == 0) {
        status = lnaDriver[channel]->setGateEnable(enable);
        if (reportIfError(sender, status, "LNA_GATE_ENABLE_ERROR", "Failed to enable Gate.")) {
            return;
        }
        Stream &out = sender.getSerial();
        printYAMLHeader(out, "ok");
        printYAMLKeyValue(out, "command", "LNA_ENABLE", 2, true);
        printYAMLKeyValue(out, "channel", String(channel + 1), 2, false);
        printYAMLKeyValue(out, "target", "GATE", 2, true);
        printYAMLKeyValue(out, "enabled", String("true"), 2, false);
        printYAMLMessage(out, "Gate enabled");
    } else {
        reportError(sender, "Invalid target. Use DRAIN or GATE.", "Invalid target");
    }
}

void cmdLNADisable(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    const char* target = args[1].getString();
    bool enable = false;
    uint8_t status;
    if (strcmp(target, "DRAIN") == 0) {
        status = lnaDriver[channel]->setDrainEnable(enable);
        if (reportIfError(sender, status, "LNA_DRAIN_ENABLE_ERROR", "Failed to enable Drain.")) {
            return;
        }
        Stream &out = sender.getSerial();
        printYAMLHeader(out, "ok");
        printYAMLKeyValue(out, "command", "LNA_ENABLE", 2, true);
        printYAMLKeyValue(out, "channel", String(channel + 1), 2, false);
        printYAMLKeyValue(out, "target", "DRAIN", 2, true);
        printYAMLKeyValue(out, "enabled", String("true"), 2, false);
        printYAMLMessage(out, "Drain enabled");
    } else if (strcmp(target, "GATE") == 0) {
        status = lnaDriver[channel]->setGateEnable(enable);
        if (reportIfError(sender, status, "LNA_GATE_ENABLE_ERROR", "Failed to enable Gate.")) {
            return;
        }
        Stream &out = sender.getSerial();
        printYAMLHeader(out, "ok");
        printYAMLKeyValue(out, "command", "LNA_ENABLE", 2, true);
        printYAMLKeyValue(out, "channel", String(channel + 1), 2, false);
        printYAMLKeyValue(out, "target", "GATE", 2, true);
        printYAMLKeyValue(out, "enabled", String("true"), 2, false);
        printYAMLMessage(out, "Gate enabled");
    } else {
        reportError(sender, "Invalid target. Use DRAIN or GATE.", "Invalid target");
    }
}

void cmdLNAGetAll(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    const char* target = args[1].getString();
    float shuntVoltage, busVoltage, current, power;
    uint16_t dacValue;
    bool enable;
    uint8_t status;

    if (strcasecmp(target, "DRAIN") == 0) {
        status = lnaDriver[channel]->readDrain(dacValue);
        if (reportIfError(sender, status, "LNA_DAC_READ_ERROR", "Failed to read Drain DAC value.")) {
            return;
        }
        status = lnaDriver[channel]->getDrainShuntVoltage_mV(shuntVoltage);
        if (reportIfError(sender, status, "LNA_SHUNT_READ_ERROR", "Failed to read Drain shunt voltage.")) {
            return;
        }
        status = lnaDriver[channel]->getDrainBusVoltage_V(busVoltage);
        if (reportIfError(sender, status, "LNA_BUS_READ_ERROR", "Failed to read Drain bus voltage.")) {
            return;
        }
        status = lnaDriver[channel]->getDrainCurrent_mA(current);
        if (reportIfError(sender, status, "LNA_CURRENT_READ_ERROR", "Failed to read Drain current.")) {
            return;
        }
        status = lnaDriver[channel]->getDrainPower_mW(power);
        if (reportIfError(sender, status, "LNA_POWER_READ_ERROR", "Failed to read Drain power.")) {
            return;
        }
        status = lnaDriver[channel]->getDrainEnable(enable);
        if (reportIfError(sender, status, "LNA_ENABLE_READ_ERROR", "Failed to read Drain enable state.")) {
            return;
        }
    } else if (strcasecmp(target, "GATE") == 0) {
        status = lnaDriver[channel]->readGate(dacValue);
        if (reportIfError(sender, status, "LNA_DAC_READ_ERROR", "Failed to read Gate DAC value.")) {
            return;
        }
        status = lnaDriver[channel]->getGateShuntVoltage_mV(shuntVoltage);
        if (reportIfError(sender, status, "LNA_SHUNT_READ_ERROR", "Failed to read Gate shunt voltage.")) {
            return;
        }
        status = lnaDriver[channel]->getGateBusVoltage_V(busVoltage);
        if (reportIfError(sender, status, "LNA_BUS_READ_ERROR", "Failed to read Gate bus voltage.")) {
            return;
        }
        status = lnaDriver[channel]->getGateCurrent_mA(current);
        if (reportIfError(sender, status, "LNA_CURRENT_READ_ERROR", "Failed to read Gate current.")) {
            return;
        }
        status = lnaDriver[channel]->getGatePower_mW(power);
        if (reportIfError(sender, status, "LNA_POWER_READ_ERROR", "Failed to read Gate power.")) {
            return;
        }
        status = lnaDriver[channel]->getGateEnable(enable);
        if (reportIfError(sender, status, "LNA_ENABLE_READ_ERROR", "Failed to read Gate enable state.")) {
            return;
        }
    } else {
        reportError(sender, "Invalid target. Use DRAIN or GATE.", "Invalid target");
        return;
    }
    Stream &out = sender.getSerial();
    printYAMLHeader(out, "ok");
    printYAMLKeyValue(out, "command", "LNA_GET", 2, true);
    printYAMLKeyValue(out, "channel", String(channel + 1), 2, false);
    printYAMLKeyValue(out, "target", String(target), 2, true);
    printYAMLKeyValue(out, "dac_value", String(dacValue), 2, false);
    printYAMLKeyValue(out, "enabled", String(enable ? "true" : "false"), 2, false);
    printYAMLKeyValue(out, "shunt_mV", String(shuntVoltage), 2, false);
    printYAMLKeyValue(out, "bus_V", String(busVoltage), 2, false);
    printYAMLKeyValue(out, "current_mA", String(current), 2, false);
    printYAMLKeyValue(out, "power_mW", String(power), 2, false);
    printYAMLMessage(out, "LNA parameters");
}

void cmdTESSetInt(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    uint32_t value = args[1].getInt();
    uint8_t status;
    status = tesDriver[channel]->setAllOutputPins(value);
    if (reportIfError(sender, status, "TES_SETINT_ERROR", "Failed to set TES TCA bits.")) {
        return;
    }
    Stream &out = sender.getSerial();
    printYAMLHeader(out, "ok");
    printYAMLKeyValue(out, "command", "TES_SETINT", 2, true);
    printYAMLKeyValue(out, "channel", String(channel + 1), 2, false);
    printYAMLKeyValue(out, "tca_bits", String("0x") + toPaddedHex(value, 5), 2, true);
    printYAMLMessage(out, "TES TCA bits set (int)");
}

void cmdTESSetHex(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    const char* hexString = args[1].getString();
    uint32_t value = strtoul(hexString, nullptr, 16);
    uint8_t status;
    if (value > 0xFFFFF) {
        reportError(sender, "TES_SETHEX_VALUE_ERROR", "Hex value exceeds 20 bits.");
        return;
    }
    status = tesDriver[channel]->setAllOutputPins(value);
    if (reportIfError(sender, status, "TES_SETHEX_ERROR", "Failed to set TES TCA bits.")) {
        return;
    }
    Stream &out = sender.getSerial();
    printYAMLHeader(out, "ok");
    printYAMLKeyValue(out, "command", "TES_SETHEX", 2, true);
    printYAMLKeyValue(out, "channel", String(channel + 1), 2, false);
    printYAMLKeyValue(out, "tca_bits", String("0x") + toPaddedHex(value, 5), 2, true);
    printYAMLMessage(out, "TES TCA bits set (hex)");
}

void cmdTESEnable(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    bool enable = true;
    uint8_t status;
    status = tesDriver[channel]->setOutEnable(enable);
    if (reportIfError(sender, status, "TES_ENABLE_ERROR", "Failed to enable TES outputs.")) {
        return;
    }
    Stream &out = sender.getSerial();
    printYAMLHeader(out, "ok");
    printYAMLKeyValue(out, "command", "TES_ENABLE", 2, true);
    printYAMLKeyValue(out, "channel", String(channel + 1), 2, false);
    printYAMLKeyValue(out, "enabled", String("true"), 2, false);
    printYAMLMessage(out, "TES outputs enabled");
}

void cmdTESDisable(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    bool enable = false;
    uint8_t status;
    status = tesDriver[channel]->setOutEnable(enable);
    if (reportIfError(sender, status, "TES_DISABLE_ERROR", "Failed to disable TES outputs.")) {
        return;
    }
    Stream &out = sender.getSerial();
    printYAMLHeader(out, "ok");
    printYAMLKeyValue(out, "command", "TES_DISABLE", 2, true);
    printYAMLKeyValue(out, "channel", String(channel + 1), 2, false);
    printYAMLKeyValue(out, "enabled", String("false"), 2, false);
    printYAMLMessage(out, "TES outputs disabled");
}

void cmdTESShunt(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    float shuntVoltage;
    uint8_t status = tesDriver[channel]->getShuntVoltage_mV(shuntVoltage);
    if (reportIfError(sender, status, "TES_SHUNT_READ_ERROR", "Failed to read TES shunt voltage.")) {
        return;
    }
    Stream &out = sender.getSerial();
    printYAMLHeader(out, "ok");
    printYAMLKeyValue(out, "command", "TES_SHUNT", 2, true);
    printYAMLKeyValue(out, "channel", String(channel + 1), 2, false);
    printYAMLKeyValue(out, "shunt_mV", String(shuntVoltage), 2, false);
    printYAMLMessage(out, "TES shunt voltage (mV)");
}

void cmdTESBus(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    float busVoltage;
    uint8_t status = tesDriver[channel]->getBusVoltage_V(busVoltage);
    if (reportIfError(sender, status, "TES_BUS_READ_ERROR", "Failed to read TES bus voltage.")) {
        return;
    }
    Stream &out = sender.getSerial();
    printYAMLHeader(out, "ok");
    printYAMLKeyValue(out, "command", "TES_BUS", 2, true);
    printYAMLKeyValue(out, "channel", String(channel + 1), 2, false);
    printYAMLKeyValue(out, "bus_V", String(busVoltage), 2, false);
    printYAMLMessage(out, "TES bus voltage (V)");
}

void cmdTESCurrent(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    float current;
    uint8_t status = tesDriver[channel]->getCurrent_mA(current);
    if (reportIfError(sender, status, "TES_CURRENT_READ_ERROR", "Failed to read TES current.")) {
        return;
    }
    Stream &out = sender.getSerial();
    printYAMLHeader(out, "ok");
    printYAMLKeyValue(out, "command", "TES_CURRENT", 2, true);
    printYAMLKeyValue(out, "channel", String(channel + 1), 2, false);
    printYAMLKeyValue(out, "current_mA", String(current), 2, false);
    printYAMLMessage(out, "TES current (mA)");
}

void cmdTESPower(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    float power;
    uint8_t status = tesDriver[channel]->getPower_mW(power);
    if (reportIfError(sender, status, "TES_POWER_READ_ERROR", "Failed to read TES power.")) {
        return;
    }
    Stream &out = sender.getSerial();
    printYAMLHeader(out, "ok");
    printYAMLKeyValue(out, "command", "TES_POWER", 2, true);
    printYAMLKeyValue(out, "channel", String(channel + 1), 2, false);
    printYAMLKeyValue(out, "power_mW", String(power), 2, false);
    printYAMLMessage(out, "TES power (mW)");
}

void cmdTESSet(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    float current_mA = args[1].getFloat();
    uint32_t finalState;
    uint8_t status;
    status = tesDriver[channel]->setCurrent_mA(current_mA);
    if (reportIfError(sender, status, "TES_SET_CURRENT_ERROR", "Failed to set TES output current.")) {
        return;
    }
    status = tesDriver[channel]->getCurrent_mA(current_mA);
    if (reportIfError(sender, status, "TES_SET_CURRENT_ERROR", "Failed to read set TES current.")) {
        return;
    }
    status = tesDriver[channel]->getAllOutputPins(finalState);
    if (reportIfError(sender, status, "TES_SET_CURRENT_ERROR", "Failed to read TES TCA bits.")) {
        return;
    }
    Stream &out = sender.getSerial();
    printYAMLHeader(out, "ok");
    printYAMLKeyValue(out, "command", "TES_SET", 2, true);
    printYAMLKeyValue(out, "channel", String(channel + 1), 2, false);
    printYAMLKeyValue(out, "current_mA", String(current_mA), 2, false);
    printYAMLKeyValue(out, "tca_bits", String("0x") + toPaddedHex(finalState, 5), 2, true);
    printYAMLMessage(out, "TES output current set");
}

void cmdTESInc(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    uint32_t delta = args[1].getInt();
    uint32_t finalState;
    uint8_t status;
    status = tesDriver[channel]->bumpOutputPins(static_cast<int32_t>(delta));
    if (reportIfError(sender, status, "TES_INC_ERROR", "Failed to increase TES TCA bits.")) {
        return;
    }
    status  = tesDriver[channel]->getAllOutputPins(finalState);
    if (reportIfError(sender, status, "TES_INC_ERROR", "Failed to read TES TCA bits.")) {
        return;
    }
    Stream &out = sender.getSerial();
    printYAMLHeader(out, "ok");
    printYAMLKeyValue(out, "command", "TES_INC", 2, true);
    printYAMLKeyValue(out, "channel", String(channel + 1), 2, false);
    printYAMLKeyValue(out, "delta", String(delta), 2, false);
    printYAMLKeyValue(out, "tca_bits", String("0x") + toPaddedHex(finalState, 5), 2, true);
    printYAMLMessage(out, "TES TCA bits increased");
}

void cmdTESDec(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    uint32_t delta = args[1].getInt();
    uint32_t finalState;
    uint8_t status;
    status = tesDriver[channel]->bumpOutputPins(-static_cast<int32_t>(delta));
    if (reportIfError(sender, status, "TES_DEC_ERROR", "Failed to decrease TES TCA bits.")) {
        return;
    }
    status  = tesDriver[channel]->getAllOutputPins(finalState);
    if (reportIfError(sender, status, "TES_DEC_ERROR", "Failed to read TES TCA bits.")) {
        return;
    }
    Stream &out = sender.getSerial();
    printYAMLHeader(out, "ok");
    printYAMLKeyValue(out, "command", "TES_DEC", 2, true);
    printYAMLKeyValue(out, "channel", String(channel + 1), 2, false);
    printYAMLKeyValue(out, "delta", String(delta), 2, false);
    printYAMLKeyValue(out, "tca_bits", String("0x") + toPaddedHex(finalState, 5), 2, true);
    printYAMLMessage(out, "TES TCA bits decreased");
}

void cmdTESBits(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    uint32_t currentState; 
    uint8_t status;
    status = tesDriver[channel]->getAllOutputPins(currentState);
    if (reportIfError(sender, status, "TES_TCA_READ_ERROR", "Failed to read TES TCA bits.")) {
        return;
    }
    Stream &out = sender.getSerial();
    printYAMLHeader(out, "ok");
    printYAMLKeyValue(out, "command", "TES_BITS", 2, true);
    printYAMLKeyValue(out, "channel", String(channel + 1), 2, false);
    printYAMLKeyValue(out, "tca_bits", String("0x") + toPaddedHex(currentState, 5), 2, true);
    printYAMLMessage(out, "TES TCA bits (hex)");
}

void cmdTESGetAll(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    float shuntVoltage, busVoltage, current, power;
    uint8_t status;
    bool enabled;
    uint32_t tcaBits;
    status = tesDriver[channel]->getShuntVoltage_mV(shuntVoltage);
    if (reportIfError(sender, status, "TES_SHUNT_READ_ERROR", "Failed to read TES shunt voltage.")) {
        return;
    }
    status = tesDriver[channel]->getBusVoltage_V(busVoltage);
    if (reportIfError(sender, status, "TES_BUS_READ_ERROR", "Failed to read TES bus voltage.")) {
        return;
    }
    status = tesDriver[channel]->getCurrent_mA(current);
    if (reportIfError(sender, status, "TES_CURRENT_READ_ERROR", "Failed to read TES current.")) {
        return;
    }
    status = tesDriver[channel]->getPower_mW(power);
    if (reportIfError(sender, status, "TES_POWER_READ_ERROR", "Failed to read TES power.")) {
        return;
    }
    status = tesDriver[channel]->getAllOutputPins(tcaBits);
    if (reportIfError(sender, status, "TES_TCA_READ_ERROR", "Failed to read TES TCA bits.")) {
        return;
    }
    status = tesDriver[channel]->getOutEnable(enabled);
    if (reportIfError(sender, status, "TES_ENABLE_READ_ERROR", "Failed to read TES enable state.")) {
        return;
    }
    Stream &out = sender.getSerial();
    printYAMLHeader(out, "ok");
    printYAMLKeyValue(out, "command", "TES_GET", 2, true);
    printYAMLKeyValue(out, "channel", String(channel + 1), 2, false);
    printYAMLKeyValue(out, "enabled", String(enabled ? "true" : "false"), 2, false);
    printYAMLKeyValue(out, "tca_bits", String("0x") + toPaddedHex(tcaBits, 5), 2, true);
    printYAMLKeyValue(out, "shunt_mV", String(shuntVoltage), 2, false);
    printYAMLKeyValue(out, "bus_V", String(busVoltage), 2, false);
    printYAMLKeyValue(out, "current_mA", String(current), 2, false);
    printYAMLKeyValue(out, "power_mW", String(power), 2, false);
    printYAMLMessage(out, "TES parameters");
}
