#include "MCP4728.h"

MCP4728::MCP4728(uint8_t i2cAddress) : _i2cAddress(i2cAddress) {}

uint8_t MCP4728::begin() {
    Wire.begin();
    mcp.begin(_i2cAddress);
    RETURN_IF_ERROR(writeDAC(MCP4728_CHANNEL_A, 0));
    RETURN_IF_ERROR(writeDAC(MCP4728_CHANNEL_B, 0));
    return 0;
}

uint8_t MCP4728::writeDAC(MCP4728_channel_t channel, uint16_t value) {
    value = value & 0x0FFF; // Ensure value is 12-bit
    return mcp.setChannelValue(channel, value) == true ? 0 : 1;
}

uint8_t MCP4728::readDAC(MCP4728_channel_t channel, uint16_t& value) {
    value = mcp.getChannelValue(channel);
    return 0;
}