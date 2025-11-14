#include "INA219.h"

// Configuration Register Bits
#define INA219_CONFIG_BVOLTAGERANGE_32V (0x01 << 13) // 0-32V Range
#define INA219_CONFIG_GAIN_8_320MV      (0x04 << 11) // Gain 8, 320mV shunt voltage range
#define INA219_CONFIG_BADCRES_12BIT     (0x03 << 7)  // 12-bit bus ADC resolution
#define INA219_CONFIG_SADCRES_12BIT_128S (0x09 << 3) // 12-bit shunt ADC resolution, 128 samples
#define INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS (0x07) // Shunt and Bus, Continuous

INA219::INA219(uint8_t i2cAddress) : _i2cAddress(i2cAddress), _router(nullptr), _route(nullptr),
                                     _currentDivider_mA(0), _powerMultiplier_mW(0) {}

INA219::INA219(uint8_t i2cAddress, Router* router, I2CRoute* route)
    : _i2cAddress(i2cAddress), _router(router), _route(route),
      _currentDivider_mA(0), _powerMultiplier_mW(0) {}

void INA219::begin() {
    Wire.begin();
    calibrate(INA219_RSHUNT, INA219_MAX_EXPECTED_CURRENT); // Default calibration: 10 ohm shunt, 32mA max current
}

void INA219::begin(float shuntResistance, float maxCurrent) {
    Wire.begin();
    calibrate(shuntResistance, maxCurrent); // Default calibration: 10 ohm shunt, 32mA max current
}

void INA219::calibrate(float shuntResistance, float maxCurrent) {
    // Calculate calibration register value
    // Calibration value = trunc (0.04096 / (Current_LSB * R_Shunt))
    // Current_LSB = maxCurrent / 32768 (for 12-bit ADC)
    // Simplified: Cal = (0.04096 * 32768) / (maxCurrent * R_Shunt)
    // Cal = 1342.17728 / (maxCurrent * R_Shunt)

    float current_LSB = maxCurrent / 32768.0; // Amps per LSB
    uint16_t calValue = (uint16_t)(0.04096 / (current_LSB * shuntResistance));

    writeRegister(INA219_REG_CALIBRATION, calValue);

    // These values are used for converting raw register values to meaningful units
    _currentDivider_mA = 1 / (current_LSB * 1000.0) ; // Current LSB in mA
    _powerMultiplier_mW = 20.0 * _currentDivider_mA; // Power LSB in mW (Power LSB = 20 * Current LSB)
}

float INA219::getShuntVoltage_mV() {
    uint16_t value = readRegister(INA219_REG_SHUNTVOLTAGE);
    return (int16_t)value * 0.01; // LSB = 10 uV = 0.01 mV
}

float INA219::getBusVoltage_V() {
    uint16_t value = readRegister(INA219_REG_BUSVOLTAGE);
    value >>= 3; // Shift to get rid of CNVR and OVF bits
    return (float)value * 0.004; // LSB = 4 mV = 0.004 V
}

float INA219::getCurrent_mA() {
    uint16_t value = readRegister(INA219_REG_CURRENT);
    return (int16_t)value / _currentDivider_mA;
}

float INA219::getPower_mW() {
    uint16_t value = readRegister(INA219_REG_POWER);
    return (int16_t)value / _powerMultiplier_mW;
}

void INA219::writeRegister(uint8_t reg, uint16_t value) {
    if (_router != nullptr && _route != nullptr) {
        _router->routeTo(_route);
    }

    Wire.beginTransmission(_i2cAddress);
    Wire.write(reg);
    Wire.write((value >> 8) & 0xFF); // High byte
    Wire.write(value & 0xFF);       // Low byte
    Wire.endTransmission();

    if (_router != nullptr && _route != nullptr) {
        _router->endRoute(_route);
    }
}

uint16_t INA219::readRegister(uint8_t reg) {
    if (_router != nullptr && _route != nullptr) {
        _router->routeTo(_route);
    }

    Wire.beginTransmission(_i2cAddress);
    Wire.write(reg);
    Wire.endTransmission(false); // Send restart
    Wire.requestFrom(_i2cAddress, (uint8_t)2);

    uint16_t value = 0;
    if (Wire.available() == 2) {
        value = Wire.read() << 8;
        value |= Wire.read();
    }

    if (_router != nullptr && _route != nullptr) {
        _router->endRoute(_route);
    }
    return value;
}

void INA219::printRoute() {
    if (_router != nullptr && _route != nullptr) {
        Serial.print("INA219 Route: ");
        I2CRoute* current = _route;
        while (current != nullptr) {
            if (current->hub != nullptr) {
                Serial.print(" -> 0x");
                Serial.print(current->hub->get_i2cAddress(), HEX);
            }
            current = current->next;
        }
        Serial.println();
    } else {
        Serial.println("INA219: No routing information available.");
    }
}