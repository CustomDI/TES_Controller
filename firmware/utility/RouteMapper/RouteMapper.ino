#include <StaticSerialCommands.h>

#include "src/drivers/LTC4302.h"
#include "src/routers/Router.h"
#include "src/devices/TESDriver.h"

// Simple utility sketch to walk known device routes and print a network map.

// Configure device counts and addresses here for the system under test.
#define NUM_TES 1
const uint8_t DEFAULT_TES_ADDRESSES[NUM_TES] = { 0x6D };

// Base hub address (match your hardware)
#define BASE_HUB_LTC4302_ADDR 0x7E

// Global objects used by the utility
LTC4302 baseHub(BASE_HUB_LTC4302_ADDR);
Router router(&baseHub);

LTC4302* tesLTC[NUM_TES];
TESDriver* tesDriver[NUM_TES];

// Helper to print a route path
void printRoutePath(I2CRoute* route) {
    Serial.print("Route path: base");
    I2CRoute* cur = route;
    while (cur != nullptr) {
        if (cur->hub != nullptr) {
            Serial.print(" -> 0x");
            if (cur->hub->get_i2cAddress() < 16) Serial.print("0");
            Serial.print(cur->hub->get_i2cAddress(), HEX);
        }
        cur = cur->next;
    }
    Serial.println();
}

// Walk all known routes and produce a simple map
void mapNetwork() {
    Serial.println("--- Network map start ---");

    // Print base hub and scan base bus
    Serial.print("Base hub: 0x");
    Serial.println(baseHub.get_i2cAddress(), HEX);

    Serial.println("Scanning base bus for devices...");
    router.scanDevicesAtEndpoint();

    // Walk TES drivers
    for (int i = 0; i < NUM_TES; ++i) {
        if (tesDriver[i] == nullptr) continue;
        I2CRoute route = tesDriver[i]->getRouteToTesLtc4302();
        Serial.print("TES #"); Serial.print(i); Serial.println(":");
        // Build a temporary linked-list pointer
        I2CRoute* routePtr = &route;
        printRoutePath(routePtr);

        // Route to the endpoint and scan
        Serial.println("Scanning endpoint for this TES route...");
        router.scanDevicesAtEndpoint(routePtr);
        Serial.println();
    }

    Serial.println("--- Network map end ---");
}

void initDeviceArrays() {
    for (int i = 0; i < NUM_TES; ++i) {
        uint8_t addr = DEFAULT_TES_ADDRESSES[i];
        tesLTC[i] = new LTC4302(addr);
        tesDriver[i] = new TESDriver(tesLTC[i], &router);
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("RouteMapper starting...");

    initDeviceArrays();

    uint8_t status = router.begin();
    if (status) {
        Serial.println("Error initializing router/base hub");
    }

    // Enable base bus briefly for discovery
    status = baseHub.enableBus();
    if (status) {
        Serial.println("Error enabling base bus");
    }

    // Optional: initialize drivers so their internal helpers are available
    for (int i = 0; i < NUM_TES; ++i) {
        if (tesDriver[i]) {
            status = tesDriver[i]->begin();
            if (status) {
                Serial.print("Warning: TES driver "); Serial.print(i); Serial.println(" begin() failed (continuing)");
            }
            // Always disconnect after begin (TESDriver::begin disconnects at end)
        }
    }

    // Produce the map
    mapNetwork();

    // Disable base bus when done
    baseHub.disableBus();

    Serial.println("RouteMapper complete.\n");
}

void loop() {
    // idle - utility runs once in setup
}
