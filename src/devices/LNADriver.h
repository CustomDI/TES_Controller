#ifndef LNA_DRIVER_H
#define LNA_DRIVER_H

#include <Arduino.h>
#include "../routers/Router.h"
#include "../drivers/MCP4728.h"
#include "../drivers/INA219.h"
#include "../drivers/LTC4302.h"
#include "../helpers/error.h"

// Define I2C addresses for devices behind the LNA LTC4302
#define LNA_MCP4728_ADDR      0x60 // Address for MCP4728 behind LNA driver
#define LNA_INA_DRAIN_ADDR    0x40 // Address for INA219_1 behind LNA driver
#define LNA_INA_GATE_ADDR     0x41 // Address for INA219_2 behind LNA driver
#define LNA_INA_SHUNT_RESISTANCE_OHMS 5.0f // Shunt resistance for INA219s
#define LNA_INA_MAX_EXPECTED_CURRENT_AMPS .064f // Max expected current for INA219s

// MCP4728 channels for LNA functions
#define LNA_DRAIN_CHANNEL MCP4728_CHANNEL_A
#define LNA_GATE_CHANNEL  MCP4728_CHANNEL_B
class LNADriver {
public:
    LNADriver(LTC4302* lnaLtc4302, Router* router); // Removed baseHubChannel
    uint8_t begin();
    I2CRoute getRouteToLnaLtc4302() { return _routeToLnaLtc4302; } // Accessor for the route

    // // Methods to interact with the LNA's MCP4728
    uint8_t writeDrain(uint16_t value);
    uint8_t writeGate(uint16_t value);
    uint8_t readDrain(uint16_t& value);
    uint8_t readGate(uint16_t& value);

    // Methods to interact with the LNA's INA219s
    uint8_t getDrainShuntVoltage_mV(float& shuntVoltage);
    uint8_t getDrainBusVoltage_V(float& busVoltage);
    uint8_t getDrainCurrent_mA(float& current);
    uint8_t getDrainPower_mW(float& power);

    uint8_t getGateShuntVoltage_mV(float& shuntVoltage);
    uint8_t getGateBusVoltage_V(float& busVoltage);
    uint8_t getGateCurrent_mA(float& current);
    uint8_t getGatePower_mW(float& power);

    // Methods to control GPIOs on the LNA LTC4302
    uint8_t setDrainEnable(bool state);
    uint8_t setGateEnable(bool state);
    uint8_t getDrainEnable(bool& state);
    uint8_t getGateEnable(bool& state);

private:
    LTC4302* _lnaLtc4302; // Pointer to the LNA's LTC4302 instance
    Router* _router;

    // Routes for devices behind the LNA LTC4302
    I2CRoute _routeToLnaLtc4302;
    // The following routes are no longer needed as the LTC4302 does not have channels

    MCP4728 _lnaDac;
    INA219 _lnaInaDrain;
    INA219 _lnaInaGate;

    uint8_t connect();
    uint8_t disconnect();
};

#endif // LNA_DRIVER_H
