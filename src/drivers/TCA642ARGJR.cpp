#include "TCA642ARGJR.h"

TCA642ARGJR::TCA642ARGJR(uint8_t address) {
    _address = address;
    _router = nullptr;
    _route = nullptr;
}

TCA642ARGJR::TCA642ARGJR(uint8_t address, Router* router, I2CRoute* route):
    _address(address), _router(router), _route(route) {
    }

void TCA642ARGJR::begin() {
    Wire.begin();
    // By default configure all pins as outputs (0) and clear outputs.
    uint8_t config[3] = {0x00, 0x00, 0x00};
    writeRegisters(TCA642ARGJR_CONFIG_PORT0, config, 3);

    uint8_t outputs[3] = {0xFF, 0xFF, 0xFF};
    writeRegisters(TCA642ARGJR_OUTPUT_PORT0, outputs, 3);
}

void TCA642ARGJR::setOutputPin(uint8_t pin, bool state) {
    if (pin >= 24) return;
    uint8_t port = pin / 8;
    uint8_t bit = pin % 8;
    uint8_t reg = TCA642ARGJR_OUTPUT_PORT0 + port;
    uint8_t currentValue = readRegister(reg);

    if (state) currentValue |= (1 << bit);
    else currentValue &= ~(1 << bit);

    writeRegister(reg, currentValue);
}

bool TCA642ARGJR::getOutputPin(uint8_t pin) {
    if (pin >= 24) return false;
    uint8_t port = pin / 8;
    uint8_t bit = pin % 8;
    uint8_t reg = TCA642ARGJR_OUTPUT_PORT0 + port;
    uint8_t currentValue = readRegister(reg);
    return ((currentValue >> bit) & 0x01) != 0;
}

void TCA642ARGJR::setAllOutputPins(uint32_t state) {
    uint8_t out[3];
    out[0] = state & 0xFF;
    out[1] = (state >> 8) & 0xFF;
    out[2] = (state >> 16) & 0xFF;
    writeRegisters(TCA642ARGJR_OUTPUT_PORT0, out, 3);
}

uint32_t TCA642ARGJR::getAllOutputPins() {
    uint8_t out[3];
    readRegisters(TCA642ARGJR_OUTPUT_PORT0, out, 3);
    return ((uint32_t)out[2] << 16) | ((uint32_t)out[1] << 8) | out[0];
}

void TCA642ARGJR::writeRegister(uint8_t reg, uint8_t value) {
    if (_router != nullptr && _route != nullptr) {
        _router->routeTo(_route);
    }
    Wire.beginTransmission(_address);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();

    if (_router != nullptr && _route != nullptr) {
        _router->endRoute(_route);
    }
}

uint8_t TCA642ARGJR::readRegister(uint8_t reg) {
    if (_router != nullptr && _route != nullptr) {
        _router->routeTo(_route);
    }

    uint8_t value = 0;
    Wire.beginTransmission(_address);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom(_address, (uint8_t)1);
    if (Wire.available()) value = Wire.read();

    if (_router != nullptr && _route != nullptr) {
        _router->endRoute(_route);
    }
    return value;
}

void TCA642ARGJR::writeRegisters(uint8_t startReg, const uint8_t* data, size_t length) {
    if (_router != nullptr && _route != nullptr) {
        _router->routeTo(_route);
    }

    for (size_t i = 0; i < length; ++i) {
        Wire.beginTransmission(_address);
        Wire.write(startReg + (uint8_t)i);
        Wire.write(data[i]);
        Wire.endTransmission();
    }

    if (_router != nullptr && _route != nullptr) {
        _router->endRoute(_route);
    }
}

void TCA642ARGJR::readRegisters(uint8_t startReg, uint8_t* data, size_t length) {
    if (_router != nullptr && _route != nullptr) {
        _router->routeTo(_route);
    }
    size_t i = 0;

    for (i; i < length; ) {
        Wire.beginTransmission(_address);
        Wire.write(startReg + i);
        Wire.endTransmission();
        Wire.requestFrom(_address, (uint8_t)1);
        if (Wire.available()) data[i++] = Wire.read();
    }
    // Diagnostic print for read status

    if (_router != nullptr && _route != nullptr) {
        _router->endRoute(_route);
    }
}

// Print router for debugging
void TCA642ARGJR::printRoute() {
    if (_router == nullptr || _route == nullptr) {
        Serial.println("No routing information available.");
        return;
    }
    Serial.print("Routing to TCA642ARGJR at 0x");
    Serial.print(_address, HEX);
    Serial.print(" via route: ");
    I2CRoute* current = _route;
    while (current != nullptr) {
        if (current->hub != nullptr) {
            Serial.print(" -> 0x");
            Serial.print(current->hub->get_i2cAddress(), HEX);
        }
        current = current->next;
    }
    Serial.println();
}