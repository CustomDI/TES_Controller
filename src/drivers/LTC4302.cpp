#include "LTC4302.h"

LTC4302::LTC4302(uint8_t i2cAddress) : _i2cAddress(i2cAddress) {}

void LTC4302::begin() {
    Wire.begin();
    disableBus(); // Start with bus disabled
    setGPIO(1, true); // Set GPIO1 HIGH
    setGPIO(2, true); // Set GPIO2 HIGH
}

uint8_t LTC4302::readRegister(uint8_t reg) {
    Wire.beginTransmission(_i2cAddress);
    Wire.write(reg);
    Wire.endTransmission(false); // Send restart
    Wire.requestFrom(_i2cAddress, (uint8_t)1);
    if (Wire.available()) {
        return Wire.read();
    }
    return 0; // Error or no data
}

void LTC4302::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(_i2cAddress);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
}

void LTC4302::writeRegister(uint8_t value) {
    Wire.beginTransmission(_i2cAddress);
    Wire.write(value);
    Wire.endTransmission();
}

void LTC4302::setGPIO(uint8_t gpioPin, bool state) {
    // GPIO1 -> bit 5, GPIO2 -> bit 6 in register 0x01
    if (gpioPin < 1 || gpioPin > 2) {
        Serial.println("LTC4302: Invalid GPIO pin (use 1 or 2)");
        return;
    }
    uint8_t bit = (gpioPin == 1) ? (1 << 5) : (1 << 6);
    uint8_t regValue = readRegister(0x01);
    if (state) {
        // Serial.println("LTC4302: Setting GPIO" + String(gpioPin) + " HIGH on address 0x" + String(_i2cAddress, HEX));
        regValue |= bit;
    } else {
        // Serial.println("LTC4302: Setting GPIO" + String(gpioPin) + " LOW on address 0x" + String(_i2cAddress, HEX));
        regValue &= ~bit;
    }
    writeRegister(regValue);
}

void LTC4302::getGPIO(uint8_t gpioPin, bool& state) {
    // GPIO1 -> bit 5, GPIO2 -> bit 6 in register 0x01
    if (gpioPin < 1 || gpioPin > 2) {
        Serial.println("LTC4302: Invalid GPIO pin (use 1 or 2)");
        state = false;
        return;
    }
    uint8_t bit = (gpioPin == 1) ? (1 << 5) : (1 << 6);
    uint8_t regValue = readRegister(0x01);
    state = (regValue & bit) != 0;
}

void LTC4302::enableBus() {
    // Assuming a specific register and bit for enabling the bus.
    // This needs to be confirmed with the LTC4302 datasheet.
    // For now, let's assume register 0x01 controls the bus enable,
    // and setting bit 0 enables it.
    // Serial.println("LTC4302: Enabling bus on address 0x" + String(_i2cAddress, HEX));
    uint8_t regValue = readRegister(0x01);
    writeRegister(regValue | 1 << 7); // Set bit 7
}

void LTC4302::disableBus() {
    // Assuming a specific register and bit for disabling the bus.
    // This needs to be confirmed with the LTC4302 datasheet.
    // For now, let's assume register 0x01 controls the bus enable,
    // and clearing bit 0 disables it.
    // Serial.println("LTC4302: Disabling bus on address 0x" + String(_i2cAddress, HEX));
    uint8_t regValue = readRegister(0x01);
    writeRegister(regValue & ~(1 << 7)); // Clear bit 7
}
