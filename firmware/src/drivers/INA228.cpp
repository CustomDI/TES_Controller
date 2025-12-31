#include "INA228.h"


INA228::INA228(uint8_t i2cAddress) : _i2cAddress(i2cAddress) {}

uint8_t INA228::begin() {
    Wire.begin();
    _ina228.begin(_i2cAddress);
    return calibrate(INA228_RSHUNT, INA228_MAX_EXPECTED_CURRENT);
}

uint8_t INA228::begin(float shuntResistance, float maxCurrent) {
    Wire.begin();
    _ina228.begin(_i2cAddress);
    return calibrate(shuntResistance, maxCurrent); // Default calibration: 10 ohm shunt, 32mA max current
}

uint8_t INA228::calibrate(float shuntResistance, float maxCurrent) {
    _ina228.setShunt(shuntResistance, maxCurrent);
    return 0;
}

uint8_t INA228::getShuntVoltage_mV(float &shuntVoltage) {
    shuntVoltage = _ina228.getShuntVoltage_mV();
    return 0;
}

uint8_t INA228::getBusVoltage_V(float &busVoltage) {
    busVoltage = _ina228.getBusVoltage_V();
    return 0;
}

uint8_t INA228::getCurrent_mA(float &current) {
    current = _ina228.getCurrent_mA();
    return 0;
}

uint8_t INA228::getPower_mW(float &power) {
    power = _ina228.getPower_mW();
    return 0;
}