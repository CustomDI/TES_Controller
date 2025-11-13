#ifndef ROUTER_H
#define ROUTER_H

#include <Arduino.h>
#include "../drivers/LTC4302.h"

// Define a structure to represent a route to a device
struct I2CRoute {
    LTC4302* hub;       // Pointer to the LTC4302 hub at this level
    I2CRoute* next;     // Pointer to the next hop in the route (for cascaded hubs)
};

class Router {
public:
    Router(LTC4302* baseHub);
    void begin();
    void routeTo(I2CRoute* route);
    void endRoute(I2CRoute* route);
    void scanDevicesAtEndpoint(I2CRoute* route); // New method to scan devices at the endpoint of a route
    LTC4302* get_baseHub() { return _baseHub; }
private:
    LTC4302* _baseHub;
};

#endif // ROUTER_H
