#ifndef TES_DRIVER_H
#define TES_DRIVER_H

#include <Arduino.h>
#include "../routers/Router.h"
#include "../drivers/LTC4302.h" // Include LTC4302 for the internal instance

class TESDriver {
public:
    TESDriver(LTC4302* tesLtc4302, Router* router); // Removed baseHubChannel
    void begin();

    // Placeholder methods for interacting with TES devices
    // These will be implemented once the specific TES device drivers are known.
    void writeTesDevice(uint16_t value); // Removed deviceChannel
    uint16_t readTesDevice(); // Removed deviceChannel
    I2CRoute getRouteToTesLtc4302() { return _routeToTesLtc4302; } // Accessor for the route

    // GPIO functionality at LTC4302
    void setOutEnable(bool state);

private:
    LTC4302* _tesLtc4302; // Pointer to the TES driver's LTC4302 instance
    Router* _router;

    // Route for the TES LTC4302 itself
    I2CRoute _routeToTesLtc4302;

    // Placeholder for TES device routes (e.g., if multiple devices are behind this LTC)
    // For 12 devices, these would likely be an array or a more complex structure.
    // For now, we'll keep it simple, assuming direct interaction through the LTC.
};

#endif // TES_DRIVER_H
