#ifndef TCA642ARGJR_H
#define TCA642ARGJR_H

#include <Arduino.h>
#include <Wire.h>
#include "../routers/Router.h"
#include "../helpers/error.h"

// Default 7-bit I2C base address used by project (TESDriver overrides to 0x22)
#define TCA642ARGJR_ADDRESS 0x22

// TCA6424 register map (3 ports of 8 bits = 24 GPIOs)
// Input port registers
#define TCA642ARGJR_INPUT_PORT0 0x00
#define TCA642ARGJR_INPUT_PORT1 0x01
#define TCA642ARGJR_INPUT_PORT2 0x02
// Output port registers
#define TCA642ARGJR_OUTPUT_PORT0 0x04
#define TCA642ARGJR_OUTPUT_PORT1 0x05
#define TCA642ARGJR_OUTPUT_PORT2 0x06
// Polarity inversion registers
#define TCA642ARGJR_POLARITY_INV_PORT0 0x08
#define TCA642ARGJR_POLARITY_INV_PORT1 0x09
#define TCA642ARGJR_POLARITY_INV_PORT2 0x0A
// Configuration registers (1 = input, 0 = output)
#define TCA642ARGJR_CONFIG_PORT0 0x0C
#define TCA642ARGJR_CONFIG_PORT1 0x0D
#define TCA642ARGJR_CONFIG_PORT2 0x0E

class TCA642ARGJR {
public:
    TCA642ARGJR(uint8_t address = TCA642ARGJR_ADDRESS);

    uint8_t begin();
    // Basic helpers
    uint8_t setOutputPin(uint8_t pin, bool state);
    uint8_t getOutputPin(uint8_t pin, bool& state);
    uint8_t setAllOutputPins(uint32_t state); // 24-bit value (lower 24 bits used)
    uint8_t getAllOutputPins(uint32_t& state);  // 24-bit value (lower 24 bits used)

private:
    uint8_t _address;

    uint8_t writeRegister(uint8_t reg, uint8_t value);
    uint8_t readRegister(uint8_t reg, uint8_t& value);
    uint8_t writeRegisters(uint8_t startReg, const uint8_t* data, size_t length);
    uint8_t readRegisters(uint8_t startReg, uint8_t* data, size_t length);
};

#endif // TCA642ARGJR_H