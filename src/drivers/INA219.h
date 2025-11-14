#ifndef INA219_H
#define INA219_H

#include <Arduino.h>
#include <Wire.h>
#include "../routers/Router.h"

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
    INA219(uint8_t i2cAddress, Router* router, I2CRoute* route);

    void begin();
    void begin(float shuntResistance, float maxCurrent);
    void calibrate(float shuntResistance, float maxCurrent); // shuntResistance in ohms, maxCurrent in Amps

    float getShuntVoltage_mV();
    float getBusVoltage_V();
    float getCurrent_mA();
    float getPower_mW();

    void printRoute(); // For debugging

private:
    uint8_t _i2cAddress;
    Router* _router;
    I2CRoute* _route;

    void writeRegister(uint8_t reg, uint16_t value);
    uint16_t readRegister(uint8_t reg);

    float _currentDivider_mA;
    float _powerMultiplier_mW;
};

#endif // INA219_H
