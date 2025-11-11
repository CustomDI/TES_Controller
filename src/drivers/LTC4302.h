#ifndef LTC4302_H
#define LTC4302_H

#include <Arduino.h>
#include <Wire.h>

class LTC4302 {
public:
    LTC4302(uint8_t i2cAddress);
    void begin();
    uint8_t readRegister(uint8_t reg);
    void writeRegister(uint8_t reg, uint8_t value);
    void writeRegister(uint8_t value);
    void enableBus();
    void setGPIO(uint8_t gpioPin, bool state);
    void disableBus();
    uint8_t get_i2cAddress() { return _i2cAddress; }

private:
    uint8_t _i2cAddress;
    // Add a member to store the currently selected channel if needed for state tracking
    // uint8_t _currentChannel;
};

#endif // LTC4302_H