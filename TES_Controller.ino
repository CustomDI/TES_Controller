#include "src/drivers/LTC4302.h"
#include "src/drivers/MCP4728.h"
#include "src/drivers/INA219.h" // Include the INA219 header
#include "src/routers/Router.h" // Include the Router header
#include "src/devices/LNADriver.h" // Include the LNADriver header
#include "src/devices/TESDriver.h" // Include the TESDriver header

// Define I2C addresses for the devices
#define BASE_HUB_LTC4302_ADDR 0x7E // Address for the base hub LTC4302
#define BASE_HUB_MCP4728_ADDR 0x61 // Address for the main MCP4728

// Make TES/LNA counts configurable in one place
#define NUM_TES 12
#define NUM_LNA 2

// Default address lists (easy to edit / override)
const uint8_t DEFAULT_TES_ADDRESSES[NUM_TES] = {
        0x72, 0x62, 0x63, 0x64, 0x65, 0x66,
        0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C
};

const uint8_t DEFAULT_LNA_ADDRESSES[NUM_LNA] = {
        0x6D, 0x6E
};

// Base hub (unchanged)
LTC4302 baseHub(BASE_HUB_LTC4302_ADDR);

// Router uses baseHub
Router router(&baseHub);

// Route for main MCP4728 (Base Hub -> MCP4728)
I2CRoute routeToMainMCP4728 = { &baseHub, nullptr };
MCP4728 mainDac(BASE_HUB_MCP4728_ADDR, &router, &routeToMainMCP4728);

// Use pointer arrays so the active count can vary at runtime/compile-time
LTC4302* tesLTC[NUM_TES];
TESDriver* tesDriver[NUM_TES];

LTC4302* lnaLTC[NUM_LNA];
LNADriver* lnaDriver[NUM_LNA];

// Helper to initialize devices (call early in setup before begin() calls)
void initDeviceArrays() {
    // Initialize TES LTC4302s and TESDrivers
    for (int i = 0; i < NUM_TES; ++i) {
            uint8_t addr = DEFAULT_TES_ADDRESSES[i];
            tesLTC[i] = new LTC4302(addr);
            tesDriver[i] = new TESDriver(tesLTC[i], &router);
    }

    // Initialize LNA LTC4302s and LNADrivers
    for (int i = 0; i < NUM_LNA; ++i) {
            uint8_t addr = DEFAULT_LNA_ADDRESSES[i];
            lnaLTC[i] = new LTC4302(addr);
            lnaDriver[i] = new LNADriver(lnaLTC[i], &router);
    }
}

// Function to check if an I2C device acknowledges its address
bool isDeviceConnected(uint8_t address) {
    Wire.beginTransmission(address);
    return Wire.endTransmission() == 0;
}

void setup() {
    Serial.begin(115200);
    Serial.println("TES Controller Starting...");

    initDeviceArrays();
    // Check if the main MCP4728 is at the default address 0x60
    // This check needs to be routed through the baseHub if the MCP4728 is behind it.
    // For now, assuming direct access for the check, or that routing is handled internally by Wire.
    // For simplicity, we'll assume the router is already set up and can handle the check.
    baseHub.begin();
    router.begin(); // Initialize the router (which initializes the base hub)
    // Enable the I2C bus through the base hub LTC4302
    baseHub.enableBus();
    mainDac.begin();

    // Initialize TESs
    for (int i = 0; i < NUM_TES; ++i) {
        tesDriver[i]->begin();
    }
    // Initialize LNAs
    for (int i = 0; i < NUM_LNA; ++i) {
        lnaDriver[i]->begin();
    }
    // mainDac.writeDAC(MCP4728_CHANNEL_A, 1024); // Set channel A to ~1/4-scale (4095 max)
    Serial.println("Initialization complete.");
    // Scan each TES LTC endpoint to confirm devices are reachable through the base hub
    // Serial.println("Scanning TES endpoints behind base hub:");
    // for (int i = 0; i < 12; ++i) {
    //     I2CRoute endpoint = { &tesLTC[i], nullptr };
    //     I2CRoute chain = { &baseHub, &endpoint };
    //     router.scanDevicesAtEndpoint(&chain);
    // }

    // // Quick diagnostic: write/read each TES TCA outputs to verify I2C path
    // Serial.println("Running TCA output diagnostics...");
    // for (int i = 0; i < 12; ++i) {
    //     Serial.print("TES Driver "); Serial.print(i+1); Serial.println(": write all ones");
    //     tesDriver[i].setAllOutputPins(0xFFFFFF); // set all 24 bits high
    //     delay(50);
    //     uint32_t val = tesDriver[i].getAllOutputPins();
    //     uint8_t raw[3];
    //     tesDriver[i].readOutputRegisters(raw);
    //     Serial.print("  readAllOutputPins: 0x"); Serial.println(val, HEX);
    //     Serial.print("  raw bytes: ");
    //     for (int b=0;b<3;++b){ Serial.print("0x"); Serial.print(raw[b], HEX); Serial.print(" "); }
    //     Serial.println();

    //     Serial.print("TES Driver "); Serial.print(i+1); Serial.println(": write all zeros");
    //     tesDriver[i].setAllOutputPins(0x000000); // clear
    //     delay(50);
    //     val = tesDriver[i].getAllOutputPins();
    //     tesDriver[i].readOutputRegisters(raw);
    //     Serial.print("  readAllOutputPins: 0x"); Serial.println(val, HEX);
    //     Serial.print("  raw bytes: ");
    //     for (int b=0;b<3;++b){ Serial.print("0x"); Serial.print(raw[b], HEX); Serial.print(" "); }
    //     Serial.println();
    // }
}

// ======== DAC TESTS ========
// void testOn() {
//     mainDac.writeDAC(MCP4728_CHANNEL_A, 1024); // Set channel A to mid-scale (2048 out of 4095)
// }

// void testOff() {
//     mainDac.writeDAC(MCP4728_CHANNEL_A, 0); // Set channel A to 0
// }

//  ======== TES TCA TOGGLE TESTS ========
// Cycle a single bit across all TES drivers (pins 0..23)
void cycleOutputsOnce(uint32_t outputs) {
    Serial.print("Cycling TES TCA outputs... to value:");
    Serial.println(outputs, HEX);
    float current_mA = 0; 
    for (int i = 0; i < 12; ++i) {
        tesDriver[i]->setOutEnable(false); // Enable outputs
        tesDriver[i]->setAllOutputPins(outputs);
    }
    delay(500); // Wait for outputs to stabilize
    // Read back current from each TES driver
    for (int i = 0; i < 12; ++i) {
        current_mA = tesDriver[i]->getCurrent_mA();
        Serial.print("TES Driver ");
        Serial.print(i + 1);
        Serial.print(" - Current: ");
        Serial.print(current_mA);
        Serial.println(" mA");
    }
}

//  ======== TES LTC TOGGLE TESTS ========
// void testOn() {
//     for (int i = 0; i < 12; ++i) {
//         tesDriver[i].setOutEnable(true);
//     }
// }

// void testOff() {
//     for (int i = 0; i < 12; ++i) {
//         tesDriver[i].setOutEnable(false);
//     }
// }

// ========== LNA GATE/DRAIN TESTS =======
// void testOn() {
//     float drainV = 0;
//     float gateV = 0;
//     float drainmA = 0;
//     float gatemA = 0;
//     for (int i = 0; i < 2; ++i) {
//         lnaDriver[i].setDrainEnable(true);
//         lnaDriver[i].setGateEnable(true);
//         delay(1000);
//         lnaDriver[i].writeDrain(0x0FFF); // Max value
//         lnaDriver[i].writeGaate(0x0FFF); // Max value
//         delay(1000);
//         drainV = lnaDriver[i].getDrainBusVoltage_V();
//         gateV = lnaDriver[i].getGateBusVoltage_V();
//         drainmA = lnaDriver[i].getDrainCurrent_mA();
//         gatemA = lnaDriver[i].getGateCurrent_mA();
//         Serial.print("[ON] LNA Driver ");
//         Serial.print(i + 1);
//         Serial.print(" - Drain V: ");
//         Serial.print(drainV);
//         Serial.print(" V, Gate V: ");
//         Serial.print(gateV);
//         Serial.print(" V, Drain I: ");
//         Serial.print(drainmA);
//         Serial.print(" mA, Gate I: ");
//         Serial.print(gatemA);
//         Serial.println(" mA");
//     }
// }

// void testOff() {
//     float drainV = 0;
//     float gateV = 0;
//     float drainmA = 0;
//     float gatemA = 0;
//     for (int i = 0; i < 2; ++i) {
//         lnaDriver[i].setDrainEnable(false);
//         lnaDriver[i].setGateEnable(false);
//         delay(1000);
//         lnaDriver[i].writeDrain(0); // Min value
//         lnaDriver[i].writeGaate(0); // Min value
//         delay(1000);
//         drainV = lnaDriver[i].getDrainBusVoltage_V();
//         gateV = lnaDriver[i].getGateBusVoltage_V();
//         drainmA = lnaDriver[i].getDrainCurrent_mA();
//         gatemA = lnaDriver[i].getGateCurrent_mA();
//         Serial.print("[OFF] LNA Driver ");
//         Serial.print(i + 1);
//         Serial.print(" - Drain V: ");
//         Serial.print(drainV);
//         Serial.print(" V, Gate V: ");
//         Serial.print(gateV);
//         Serial.print(" V, Drain I: ");
//         Serial.print(drainmA);
//         Serial.print(" mA, Gate I: ");
//         Serial.print(gatemA);
//         Serial.println(" mA");
//     }
// }


void loop() {
    cycleOutputsOnce(0x000000);
    delay(5000);
    cycleOutputsOnce(0xFFFFFF);
    delay(5000);
}
