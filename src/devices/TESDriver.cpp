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
      _tca(TES_TCA_ADDR, router, &_routeToTesLtc4302),
      _ina(TES_INA_ADDR){}

void TESDriver::begin() {
    _tesLtc4302->begin(); // Call begin on the pointer
    connect();
    _ina.begin(); // Initialize INA219
    disconnect();
    _tca.begin(); // Initialize TCA642ARGJR
}

void TESDriver::setOutEnable(bool state) {
    state = !state; // Invert logic: HIGH = disable, LOW = enable
    _router->routeTo(&_routeToTesLtc4302); // This will route to the TES LTC4302
    _tesLtc4302->setGPIO(2, state); // GPIO2 controls OUT_EN
    _tesLtc4302->setGPIO(1, state); // GPIO1 controls OUT_EN
    _router->endRoute(&_routeToTesLtc4302); // End routing
}

bool TESDriver::getOutEnable() { 
    bool state;
    _router->routeTo(&_routeToTesLtc4302); // This will route to the TES LTC4302
    _tesLtc4302->getGPIO(2, state); // GPIO2 controls OUT_EN
    _router->endRoute(&_routeToTesLtc4302); // End routing
    return !state; // Invert logic for return value
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
    uint32_t value = _tca.getAllOutputPins();
    value &= 0xFFFFFu; //Mask to 20 bits
    return value;
}

void TESDriver::readOutputRegisters(uint8_t out[3]) {
    // Directly read the TCA's output registers via the TCA driver
    _tca.readRegisters( (uint8_t)TCA642ARGJR_OUTPUT_PORT0, out, 3 );
}

bool TESDriver::setCurrent_mA(float target_mA, uint32_t* finalState, float* finalMeasured,
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
        if (v <= 0.0f) {
            this->setAllOutputPins(0u);
            return;
        }
        // round to nearest integer
        uint32_t s = (uint32_t) (v + 0.5f);
        if (s > max20) s = max20;
        this->setAllOutputPins(s);
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

void TESDriver::bumpOutputPins(int8_t delta) {
    uint32_t currentState = getAllOutputPins();
    int32_t newState = (int32_t)currentState + (int32_t)delta;
    if (newState < 0) newState = 0;
    if (newState > 0xFFFFFu) newState = 0xFFFFFu; // Clamp to 20 bits
    setAllOutputPins((uint32_t)newState);
}

uint8_t TESDriver::connect() {
    return _router->routeTo(&_routeToTesLtc4302);
}

uint8_t TESDriver::disconnect() {
    return _router->endRoute(&_routeToTesLtc4302);
}