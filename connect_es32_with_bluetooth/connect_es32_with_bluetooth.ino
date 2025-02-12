#include "BluetoothSerial.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"

BluetoothSerial SerialBT;

void setup() {
  Serial.begin(115200);  // Start the Serial Monitor
  esp_log_level_set("*", ESP_LOG_DEBUG);  // Enable detailed logs for debugging
  
  // Set a fixed PIN
  // esp_bt_gap_set_pin(ESP_BT_PIN_TYPE_FIXED, 4, (uint8_t*)"1234");
  
  // Initialize Bluetooth with a device name
  if (!SerialBT.begin("ESP32_BT")) {
    Serial.println("An error occurred during Bluetooth initialization.");
  } else {
    Serial.println("Bluetooth started. Ready to pair with PIN 1234.");
  }
}

void loop() {
  if (SerialBT.available()) {
    String message = SerialBT.readString();
    Serial.println("Received: " + message);
  }
   delay(20);
}
