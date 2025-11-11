#include "MCP4728.h"

// Commands for MCP4728
#define MCP4728_CMD_WRITE_DAC_FAST     0b00000000 // C2 C1 C0 = 000 (Fast Mode)
#define MCP4728_CMD_WRITE_DAC_INDIV    0b01000000 // C2 C1 C0 = 010 (Individual DAC Write)
#define MCP4728_CMD_WRITE_ALL_DAC      0b01010000 // C2 C1 C0 = 011 (Write All DACs)
#define MCP4728_CMD_WRITE_VREF_PD_GAIN 0b01100000 // C2 C1 C0 = 100 (Write VREF, Power-Down, Gain)
#define MCP4728_CMD_PROGRAM_I2C_ADDR   0b01110000 // C2 C1 C0 = 111 (Write I2C Address Bits) - as per user feedback

// Channel selection for individual DAC write
#define MCP4728_CHANNEL_A 0b00000000
#define MCP4728_CHANNEL_B 0b00000010
#define MCP4728_CHANNEL_C 0b00000100
#define MCP4728_CHANNEL_D 0b00000110

MCP4728::MCP4728(uint8_t i2cAddress) : _i2cAddress(i2cAddress), _router(nullptr), _route(nullptr) {}

MCP4728::MCP4728(uint8_t i2cAddress, Router* router, I2CRoute* route)
    : _i2cAddress(i2cAddress), _router(router), _route(route) {}

void MCP4728::begin() {
    Wire.begin();
}

void MCP4728::sendCommand(uint8_t cmd, uint16_t data) {
    if (_router != nullptr && _route != nullptr) {
        _router->routeTo(_route);
    }

    Wire.beginTransmission(_i2cAddress);
    Wire.write(cmd);
    Wire.write((data >> 8) & 0xFF); // High byte
    Wire.write(data & 0xFF);       // Low byte
    Wire.endTransmission();

    if (_router != nullptr && _route != nullptr) {
        _router->endRoute(_route); // Deselect channels after communication
    }
}

void MCP4728::writeDAC(uint8_t channel, uint16_t value, bool fastMode) {
    if (fastMode) {
        // Fast mode command: C2 C1 C0 = 000, then 12-bit DAC data
        // The channel is implicitly handled by the order of writes if using sequential fast mode,
        // but for individual fast mode, it's usually a direct write.
        // The MCP4728 datasheet implies fast mode is for single DAC writes or sequential.
        // For simplicity, we'll use the individual write command for specific channels.
        // If true fast mode (single byte command) is needed, this function would need to be more specific.
        // For now, we'll use the individual write command with the appropriate channel bits.
        uint8_t cmd = MCP4728_CMD_WRITE_DAC_INDIV | (channel & 0x06); // C1 C0 bits for channel
        sendCommand(cmd, value);
    } else {
        // Individual DAC write command: C2 C1 C0 = 010, then channel bits, then 12-bit DAC data
        uint8_t cmd = MCP4728_CMD_WRITE_DAC_INDIV | (channel & 0x06); // C1 C0 bits for channel
        sendCommand(cmd, value);
    }
}

void MCP4728::writeAllDACs(uint16_t valueA, uint16_t valueB, uint16_t valueC, uint16_t valueD, bool fastMode) {
    // For writing to all DACs, the fastMode parameter is less relevant as the command structure is fixed.
    // The command is 0b01010000 (Write All DACs), followed by 4x 16-bit data (12-bit DAC value + 4 control bits).
    // The control bits (VREF, PD1, PD0, Gain) are usually part of the 16-bit data for each DAC.
    // For simplicity, we'll assume default VREF, PD, Gain settings and only write the 12-bit DAC value.
    // The MCP4728 expects 16 bits per DAC, where the upper 4 bits are control bits.
    // For now, we'll just send the 12-bit value, assuming control bits are 0.
    // This needs to be refined based on actual control bit requirements.

    Wire.beginTransmission(_i2cAddress);
    Wire.write(MCP4728_CMD_WRITE_ALL_DAC);

    // DAC A
    Wire.write((valueA >> 8) & 0xFF);
    Wire.write(valueA & 0xFF);
    // DAC B
    Wire.write((valueB >> 8) & 0xFF);
    Wire.write(valueB & 0xFF);
    // DAC C
    Wire.write((valueC >> 8) & 0xFF);
    Wire.write(valueC & 0xFF);
    // DAC D
    Wire.write((valueD >> 8) & 0xFF);
    Wire.write(valueD & 0xFF);

    Wire.endTransmission();
}

void MCP4728::writeDACWithPowerDown(uint8_t channel, uint16_t value, uint8_t powerDownMode) {
    // The powerDownMode bits (PD1, PD0) are part of the control byte.
    // This command is typically combined with VREF and Gain settings.
    // For now, we'll use the individual DAC write command and embed power-down bits.
    // The MCP4728 datasheet shows PD1 and PD0 as bits 5 and 4 of the control byte.
    // The control byte is the first byte of the 16-bit data for each DAC.

    uint8_t controlByte = (powerDownMode & 0x03) << 4; // PD1, PD0 in bits 5,4
    // Assuming VREF and Gain are 0 for now.

    Wire.beginTransmission(_i2cAddress);
    Wire.write(MCP4728_CMD_WRITE_DAC_INDIV | (channel & 0x06)); // Command + Channel
    Wire.write(controlByte | ((value >> 8) & 0x0F)); // Control byte (PD bits) + upper 4 bits of DAC value
    Wire.write(value & 0xFF);                       // Lower 8 bits of DAC value
    Wire.endTransmission();
}