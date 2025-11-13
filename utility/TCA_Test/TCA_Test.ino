#include <Wire.h>

#define TCA_ADDR 0x22   // TCA6424A address
#define LTC_ADDR 0x63   // LTC4302 hub address

// Register bases
#define REG_INPUT0   0x00
#define REG_OUTPUT0  0x04
#define REG_POL0     0x08
#define REG_CONFIG0  0x0C

// -------------------------------------------------------------
//  ROUTING CONTROL (LTC4302 + upstream router at 0x7E)
// -------------------------------------------------------------
void startComms() {
    // Upstream router enable (might also need manual mode)
    Wire.beginTransmission(0x7E);
    Wire.write(0b01000000);  // FORCE enable
    Wire.endTransmission();

    // LTC4302 manual enable
    Wire.beginTransmission(LTC_ADDR);
    Wire.write(0b01000000);  // FORCE enable
    Wire.endTransmission();
}

void stopComms() {
    // Disable LTC4302 channel
    Wire.beginTransmission(LTC_ADDR);
    Wire.write(0b00000000);
    Wire.endTransmission();

    // Disable upstream router
    Wire.beginTransmission(0x7E);
    Wire.write(0b00000000);
    Wire.endTransmission();
}

// -------------------------------------------------------------
//  LOW-LEVEL TCA READ/WRITE WITH ROUTING
// -------------------------------------------------------------
void writeRegisters(uint8_t startReg, const uint8_t* data, size_t len) {
    startComms();

    Wire.beginTransmission(TCA_ADDR);
    Wire.write(startReg);
    for (size_t i = 0; i < len; i++) {
        Wire.write(data[i]);
    }
    Wire.endTransmission();

    stopComms();
}

void readRegisters(uint8_t startReg, uint8_t* buf, size_t len) {
    startComms();

    Wire.beginTransmission(TCA_ADDR);
    Wire.write(startReg);
    Wire.endTransmission(false);  // repeated start

    Wire.requestFrom(TCA_ADDR, (uint8_t)len);
    for (size_t i = 0; i < len; i++) {
        buf[i] = Wire.read();
    }

    stopComms();
}

// -------------------------------------------------------------
//  SIMPLE LED TEST
// -------------------------------------------------------------
void setup() {
    Serial.begin(115200);
    Wire.begin();
    delay(100);

    Serial.println("\nTCA6424A LED + LTC4302 Routing Test");

    // Make all pins outputs (config = 0)
    uint8_t cfg[3] = {0x00, 0x00, 0x00};
    writeRegisters(REG_CONFIG0, cfg, 3);
    delay(5);
    writeRegisters(REG_CONFIG0, cfg, 3);   // double-write for safety
}

void loop() {
    uint8_t outs[3];

    // ---------------------------
    // ONLY PORT 0 ON
    // ---------------------------
    outs[0] = 0x00;  // LED ON (active-low)
    outs[1] = 0xFF;  // LED OFF
    outs[2] = 0xFF;
    writeRegisters(REG_OUTPUT0, outs, 3);
    Serial.println("PORT 0 should be lit");
    delay(1000);

    // ---------------------------
    // ONLY PORT 1 ON
    // ---------------------------
    outs[0] = 0xFF;
    outs[1] = 0x00;
    outs[2] = 0xFF;
    writeRegisters(REG_OUTPUT0, outs, 3);
    Serial.println("PORT 1 should be lit");
    delay(1000);

    // ---------------------------
    // ONLY PORT 2 ON
    // ---------------------------
    outs[0] = 0xFF;
    outs[1] = 0xFF;
    outs[2] = 0x00;
    writeRegisters(REG_OUTPUT0, outs, 3);
    Serial.println("PORT 2 should be lit");
    delay(1000);
}
