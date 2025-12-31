#include "drivers/INA228.h"
// Ensure the driver implementation is compiled with this sketch (resolves linker errors
// when the build system doesn't automatically compile .cpp files in subfolders).
#include "drivers/INA228.cpp"

#define INA_ADDR 0x40
#define SHUNT_RESISTANCE 1.0f // ohms
#define MAX_EXPECTED_CURRENT 0.16384f // amps

INA228 ina(INA_ADDR);
// -------------------------------------------------------------
//  SIMPLE LED TEST
// -------------------------------------------------------------
void setup() {
    Serial.begin(115200);
    Wire.begin();
    ina.begin(SHUNT_RESISTANCE, MAX_EXPECTED_CURRENT);
}

void loop() {
    float shuntVoltage, busVoltage, current, power;
    ina.getShuntVoltage_mV(shuntVoltage);
    ina.getBusVoltage_V(busVoltage);
    ina.getCurrent_mA(current);
    ina.getPower_mW(power);

    Serial.print("Shunt Voltage (mV): "); Serial.println(shuntVoltage, 7);
    Serial.print("Bus Voltage (V): "); Serial.println(busVoltage, 7);
    Serial.print("Current (mA): "); Serial.println(current, 7);
    Serial.print("Power (mW): "); Serial.println(power, 7);
    Serial.println("-----------------------");

    delay(2000);
}
