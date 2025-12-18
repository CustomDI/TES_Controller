#include "INA219.h"

// Configuration Register Bits
#define INA219_CONFIG_BVOLTAGERANGE_32V (0x01 << 13) // 0-32V Range
#define INA219_CONFIG_GAIN_8_320MV      (0x04 << 11) // Gain 8, 320mV shunt voltage range
#define INA219_CONFIG_BADCRES_12BIT     (0x03 << 7)  // 12-bit bus ADC resolution
#define INA219_CONFIG_SADCRES_12BIT_128S (0x09 << 3) // 12-bit shunt ADC resolution, 128 samples
#define INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS (0x07) // Shunt and Bus, Continuous

INA219::INA219(uint8_t i2cAddress) : _i2cAddress(i2cAddress), _currentDivider_mA(0), _powerMultiplier_mW(0) {}

uint8_t INA219::begin() {
    Wire.begin();
    return calibrate(INA219_RSHUNT, INA219_MAX_EXPECTED_CURRENT); // Default calibration: 10 ohm shunt, 32mA max current
}

uint8_t INA219::begin(float shuntResistance, float maxCurrent) {
    Wire.begin();
    return calibrate(shuntResistance, maxCurrent); // Default calibration: 10 ohm shunt, 32mA max current
}

uint8_t INA219::calibrate(float shuntResistance, float maxCurrent) {
    // Calculate calibration register value
    // Calibration value = trunc (0.04096 / (Current_LSB * R_Shunt))
    // Current_LSB = maxCurrent / 32768 (for 12-bit ADC)
    // Simplified: Cal = (0.04096 * 32768) / (maxCurrent * R_Shunt)
    // Cal = 1342.17728 / (maxCurrent * R_Shunt)

    float current_LSB = maxCurrent / 32768.0; // Amps per LSB
    uint16_t calValue = (uint16_t)(0.04096 / (current_LSB * shuntResistance));

    RETURN_IF_ERROR(writeRegister(INA219_REG_CALIBRATION, calValue));

    // These values are used for converting raw register values to meaningful units
    _currentDivider_mA = 1 / (current_LSB * 1000.0) ; // Current LSB in mA
    _powerMultiplier_mW = 20.0 * _currentDivider_mA; // Power LSB in mW (Power LSB = 20 * Current LSB)
    return 0;
}

uint8_t INA219::getShuntVoltage_mV(float &shuntVoltage) {
    uint16_t value;
    RETURN_IF_ERROR(readRegister(INA219_REG_SHUNTVOLTAGE, value));
    shuntVoltage = (int16_t)value * 0.01; // LSB = 10 uV = 0.01 mV
    return 0;
}

uint8_t INA219::getBusVoltage_V(float &busVoltage) {
    uint16_t value;
    RETURN_IF_ERROR(readRegister(INA219_REG_BUSVOLTAGE, value));
    value >>= 3; // Shift to get rid of CNVR and OVF bits
    busVoltage = (float)value * 0.004; // LSB = 4 mV = 0.004 V
    return 0;
}

uint8_t INA219::getCurrent_mA(float &current) {
    uint16_t value;
    RETURN_IF_ERROR(readRegister(INA219_REG_CURRENT, value));
    current = (int16_t)value / _currentDivider_mA;
    return 0;
}

uint8_t INA219::getPower_mW(float &power) {
    uint16_t value;
    RETURN_IF_ERROR(readRegister(INA219_REG_POWER, value));
    power = (int16_t)value / _powerMultiplier_mW;
    return 0;
}

uint8_t INA219::writeRegister(uint8_t reg, uint16_t value) {
    Wire.beginTransmission(_i2cAddress);
    Wire.write(reg);
    Wire.write((value >> 8) & 0xFF); // High byte
    Wire.write(value & 0xFF);       // Low byte
    return Wire.endTransmission();
}

uint8_t INA219::readRegister(uint8_t reg, uint16_t& value) {
    Wire.beginTransmission(_i2cAddress);
    Wire.write(reg);
    RETURN_IF_ERROR(Wire.endTransmission(false)); // Send restart
    Wire.requestFrom(_i2cAddress, (uint8_t)2);
    
    value = 0;
    if (Wire.available() == 2) {
        value = Wire.read() << 8;
        value |= Wire.read();
    }
    return 0;
}