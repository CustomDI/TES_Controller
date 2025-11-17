#include "TCA642ARGJR.h"

TCA642ARGJR::TCA642ARGJR(uint8_t address): _address(address) {}

uint8_t TCA642ARGJR::begin() {
    Wire.begin();
    // By default configure all pins as outputs (0) and clear outputs.
    uint8_t config[3] = {0x00, 0x00, 0x00};
    RETURN_IF_ERROR(writeRegisters(TCA642ARGJR_CONFIG_PORT0, config, 3));

    uint8_t outputs[3] = {0xFF, 0xFF, 0xFF};
    RETURN_IF_ERROR(writeRegisters(TCA642ARGJR_OUTPUT_PORT0, outputs, 3));
    return 0;
}

uint8_t TCA642ARGJR::setOutputPin(uint8_t pin, bool state) {
    if (pin >= 24) return 10;
    uint8_t port = pin / 8;
    uint8_t bit = pin % 8;
    uint8_t reg = TCA642ARGJR_OUTPUT_PORT0 + port;
    uint8_t currentValue;

    RETURN_IF_ERROR(readRegister(reg, currentValue));

    if (state) currentValue |= (1 << bit);
    else currentValue &= ~(1 << bit);

    return writeRegister(reg, currentValue);
}

uint8_t TCA642ARGJR::getOutputPin(uint8_t pin, bool& state) {
    if (pin >= 24) return 10;
    uint8_t port = pin / 8;
    uint8_t bit = pin % 8;
    uint8_t reg = TCA642ARGJR_OUTPUT_PORT0 + port;
    uint8_t currentValue;
    RETURN_IF_ERROR(readRegister(reg, currentValue));
    state = ((currentValue >> bit) & 0x01) != 0;
    return 0;
}

uint8_t TCA642ARGJR::setAllOutputPins(uint32_t state) {
    uint8_t out[3];
    out[0] = state & 0xFF;
    out[1] = (state >> 8) & 0xFF;
    out[2] = (state >> 16) & 0xFF;
    return writeRegisters(TCA642ARGJR_OUTPUT_PORT0, out, 3);
}

uint8_t TCA642ARGJR::getAllOutputPins(uint32_t& state) {
    uint8_t out[3];
    RETURN_IF_ERROR(readRegisters(TCA642ARGJR_OUTPUT_PORT0, out, 3));
    state = ((uint32_t)out[2] << 16) | ((uint32_t)out[1] << 8) | out[0];
    return 0;
}

uint8_t TCA642ARGJR::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(_address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission();
}

uint8_t TCA642ARGJR::readRegister(uint8_t reg, uint8_t& value) {
    uint8_t value = 0;
    Wire.beginTransmission(_address);
    Wire.write(reg);
    RETURN_IF_ERROR(Wire.endTransmission());
    Wire.requestFrom(_address, (uint8_t)1);
    if (Wire.available()) value = Wire.read();
    return 0;
}

uint8_t TCA642ARGJR::writeRegisters(uint8_t startReg, const uint8_t* data, size_t length) {
    for (size_t i = 0; i < length; ++i) {
        RETURN_IF_ERROR(writeRegister(startReg + i, data[i]));
    }
    return 0;
}

uint8_t TCA642ARGJR::readRegisters(uint8_t startReg, uint8_t* data, size_t length) {
    size_t i = 0;
    for (i; i < length;) {
        RETURN_IF_ERROR(readRegister(startReg + i, data[i++]));
    }
    return 0;
}
