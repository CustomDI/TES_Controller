#ifndef INA228_H
#define INA228_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_INA228.h>

#define INA228_RSHUNT 10 // Default shunt resistance in ohms
#define INA228_MAX_EXPECTED_CURRENT 0.032 // Default max expected current in Amps (32mA)

class INA228 {
public:
    INA228(uint8_t i2cAddress);

    uint8_t begin();
    uint8_t begin(float shuntResistance, float maxCurrent);
    uint8_t calibrate(float shuntResistance, float maxCurrent); // shuntResistance in ohms, maxCurrent in Amps

    uint8_t getShuntVoltage_mV(float& shuntVoltage);
    uint8_t getBusVoltage_V(float& busVoltage);
    uint8_t getCurrent_mA(float& current);
    uint8_t getPower_mW(float& power);

private:
    uint8_t _i2cAddress;
    Adafruit_INA228 _ina228;
};

#endif // INA228_H
