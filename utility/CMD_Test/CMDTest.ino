#include <StaticSerialCommands.h>
// ---------------------------------------------------------------------------
// Arguments
// ---------------------------------------------------------------------------

#define NUM_LNA 2
#define NUM_TES 12

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

// ---------------------------------------------------------------------------
// Handlers
// ---------------------------------------------------------------------------
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

// ---------------------------------------------------------------------------
// Command Table
// ---------------------------------------------------------------------------
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
};

// ---------------------------------------------------------------------------
// Handler Implementations
// ---------------------------------------------------------------------------
void cmdLNA(SerialCommands& sender, Args& args) {
    sender.listAllCommands(lnaCommands, sizeof(lnaCommands) / sizeof(Command));
}

void cmdTES(SerialCommands& sender, Args& args) {
    sender.listAllCommands(tesCommands, sizeof(tesCommands) / sizeof(Command));
}

void cmdDAC(SerialCommands& sender, Args& args) {
    args = args;
    uint16_t value = args[0].getInt();
    // Implement setting main DAC value
    sender.getSerial().print("Main DAC Set to ");
    sender.getSerial().println(value);  
}

void cmdLNASet(SerialCommands& sender, Args& args) {
    args = args;
    uint8_t channel = args[0].getInt() - 1;
    const char* target = args[1].getString();
    uint16_t value = args[2].getInt();
    // Implement setting DAC value for LNA channel
    sender.getSerial().print("LNA Channel ");
    sender.getSerial().print(channel + 1);
    sender.getSerial().print(" Set ");
    sender.getSerial().print(target);
    sender.getSerial().print(" DAC to ");
    sender.getSerial().println(value);  
}

void cmdLNAShunt(SerialCommands& sender, Args& args) {
    args = args;
    uint8_t channel = args[0].getInt() - 1;
    const char* target = args[1].getString();
    // Implement getting shunt voltage for LNA channel
    sender.getSerial().print("LNA Channel ");
    sender.getSerial().print(channel + 1);
    sender.getSerial().print(" ");
    sender.getSerial().print(target);
    sender.getSerial().println(" Shunt Voltage: [value] mV");  
}

void cmdLNABus(SerialCommands& sender, Args& args) {
    args = args;
    uint8_t channel = args[0].getInt() - 1;
    const char* target = args[1].getString();
    // Implement getting bus voltage for LNA channel
    sender.getSerial().print("LNA Channel ");
    sender.getSerial().print(channel + 1);
    sender.getSerial().print(" ");
    sender.getSerial().print(target);
    sender.getSerial().println(" Bus Voltage: [value] V");  
}

void cmdLNACurrent(SerialCommands& sender, Args& args) {
    args = args;
    uint8_t channel = args[0].getInt() - 1;
    const char* target = args[1].getString();
    // Implement getting current for LNA channel
    sender.getSerial().print("LNA Channel ");
    sender.getSerial().print(channel + 1);
    sender.getSerial().print(" ");
    sender.getSerial().print(target);
    sender.getSerial().println(" Current: [value] mA");  
}

void cmdLNAPower(SerialCommands& sender, Args& args) {
    args = args;
    uint8_t channel = args[0].getInt() - 1;
    const char* target = args[1].getString();
    // Implement getting power for LNA channel
    sender.getSerial().print("LNA Channel ");
    sender.getSerial().print(channel + 1);
    sender.getSerial().print(" ");
    sender.getSerial().print(target);
    sender.getSerial().println(" Power: [value] mW");  
}

void cmdLNAEnable(SerialCommands& sender, Args& args) {
    args = args;
    uint8_t channel = args[0].getInt() - 1;
    const char* target = args[1].getString();
    bool enable = args[2].getInt() != 0;
    // Implement enabling/disabling for LNA channel
    sender.getSerial().print("LNA Channel ");
    sender.getSerial().print(channel + 1);
    sender.getSerial().print(" ");
    sender.getSerial().print(target);
    sender.getSerial().print(enable ? " Enabled" : " Disabled");
    sender.getSerial().println();  
}

void cmdTESSet(SerialCommands& sender, Args& args) {
    args = args;
    uint8_t channel = args[0].getInt() - 1;
    uint32_t value = args[1].getInt();
    if (value == 0) {
        sender.getSerial().println("Error: TES TCA Bits value cannot be zero.");
    }
    // Implement setting TCA output bits for TES channel
    sender.getSerial().print("TES Channel ");
    sender.getSerial().print(channel + 1);
    sender.getSerial().print( " set TCA Bits to 0x");
    sender.getSerial().println(value, HEX);
}

void cmdTESEnable(SerialCommands& sender, Args& args) {
    args = args;
    uint8_t channel = args[0].getInt() - 1;
    bool enable = args[1].getInt() != 0;
    // Implement enabling/disabling for TES channel
    sender.getSerial().print("TES Channel ");
    sender.getSerial().print(channel + 1);
    sender.getSerial().print(enable ? " Enabled" : " Disabled");
    sender.getSerial().println();  
}

void cmdTESShunt(SerialCommands& sender, Args& args) {
    args = args;
    uint8_t channel = args[0].getInt() - 1;
    // Implement getting shunt voltage for TES channel
    sender.getSerial().print("TES Channel ");
    sender.getSerial().print(channel + 1);
    sender.getSerial().println(" Shunt Voltage: [value] mV");  
}

void cmdTESBus(SerialCommands& sender, Args& args) {
    args = args;
    uint8_t channel = args[0].getInt() - 1;
    // Implement getting bus voltage for TES channel
    sender.getSerial().print("TES Channel ");
    sender.getSerial().print(channel + 1);
    sender.getSerial().println(" Bus Voltage: [value] V");  
}

void cmdTESCurrent(SerialCommands& sender, Args& args) {
    args = args;
    uint8_t channel = args[0].getInt() - 1;
    // Implement getting current for TES channel
    sender.getSerial().print("TES Channel ");
    sender.getSerial().print(channel + 1);
    sender.getSerial().println(" Current: [value] mA");  
}

void cmdTESPower(SerialCommands& sender, Args& args) {
    args = args;
    uint8_t channel = args[0].getInt() - 1;
    // Implement getting power for TES channel
    sender.getSerial().print("TES Channel ");
    sender.getSerial().print(channel + 1);
    sender.getSerial().println(" Power: [value] mW");  
}
// ---------------------------------------------------------------------------
// SerialCommands Instance
// ---------------------------------------------------------------------------

SerialCommands serialCommands(Serial, commands, sizeof(commands) / sizeof(Command));

// ---------------------------------------------------------------------------
// Setup / Loop
// ---------------------------------------------------------------------------

void setup() {
    Serial.begin(115200);
}

void loop() {
    serialCommands.readSerial();
}