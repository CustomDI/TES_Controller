#ifndef MCP4728_H
#define MCP4728_H

#include <Arduino.h>
#include <Wire.h>
#include "../routers/Router.h"

class MCP4728 {
public:
    // Constructor for a single MCP4728 device (no routing)
    MCP4728(uint8_t i2cAddress);

    // Constructor for MCP4728 with a specific route
    MCP4728(uint8_t i2cAddress, Router* router, I2CRoute* route);

    void begin();

    // Write to a specific DAC channel (A, B, C, D)
    void writeDAC(uint8_t channel, uint16_t value, bool fastMode = false);

    // Write to all DAC channels simultaneously
    void writeAllDACs(uint16_t valueA, uint16_t valueB, uint16_t valueC, uint16_t valueD, bool fastMode = false);

    // Write to a specific DAC channel with power-down mode
    void writeDACWithPowerDown(uint8_t channel, uint16_t value, uint8_t powerDownMode);

private:
    uint8_t _i2cAddress; // Current I2C address of the device
    Router* _router; // Pointer to the router instance
    I2CRoute* _route; // Pointer to the specific route for this device
    void sendCommand(uint8_t cmd, uint16_t data);
};

#endif // MCP4728_H
