#ifndef LTC4302_H
#define LTC4302_H

#include <Arduino.h>
#include <Wire.h>
#include "../helpers/error.h"

class LTC4302 {
public:
    LTC4302(uint8_t i2cAddress);
    uint8_t begin();
    uint8_t readRegister(uint8_t reg, uint8_t& value);
    uint8_t writeRegister(uint8_t reg, uint8_t value);
    uint8_t writeRegister(uint8_t value);
    uint8_t setGPIO(uint8_t gpioPin, bool state);
    uint8_t getGPIO(uint8_t gpioPin, bool& state);
    uint8_t enableBus();
    uint8_t disableBus();
    uint8_t get_i2cAddress() { return _i2cAddress; }

private:
    uint8_t _i2cAddress;
    // Add a member to store the currently selected channel if needed for state tracking
    // uint8_t _currentChannel;
};

#endif // LTC4302_H