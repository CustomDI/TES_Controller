#include "LNADriver.h"

// The LTC4302 acts as an I2C repeater, not a multiplexer with channels.
// Therefore, internal channel definitions for devices behind it are not needed for routing.
// The MCP4728 still has its own internal channels (A, B, C, D) for DAC selection.

// Define I2C addresses for devices behind the LNA LTC4302
#define LNA_MCP4728_ADDR 0x60 // Default address for MCP4728 behind LNA driver (0x60 on startup)
#define LNA_INA1_ADDR    0x40 // Example address for INA219_1 behind LNA driver
#define LNA_INA2_ADDR    0x41 // Example address for INA219_2 behind LNA driver

LNADriver::LNADriver(LTC4302* lnaLtc4302, Router* router)
    : _lnaLtc4302(lnaLtc4302), _router(router),
      // Initialize the route to the LNA LTC4302 itself
      // The route consists of the LNA LTC4302, with no further hops in this specific route step.
      _routeToLnaLtc4302({_lnaLtc4302, nullptr}),
      // Initialize device drivers with their specific routes
      // The route passed to the device driver is the route to the LNA LTC4302.
      // The Router will enable/disable the bus for this LTC4302.
      _lnaDac(LNA_MCP4728_ADDR, _router, &_routeToLnaLtc4302),
      _lnaIna1(LNA_INA1_ADDR, _router, &_routeToLnaLtc4302),
      _lnaIna2(LNA_INA2_ADDR, _router, &_routeToLnaLtc4302)
{
    // The Router::routeTo() method will enable the bus for the LNA LTC4302.
    // Subsequent I2C communication will then directly address the devices behind it.
}

void LNADriver::begin() {
    _lnaLtc4302->begin(); // Call begin on the pointer
    _lnaDac.begin();
    _lnaIna1.begin();
    _lnaIna2.begin();

    // // Calibrate INA219s (example values, adjust as needed)
    // _lnaIna1.calibrate(0.1, 1.0); // 0.1 Ohm shunt, max 1.0 Amp
    // _lnaIna2.calibrate(0.1, 1.0); // 0.1 Ohm shunt, max 1.0 Amp
}

// Methods to interact with the LNA's MCP4728
void LNADriver::writeLnaDac(uint8_t channel, uint16_t value, bool fastMode) {
    _lnaDac.writeDAC(channel, value, fastMode);
}

void LNADriver::writeAllLnaDacs(uint16_t valueA, uint16_t valueB, uint16_t valueC, uint16_t valueD, bool fastMode) {
    _lnaDac.writeAllDACs(valueA, valueB, valueC, valueD, fastMode);
}

void LNADriver::writeLnaDacWithPowerDown(uint8_t channel, uint16_t value, uint8_t powerDownMode) {
    _lnaDac.writeDACWithPowerDown(channel, value, powerDownMode);
}

// Methods to interact with the LNA's INA219s
float LNADriver::getIna1ShuntVoltage_mV() {
    return _lnaIna1.getShuntVoltage_mV();
}

float LNADriver::getIna1BusVoltage_V() {
    return _lnaIna1.getBusVoltage_V();
}

float LNADriver::getIna1Current_mA() {
    return _lnaIna1.getCurrent_mA();
}

float LNADriver::getIna1Power_mW() {
    return _lnaIna1.getPower_mW();
}

float LNADriver::getIna2ShuntVoltage_mV() {
    return _lnaIna2.getShuntVoltage_mV();
}

float LNADriver::getIna2BusVoltage_V() {
    return _lnaIna2.getBusVoltage_V();
}

float LNADriver::getIna2Current_mA() {
    return _lnaIna2.getCurrent_mA();
}

float LNADriver::getIna2Power_mW() {
    return _lnaIna2.getPower_mW();
}
