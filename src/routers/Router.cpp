#include "Router.h"

Router::Router(LTC4302* baseHub) : _baseHub(baseHub) {}

void Router::begin() {
    // The base hub should already be initialized in setup, but we can ensure it here.
    _baseHub->begin();
}

void Router::routeTo(I2CRoute* route) {
    I2CRoute* current = route;
    while (current != nullptr) {
        if (current->hub != nullptr) {
            current->hub->enableBus(); // Enable the bus for the current hub
        }
        current = current->next;
    }
}

void Router::scanDevicesAtEndpoint(I2CRoute* route) {
    Serial.print("Scanning I2C devices at endpoint of route: ");
    I2CRoute* current = route;
    while (current != nullptr) {
        if (current->hub != nullptr) {
            Serial.print(" -> 0x");
            Serial.print(current->hub->get_i2cAddress(), HEX);
        }
        current = current->next;
    }
    Serial.println();

    // Route to the endpoint
    routeTo(route);

    Serial.println("I2C Scanner found devices at:");
    uint8_t count = 0;
    for (uint8_t addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        uint8_t error = Wire.endTransmission();

        if (error == 0) {
            Serial.print("  I2C device found at address 0x");
            if (addr < 16) {
                Serial.print("0");
            }
            Serial.print(addr, HEX);
            Serial.println(" !");
            count++;
        } else if (error == 4) {
            Serial.print("  Unknown error at address 0x");
            if (addr < 16) {
                Serial.print("0");
            }
            Serial.println(addr, HEX);
        }
    }
    if (count == 0) {
        Serial.println("  No I2C devices found.");
    }

    // End the route
    endRoute(route);
}

void Router::endRoute(I2CRoute* route) {
    if (route == nullptr) return;

    // Recurse to the end of the chain first
    endRoute(route->next);
    // Then disable this hub on the way back
    if (route->hub != nullptr) {
        route->hub->disableBus();
    }
}
