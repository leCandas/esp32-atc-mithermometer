#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEAdvertisedDevice.h>

const int ble_scanTime = 70; // BLE scan time in seconds

BLEScan *pBLEScan;

boolean insideReported = false;
boolean outsideReported = false;

int outsideSquence = 0;
int insideSquence = 0;

class CustomAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {

    void onResult(BLEAdvertisedDevice advertisedDevice) {
        boolean isOutside;
        if(advertisedDevice.getName() == "ATC_69217C") {
          isOutside = true;
        } else if (advertisedDevice.getName() == "ATC_3B9AB8") {
          isOutside = false;
        } else {
          return;
        }

        uint8_t* payload = advertisedDevice.getPayload();
        size_t payloadLength = advertisedDevice.getPayloadLength();
        std::string deviceAddress = advertisedDevice.getAddress().toString();
                
        // temperature, 2 bytes, 16-bit signed integer (LE), 0.1 °C
        const int16_t temperature = uint16_t(payload[11]) | (uint16_t(payload[10]) << 8);
        float temp = temperature / 10.0f;

        // humidity, 1 byte, 8-bit unsigned integer, 1.0 %
        const int8_t humidity = payload[12];
        
        // battery, 1 byte, 8-bit unsigned integer,  1.0 %
        const int8_t battery = payload[13];

        // battery, 2 bytes, 16-bit unsigned integer,  0.001 V
        const int16_t battery_voltage = uint16_t(payload[15]) | (uint16_t(payload[14]) << 8);
        float voltage = battery_voltage / 1.0e3f;

        const int8_t counter = payload[16];
        if (isOutside && outsideSquence != counter) {
            outsideSquence = counter;
            outsideReported = true;
        } else if (!isOutside && insideSquence != counter) {
            insideSquence = counter;
            insideReported = true;
        } else {
            // duplicate data
            return;
        }

        if (insideReported && outsideReported) {
            pBLEScan->stop();
        }

        Serial.printf("%s Temp: %.1f Humi: %d Batt: %d Voltage: %.2f Seq: %d", isOutside ? "Outside" : "Inside", temp, humidity, battery, voltage, counter);
        Serial.println();
    }
};