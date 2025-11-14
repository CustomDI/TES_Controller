#include "LNADriver.h"

// The LTC4302 acts as an I2C repeater, not a multiplexer with channels.
// Therefore, internal channel definitions for devices behind it are not needed for routing.
// The MCP4728 still has its own internal channels (A, B, C, D) for DAC selection.


LNADriver::LNADriver(LTC4302* lnaLtc4302, Router* router)
    : _lnaLtc4302(lnaLtc4302), _router(router),
      // Initialize the route to the LNA LTC4302 itself
      // The route consists of the LNA LTC4302, with no further hops in this specific route step.
      _routeToLnaLtc4302({_lnaLtc4302, nullptr}),
      // Initialize device drivers with their specific routes
      // The route passed to the device driver is the route to the LNA LTC4302.
      // The Router will enable/disable the bus for this LTC4302.
      _lnaDac(LNA_MCP4728_ADDR, _router, &_routeToLnaLtc4302),
      _lnaInaDrain(LNA_INA_DRAIN_ADDR, _router, &_routeToLnaLtc4302),
      _lnaInaGate(LNA_INA_GATE_ADDR, _router, &_routeToLnaLtc4302)
{
    // The Router::routeTo() method will enable the bus for the LNA LTC4302.
    // Subsequent I2C communication will then directly address the devices behind it.
}

void LNADriver::begin() {
    _lnaLtc4302->begin(); // Call begin on the pointer
    _lnaDac.begin();
    _lnaInaDrain.begin(LNA_INA_SHUNT_RESISTANCE_OHMS, LNA_INA_MAX_EXPECTED_CURRENT_AMPS);
    _lnaInaGate.begin(LNA_INA_SHUNT_RESISTANCE_OHMS, LNA_INA_MAX_EXPECTED_CURRENT_AMPS);
}

// Methods to interact with the LNA's MCP4728
void LNADriver::writeDrain(uint16_t value) {
    _lnaDac.writeDAC(LNA_DRAIN_CHANNEL, value); // Channel A controls Drain
}
void LNADriver::writeGate(uint16_t value) {
    _lnaDac.writeDAC(LNA_GATE_CHANNEL, value); // Channel B controls Gate
}

uint16_t LNADriver::readDrain() {
    return _lnaDac.readDAC(LNA_DRAIN_CHANNEL);
}

uint16_t LNADriver::readGate() {
    return _lnaDac.readDAC(LNA_GATE_CHANNEL);
}

float LNADriver::getDrainShuntVoltage_mV() {
    return _lnaInaDrain.getShuntVoltage_mV();
}

float LNADriver::getDrainBusVoltage_V() {
    return _lnaInaDrain.getBusVoltage_V();
}

float LNADriver::getDrainCurrent_mA() {
    return _lnaInaDrain.getCurrent_mA();
}

float LNADriver::getDrainPower_mW() {
    return _lnaInaDrain.getPower_mW();
}

float LNADriver::getGateShuntVoltage_mV() {
    return _lnaInaGate.getShuntVoltage_mV();
}

float LNADriver::getGateBusVoltage_V() {
    return _lnaInaGate.getBusVoltage_V();
}

float LNADriver::getGateCurrent_mA() {
    return _lnaInaGate.getCurrent_mA();
}

float LNADriver::getGatePower_mW(){
    return _lnaInaGate.getPower_mW();
}

void LNADriver::setGateEnable(bool state) { // False is enable, true is disable
    state = !state; // Invert state for LTC4302 GPIO logic
    _router->routeTo(&_routeToLnaLtc4302); // This will route to the TES LTC4302
    _lnaLtc4302->setGPIO(1, state);
    _router->endRoute(&_routeToLnaLtc4302); // End routing after operation
}

bool LNADriver::getDrainEnable() { 
    bool state;
    _router->routeTo(&_routeToLnaLtc4302); // This will route to the TES LTC4302
    _lnaLtc4302->getGPIO(2, state);
    _router->endRoute(&_routeToLnaLtc4302); // End routing after operation
    return !state; // Invert logic for return value
 }

 bool LNADriver::getGateEnable() {
    bool state;
    _router->routeTo(&_routeToLnaLtc4302); // This will route to the TES LTC4302
    _lnaLtc4302->getGPIO(1, state);
    _router->endRoute(&_routeToLnaLtc4302); // End routing after operation
    return !state; // Invert logic for return value
 }

 void LNADriver::setDrainEnable(bool state) {
     state = !state;  // Invert state for LTC4302 GPIO logic
     _router->routeTo(
         &_routeToLnaLtc4302);  // This will route to the TES LTC4302
     _lnaLtc4302->setGPIO(2, state);
     _router->endRoute(&_routeToLnaLtc4302);  // End routing after operation
 }