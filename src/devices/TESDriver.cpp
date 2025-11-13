#include "TESDriver.h"

// Each TESDriver manages a single TES device, which is behind its own LTC4302.
// Therefore, the concept of "channels on the TES LTC4302 for its devices" is simplified.
// The TES device itself is directly accessed through the LTC.


TESDriver::TESDriver(LTC4302* tesLtc4302, Router* router)
    : _tesLtc4302(tesLtc4302),
      _router(router),
      // Initialize the route to the TES LTC4302 itself
      _routeToTesLtc4302({_tesLtc4302, nullptr}),
      _tca(TES_TCA_ADDR, router, &_routeToTesLtc4302),
      _ina(TES_INA_ADDR, router, &_routeToTesLtc4302){}

void TESDriver::begin() {
    _tesLtc4302->begin(); // Call begin on the pointer    
    _ina.begin(); // Initialize INA219
    _tca.begin(); // Initialize TCA642ARGJR
}

void TESDriver::setOutEnable(bool state) {
    state = !state; // Invert logic: HIGH = disable, LOW = enable
    _router->routeTo(&_routeToTesLtc4302); // This will route to the TES LTC4302
    _tesLtc4302->setGPIO(2, state); // GPIO2 controls OUT_EN
    _tesLtc4302->setGPIO(1, state); // GPIO1 controls OUT_EN
    _router->endRoute(&_routeToTesLtc4302); // End routing
}

float TESDriver::getBusVoltage_V(){
    return _ina.getBusVoltage_V();
}

float TESDriver::getShuntVoltage_mV(){
    return _ina.getShuntVoltage_mV();
}

float TESDriver::getCurrent_mA(){
    return _ina.getCurrent_mA();
}

float TESDriver::getPower_mW(){
    return _ina.getPower_mW();
}

void TESDriver::setOutputPin(uint8_t pin, bool state) {
    _tca.setOutputPin(pin, state);
}
bool TESDriver::getOutputPin(uint8_t pin) {
    return _tca.getOutputPin(pin);
}
void TESDriver::setAllOutputPins(uint32_t state) {
    _tca.setAllOutputPins(state);
}
uint32_t TESDriver::getAllOutputPins() {
    return _tca.getAllOutputPins();
}

void TESDriver::readOutputRegisters(uint8_t out[3]) {
    // Directly read the TCA's output registers via the TCA driver
    _tca.readRegisters( (uint8_t)TCA642ARGJR_OUTPUT_PORT0, out, 3 );
}
