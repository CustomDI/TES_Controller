#include "MCP4728.h"

MCP4728::MCP4728(uint8_t i2cAddress) : _i2cAddress(i2cAddress), _router(nullptr), _route(nullptr) {}

MCP4728::MCP4728(uint8_t i2cAddress, Router* router, I2CRoute* route)
    : _i2cAddress(i2cAddress), _router(router), _route(route) {}

void MCP4728::begin() {
    Wire.begin();
    mcp.begin(_i2cAddress);
    mcp.setChannelValue(MCP4728_CHANNEL_A, 0);
    mcp.setChannelValue(MCP4728_CHANNEL_B, 0);
}

bool MCP4728::writeDAC(MCP4728_channel_t channel, uint16_t value) {
    bool status;
    value = value & 0x0FFF; // Ensure value is 12-bit
    if (_router != nullptr && _route != nullptr) {
        _router->routeTo(_route);
    }
    status = mcp.setChannelValue(channel, value);
    if (_router != nullptr && _route != nullptr) {
        _router->endRoute(_route); // Deselect channels after communication
    }
    return status;
}

uint16_t MCP4728::readDAC(MCP4728_channel_t channel) {
    uint16_t value;
    if (_router != nullptr && _route != nullptr) {
        _router->routeTo(_route);
    }
    value = mcp.getChannelValue(channel);
    if (_router != nullptr && _route != nullptr) {
        _router->endRoute(_route); // Deselect channels after communication
    }
    return value;
}