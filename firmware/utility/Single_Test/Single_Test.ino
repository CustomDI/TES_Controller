#include <Wire.h>
#include <Arduino.h>

int status;
uint8_t regValue;

uint8_t readRegister(uint8_t address, uint8_t reg, uint8_t& value) {
    uint8_t status;
    Wire.beginTransmission(address);
    Wire.write(reg);
    status = Wire.endTransmission(false); // Send restart
    if (status != 0) {
        return status;
    }
    Wire.requestFrom(address, (uint8_t)1);
    if (Wire.available()) {
        value = Wire.read();
    } else {
        return 10;
    }
    return 0;
}

uint8_t writeRegister(uint8_t address, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(value);
    return Wire.endTransmission();
}

uint8_t enableBus(uint8_t address) {
    // Set bit 7 of register 0x01 to enable the bus
    uint8_t regValue;
    uint8_t status = readRegister(address, 0x01, regValue);
    if (status != 0) {
        return status;
    }
    regValue |= (1 << 7);
    return writeRegister(address, regValue);
}

uint8_t disableBus(uint8_t address) {
    // Clear bit 7 of register 0x01 to disable the bus
    uint8_t regValue;
    uint8_t status = readRegister(address, 0x01, regValue);
    if (status != 0) {
        return status;
    }
    regValue &= ~(1 << 7);
    return writeRegister(address, regValue);
}

uint8_t printAvailableDevices() {
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
    return count;
}

void setup() {
    Serial.begin(115200);
    Wire.begin();
}

void loop() {
    status = enableBus(0x7E);
    if (status) {
        Serial.println("Error enabling bus on LTC4302 at 0x7E status = " + String(status));
    } else {
        Serial.println("Bus enabled on LTC4302 at 0x7E status = " + String(status));
    }
    
    delay(2000);

    printAvailableDevices();
    
    delay(2000);

    status = enableBus(0x6D);
    if (status) {
        Serial.println("Error enabling bus on LTC4302 at 0x6D status = " + String(status));
    } else {
        Serial.println("Bus enabled on LTC4302 at 0x6D status =" + String(status));
    }

    delay(2000); // Wait for 2 seconds

    status = disableBus(0x6D);
    if (status) {
        Serial.println("Error disabling bus on LTC4302 at 0x6D status = " + String(status));
    } else {
        Serial.println("Bus disabled on LTC4302 at 0x6D status =" + String(status));
    }

    delay(2000);

    status = disableBus(0x7E);
    if (status) {
        Serial.println("Error disabling bus on LTC4302 at 0x7E status = " + String(status));
    } else {
        Serial.println("Bus disabled on LTC4302 at 0x7E status =" + String(status));
    }
    delay(5000);
}