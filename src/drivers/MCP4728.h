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

    uint8_t begin();

    // Write to a specific DAC channel (A, B, C, D)
    uint8_t writeDAC(MCP4728_channel_t channel, uint16_t value);
    // Read the current value of a specific DAC channel (A, B, C, D)
    uint8_t readDAC(MCP4728_channel_t channel, uint16_t &value);

private:
    uint8_t _i2cAddress; // Current I2C address of the device
    Adafruit_MCP4728 mcp;
};

#endif // MCP4728_H
