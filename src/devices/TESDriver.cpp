#include "TESDriver.h"

// Each TESDriver manages a single TES device, which is behind its own LTC4302.
// Therefore, the concept of "channels on the TES LTC4302 for its devices" is simplified.
// The TES device itself is directly accessed through the LTC.


TESDriver::TESDriver(LTC4302* tesLtc4302, Router* router)
    : _tesLtc4302(tesLtc4302), _router(router),
      // Initialize the route to the TES LTC4302 itself
      _routeToTesLtc4302({_tesLtc4302, nullptr})
{
    // The Router::routeTo() method will dynamically build the full path based on the 'next' pointers
    // within the I2CRoute structure.
    // For the TES devices, the route starts at the base hub, goes to the TES LTC4302.
}

void TESDriver::begin() {
    _tesLtc4302->begin(); // Call begin on the pointer
    // Additional initialization for TES devices if needed
}

// Placeholder methods for interacting with TES devices
void TESDriver::writeTesDevice(uint16_t value) {
    // Route to the specific TES LTC4302
    _router->routeTo(&_routeToTesLtc4302); // This will route to the TES LTC4302
    _tesLtc4302->enableBus(); // Enable the bus for the TES device (assuming it's directly on the LTC bus)

    // Placeholder for actual I2C write to the TES device
    // This would involve knowing the TES device's address and command structure.
    Serial.print("TESDriver: Writing ");
    Serial.print(value);
    Serial.println(" to TES device.");

    _tesLtc4302->disableBus(); // Disable the bus after communication
    _router->endRoute(&_routeToTesLtc4302); // End routing
}

void TESDriver::setOutEnable(bool state) {
    _router->routeTo(&_routeToTesLtc4302); // This will route to the TES LTC4302
    _tesLtc4302->setGPIO(2, state); // GPIO2 controls OUT_EN
    _tesLtc4302->setGPIO(1, state); // GPIO1 controls OUT_EN
    _router->endRoute(&_routeToTesLtc4302); // End routing
}

uint16_t TESDriver::readTesDevice() {
    // Route to the specific TES LTC4302
    _router->routeTo(&_routeToTesLtc4302);
    _tesLtc4302->enableBus();

    // Placeholder for actual I2C read from the TES device
    // This would involve knowing the TES device's address and command structure.
    Serial.println("TESDriver: Reading from TES device.");

    _tesLtc4302->disableBus();
    _router->endRoute(&_routeToTesLtc4302);
    return 0; // Placeholder return value
}
