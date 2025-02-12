// #include <WiFi.h>
// #include <PubSubClient.h>
// #include <ArduinoJson.h>
// #include <Preferences.h>
// #include <BluetoothSerial.h>

// // Pin definitions
// #define OUTPUT_PIN 32
// #define INPUT_PIN 33

// // Debugging macro
// #define DEBUG 1
// #if DEBUG
//   #define DEBUG_PRINT(x) Serial.println(x)
// #else
//   #define DEBUG_PRINT(x)
// #endif

// // Configuration storage and Bluetooth objects
// Preferences preferences;
// BluetoothSerial SerialBT;

// // Configuration variables
// char ssid[32] = "Providence";         // Default Wi-Fi SSID
// char password[64] = "Provider123";    // Default Wi-Fi password
// char mqtt_server[32] = "192.168.0.148"; // Default MQTT server address
// char mqtt_username[32] = "testuser";  // Default MQTT username
// char mqtt_password[32] = "password";  // Default MQTT password
// int mqtt_port = 1883;                 // Default MQTT port

// WiFiClient espClient;
// PubSubClient client(espClient);

// // Function to load configuration from NVS
// void loadConfig() {
//   preferences.begin("config", true); // Read-only mode
//   preferences.getString("ssid", ssid, sizeof(ssid));
//   preferences.getString("password", password, sizeof(password));
//   preferences.getString("mqtt_server", mqtt_server, sizeof(mqtt_server));
//   preferences.getString("mqtt_user", mqtt_username, sizeof(mqtt_username));
//   preferences.getString("mqtt_pass", mqtt_password, sizeof(mqtt_password));
//   mqtt_port = preferences.getInt("mqtt_port", mqtt_port);
//   preferences.end();
//   DEBUG_PRINT("Configuration loaded from NVS.");
// }

// // Function to save configuration to NVS
// void saveConfig(const JsonDocument& jsonDoc) {
//   preferences.begin("config", false); // Write mode
//   preferences.putString("ssid", jsonDoc["ssid"] | ssid);
//   preferences.putString("password", jsonDoc["password"] | password);
//   preferences.putString("mqtt_server", jsonDoc["mqtt_server"] | mqtt_server);
//   preferences.putString("mqtt_user", jsonDoc["mqtt_user"] | mqtt_username);
//   preferences.putString("mqtt_pass", jsonDoc["mqtt_pass"] | mqtt_password);
//   preferences.putInt("mqtt_port", jsonDoc["mqtt_port"] | mqtt_port);
//   preferences.end();
//   DEBUG_PRINT("Configuration saved to NVS.");
// }

// // Function to handle Bluetooth configuration
// void handleBluetoothConfig() {
//   if (SerialBT.available()) {
//     char command[256];
//     size_t len = SerialBT.readBytesUntil('\n', command, sizeof(command) - 1);
//     command[len] = '\0'; // Null-terminate the command
//     StaticJsonDocument<128> jsonDoc;
//     if (deserializeJson(jsonDoc, command)) {
//       DEBUG_PRINT("Failed to parse Bluetooth JSON.");
//       return;
//     }
//     saveConfig(jsonDoc);
//     ESP.restart(); // Restart to apply changes
//   }
// }

// // Function to reconnect to MQTT
// void reconnectMQTT() {
//   while (!client.connected()) {
//     DEBUG_PRINT("Connecting to MQTT...");
//     char clientId[20];
//     snprintf(clientId, sizeof(clientId), "ESP32-%04X", random(0xffff));
//     if (client.connect(clientId, mqtt_username, mqtt_password)) {
//       DEBUG_PRINT("Connected to MQTT.");
//       client.subscribe("esp32/config");
//       client.subscribe("esp32/command");
//     } else {
//       DEBUG_PRINT("Failed to connect to MQTT.");
//       delay(5000);
//     }
//   }
// }

// // Publish status to MQTT
// void publishStatus() {
//   StaticJsonDocument<96> jsonDoc;
//   jsonDoc["input_pin"] = digitalRead(INPUT_PIN);
//   jsonDoc["output_pin"] = digitalRead(OUTPUT_PIN);
//   char buffer[96];
//   size_t n = serializeJson(jsonDoc, buffer);
//   client.publish("esp32/status", buffer, n);
//   DEBUG_PRINT("Published status to esp32/status.");
// }

// // MQTT callback function
// void mqttCallback(char* topic, byte* payload, unsigned int length) {
//   payload[length] = '\0'; // Null-terminate the payload
//   StaticJsonDocument<96> jsonDoc;
//   if (deserializeJson(jsonDoc, payload)) {
//     DEBUG_PRINT("Failed to parse JSON payload.");
//     return;
//   }

//   if (strcmp(topic, "esp32/config") == 0) {
//     saveConfig(jsonDoc);
//     ESP.restart();
//   } else if (strcmp(topic, "esp32/command") == 0) {
//     const char* action = jsonDoc["action"];
//     if (action) {
//       digitalWrite(OUTPUT_PIN, strcmp(action, "ON") == 0 ? HIGH : LOW);
//       DEBUG_PRINT(String("GPIO 32 set to ") + action);
//       publishStatus();
//     }
//   }
// }

// void setup() {
//   Serial.begin(9600);
//   SerialBT.begin("ESP32_BT");
//   pinMode(OUTPUT_PIN, OUTPUT);
//   pinMode(INPUT_PIN, INPUT);

//   loadConfig();

//   WiFi.begin(ssid, password);
//   DEBUG_PRINT("Connecting to WiFi...");
//   while (WiFi.status() != WL_CONNECTED) {
//     delay(1000);
//     DEBUG_PRINT(".");
//   }
//   DEBUG_PRINT(String("Connected to WiFi. IP: ") + WiFi.localIP().toString());

//   client.setServer(mqtt_server, mqtt_port);
//   client.setCallback(mqttCallback);
// }

// void loop() {
//   if (!client.connected()) {
//     reconnectMQTT();
//   }
//   client.loop();
//   handleBluetoothConfig();
//   //publishStatus(); // Periodically publish status
//   delay(5000); // Delay for status updates
// }



#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <BluetoothSerial.h>
#include <esp_bt_device.h>
#include <esp_bt_main.h>
#include <esp_gap_bt_api.h>

// Pin definitions
#define OUTPUT_PIN 32
#define INPUT_PIN 33

// Debugging macro
#define DEBUG 1
#if DEBUG
  #define DEBUG_PRINT(x) Serial.println(x)
#else
  #define DEBUG_PRINT(x)
#endif

// Configuration storage and Bluetooth objects
Preferences preferences;
BluetoothSerial SerialBT;

// Configuration variables
char ssid[32] = "Providence";         // Default Wi-Fi SSID
char password[64] = "Provider123";    // Default Wi-Fi password
char mqtt_server[32] = "192.168.0.148"; // Default MQTT server address
char mqtt_username[32] = "testuser";  // Default MQTT username
char mqtt_password[32] = "password";  // Default MQTT password
int mqtt_port = 1883;                 // Default MQTT port

WiFiClient espClient;
PubSubClient client(espClient);

// Function to load configuration from NVS
void loadConfig() {
  preferences.begin("config", true); // Read-only mode
  preferences.getString("ssid", ssid, sizeof(ssid));
  preferences.getString("password", password, sizeof(password));
  preferences.getString("mqtt_server", mqtt_server, sizeof(mqtt_server));
  preferences.getString("mqtt_user", mqtt_username, sizeof(mqtt_username));
  preferences.getString("mqtt_pass", mqtt_password, sizeof(mqtt_password));
  mqtt_port = preferences.getInt("mqtt_port", mqtt_port);
  preferences.end();
  DEBUG_PRINT("Configuration loaded from NVS.");
}

// Function to save configuration to NVS
void saveConfig(const JsonDocument& jsonDoc) {
  preferences.begin("config", false); // Write mode
  preferences.putString("ssid", jsonDoc["ssid"] | ssid);
  preferences.putString("password", jsonDoc["password"] | password);
  preferences.putString("mqtt_server", jsonDoc["mqtt_server"] | mqtt_server);
  preferences.putString("mqtt_user", jsonDoc["mqtt_user"] | mqtt_username);
  preferences.putString("mqtt_pass", jsonDoc["mqtt_pass"] | mqtt_password);
  preferences.putInt("mqtt_port", jsonDoc["mqtt_port"] | mqtt_port);
  preferences.end();
  DEBUG_PRINT("Configuration saved to NVS.");
}

// Function to handle Bluetooth configuration
void handleBluetoothConfig() {
  if (SerialBT.available()) {
    char command[256];
    size_t len = SerialBT.readBytesUntil('\n', command, sizeof(command) - 1);
    command[len] = '\0'; // Null-terminate the command
    StaticJsonDocument<128> jsonDoc;
    if (deserializeJson(jsonDoc, command)) {
      DEBUG_PRINT("Failed to parse Bluetooth JSON.");
      return;
    }
    saveConfig(jsonDoc);
    ESP.restart(); // Restart to apply changes
  }
}

// Function to reconnect to MQTT
void reconnectMQTT() {
  while (!client.connected()) {
    DEBUG_PRINT("Connecting to MQTT...");
    char clientId[20];
    snprintf(clientId, sizeof(clientId), "ESP32-%04X", random(0xffff));
    if (client.connect(clientId, mqtt_username, mqtt_password)) {
      DEBUG_PRINT("Connected to MQTT.");
      client.subscribe("esp32/config");
      client.subscribe("esp32/command");
    } else {
      DEBUG_PRINT("Failed to connect to MQTT.");
      delay(5000);
    }
  }
}

// Publish status to MQTT
void publishStatus() {
  StaticJsonDocument<96> jsonDoc;
  // jsonDoc["input_pin"] = digitalRead(INPUT_PIN);
  // jsonDoc["output_pin"] = digitalRead(OUTPUT_PIN);
  if (digitalRead(OUTPUT_PIN) == 1) {
        jsonDoc["status"] = "success";
        jsonDoc["state"] = "ON";
        jsonDoc["message"] = "GPIO 32 set to HIGH";     
    } else if (digitalRead(OUTPUT_PIN) == 0) {
        jsonDoc["status"] = "success";
        jsonDoc["state"] = "OFF";
        jsonDoc["message"] = "GPIO 32 set to LOW";
        
    } 
  char buffer[96];
  size_t n = serializeJson(jsonDoc, buffer);
  client.publish("esp32/status", buffer, n);
  DEBUG_PRINT("Published status to esp32/status.");
}

// MQTT callback function
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0'; // Null-terminate the payload
  StaticJsonDocument<96> jsonDoc;
  if (deserializeJson(jsonDoc, payload)) {
    DEBUG_PRINT("Failed to parse JSON payload.");
    return;
  }

  if (strcmp(topic, "esp32/config") == 0) {
    saveConfig(jsonDoc);
    ESP.restart();
  } else if (strcmp(topic, "esp32/command") == 0) {
    const char* action = jsonDoc["action"];
    if (action) {
      digitalWrite(OUTPUT_PIN, strcmp(action, "ON") == 0 ? HIGH : LOW);
      DEBUG_PRINT(String("GPIO 32 set to ") + action);
      publishStatus();
    }
  }
}

void setup() {
  Serial.begin(9600);
  SerialBT.begin("ESP32_BT");
 // Set Bluetooth PIN
  esp_bt_pin_type_t pin_type = ESP_BT_PIN_TYPE_FIXED;
  esp_bt_pin_code_t pin_code = {'1', '2', '3', '4'}; // 4-digit PIN
  esp_bt_gap_set_pin(pin_type, 4, pin_code);

  pinMode(OUTPUT_PIN, OUTPUT);
  pinMode(INPUT_PIN, INPUT);

  loadConfig();

  WiFi.begin(ssid, password);
  DEBUG_PRINT("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    DEBUG_PRINT(".");
  }
  DEBUG_PRINT(String("Connected to WiFi. IP: ") + WiFi.localIP().toString());

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttCallback);
}

void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();
  handleBluetoothConfig();
  //publishStatus(); // Periodically publish status
  delay(5000); // Delay for status updates
}
