#ifndef MCP4728_H
#define MCP4728_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MCP4728.h>
#include "../routers/Router.h"

class MCP4728 {
public:
    // Constructor for a single MCP4728 device (no routing)
    MCP4728(uint8_t i2cAddress);

    // Constructor for MCP4728 with a specific route
    MCP4728(uint8_t i2cAddress, Router* router, I2CRoute* route);

    void begin();

    // Write to a specific DAC channel (A, B, C, D)
    bool writeDAC(MCP4728_channel_t channel, uint16_t value);
    // Read the current value of a specific DAC channel (A, B, C, D)
    uint16_t readDAC(MCP4728_channel_t channel);

private:
    uint8_t _i2cAddress; // Current I2C address of the device
    Router* _router; // Pointer to the router instance
    I2CRoute* _route; // Pointer to the specific route for this device
    Adafruit_MCP4728 mcp;
};

#endif // MCP4728_H
