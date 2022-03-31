// Change device names inside BLEAdvertisedDeviceCallbacks
// To switch protocol just include the relevant one.
// Be sure your device and included protocol is same.
// To sync sequence just power up atc devices at the same time.

//#include "atc_mi_like.h"
#include "atc_custom.h"

void taskScanBleDevices(void* pvParameters) {
    while (true) {
        Serial.println("Starting new scan...");
        pBLEScan = BLEDevice::getScan();
        pBLEScan->start(ble_scanTime, false);
        pBLEScan->clearResults();
        insideReported = false;
        outsideReported = false;
        vTaskDelay(5000 / portTICK_RATE_MS);
    }
}

void setup() {
    Serial.begin(115200);
    BLEDevice::init("m5-atc-reader");

    pBLEScan = BLEDevice::getScan();

    //pBLEScan->setAdvertisedDeviceCallbacks(new MiLikeAdvertisedDeviceCallbacks(), true);
    pBLEScan->setAdvertisedDeviceCallbacks(new CustomAdvertisedDeviceCallbacks(), true);

    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(50);
    pBLEScan->setWindow(40);

    TaskHandle_t bleScanHandle = nullptr;
    xTaskCreate(taskScanBleDevices, "ble-scan", 2048, nullptr, tskIDLE_PRIORITY, &bleScanHandle);
}

void loop() {
  // put your main code here, to run repeatedly:
}