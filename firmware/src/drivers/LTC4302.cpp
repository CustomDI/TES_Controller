#include "LTC4302.h"

LTC4302::LTC4302(uint8_t i2cAddress) : _i2cAddress(i2cAddress) {}

uint8_t LTC4302::begin() {
    Wire.begin();
    RETURN_IF_ERROR(disableBus()); // Start with bus disabled
    RETURN_IF_ERROR(setGPIO(1, true)); // Set GPIO1 HIGH
    RETURN_IF_ERROR(setGPIO(2, true)); // Set GPIO2 HIGH
}

uint8_t LTC4302::readRegister(uint8_t reg, uint8_t& value) {
    Wire.beginTransmission(_i2cAddress);
    Wire.write(reg);
    RETURN_IF_ERROR(Wire.endTransmission(false)); // Send restart
    Wire.requestFrom(_i2cAddress, (uint8_t)1);
    if (Wire.available()) {
        value = Wire.read();
    } else {
        return 10;
    }
    return 0;
}

uint8_t LTC4302::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(_i2cAddress);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission();
}

uint8_t LTC4302::writeRegister(uint8_t value) {
    Wire.beginTransmission(_i2cAddress);
    Wire.write(value);
    return Wire.endTransmission();
}

uint8_t LTC4302::setGPIO(uint8_t gpioPin, bool state) {
    // GPIO1 -> bit 5, GPIO2 -> bit 6 in register 0x01
    if (gpioPin < 1 || gpioPin > 2) {
        Serial.println("LTC4302: Invalid GPIO pin (use 1 or 2)");
        return 10;
    }
    uint8_t bit = (gpioPin == 1) ? (1 << 5) : (1 << 6);
    uint8_t regValue;
    RETURN_IF_ERROR(readRegister(0x01, regValue));
    if (state) {
        // Serial.println("LTC4302: Setting GPIO" + String(gpioPin) + " HIGH on address 0x" + String(_i2cAddress, HEX));
        regValue |= bit;
    } else {
        // Serial.println("LTC4302: Setting GPIO" + String(gpioPin) + " LOW on address 0x" + String(_i2cAddress, HEX));
        regValue &= ~bit;
    }
    return writeRegister(regValue);
}

uint8_t LTC4302::getGPIO(uint8_t gpioPin, bool& state) {
    // GPIO1 -> bit 5, GPIO2 -> bit 6 in register 0x01
    if (gpioPin < 1 || gpioPin > 2) {
        Serial.println("LTC4302: Invalid GPIO pin (use 1 or 2)");
        state = false;
        return 10;
    }
    uint8_t bit = (gpioPin == 1) ? (1 << 5) : (1 << 6);
    uint8_t regValue;
    RETURN_IF_ERROR(readRegister(0x01, regValue));
    state = (regValue & bit) != 0;
    return 0;
}

uint8_t LTC4302::enableBus() {
    // Assuming a specific register and bit for enabling the bus.
    // This needs to be confirmed with the LTC4302 datasheet.
    // For now, let's assume register 0x01 controls the bus enable,
    // and setting bit 0 enables it.
    // Serial.println("LTC4302: Enabling bus on address 0x" + String(_i2cAddress, HEX));
    uint8_t regValue;
    RETURN_IF_ERROR(readRegister(0x01, regValue));
    return writeRegister(regValue | 1 << 7); // Set bit 7
}

uint8_t LTC4302::disableBus() {
    // Assuming a specific register and bit for disabling the bus.
    // This needs to be confirmed with the LTC4302 datasheet.
    // For now, let's assume register 0x01 controls the bus enable,
    // and clearing bit 0 disables it.
    // Serial.println("LTC4302: Disabling bus on address 0x" + String(_i2cAddress, HEX));
    uint8_t regValue; 
    RETURN_IF_ERROR(readRegister(0x01, regValue));
    return writeRegister(regValue & ~(1 << 7)); // Clear bit 7
}
