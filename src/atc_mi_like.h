#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEAdvertisedDevice.h>

const int ble_scanTime = 130; // BLE scan time in seconds

BLEScan *pBLEScan;

boolean insideReported = false;
boolean outsideReported = false;

enum XiaomiEventType {
    XiaomiTemperatureAndHumidity = 0x0D,
    XiaomiBattery = 0x0A,
    XiaomiHumidity = 0x06,
    XiaomiTemperature = 0x04
};

class MiLikeAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {

    uint8_t* findServiceData(uint8_t* data, size_t length, uint8_t* foundBlockLength) {
        uint8_t* rightBorder = data + length;
        while (data < rightBorder) {
            uint8_t blockLength = *data;
            if (blockLength < 5) {
                data += (blockLength+1);
                continue;
            }
            uint8_t blockType = *(data+1);
            uint16_t serviceType = *(uint16_t*)(data + 2);
            if (blockType == 0x16 && serviceType == 0xfe95 && *(data + 4) == 0x50) {
                *foundBlockLength = blockLength-3;
                return data+4;
            }
            data += (blockLength+1);
        }   
        return nullptr;
    }

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

        uint8_t serviceDataLength=0;
        uint8_t* serviceData = findServiceData(payload, payloadLength, &serviceDataLength);
        if (serviceData == nullptr) {
            return;
        }

        // 11th byte is an event type
        switch (serviceData[11]) {
            case XiaomiEventType::XiaomiTemperatureAndHumidity: {
                float temp = *(uint16_t*)(serviceData + 11 + 3) / 10.0;
                float humidity = *(uint16_t*)(serviceData + 11 + 5) / 10.0;
                Serial.printf("%s Temp: %.1f Humidity: %.1f\n", isOutside ? "Outside" : "Inside", temp, humidity);
                if (isOutside) {
                    outsideReported = true;
                } else {
                    insideReported = true;
                }
            }
            break;
            case XiaomiEventType::XiaomiBattery: {
                int battery = *(serviceData + 11 + 3);
                Serial.printf("%s Battery: %d\n", isOutside ? "Outside" : "Inside", battery);
            }
            break;
        }

        if (insideReported && outsideReported) {
            pBLEScan->stop();
        }
    }
};