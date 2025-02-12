// // This example code is in the Public Domain (or CC0 licensed, at your option.)
// // By Evandro Copercini - 2018
// //
// // This example creates a bridge between Serial and Classical Bluetooth (SPP)
// // and also demonstrate that SerialBT have the same functionalities of a normal Serial
// // Note: Pairing is authenticated automatically by this device

// #include "BluetoothSerial.h"

// String device_name = "ESP32-BT-Slave";

// // Check if Bluetooth is available
// #if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
// #error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
// #endif

// // Check Serial Port Profile
// #if !defined(CONFIG_BT_SPP_ENABLED)
// #error Serial Port Profile for Bluetooth is not available or not enabled. It is only available for the ESP32 chip.
// #endif

// BluetoothSerial SerialBT;

// void setup() {
//   Serial.begin(115200);
//   SerialBT.begin(device_name);  //Bluetooth device name
//   //SerialBT.deleteAllBondedDevices(); // Uncomment this to delete paired devices; Must be called after begin
//   Serial.printf("The device with name \"%s\" is started.\nNow you can pair it with Bluetooth!\n", device_name.c_str());
// }

// void loop() {
//   if (Serial.available()) {
//     SerialBT.write(Serial.read());
//   }
//   if (SerialBT.available()) {
//     Serial.write(SerialBT.read());
//   }
//   delay(20);
// }




// CODE SAMPLE 2
/*
  Bluetooth Serial Client Demo
  bluetooth_serial_client.ino
  Demonstrates operation of BluetoothSerial Library
  Based upon open-source code sample by Evandro Copercini - 2018
  
  DroneBot Workshop 2024
  https://dronebotworkshop.com
*/

// // Include BluetoothSerial library.
// #include "BluetoothSerial.h"

// // Name of Bluetooth client.
// String device_name = "ESP32-BT-Client";

// // Check if Bluetooth is available.
// #if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
// #error Bluetooth is not enabled!
// #endif

// // Check if Serial Port Profile (SPP) is available.
// #if !defined(CONFIG_BT_SPP_ENABLED)
// #error Serial Port Profile for Bluetooth is not available or not enabled!
// #endif

// // Create a BluetoothSerial object.
// BluetoothSerial SerialBT;

// void setup() {
//   // Start Serial Monitor.
//   Serial.begin(115200);

//   // Start Bluetooth Serial.
//   SerialBT.begin(device_name);

//   // Uncomment the next line to delete previously paired devices. Must be called directly after bluetooth begin.
//   //SerialBT.deleteAllBondedDevices();

//   // Print to serial monitor.
//   Serial.printf("The device \"%s\" is started and can be paired with Bluetooth.\n", device_name.c_str());
// }

// void loop() {
//   // Check for messsage from serial monitor.
//   if (Serial.available()) {
//     // Write message to paired Bluetooth device.
//     SerialBT.write(Serial.read());
//   }

//   // Check for message from paired Bluetooth device.
//   if (SerialBT.available()) {
//     // Write message to serial monitor.
//     Serial.write(SerialBT.read());
//   }
//   delay(20);
// }


// CODE SAMPLE


/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updates by chegewara
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  BLEDevice::init("ESP32_BT");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic =
    pService->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

  pCharacteristic->setValue("Hello World says Neil");
  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(2000);
}