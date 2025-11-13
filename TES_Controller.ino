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

constexpr auto enableArg =
    ARG(ArgType::Int, 0, 1, "ENABLE"); 

constexpr auto tesTCAArg = 
    ARG(ArgType::Int, 0, 0xFFFFFF, "BITS");

void cmdLNA(SerialCommands& sender, Args& args);
void cmdTES(SerialCommands& sender, Args& args);

void cmdDAC(SerialCommands& sender, Args& args);

void cmdLNASet(SerialCommands& sender, Args& args);
void cmdLNAShunt(SerialCommands& sender, Args& args);
void cmdLNABus(SerialCommands& sender, Args& args);
void cmdLNACurrent(SerialCommands& sender, Args& args);
void cmdLNAPower(SerialCommands& sender, Args& args);
void cmdLNAEnable(SerialCommands& sender, Args& args);

void cmdTESSet(SerialCommands& sender, Args& args);
void cmdTESEnable(SerialCommands& sender, Args& args);
void cmdTESShunt(SerialCommands& sender, Args& args);
void cmdTESBus(SerialCommands& sender, Args& args);
void cmdTESCurrent(SerialCommands& sender, Args& args);
void cmdTESPower(SerialCommands& sender, Args& args);

void cmdHelp(SerialCommands& sender, Args& args);

Command lnaCommands[] = {
    COMMAND(cmdLNASet, "SET", dacValueArg, nullptr, "Set Gate/Drain DAC Value"),
    COMMAND(cmdLNAEnable, "ENABLE", enableArg, nullptr, "Enable/Disable Gate/Drain"),
    COMMAND(cmdLNAShunt, "SHUNT", nullptr, "Get Gate/Drain Shunt Voltage (mV)"),
    COMMAND(cmdLNABus, "BUS", nullptr, "Get Gate/Drain Bus Voltage (V)"),
    COMMAND(cmdLNACurrent, "CURRENT", nullptr, "Get Gate/Drain Current (mA)"),
    COMMAND(cmdLNAPower, "POWER", nullptr, "Get Gate/Drain Power (mW)"),
};

Command tesCommands[] = {
    COMMAND(cmdTESSet, "SET", tesTCAArg, nullptr, "Set TES TCA Output Bits"),
    COMMAND(cmdTESEnable, "ENABLE", enableArg, nullptr, "Enable/Disable TES Outputs"),
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

void setup() {
    Serial.begin(115200);
    Serial.println("TES Controller Starting...");

    initDeviceArrays();
    // Check if the main MCP4728 is at the default address 0x60
    // This check needs to be routed through the baseHub if the MCP4728 is behind it.
    // For now, assuming direct access for the check, or that routing is handled internally by Wire.
    // For simplicity, we'll assume the router is already set up and can handle the check.
    baseHub.begin();
    router.begin(); // Initialize the router (which initializes the base hub)
    // Enable the I2C bus through the base hub LTC4302
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
    bool enable = args[2].getInt() != 0;
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

void cmdTESSet(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    uint32_t value = args[1].getInt();
    tesDriver[channel]->setAllOutputPins(value);
    sender.getSerial().print("TES Channel ");
    sender.getSerial().print(channel + 1);
    sender.getSerial().print( " set TCA Bits to 0x");
    sender.getSerial().println(value, HEX);
}

void cmdTESEnable(SerialCommands& sender, Args& args) {
    uint8_t channel = args[0].getInt() - 1;
    bool enable = args[1].getInt() != 0;
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
