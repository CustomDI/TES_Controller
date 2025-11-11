#include "src/drivers/LTC4302.h"
#include "src/drivers/MCP4728.h"
#include "src/drivers/INA219.h" // Include the INA219 header
#include "src/routers/Router.h" // Include the Router header
#include "src/devices/LNADriver.h" // Include the LNADriver header
#include "src/devices/TESDriver.h" // Include the TESDriver header

// Define I2C addresses for the devices
// These are example addresses, refer to your hardware configuration
#define BASE_HUB_LTC4302_ADDR 0x7E // Example address for the base hub LTC4302
#define BASE_HUB_MCP4728_ADDR 0x61 // New address for the main MCP4728 after programming

// I2C addresses for the 12 TES LTC4302s (0x61 to 0x6C)
#define TES_LTC4302_ADDR_1  0x61
#define TES_LTC4302_ADDR_2  0x62
#define TES_LTC4302_ADDR_3  0x63
#define TES_LTC4302_ADDR_4  0x64
#define TES_LTC4302_ADDR_5  0x65
#define TES_LTC4302_ADDR_6  0x66
#define TES_LTC4302_ADDR_7  0x67
#define TES_LTC4302_ADDR_8  0x68
#define TES_LTC4302_ADDR_9  0x69
#define TES_LTC4302_ADDR_10 0x6A
#define TES_LTC4302_ADDR_11 0x6B
#define TES_LTC4302_ADDR_12 0x6C

// I2C addresses for LNA driver LTC4302s
#define LNA_LTC4302_ADDR_1 0x6D // Address for LNA Driver 1's LTC
#define LNA_LTC4302_ADDR_2 0x6E // Address for LNA Driver 2's LTC

// Create instances of the drivers
LTC4302 baseHub(BASE_HUB_LTC4302_ADDR);

// 12 LTC4302 instances for TES devices (use an array)
LTC4302 tesLTC[12] = {
    LTC4302(TES_LTC4302_ADDR_1),
    LTC4302(TES_LTC4302_ADDR_2),
    LTC4302(TES_LTC4302_ADDR_3),
    LTC4302(TES_LTC4302_ADDR_4),
    LTC4302(TES_LTC4302_ADDR_5),
    LTC4302(TES_LTC4302_ADDR_6),
    LTC4302(TES_LTC4302_ADDR_7),
    LTC4302(TES_LTC4302_ADDR_8),
    LTC4302(TES_LTC4302_ADDR_9),
    LTC4302(TES_LTC4302_ADDR_10),
    LTC4302(TES_LTC4302_ADDR_11),
    LTC4302(TES_LTC4302_ADDR_12)
};

// LTC4302 instances for LNA drivers (use an array)
LTC4302 lnaLTC[2] = {
    LTC4302(LNA_LTC4302_ADDR_1), // LTC for LNA Driver 1 (0x6D)
    LTC4302(LNA_LTC4302_ADDR_2)  // LTC for LNA Driver 2 (0x6E)
};

// Create a Router instance with the base hub
Router router(&baseHub);

// Define routes
// Route for the main MCP4728: Base Hub -> MCP4728
I2CRoute routeToMainMCP4728 = {&baseHub, nullptr};
MCP4728 mainDac(BASE_HUB_MCP4728_ADDR, &router, &routeToMainMCP4728); // Initialize with default address 0x60

// Create instances of the TESDrivers (array)
TESDriver tesDriver[12] = {
    TESDriver(&tesLTC[0], &router),
    TESDriver(&tesLTC[1], &router),
    TESDriver(&tesLTC[2], &router),
    TESDriver(&tesLTC[3], &router),
    TESDriver(&tesLTC[4], &router),
    TESDriver(&tesLTC[5], &router),
    TESDriver(&tesLTC[6], &router),
    TESDriver(&tesLTC[7], &router),
    TESDriver(&tesLTC[8], &router),
    TESDriver(&tesLTC[9], &router),
    TESDriver(&tesLTC[10], &router),
    TESDriver(&tesLTC[11], &router)
};

// Create instances of the LNADrivers (array)
LNADriver lnaDriver[2] = {
    LNADriver(&lnaLTC[0], &router),
    LNADriver(&lnaLTC[1], &router)
};

// Function to check if an I2C device acknowledges its address
bool isDeviceConnected(uint8_t address) {
    Wire.beginTransmission(address);
    return Wire.endTransmission() == 0;
}

void setup() {
    Serial.begin(115200);
    Serial.println("TES Controller Starting...");
    Wire.begin();

    // Check if the main MCP4728 is at the default address 0x60
    // This check needs to be routed through the baseHub if the MCP4728 is behind it.
    // For now, assuming direct access for the check, or that routing is handled internally by Wire.
    // For simplicity, we'll assume the router is already set up and can handle the check.
    baseHub.begin();

    // Initialize all TES LTCs
    for (int i = 0; i < 12; ++i) {
        tesLTC[i].begin();
    }

    // Initialize LNA LTCs
    for (int i = 0; i < 2; ++i) {
        lnaLTC[i].begin();
    }
    mainDac.begin(); // Initialize with default address 0x60

    // Initialize LNADrivers
    for (int i = 0; i < 2; ++i) {
        lnaDriver[i].begin();
    }

    // Initialize all TES Drivers
    for (int i = 0; i < 12; ++i) {
        tesDriver[i].begin();
    }

    router.begin(); // Initialize the router (which initializes the base hub)

    // Enable the I2C bus through the base hub LTC4302
    baseHub.enableBus();
    Serial.println("Base Hub LTC4302 bus enabled.");
    router.scanDevicesAtEndpoint(nullptr); // Scan directly at the base hub

    // Scan endpoints for TES Driver 10 and LNA Driver 1
    Serial.println("Scanning TES Driver 2 endpoint...");
    auto tesRoute = tesDriver[1].getRouteToTesLtc4302();
    router.scanDevicesAtEndpoint(&tesRoute);

    Serial.println("Scanning LNA Driver 2 endpoint...");
    auto lnaRoute = lnaDriver[1].getRouteToLnaLtc4302();
    router.scanDevicesAtEndpoint(&lnaRoute);


    // // --- Example: Interacting with main MCP4728 using the Router ---
    // Serial.println("\nInteracting with main MCP4728...");
    // uint16_t dacValue = 2048; // Mid-range value for a 12-bit DAC (0-4095)
    // mainDac.writeDAC(MCP4728_CHANNEL_A, dacValue);
    // Serial.print("Wrote ");
    // Serial.print(dacValue);
    // Serial.println(" to main MCP4728 DAC Channel A.");

    // // --- Example: Scanning devices at the endpoint of the main MCP4728 route ---
    // Serial.println("\nScanning devices behind the main MCP4728 route:");
    // router.scanDevicesAtEndpoint(&routeToMainMCP4728);

    // // --- Example: Interacting with TES devices using the TESDrivers ---
    // Serial.println("\nInteracting with TES devices via TESDrivers...");
    
    // // Write to all 12 TES devices
    // for (int i = 0; i < 12; ++i) {
    //     uint16_t tesValue = 100 + (i * 10); // Example value
    //     TESDriver* currentTesDriver;

    //     switch (i) {
    //         case 0: currentTesDriver = &tesDriver1; break;
    //         case 1: currentTesDriver = &tesDriver2; break;
    //         case 2: currentTesDriver = &tesDriver3; break;
    //         case 3: currentTesDriver = &tesDriver4; break;
    //         case 4: currentTesDriver = &tesDriver5; break;
    //         case 5: currentTesDriver = &tesDriver6; break;
    //         case 6: currentTesDriver = &tesDriver7; break;
    //         case 7: currentTesDriver = &tesDriver8; break;
    //         case 8: currentTesDriver = &tesDriver9; break;
    //         case 9: currentTesDriver = &tesDriver10; break;
    //         case 10: currentTesDriver = &tesDriver11; break;
    //         case 11: currentTesDriver = &tesDriver12; break;
    //         default: continue; // Should not happen
    //     }
    //     currentTesDriver->writeTesDevice(tesValue);
    //     Serial.print("Wrote ");
    //     Serial.print(tesValue);
    //     Serial.print(" to TES Driver ");
    //     Serial.print(i + 1);
    //     Serial.println(".");
    // }

    // // Example of reading from a specific TES device (e.g., TES Driver 5)
    // uint16_t readTesValue = tesDriver5.readTesDevice();
    // Serial.print("Read ");
    // Serial.print(readTesValue);
    // Serial.println(" from TES Driver 5.");

    // // --- Example: Scanning devices at the endpoint of a TES route ---
    // Serial.println("\nScanning devices behind TES Driver 1 LTC (0x61):");
    // router.scanDevicesAtEndpoint(&tesDriver1._routeToTesLtc4302);

    // Serial.println("\nScanning devices behind TES Driver 5 LTC (0x65):");
    // router.scanDevicesAtEndpoint(&tesDriver5._routeToTesLtc4302);

    // // --- Example: Interacting with LNA devices using the LNADrivers ---
    // Serial.println("\nInteracting with LNA devices via LNADrivers...");
    // uint16_t lnaDacValue1 = 1024;
    // lnaDriver1.writeLnaDac(MCP4728_CHANNEL_B, lnaDacValue1);
    // Serial.print("Wrote ");
    // Serial.print(lnaDacValue1);
    // Serial.println(" to LNA Driver 1 MCP4728 DAC Channel B.");

    // uint16_t lnaDacValue2 = 1500;
    // lnaDriver2.writeLnaDac(MCP4728_CHANNEL_A, lnaDacValue2);
    // Serial.print("Wrote ");
    // Serial.print(lnaDacValue2);
    // Serial.println(" to LNA Driver 2 MCP4728 DAC Channel A.");

    // // Read from LNA Driver 1 INA219_1
    // Serial.print("LNA Driver 1 INA1 Bus Voltage: ");
    // Serial.print(lnaDriver1.getIna1BusVoltage_V());
    // Serial.println(" V");
    // Serial.print("LNA Driver 1 INA1 Current: ");
    // Serial.print(lnaDriver1.getIna1Current_mA());
    // Serial.println(" mA");

    // // Read from LNA Driver 2 INA219_1
    // Serial.print("LNA Driver 2 INA1 Bus Voltage: ");
    // Serial.print(lnaDriver2.getIna1BusVoltage_V());
    // Serial.println(" V");
    // Serial.print("LNA Driver 2 INA1 Current: ");
    // Serial.print(lnaDriver2.getIna1Current_mA());
    // Serial.println(" mA");

    // // --- Example: Scanning devices at the endpoint of an LNA route ---
    // Serial.println("\nScanning devices behind LNA Driver 1 LTC (0x6D):");
    // router.scanDevicesAtEndpoint(&lnaDriver1._routeToLnaLtc4302);
}

void loop() {
    // Main loop can be used for continuous monitoring or control
    // For now, it's empty as the setup performs the initial configuration.
    // Toggle Out Enable
    tesDriver[1].setOutEnable(true);
    delay(2000);
    tesDriver[1].setOutEnable(false);
    delay(2000);
}
