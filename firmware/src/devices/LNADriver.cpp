#include "LNADriver.h"

// The LTC4302 acts as an I2C repeater, not a multiplexer with channels.
// Therefore, internal channel definitions for devices behind it are not needed
// for routing. The MCP4728 still has its own internal channels (A, B, C, D) for
// DAC selection.

LNADriver::LNADriver(LTC4302* lnaLtc4302, Router* router)
    : _lnaLtc4302(lnaLtc4302),
      _router(router),
      _routeToLnaLtc4302({_lnaLtc4302, nullptr}),
      _lnaDac(LNA_MCP4728_ADDR),
      _lnaInaDrain(LNA_INA_DRAIN_ADDR),
      _lnaInaGate(LNA_INA_GATE_ADDR) {
}

uint8_t LNADriver::begin() {
    RETURN_IF_ERROR(_lnaLtc4302->begin());
    RETURN_IF_ERROR(connect());
    RETURN_IF_ERROR(_lnaDac.begin());
    RETURN_IF_ERROR(_lnaInaDrain.begin(LNA_INA_SHUNT_RESISTANCE_OHMS,
                       LNA_INA_MAX_EXPECTED_CURRENT_AMPS));
    RETURN_IF_ERROR(_lnaInaGate.begin(LNA_INA_SHUNT_RESISTANCE_OHMS,
                      LNA_INA_MAX_EXPECTED_CURRENT_AMPS));
    return disconnect();
}

// Methods to interact with the LNA's MCP4728
uint8_t LNADriver::writeDrain(uint16_t value) {
    RETURN_IF_ERROR(connect());
    RETURN_IF_ERROR(_lnaDac.writeDAC(LNA_DRAIN_CHANNEL, value));  // Channel A controls Drain
    return disconnect();
}
uint8_t LNADriver::writeGate(uint16_t value) {
    RETURN_IF_ERROR(connect());
    RETURN_IF_ERROR(_lnaDac.writeDAC(LNA_DRAIN_CHANNEL, value));  // Channel A controls Drain
    return disconnect();
}

uint8_t LNADriver::readDrain(uint16_t& value) { 
    RETURN_IF_ERROR(connect());
    RETURN_IF_ERROR(_lnaDac.readDAC(LNA_DRAIN_CHANNEL, value));
    return disconnect();
}

uint8_t LNADriver::readGate(uint16_t& value) {
    RETURN_IF_ERROR(connect());
    RETURN_IF_ERROR(_lnaDac.readDAC(LNA_GATE_CHANNEL, value));
    return disconnect();
}

uint8_t LNADriver::setDrainCurrent(float& target_mA, uint16_t& dacValue, uint8_t delayMs) {
    if (!(target_mA >= 0.0f && target_mA <= 64.0f)) {
        return 10; // invalid argument
    }
    RETURN_IF_ERROR(connect());
    float readCurrent = 0;
    dacValue = 0;
    while(readCurrent < target_mA && dacValue < 4095) {
        dacValue++;
        RETURN_IF_ERROR(_lnaDac.writeDAC(LNA_DRAIN_CHANNEL, dacValue));
        delay(delayMs); // Allow time for settling
        RETURN_IF_ERROR(_lnaInaDrain.getCurrent_mA(readCurrent));
    }
    dacValue = (dacValue > 0 && dacValue < 4095) ? dacValue - 1 : 0;
    RETURN_IF_ERROR(_lnaDac.writeDAC(LNA_DRAIN_CHANNEL, dacValue));
    RETURN_IF_ERROR(_lnaInaDrain.getCurrent_mA(target_mA));
    return disconnect();
}

uint8_t LNADriver::setGateCurrent(float& target_mA, uint16_t& dacValue, uint8_t delayMs) {
    if (!(target_mA >= 0.0f && target_mA <= 64.0f)) {
        return 10; // invalid argument
    }
    target_mA = -target_mA; // Gate current is negative
    RETURN_IF_ERROR(connect());
    float readCurrent = 0;
    dacValue = 0;
    while(readCurrent > target_mA && dacValue < 4095) {
        dacValue++;
        RETURN_IF_ERROR(_lnaDac.writeDAC(LNA_GATE_CHANNEL, dacValue));
        delay(delayMs); // Allow time for settling
        RETURN_IF_ERROR(_lnaInaGate.getCurrent_mA(readCurrent));
    }
    dacValue = (dacValue > 0 && dacValue < 4095) ? dacValue - 1 : 0;
    RETURN_IF_ERROR(_lnaDac.writeDAC(LNA_GATE_CHANNEL, dacValue));
    RETURN_IF_ERROR(_lnaInaGate.getCurrent_mA(target_mA));
    return disconnect();
}

uint8_t LNADriver::setDrainVoltage(float& target_V, uint16_t& dacValue, uint8_t delayMs) {
    if (!(target_V >= 0.0f && target_V <= 5.0f)) {
        return 10; // invalid argument
    }
    RETURN_IF_ERROR(connect());
    float readVoltage = 0;
    dacValue = 0;
    while(readVoltage < target_V && dacValue < 4095) {
        dacValue++;
        RETURN_IF_ERROR(_lnaDac.writeDAC(LNA_DRAIN_CHANNEL, dacValue));
        delay(delayMs); // Allow time for settling
        RETURN_IF_ERROR(_lnaInaDrain.getBusVoltage_V(readVoltage));
    }
    dacValue = (dacValue > 0 && dacValue < 4095) ? dacValue - 1 : 0;
    RETURN_IF_ERROR(_lnaDac.writeDAC(LNA_DRAIN_CHANNEL, dacValue));
    RETURN_IF_ERROR(_lnaInaDrain.getBusVoltage_V(target_V));
    return disconnect();
}

uint8_t LNADriver::setGateVoltage(float& target_V, uint16_t& dacValue, uint8_t delayMs) {
    if (!(target_V >= 0.0f && target_V <= 5.0f)) {
        return 10; // invalid argument
    }
    RETURN_IF_ERROR(connect());
    float readVoltage = 0;
    dacValue = 0;
    while(readVoltage < target_V && dacValue < 4095) {
        dacValue++;
        RETURN_IF_ERROR(_lnaDac.writeDAC(LNA_GATE_CHANNEL, dacValue));
        delay(delayMs); // Allow time for settling
        RETURN_IF_ERROR(_lnaInaGate.getBusVoltage_V(readVoltage));
    }
    dacValue = (dacValue > 0 && dacValue < 4095) ? dacValue - 1 : 0;
    RETURN_IF_ERROR(_lnaDac.writeDAC(LNA_GATE_CHANNEL, dacValue));
    RETURN_IF_ERROR(_lnaInaGate.getBusVoltage_V(target_V));
    target_V = -target_V; // Gate voltage is negative
    return disconnect();
}

uint8_t LNADriver::getDrainShuntVoltage_mV(float& shuntVoltage) {
    RETURN_IF_ERROR(connect());
    RETURN_IF_ERROR(_lnaInaDrain.getShuntVoltage_mV(shuntVoltage));
    return disconnect();
}

uint8_t LNADriver::getDrainBusVoltage_V(float& busVoltage) {
    RETURN_IF_ERROR(connect());
    RETURN_IF_ERROR(_lnaInaDrain.getBusVoltage_V(busVoltage));
    return disconnect();
}

uint8_t LNADriver::getDrainCurrent_mA(float& current) {
    RETURN_IF_ERROR(connect());
    RETURN_IF_ERROR(_lnaInaDrain.getCurrent_mA(current));
    return disconnect();
}

uint8_t LNADriver::getDrainPower_mW(float& power) {
    RETURN_IF_ERROR(connect());
    RETURN_IF_ERROR(_lnaInaDrain.getPower_mW(power));
    return disconnect();
}

uint8_t LNADriver::getGateShuntVoltage_mV(float& shuntVoltage) {
    RETURN_IF_ERROR(connect());
    RETURN_IF_ERROR(_lnaInaGate.getShuntVoltage_mV(shuntVoltage));
    return disconnect();
}

uint8_t LNADriver::getGateBusVoltage_V(float& busVoltage) {
    RETURN_IF_ERROR(connect());
    RETURN_IF_ERROR(_lnaInaGate.getBusVoltage_V(busVoltage));
    busVoltage = -busVoltage; // Drain voltage is negative
    return disconnect();
}

uint8_t LNADriver::getGateCurrent_mA(float& current) {
    RETURN_IF_ERROR(connect());
    RETURN_IF_ERROR(_lnaInaGate.getCurrent_mA(current));
    return disconnect();
}

uint8_t LNADriver::getGatePower_mW(float& power) {
    RETURN_IF_ERROR(connect());
    RETURN_IF_ERROR(_lnaInaGate.getPower_mW(power));
    return disconnect();
}

uint8_t LNADriver::setGateEnable(
    bool state) {    // False is enable, true is disable
    state = !state;  // Invert state for LTC4302 GPIO logic
    RETURN_IF_ERROR(connect());
    RETURN_IF_ERROR(_lnaLtc4302->setGPIO(1, state));
    return disconnect();
}

uint8_t LNADriver::setDrainEnable(bool state) {
    state = !state;  // Invert state for LTC4302 GPIO logic
    RETURN_IF_ERROR(connect());
    RETURN_IF_ERROR(_lnaLtc4302->setGPIO(2, state));
    return disconnect();
}

uint8_t LNADriver::getGateEnable(bool& state) {
    RETURN_IF_ERROR(connect());
    RETURN_IF_ERROR(_lnaLtc4302->getGPIO(1, state));
    state = !state;  // Invert logic for return value
    return disconnect();
}

uint8_t LNADriver::getDrainEnable(bool& state) {
    RETURN_IF_ERROR(connect());
    RETURN_IF_ERROR(_lnaLtc4302->getGPIO(2, state));
    state = !state;  // Invert logic for return value
    return disconnect();
}

uint8_t LNADriver::connect() { return _router->routeTo(&_routeToLnaLtc4302); }

uint8_t LNADriver::disconnect() {
    return _router->endRoute(&_routeToLnaLtc4302);
}
