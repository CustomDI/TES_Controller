#include "TESDriver.h"

// Each TESDriver manages a single TES device, which is behind its own LTC4302.
// Therefore, the concept of "channels on the TES LTC4302 for its devices" is simplified.
// The TES device itself is directly accessed through the LTC.


TESDriver::TESDriver(LTC4302* tesLtc4302, Router* router)
    : _tesLtc4302(tesLtc4302),
      _router(router),
      // Initialize the route to the TES LTC4302 itself
      _routeToTesLtc4302({_tesLtc4302, nullptr}),
      _tca(TES_TCA_ADDR),
      _ina(TES_INA_ADDR){}

uint8_t TESDriver::begin() {
    RETURN_IF_ERROR(_tesLtc4302->begin()); // Call begin on the pointer
    RETURN_IF_ERROR(connect());
    RETURN_IF_ERROR(_tca.begin()); // Initialize TCA642ARGJR
    RETURN_IF_ERROR(_ina.begin()); // Initialize INA219
    return disconnect();
}

uint8_t TESDriver::setOutEnable(bool state) {
    state = !state; // Invert logic: HIGH = disable, LOW = enable
    RETURN_IF_ERROR(connect());
    RETURN_IF_ERROR(_tesLtc4302->setGPIO(2, state)); // GPIO2 controls OUT_EN
    RETURN_IF_ERROR(_tesLtc4302->setGPIO(1, state)); // GPIO1 controls OUT_EN
    return disconnect();
}

uint8_t TESDriver::getOutEnable(bool& state) { 
    RETURN_IF_ERROR(connect());
    RETURN_IF_ERROR(_tesLtc4302->getGPIO(2, state)); // GPIO2 controls OUT_EN
    state = !state;
    return disconnect();
 }

uint8_t TESDriver::getBusVoltage_V(float& busVoltage){
    RETURN_IF_ERROR(connect());
    RETURN_IF_ERROR(_ina.getBusVoltage_V(busVoltage));
    return disconnect();
}

uint8_t TESDriver::getShuntVoltage_mV(float& shuntVoltage){
    RETURN_IF_ERROR(connect());
    RETURN_IF_ERROR(_ina.getShuntVoltage_mV(shuntVoltage));
    return disconnect();
}

uint8_t TESDriver::getCurrent_mA(float& current){
    RETURN_IF_ERROR(connect());
    RETURN_IF_ERROR(_ina.getCurrent_mA(current));
    return disconnect();
}

uint8_t TESDriver::getPower_mW(float& power){
    RETURN_IF_ERROR(connect());
    RETURN_IF_ERROR(_ina.getPower_mW(power));
    return disconnect();
}

uint8_t TESDriver::setOutputPin(uint8_t pin, bool state) {
    RETURN_IF_ERROR(connect());
    RETURN_IF_ERROR(_tca.setOutputPin(pin, state));
    return disconnect();
}
uint8_t TESDriver::getOutputPin(uint8_t pin, bool& state) {
    RETURN_IF_ERROR(connect());
    RETURN_IF_ERROR(_tca.getOutputPin(pin, state));
    return disconnect();
}
uint8_t TESDriver::setAllOutputPins(uint32_t state) {
    RETURN_IF_ERROR(connect());
    RETURN_IF_ERROR(_tca.setAllOutputPins(state));
    return disconnect();
}
uint8_t TESDriver::getAllOutputPins(uint32_t &state) {
    RETURN_IF_ERROR(connect());
    RETURN_IF_ERROR(_tca.getAllOutputPins(state));
    Serial.println("TESDriver::getAllOutputPins read state: 0x" + String(state, HEX));
    state &= 0xFFFFFu; //Mask to 20 bits
    return disconnect();
}

uint8_t TESDriver::setCurrent_mA(float target_mA, uint32_t* finalState, float* finalMeasured, int delayMs) {
    // Sanity: expected current range 0..20 mA
    if (!(target_mA >= 0.0f && target_mA <= 20.0f)) {
        return 10; // invalid argument
    }

    // Connect once and iterate through bits MSB->LSB greedily.
    RETURN_IF_ERROR(this->connect());

    uint32_t state = 0u; // start with all outputs off
    float measured_mA = 0.0f;

    // Apply initial state
    RETURN_IF_ERROR(this->setAllOutputPins(state));
    if (delayMs > 0) delay(delayMs);
    // Measure baseline
    RETURN_IF_ERROR(this->getCurrent_mA(measured_mA));

    // Greedy MSB-to-LSB bit setting: try each bit, keep it if it improves closeness to target
    for (int bit = 19; bit >= 0; --bit) {
        uint32_t candidate = state | ((uint32_t)1 << bit);

        // Set candidate state
        RETURN_IF_ERROR(this->setAllOutputPins(candidate));
        if (delayMs > 0) delay(delayMs);
        // Measure baseline
        float candidateMeasured = 0.0f;
        RETURN_IF_ERROR(this->getCurrent_mA(candidateMeasured));
        if (candidateMeasured >= target_mA) {
            state = candidate;
            measured_mA = candidateMeasured;
        }
    }
    delay(delayMs);
    // Disconnect route
    RETURN_IF_ERROR(this->disconnect());

    // Output final state and measured value if requested
    if (finalState) {
        *finalState = state;
    }
    if (finalMeasured) {
        *finalMeasured = measured_mA;
    }
    return 0;
}

uint8_t TESDriver::bumpOutputPins(int8_t delta) {
    uint32_t currentState;
    RETURN_IF_ERROR(getAllOutputPins(currentState));
    int32_t newState = (int32_t)currentState + (int32_t)delta;
    if (newState < 0) newState = 0;
    if (newState > 0xFFFFFu) newState = 0xFFFFFu; // Clamp to 20 bits
    return setAllOutputPins((uint32_t)newState);
}

uint8_t TESDriver::connect() {
    return _router->routeTo(&_routeToTesLtc4302);
}

uint8_t TESDriver::disconnect() {
    return _router->endRoute(&_routeToTesLtc4302);
}