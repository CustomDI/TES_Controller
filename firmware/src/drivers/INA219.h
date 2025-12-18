#ifndef INA219_H
#define INA219_H

#include <Arduino.h>
#include <Wire.h>
#include "../routers/Router.h"
#include "../helpers/error.h"

#define INA219_RSHUNT 10 // Default shunt resistance in ohms
#define INA219_MAX_EXPECTED_CURRENT 0.032 // Default max expected current in Amps (32mA)

// INA219 Register Addresses
#define INA219_REG_CONFIG       0x00
#define INA219_REG_SHUNTVOLTAGE 0x01
#define INA219_REG_BUSVOLTAGE   0x02
#define INA219_REG_POWER        0x03
#define INA219_REG_CURRENT      0x04
#define INA219_REG_CALIBRATION  0x05

class INA219 {
public:
    INA219(uint8_t i2cAddress);

    uint8_t begin();
    uint8_t begin(float shuntResistance, float maxCurrent);
    uint8_t calibrate(float shuntResistance, float maxCurrent); // shuntResistance in ohms, maxCurrent in Amps

    uint8_t getShuntVoltage_mV(float& shuntVoltage);
    uint8_t getBusVoltage_V(float& busVoltage);
    uint8_t getCurrent_mA(float& current);
    uint8_t getPower_mW(float& power);

private:
    uint8_t _i2cAddress;

    uint8_t writeRegister(uint8_t reg, uint16_t value);
    uint8_t readRegister(uint8_t reg, uint16_t& value);

    float _currentDivider_mA;
    float _powerMultiplier_mW;
};

#endif // INA219_H
