#ifndef TES_DRIVER_H
#define TES_DRIVER_H

#include <Arduino.h>
#include "../routers/Router.h"
#include "../drivers/LTC4302.h"
#include "../drivers/INA219.h"
#include "../drivers/TCA642ARGJR.h"


#define TES_INA_ADDR    0x40 // Address for INA219 behind TES driver
#define TES_TCA_ADDR    0x22 // Address for TCA642ARGJR behind TES driver

class TESDriver {
public:
    TESDriver(LTC4302* tesLtc4302, Router* router); // Removed baseHubChannel
    uint8_t begin();
    I2CRoute getRouteToTesLtc4302() { return _routeToTesLtc4302; } // Accessor for the route

    // GPIO functionality at LTC4302
    uint8_t setOutEnable(bool state);
    uint8_t getOutEnable(bool& state);

    // INA functionality
    uint8_t getShuntVoltage_mV(float& shuntVoltage);
    uint8_t getBusVoltage_V(float& busVoltage);
    uint8_t getCurrent_mA(float& current);
    uint8_t getPower_mW(float& power);

    uint8_t setCurrent_mA(float target_mA, uint32_t* finalState = nullptr, float* finalMeasured = nullptr,
                       float tolerance_mA = 0.005f, int maxIter = 24, bool increasing = false, int delayMs = 10);

    // TCA functionality
    uint8_t setOutputPin(uint8_t pin, bool state);
    uint8_t getOutputPin(uint8_t pin, bool& state);
    uint8_t setAllOutputPins(uint32_t state); // now 24-bit capable
    uint8_t getAllOutputPins(uint32_t &state);  // now 24-bit capable
    uint8_t bumpOutputPins(int8_t delta); // Adjust output pins by delta (signed)

private:
    LTC4302* _tesLtc4302; // Pointer to the TES driver's LTC4302 instance
    Router* _router;

    // Route for the TES LTC4302 itself
    I2CRoute _routeToTesLtc4302;

    // Placeholder for TES device routes (e.g., if multiple devices are behind this LTC)
    // For 12 devices, these would likely be an array or a more complex structure.
    // For now, we'll keep it simple, assuming direct interaction through the LTC.

public:
    // Expose low-level drivers for diagnostics (e.g., startup prints in TES_Controller.ino)
    TCA642ARGJR _tca;
    INA219 _ina;

    uint8_t connect();
    uint8_t disconnect();
};

#endif // TES_DRIVER_H
