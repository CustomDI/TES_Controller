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
MCP4728 mainDac(BASE_HUB_MCP4728_ADDR, &router, &routeToMainMCP4728);

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

Command commands[] = {
    COMMAND(cmdLNA, "LNA", lnaChanArg, lnaDrainGate, lnaCommands, "LNA Commands"),
    COMMAND(cmdTES, "TES", tesChanArg, tesCommands, "TES Commands"),
    COMMAND(cmdDAC, "DAC", dacValueArg, nullptr, "Set Main DAC Value"),
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

void setup() {
    Serial.begin(115200);
    Serial.println("TES Controller Starting...");

    initDeviceArrays();
    baseHub.begin();
    router.begin();

    baseHub.enableBus();
    mainDac.begin();

    // Initialize TESs
    for (int i = 0; i < NUM_TES; ++i) {
        tesDriver[i]->begin();
    }
    // Initialize LNAs
    for (int i = 0; i < NUM_LNA; ++i) {
        lnaDriver[i]->begin();
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
    uint16_t value = args[0].getInt();
    // Implement setting main DAC value
    mainDac.writeDAC(MCP4728_CHANNEL_A, value);
    sender.getSerial().print("Main DAC Set to ");
    sender.getSerial().println(value);
}

void cmdLNASet(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    const char* target = args[1].getString();
    uint16_t value = args[2].getInt();
    if (strcmp(target, "DRAIN") == 0) {
        lnaDriver[channel]->writeDrain(value);
        sender.getSerial().print("LNA Channel ");
        sender.getSerial().print(channel + 1);
        sender.getSerial().print(" Set DRAIN DAC to ");
        sender.getSerial().println(value);
    } else if (strcmp(target, "GATE") == 0) {
        lnaDriver[channel]->writeGate(value);
        sender.getSerial().print("LNA Channel ");
        sender.getSerial().print(channel + 1);
        sender.getSerial().print(" Set GATE DAC to ");
        sender.getSerial().println(value);
    } else {
        sender.getSerial().println("Error: Invalid target. Use DRAIN or GATE.");
    }
}

void cmdLNAShunt(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    const char* target = args[1].getString();
    if (strcmp(target, "DRAIN") == 0) {
        float shuntVoltage = lnaDriver[channel]->getDrainShuntVoltage_mV();
        sender.getSerial().print("LNA Channel ");
        sender.getSerial().print(channel + 1);
        sender.getSerial().print(" DRAIN Shunt Voltage: ");
        sender.getSerial().print(shuntVoltage);
        sender.getSerial().println(" mV");
    } else if (strcmp(target, "GATE") == 0) {
        float shuntVoltage = lnaDriver[channel]->getGateShuntVoltage_mV();
        sender.getSerial().print("LNA Channel ");
        sender.getSerial().print(channel + 1);
        sender.getSerial().print(" GATE Shunt Voltage: ");
        sender.getSerial().print(shuntVoltage);
        sender.getSerial().println(" mV");
    } else {
        sender.getSerial().println("Error: Invalid target. Use DRAIN or GATE.");
    }
}

void cmdLNABus(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    const char* target = args[1].getString();
    if (strcmp(target, "DRAIN") == 0) {
        float busVoltage = lnaDriver[channel]->getDrainBusVoltage_V();
        sender.getSerial().print("LNA Channel ");
        sender.getSerial().print(channel + 1);
        sender.getSerial().print(" DRAIN Bus Voltage: ");
        sender.getSerial().print(busVoltage);
        sender.getSerial().println(" V");
    } else if (strcmp(target, "GATE") == 0) {
        float busVoltage = lnaDriver[channel]->getDrainBusVoltage_V();
        sender.getSerial().print("LNA Channel ");
        sender.getSerial().print(channel + 1);
        sender.getSerial().print(" GATE Bus Voltage: ");
        sender.getSerial().print(busVoltage);
        sender.getSerial().println(" V");
    } else {
        sender.getSerial().println("Error: Invalid target. Use DRAIN or GATE.");
    }
}

void cmdLNACurrent(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    const char* target = args[1].getString();
    if (strcmp(target, "DRAIN") == 0) {
        float current = lnaDriver[channel]->getDrainCurrent_mA();
        sender.getSerial().print("LNA Channel ");
        sender.getSerial().print(channel + 1);
        sender.getSerial().print(" DRAIN Current: ");
        sender.getSerial().print(current);
        sender.getSerial().println(" mA");
    } else if (strcmp(target, "GATE") == 0) {
        float current = lnaDriver[channel]->getDrainCurrent_mA();
        sender.getSerial().print("LNA Channel ");
        sender.getSerial().print(channel + 1);
        sender.getSerial().print(" GATE Current: ");
        sender.getSerial().print(current);
        sender.getSerial().println(" mA");
    } else {
        sender.getSerial().println("Error: Invalid target. Use DRAIN or GATE.");
    }
}

void cmdLNAPower(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    const char* target = args[1].getString();
    if (strcmp(target, "DRAIN") == 0) {
        float power = lnaDriver[channel]->getDrainPower_mW();
        sender.getSerial().print("LNA Channel ");
        sender.getSerial().print(channel + 1);
        sender.getSerial().print(" DRAIN Power: ");
        sender.getSerial().print(power);
        sender.getSerial().println(" mW");
    } else if (strcmp(target, "GATE") == 0) {
        float power = lnaDriver[channel]->getDrainPower_mW();
        sender.getSerial().print("LNA Channel ");
        sender.getSerial().print(channel + 1);
        sender.getSerial().print(" GATE Power: ");
        sender.getSerial().print(power);
        sender.getSerial().println(" mW");
    } else {
        sender.getSerial().println("Error: Invalid target. Use DRAIN or GATE.");
    }
}

void cmdLNAEnable(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    const char* target = args[1].getString();
    bool enable = true;
    if (strcmp(target, "DRAIN") == 0) {
        lnaDriver[channel]->setDrainEnable(enable);
        sender.getSerial().print("LNA Channel ");
        sender.getSerial().print(channel + 1);
        sender.getSerial().print(" DRAIN ");
        sender.getSerial().print(enable ? "Enabled" : "Disabled");
        sender.getSerial().println();
    } else if (strcmp(target, "GATE") == 0) {
        lnaDriver[channel]->setGateEnable(enable);
        sender.getSerial().print("LNA Channel ");
        sender.getSerial().print(channel + 1);
        sender.getSerial().print(" GATE ");
        sender.getSerial().print(enable ? "Enabled" : "Disabled");
        sender.getSerial().println();
    } else {
        sender.getSerial().println("Error: Invalid target. Use DRAIN or GATE.");
    }
}

void cmdLNADisable(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    const char* target = args[1].getString();
    bool enable = false;
    if (strcmp(target, "DRAIN") == 0) {
        lnaDriver[channel]->setDrainEnable(enable);
        sender.getSerial().print("LNA Channel ");
        sender.getSerial().print(channel + 1);
        sender.getSerial().print(" DRAIN ");
        sender.getSerial().print(enable ? "Enabled" : "Disabled");
        sender.getSerial().println();
    } else if (strcmp(target, "GATE") == 0) {
        lnaDriver[channel]->setGateEnable(enable);
        sender.getSerial().print("LNA Channel ");
        sender.getSerial().print(channel + 1);
        sender.getSerial().print(" GATE ");
        sender.getSerial().print(enable ? "Enabled" : "Disabled");
        sender.getSerial().println();
    } else {
        sender.getSerial().println("Error: Invalid target. Use DRAIN or GATE.");
    }
}

void cmdTESGetAll(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    float shuntVoltage = tesDriver[channel]->getShuntVoltage_mV();
    float busVoltage = tesDriver[channel]->getBusVoltage_V();
    float current = tesDriver[channel]->getCurrent_mA();
    float power = tesDriver[channel]->getPower_mW();
    uint32_t tcaBits = tesDriver[channel]->getAllOutputPins();
    bool enabled = tesDriver[channel]->getOutEnable();

    sender.getSerial().print("TES Channel ");
    sender.getSerial().print(channel + 1);
    sender.getSerial().println(" Parameters:");
    sender.getSerial().print("  Enabled: ");
    sender.getSerial().println(enabled ? "Yes" : "No");
    sender.getSerial().print("  TCA Bits: 0x");
    sender.getSerial().println(toPaddedHex(tcaBits, 6));
    sender.getSerial().print("  Shunt Voltage: ");
    sender.getSerial().print(shuntVoltage);
    sender.getSerial().println(" mV");
    sender.getSerial().print("  Bus Voltage: ");
    sender.getSerial().print(busVoltage);
    sender.getSerial().println(" V");
    sender.getSerial().print("  Current: ");
    sender.getSerial().print(current);
    sender.getSerial().println(" mA");
    sender.getSerial().print("  Power: ");
    sender.getSerial().print(power);
    sender.getSerial().println(" mW");
}

void cmdTESSetInt(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    uint32_t value = args[1].getInt();
    tesDriver[channel]->setAllOutputPins(value);
    sender.getSerial().print("TES Channel ");
    sender.getSerial().print(channel + 1);
    sender.getSerial().print( " set TCA Bits to 0x");
    sender.getSerial().println(toPaddedHex(value, 6));
}

void cmdTESSetHex(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    const char* hexString = args[1].getString();
    uint32_t value = strtoul(hexString, nullptr, 16);
    tesDriver[channel]->setAllOutputPins(value);
    sender.getSerial().print("TES Channel ");
    sender.getSerial().print(channel + 1);
    sender.getSerial().print( " set TCA Bits to 0x");
    sender.getSerial().println(toPaddedHex(value, 6));
}

void cmdTESEnable(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    bool enable = true;
    tesDriver[channel]->setOutEnable(enable);
    sender.getSerial().print("TES Channel ");
    sender.getSerial().print(channel + 1);
    sender.getSerial().print(enable ? " Enabled" : " Disabled");
    sender.getSerial().println();
}

void cmdTESDisable(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    bool enable = false;
    tesDriver[channel]->setOutEnable(enable);
    sender.getSerial().print("TES Channel ");
    sender.getSerial().print(channel + 1);
    sender.getSerial().print(enable ? " Enabled" : " Disabled");
    sender.getSerial().println();
}

void cmdTESShunt(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    float shuntVoltage = tesDriver[channel]->getShuntVoltage_mV();
    sender.getSerial().print("TES Channel ");
    sender.getSerial().print(channel + 1);
    sender.getSerial().print(" Shunt Voltage: ");
    sender.getSerial().print(shuntVoltage);
    sender.getSerial().println(" mV");
}

void cmdTESBus(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    float busVoltage = tesDriver[channel]->getBusVoltage_V();
    sender.getSerial().print("TES Channel ");
    sender.getSerial().print(channel + 1);
    sender.getSerial().print(" Bus Voltage: ");
    sender.getSerial().print(busVoltage);
    sender.getSerial().println(" V");
}

void cmdTESCurrent(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    float current = tesDriver[channel]->getCurrent_mA();
    sender.getSerial().print("TES Channel ");
    sender.getSerial().print(channel + 1);
    sender.getSerial().print(" Current: ");
    sender.getSerial().print(current);
    sender.getSerial().println(" mA");
}

void cmdTESPower(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    float power = tesDriver[channel]->getPower_mW();
    sender.getSerial().print("TES Channel ");
    sender.getSerial().print(channel + 1);
    sender.getSerial().print(" Power: ");
    sender.getSerial().print(power);
    sender.getSerial().println(" mW");
}

void cmdTESSet(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    float current_mA = args[1].getFloat();
    uint32_t finalState = 0;
    tesDriver[channel]->setCurrent_mA(current_mA);
    current_mA = tesDriver[channel]->getCurrent_mA();
    finalState = tesDriver[channel]->getAllOutputPins();
    sender.getSerial().print("TES Channel ");
    sender.getSerial().print(channel + 1);
    sender.getSerial().print(" Set Output Current to ");
    sender.getSerial().print(current_mA);
    sender.getSerial().print(" mA");
    sender.getSerial().print(" (TCA Bits: 0x");
    sender.getSerial().print(toPaddedHex(finalState, 6));
    sender.getSerial().println(")");
}

void cmdTESInc(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    uint32_t delta = args[1].getInt();
    tesDriver[channel]->bumpOutputPins(delta);
    uint32_t finalState = tesDriver[channel]->getAllOutputPins();
    sender.getSerial().print("TES Channel ");
    sender.getSerial().print(channel + 1);
    sender.getSerial().print(" Increased TCA Bits by ");
    sender.getSerial().print(delta);
    sender.getSerial().print(" to 0x");
    sender.getSerial().println(toPaddedHex(finalState, 6));
}

void cmdTESDec(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    uint32_t delta = args[1].getInt();
    tesDriver[channel]->bumpOutputPins(-static_cast<int32_t>(delta));
    uint32_t finalState = tesDriver[channel]->getAllOutputPins();
    sender.getSerial().print("TES Channel ");
    sender.getSerial().print(channel + 1);
    sender.getSerial().print(" Decreased TCA Bits by ");
    sender.getSerial().print(delta);
    sender.getSerial().print(" to 0x");
    sender.getSerial().println(toPaddedHex(finalState, 6));
}

void cmdTESBits(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    uint32_t currentState = tesDriver[channel]->getAllOutputPins();
    sender.getSerial().print("TES Channel ");
    sender.getSerial().print(channel + 1);
    sender.getSerial().print(" Current TCA Bits: 0x");
    sender.getSerial().println(toPaddedHex(currentState, 6));
}

void cmdLNAGetAll(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    const char* target = args[1].getString();
    float shuntVoltage, busVoltage, current, power;
    uint16_t dacValue;
    bool enable;

    if (strcasecmp(target, "DRAIN") == 0) {
        dacValue = lnaDriver[channel]->readDrain();
        shuntVoltage = lnaDriver[channel]->getDrainShuntVoltage_mV();
        busVoltage = lnaDriver[channel]->getDrainBusVoltage_V();
        current = lnaDriver[channel]->getDrainCurrent_mA();
        power = lnaDriver[channel]->getDrainPower_mW();
        enable = lnaDriver[channel]->getDrainEnable();
    } else if (strcasecmp(target, "GATE") == 0) {
        dacValue = lnaDriver[channel]->readGate();
        shuntVoltage = lnaDriver[channel]->getGateShuntVoltage_mV();
        busVoltage = lnaDriver[channel]->getGateBusVoltage_V();
        current = lnaDriver[channel]->getGateCurrent_mA();
        power = lnaDriver[channel]->getGatePower_mW();
        enable = lnaDriver[channel]->getGateEnable();
    } else {
        sender.getSerial().println("Error: Invalid target. Use DRAIN or GATE.");
        return;
    }
    sender.getSerial().print("LNA Channel ");
    sender.getSerial().print(channel + 1);
    sender.getSerial().print(" ");
    sender.getSerial().print(target);
    sender.getSerial().println(" Parameters:");
    sender.getSerial().print("  DAC Value: ");
    sender.getSerial().println(dacValue);
    sender.getSerial().print("  Enabled: ");
    sender.getSerial().println(enable ? "Yes" : "No");
    sender.getSerial().print("  Shunt Voltage: ");
    sender.getSerial().print(shuntVoltage);
    sender.getSerial().println(" mV");
    sender.getSerial().print("  Bus Voltage: ");
    sender.getSerial().print(busVoltage);
    sender.getSerial().println(" V");
    sender.getSerial().print("  Current: ");
    sender.getSerial().print(current);
    sender.getSerial().println(" mA");
    sender.getSerial().print("  Power: ");
    sender.getSerial().print(power);
    sender.getSerial().println(" mW");
}