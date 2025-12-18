#include <StaticSerialCommands.h>

#include "src/drivers/LTC4302.h"
#include "src/drivers/MCP4728.h"
#include "src/drivers/INA219.h" // Include the INA219 header
#include "src/routers/Router.h" // Include the Router header
#include "src/devices/LNADriver.h" // Include the LNADriver header


// Define I2C addresses for the devices
#define BASE_HUB_LTC4302_ADDR 0x7E // Address for the base hub LTC4302
#define BASE_HUB_MCP4728_ADDR 0x61 // Address for the main MCP4728

// Make TES/LNA counts configurable in one place
#define NUM_LNA 5

// Default address lists (easy to edit / override)
const uint8_t DEFAULT_LNA_ADDRESSES[NUM_LNA] = {
        0x72, 0x62, 0x63, 0x64, 0x65
};

// Base hub (unchanged)
LTC4302 baseHub(BASE_HUB_LTC4302_ADDR);

// Router uses baseHub
Router router(&baseHub);

// Route for main MCP4728 (Base Hub -> MCP4728)
I2CRoute routeToMainMCP4728 = { &baseHub, nullptr };
MCP4728 mainDac(BASE_HUB_MCP4728_ADDR);

LTC4302* lnaLTC[NUM_LNA];
LNADriver* lnaDriver[NUM_LNA];

// ----- Command definitions -----------------------------------------------------------
constexpr auto lnaChanArg =
    ARG(ArgType::Int, 1, NUM_LNA, "CHANNEL");

constexpr auto lnaDrainGate = 
    ARG(ArgType::String, "DRAIN|GATE");

constexpr auto lnaCurrentArg = 
    ARG(ArgType::Float, 0, 64, "CURRENT");

constexpr auto lnaVoltageArg = 
    ARG(ArgType::Float, 0, 5, "VOLTAGE");

constexpr auto dacValueArg =
    ARG(ArgType::Int, 0, 4095, "VALUE");

constexpr auto delayMsArg = 
    ARG(ArgType::Int, 0, 10000, "DELAY_MS");

void cmdLNA(SerialCommands& sender, Args& args);

void cmdDAC(SerialCommands& sender, Args& args);
void cmdDACSet(SerialCommands& sender, Args& args);
void cmdDACGet(SerialCommands& sender, Args& args);

void cmdLNAGetAll(SerialCommands& sender, Args& args);
void cmdLNASetCurrent(SerialCommands& sender, Args& args);
void cmdLNASetVoltage(SerialCommands& sender, Args& args);
void cmdLNASetDac(SerialCommands& sender, Args& args);
void cmdLNAShunt(SerialCommands& sender, Args& args);
void cmdLNABus(SerialCommands& sender, Args& args);
void cmdLNACurrent(SerialCommands& sender, Args& args);
void cmdLNAPower(SerialCommands& sender, Args& args);
void cmdLNAEnable(SerialCommands& sender, Args& args);
void cmdLNADisable(SerialCommands& sender, Args& args);

void cmdHelp(SerialCommands& sender, Args& args);

Command lnaCommands[] = {
    COMMAND(cmdLNAGetAll, "GET", nullptr, "Get All LNA Parameters for Gate/Drain"),
    COMMAND(cmdLNAEnable, "ENABLE", nullptr, "Enable Gate/Drain"),
    COMMAND(cmdLNADisable, "DISABLE", nullptr, "Disable Gate/Drain"),
    COMMAND(cmdLNASetCurrent, "SETMA", lnaCurrentArg, nullptr, "Search and set Gate/Drain DAC Value (mA)"),
    COMMAND(cmdLNASetVoltage, "SETV", lnaVoltageArg, nullptr, "Search and set Gate/Drain DAC Value (V)"),
    COMMAND(cmdLNASetDac, "SETDAC", dacValueArg, nullptr, "Set Gate/Drain DAC Value"),
    COMMAND(cmdLNAShunt, "SHUNT", nullptr, "Get Gate/Drain Shunt Voltage (mV)"),
    COMMAND(cmdLNABus, "BUS", nullptr, "Get Gate/Drain Bus Voltage (V)"),
    COMMAND(cmdLNACurrent, "CURRENT", nullptr, "Get Gate/Drain Current (mA)"),
    COMMAND(cmdLNAPower, "POWER", nullptr, "Get Gate/Drain Power (mW)"),
};

Command dacCommands[] = {
    COMMAND(cmdDACSet, "SET", dacValueArg, nullptr, "Set Main DAC Value"),
    COMMAND(cmdDACGet, "GET", nullptr, "Get Main DAC Value"),
};

Command commands[] = {
    COMMAND(cmdLNA, "LNA", lnaChanArg, lnaDrainGate, lnaCommands, "LNA Commands"),
    COMMAND(cmdDAC, "DAC", dacCommands, "DAC Commands"),
    COMMAND(cmdHelp, "HELP", nullptr, "List All Commands"),
};

SerialCommands serialCommands(Serial, commands, sizeof(commands) / sizeof(Command));


// Helper to initialize devices (call early in setup before begin() calls)
void initDeviceArrays() {
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
    Serial.println("LNA Controller Starting...");

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

    // Initialize LNAs
    for (int i = 0; i < NUM_LNA; ++i) {
        status = lnaDriver[i]->begin();
        if (status) {
            Serial.print("Error initializing LNA Driver for channel "); Serial.println(i + 1);
        }
    }

    mainDac.writeDAC(MCP4728_CHANNEL_A, 1024); // Set channel A to ~1/4-scale (4095 max)
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

void cmdDAC(SerialCommands& sender, Args& args) {
    sender.listAllCommands(dacCommands, sizeof(dacCommands) / sizeof(Command));
}

void cmdDACSet(SerialCommands& sender, Args& args) {
    uint16_t value = args[0].getInt();
    uint8_t status;
    // Implement setting main DAC value
    status = mainDac.writeDAC(MCP4728_CHANNEL_A, value, false);
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
void cmdLNASetCurrent(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    const char* target = args[1].getString();
    float target_mA = args[2].getFloat();
    uint16_t dacValue;
    uint8_t status;
    if (strcmp(target, "DRAIN") == 0) {
        status = lnaDriver[channel]->setDrainCurrent(target_mA, dacValue, 1);
        if (reportIfError(sender, status, "LNA_SET_ERROR", "Failed to set Drain current.")) {
            return;
        }
    } else if (strcmp(target, "GATE") == 0) {
        status = lnaDriver[channel]->setGateCurrent(target_mA, dacValue, 1);
        if (reportIfError(sender, status, "LNA_SET_ERROR", "Failed to set Gate current.")) {
            return;
        }    } else {
        reportError(sender, "Invalid target. Use DRAIN or GATE.", "Invalid target");
    }
    Stream &out = sender.getSerial();
    printYAMLHeader(out, "ok");
    printYAMLKeyValue(out, "command", "LNA_SET", 2, true);
    printYAMLKeyValue(out, "channel", String(channel + 1), 2, false);
    printYAMLKeyValue(out, "target", target, 2, true);
    printYAMLKeyValue(out, "current_mA", String(target_mA, 4), 2, false);
    printYAMLKeyValue(out, "dac_value", String(dacValue), 2, false);
    printYAMLMessage(out, "LNA current set");
}

void cmdLNASetVoltage(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    const char* target = args[1].getString();
    float target_V = args[2].getFloat();
    uint16_t dacValue;
    uint8_t status;
    if (strcmp(target, "DRAIN") == 0) {
        status = lnaDriver[channel]->setDrainVoltage(target_V, dacValue, 1);
        if (reportIfError(sender, status, "LNA_SET_ERROR", "Failed to set Drain voltage.")) {
            return;
        }
    } else if (strcmp(target, "GATE") == 0) {
        status = lnaDriver[channel]->setGateVoltage(target_V, dacValue, 1);
        if (reportIfError(sender, status, "LNA_SET_ERROR", "Failed to set Gate voltage.")) {
            return;
        }    } else {
        reportError(sender, "Invalid target. Use DRAIN or GATE.", "Invalid target");
    }
    Stream &out = sender.getSerial();
    printYAMLHeader(out, "ok");
    printYAMLKeyValue(out, "command", "LNA_SET", 2, true);
    printYAMLKeyValue(out, "channel", String(channel + 1), 2, false);
    printYAMLKeyValue(out, "target", target, 2, true);
    printYAMLKeyValue(out, "voltage_V", String(target_V, 4), 2, false);
    printYAMLKeyValue(out, "dac_value", String(dacValue), 2, false);
    printYAMLMessage(out, "LNA voltage set");
}

void cmdLNASetDac(SerialCommands& sender, Args& args) {
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
        printYAMLKeyValue(out, "shunt_mV", String(shuntVoltage, 4), 2, false);
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
        printYAMLKeyValue(out, "shunt_mV", String(shuntVoltage, 4), 2, false);
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
        printYAMLKeyValue(out, "bus_V", String(busVoltage, 4), 2, false);
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
        printYAMLKeyValue(out, "bus_V", String(busVoltage, 4), 2, false);
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
        printYAMLKeyValue(out, "current_mA", String(current, 4), 2, false);
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
        printYAMLKeyValue(out, "current_mA", String(current, 4), 2, false);
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
        printYAMLKeyValue(out, "power_mW", String(power, 4), 2, false);
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
        printYAMLKeyValue(out, "power_mW", String(power, 4), 2, false);
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
        if (reportIfError(sender, status, "LNA_DRAIN_DISABLE_ERROR", "Failed to enable Drain.")) {
            return;
        }
        Stream &out = sender.getSerial();
        printYAMLHeader(out, "ok");
        printYAMLKeyValue(out, "command", "LNA_DISABLE", 2, true);
        printYAMLKeyValue(out, "channel", String(channel + 1), 2, false);
        printYAMLKeyValue(out, "target", "DRAIN", 2, true);
        printYAMLKeyValue(out, "enabled", String("true"), 2, false);
        printYAMLMessage(out, "Drain disabled");
    } else if (strcmp(target, "GATE") == 0) {
        status = lnaDriver[channel]->setGateEnable(enable);
        if (reportIfError(sender, status, "LNA_GATE_DISABLE_ERROR", "Failed to enable Gate.")) {
            return;
        }
        Stream &out = sender.getSerial();
        printYAMLHeader(out, "ok");
        printYAMLKeyValue(out, "command", "LNA_DISABLE", 2, true);
        printYAMLKeyValue(out, "channel", String(channel + 1), 2, false);
        printYAMLKeyValue(out, "target", "GATE", 2, true);
        printYAMLKeyValue(out, "enabled", String("true"), 2, false);
        printYAMLMessage(out, "Gate disabled");
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
    printYAMLKeyValue(out, "shunt_mV", String(shuntVoltage, 4), 2, false);
    printYAMLKeyValue(out, "bus_V", String(busVoltage, 4), 2, false);
    printYAMLKeyValue(out, "current_mA", String(current, 4), 2, false);
    printYAMLKeyValue(out, "power_mW", String(power, 4), 2, false);
    printYAMLMessage(out, "LNA parameters");
}
