#ifndef LNA_DRIVER_H
#define LNA_DRIVER_H

#include <Arduino.h>
#include "../routers/Router.h"
#include "../drivers/MCP4728.h"
#include "../drivers/INA219.h"
#include "../drivers/LTC4302.h" // Include LTC4302 for the internal instance

class LNADriver {
public:
    LNADriver(LTC4302* lnaLtc4302, Router* router); // Removed baseHubChannel
    void begin();
    I2CRoute getRouteToLnaLtc4302() { return _routeToLnaLtc4302; } // Accessor for the route

    // // Methods to interact with the LNA's MCP4728
    void writeLnaDac(uint8_t channel, uint16_t value, bool fastMode = false); // Channel still needed for MCP4728
    void writeAllLnaDacs(uint16_t valueA, uint16_t valueB, uint16_t valueC, uint16_t valueD, bool fastMode = false);
    void writeLnaDacWithPowerDown(uint8_t channel, uint16_t value, uint8_t powerDownMode); // Channel still needed for MCP4728

    // Methods to interact with the LNA's INA219s
    float getIna1ShuntVoltage_mV();
    float getIna1BusVoltage_V();
    float getIna1Current_mA();
    float getIna1Power_mW();

    float getIna2ShuntVoltage_mV();
    float getIna2BusVoltage_V();
    float getIna2Current_mA();
    float getIna2Power_mW();

private:
    LTC4302* _lnaLtc4302; // Pointer to the LNA's LTC4302 instance
    Router* _router;

    // Routes for devices behind the LNA LTC4302
    I2CRoute _routeToLnaLtc4302;
    // The following routes are no longer needed as the LTC4302 does not have channels

    MCP4728 _lnaDac;
    INA219 _lnaIna1;
    INA219 _lnaIna2;
};

#endif // LNA_DRIVER_H
