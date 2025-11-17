#include "TESDriver.h"
#include "../helpers/search.h"

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
    RETURN_IF_ERROR(_ina.getShuntVoltage_mV(current));
    return disconnect();
}

uint8_t TESDriver::getPower_mW(float& power){
    RETURN_IF_ERROR(connect());
    RETURN_IF_ERROR(_ina.getShuntVoltage_mV(power));
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
    state &= 0xFFFFFu; //Mask to 20 bits
    return disconnect();
}

uint8_t TESDriver::setCurrent_mA(float target_mA, uint32_t* finalState, float* finalMeasured,
                             float tolerance_mA, int maxIter, bool increasing, int delayMs) {
    // Sanity: expected current range 0..20 mA
    if (!(target_mA >= 0.0f && target_mA <= 20.0f)) {
        return false;
    }

    // Input domain: 0 .. 2^24-1 (24-bit outputs)
    const uint32_t max20 = 0xFFFFFu;
    // Apply function: receive float input (search uses float), round/clamp to 24-bit and apply
    auto apply = [this, max20](float v) {
        // clamp and round
        this->connect();
        if (v <= 0.0f) {
            this->setAllOutputPins(0u);
            return;
        }
        // round to nearest integer
        uint32_t s = (uint32_t) (v + 0.5f);
        if (s > max20) s = max20;
        this->setAllOutputPins(s);
        this->disconnect();
    };

    // Read function: return current in mA
    auto read = [this]() -> float {
        float _;
        this->getCurrent_mA(_);
        return _;
    };

    // Create binary search controller over input domain mapped to 0..max20 and output 0..20 mA
    BinarySearchController<decltype(apply), decltype(read)> bs(
        apply, read,
        0.0f, (float)max20,
        0.0f, 20.0f,
        tolerance_mA,
        maxIter,
        increasing
    );

    // Set a small settle delay so hardware can stabilize after changing outputs.
    // Uses a non-capturing lambda convertible to a function pointer.
    // Use a static variable so the lambda remains non-capturing (convertible to function pointer)
    static int s_settleDelayMs = 0;
    s_settleDelayMs = delayMs;
    bs.setSettleCallback([](){ delay(s_settleDelayMs); });

    float finalInput = 0.0f;
    float finalOutput = 0.0f;
    bool ok = bs.setTarget(target_mA, &finalInput, &finalOutput);

    // convert finalInput to uint32_t state and return values
    uint32_t finalStateLocal = 0u;
    if (finalInput > 0.0f) {
        finalStateLocal = (uint32_t)(finalInput + 0.5f);
        if (finalStateLocal > max20) finalStateLocal = max20;
    }
    if (finalState) *finalState = finalStateLocal;
    if (finalMeasured) *finalMeasured = finalOutput;

    return ok;
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