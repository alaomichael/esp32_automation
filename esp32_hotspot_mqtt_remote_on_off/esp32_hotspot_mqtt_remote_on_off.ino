// #include <WiFi.h>          // Include the Wi-Fi library for ESP32
// #include <PubSubClient.h>  // Include the PubSubClient library for MQTT
// #include <ArduinoJson.h>   // Include the ArduinoJson library for JSON parsing
// #include <Preferences.h>   // Include the Preferences library for persistent storage

// #define PIN23 23  // Define GPIO pin 23 for input (reset)
// #define PIN32 32  // Define GPIO pin 32 for output

// // Default configuration values
// // Set the SSID and Password for the Access Point
// const char *default_ssid = "ESP32-Access-Point";
// const char *default_password = "123456789";
// // const char* default_ssid = "Providence";
// // const char* default_password = "Provider123";
// const char* default_mqtt_server = "192.168.0.148";
// const char* default_mqtt_username = "testuser";
// const char* default_mqtt_password = "password";
// const int default_mqtt_port = 1883;

// // Set a static IP address, subnet, and gateway for the Access Point
// IPAddress local_IP(192, 168, 4, 1);  // Static IP for the ESP32 in AP mode
// IPAddress gateway(192, 168, 4, 1);   // Gateway (usually same as the static IP)
// IPAddress subnet(255, 255, 255, 0);  // Subnet mask


// // Global variables
// char ssid[32];
// char password[32];
// char mqtt_server[32];
// char mqtt_username[32];
// char mqtt_password[32];
// int mqtt_port;

// WiFiClient espClient;
// PubSubClient client(espClient);
// Preferences preferences; // For storing configuration persistently

// void setup() {
//   Serial.begin(9600);
//   pinMode(PIN32, OUTPUT);
//   pinMode(PIN23, INPUT_PULLUP); // Configure PIN23 with internal pull-up resistor

//   // Load configuration from NVS
//   preferences.begin("config", false);
//   loadConfiguration();

//   // Start the Wi-Fi Access Point
//   startAccessPoint();

//   // Setup MQTT client
//   client.setServer(mqtt_server, mqtt_port);
//   client.setCallback(mqttCallback);
// }

// void loop() {
//   // Handle reset if PIN35 is triggered
//   handleReset();

//   // Maintain MQTT connection
//   if (!client.connected()) {
//     reconnectMQTT();
//   }
//   client.loop();

//   // Read and publish GPIO 32 state
//   readGPIOState();

//   delay(5000); // Adjust delay as necessary
// }

// // Helper functions

// void loadConfiguration() {
//   // Load configuration from preferences or use defaults
//   if (preferences.getString("ssid", "").isEmpty()) {
//     Serial.println("No saved configuration found. Using default settings...");
//     setDefaultConfiguration();
//   } else {
//     strncpy(ssid, preferences.getString("ssid").c_str(), sizeof(ssid));
//     strncpy(password, preferences.getString("password").c_str(), sizeof(password));
//     strncpy(mqtt_server, preferences.getString("mqtt_server").c_str(), sizeof(mqtt_server));
//     strncpy(mqtt_username, preferences.getString("mqtt_username").c_str(), sizeof(mqtt_username));
//     strncpy(mqtt_password, preferences.getString("mqtt_password").c_str(), sizeof(mqtt_password));
//     mqtt_port = preferences.getInt("mqtt_port", default_mqtt_port);
//   }

//   preferences.end();  // Always end Preferences after use
// }

// void setDefaultConfiguration() {
//   strncpy(ssid, default_ssid, sizeof(ssid));
//   strncpy(password, default_password, sizeof(password));
//   strncpy(mqtt_server, default_mqtt_server, sizeof(mqtt_server));
//   strncpy(mqtt_username, default_mqtt_username, sizeof(mqtt_username));
//   strncpy(mqtt_password, default_mqtt_password, sizeof(mqtt_password));
//   mqtt_port = default_mqtt_port;
// }

// void saveConfiguration(const char* new_ssid, const char* new_password, const char* new_mqtt_server, const char* new_mqtt_username, const char* new_mqtt_password, int new_mqtt_port) {
//   preferences.putString("ssid", new_ssid);
//   preferences.putString("password", new_password);
//   preferences.putString("mqtt_server", new_mqtt_server);
//   preferences.putString("mqtt_username", new_mqtt_username);
//   preferences.putString("mqtt_password", new_mqtt_password);
//   preferences.putInt("mqtt_port", new_mqtt_port);
// }

// void startAccessPoint() {
//    // Set up the Access Point with a static IP address
//   WiFi.softAPConfig(local_IP, gateway, subnet);
//   WiFi.softAP(ssid, password);

//   // Print the IP address of the ESP32 in AP mode
//   Serial.println("Access Point Started");
//   Serial.print("ESP32 IP Address: ");
//   Serial.println(WiFi.softAPIP());  // This will display the static IP (192.168.4.1)
// }

// void handleReset() {
//   if (digitalRead(PIN23) == LOW) {
//     Serial.println("Reset triggered. Clearing configuration...");
//     preferences.clear();
//     preferences.end();
//     ESP.restart(); // Restart ESP32
//   }
// }

// void readGPIOState() {
//   int pin32Value = digitalRead(PIN32);
//   Serial.print("GPIO 32 reads: ");
//   Serial.println(pin32Value == HIGH ? "HIGH" : "LOW");
// }

// void mqttCallback(char* topic, byte* payload, unsigned int length) {
//   Serial.print("Message arrived on topic: ");
//   Serial.println(topic);

//   char buffer[length + 1];
//   memcpy(buffer, payload, length);
//   buffer[length] = '\0';
  
//   StaticJsonDocument<256> jsonDoc;
//   if (deserializeJson(jsonDoc, buffer)) {
//     Serial.println("Invalid JSON received");
//     return;
//   }

//   // Handle configuration update
//   if (strcmp(topic, "esp32/config") == 0) {
//     handleConfigurationUpdate(jsonDoc);
//   }

//   // Handle action commands
//   if (jsonDoc.containsKey("action")) {
//     handleActionCommand(jsonDoc["action"]);
//   }
// }

// void handleConfigurationUpdate(const StaticJsonDocument<256>& jsonDoc) {
//   strncpy(ssid, jsonDoc["ssid"] | ssid, sizeof(ssid));
//   strncpy(password, jsonDoc["password"] | password, sizeof(password));
//   strncpy(mqtt_server, jsonDoc["mqtt_server"] | mqtt_server, sizeof(mqtt_server));
//   strncpy(mqtt_username, jsonDoc["mqtt_username"] | mqtt_username, sizeof(mqtt_username));
//   strncpy(mqtt_password, jsonDoc["mqtt_password"] | mqtt_password, sizeof(mqtt_password));
//   mqtt_port = jsonDoc["mqtt_port"] | mqtt_port;

//   saveConfiguration(ssid, password, mqtt_server, mqtt_username, mqtt_password, mqtt_port);
//   Serial.println("Configuration updated and saved");

//   // Restart to apply new configuration
//   ESP.restart();
// }

// void handleActionCommand(const char* action) {
//   StaticJsonDocument<200> responseJson;

//   // Check if 'action' is valid
//   if (action == nullptr) {
//     StaticJsonDocument<200> errorJson;  // Create a JSON document for the error response
//     errorJson["status"] = "error";
//     errorJson["message"] = "Missing 'action' field in JSON";

//     // Serialize and publish the JSON error
//     char errorBuffer[256];
//     serializeJson(errorJson, errorBuffer);
//     client.publish("esp32/status", errorBuffer);

//     Serial.println("Missing 'action' field in JSON");
//     return;  // Exit function as there is no valid action to process
//   }

//   // Handle the action based on its value
//   if (strcmp(action, "ON") == 0) {
//     digitalWrite(PIN32, HIGH);
//     responseJson["status"] = "success";
//     responseJson["state"] = "ON";
//     responseJson["message"] = "GPIO 32 set to HIGH";
//     Serial.println("GPIO 32 set to HIGH");
//   } else if (strcmp(action, "OFF") == 0) {
//     digitalWrite(PIN32, LOW);
//     responseJson["status"] = "success";
//     responseJson["state"] = "OFF";
//     responseJson["message"] = "GPIO 32 set to LOW";
//     Serial.println("GPIO 32 set to LOW");
//   } else {
//     responseJson["status"] = "error";
//     responseJson["message"] = "Invalid action received";
//     Serial.println("Invalid action received");
//   }

//   // Serialize and publish the response
//   char responseBuffer[256];
//   serializeJson(responseJson, responseBuffer);
//   client.publish("esp32/status", responseBuffer);
// }

// void reconnectMQTT() {
//   while (!client.connected()) {
//     Serial.println("Attempting MQTT connection...");
//     String clientId = "ESP32Client-" + String(random(0xffff), HEX);
//     if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
//       Serial.println("Connected to MQTT broker");
//         client.subscribe("esp32/command");
//       client.subscribe("esp32/config");
//     } else {
//       Serial.print("Failed to connect to MQTT. Retrying...");
//       Serial.print(" failed, rc=");
//       Serial.println(client.state());
//       delay(6000);
//     }
//   }
// }

// #include <WiFi.h>            // Include the Wi-Fi library for ESP32
// #include <PubSubClient.h>    // Include the PubSubClient library for MQTT
// #include <ArduinoJson.h>     // Include the ArduinoJson library for JSON parsing
// #include <Preferences.h>     // Include the Preferences library for persistent storage
// #include <WebServer.h>       // Include the WebServer library for HTTP server functionality

// #define PIN23 23  // Define GPIO pin 23 for input (reset)
// #define PIN32 32  // Define GPIO pin 32 for output

// // Default configuration values
// const char* default_ssid = "ESP32-Access-Point";
// const char* default_password = "123456789";
// // const char* ssid = "Providence";
// // const char* password = "Provider123";
// const char* default_mqtt_server = "127.0.0.1";//"192.168.0.148";
// const char* default_mqtt_username = "testuser";
// const char* default_mqtt_password = "password";
// const int default_mqtt_port = 1883;

// // Set a static IP address, subnet, and gateway for the Access Point
// IPAddress local_IP(192, 168, 4, 1);  // Static IP for the ESP32 in AP mode
// IPAddress gateway(192, 168, 4, 1);   // Gateway (usually same as the static IP)
// IPAddress subnet(255, 255, 255, 0);  // Subnet mask

// // Global variables
// // char ssid[32];
// // char password[32];
// // Modifiable global variables
// char ssid[32] = "Providence";  // Initial value
// char password[32] = "Provider123";
// char mqtt_server[32];
// char mqtt_username[32];
// char mqtt_password[32];
// int mqtt_port;

// WiFiClient espClient;
// PubSubClient client(espClient);
// Preferences preferences;  // For storing configuration persistently
// WebServer server(80);     // Initialize WebServer on port 80

// void setup() {
//   Serial.begin(9600);
//   pinMode(PIN32, OUTPUT);
//   pinMode(PIN23, INPUT_PULLUP); // Configure PIN23 with internal pull-up resistor

//   // Load configuration from NVS
//   preferences.begin("config", false);
//   loadConfiguration();

//   // Start the Wi-Fi Access Point or Connect to Wi-Fi
//   connectToWiFi();

//   // Setup MQTT client
//   client.setServer(mqtt_server, mqtt_port);
//   client.setCallback(mqttCallback);

//   // Define routes for WebServer
//   server.on("/", handleRoot);
//   server.on("/action", HTTP_GET, handleAction);  // URL: /action?action=ON

//   // Start WebServer
//   server.begin();
// }

// void loop() {
//   // Handle reset if PIN23 is triggered
//   handleReset();

//   // Maintain MQTT connection
//   if (!client.connected()) {
//     reconnectMQTT();
//   }
//   client.loop();

//   // Handle incoming HTTP requests
//   server.handleClient();

//   delay(5000);  // Adjust delay as necessary
// }

// // Helper functions

// void loadConfiguration() {
//   // Load configuration from preferences or use defaults
//   if (preferences.getString("ssid", "").isEmpty()) {
//     Serial.println("No saved configuration found. Using default settings...");
//     setDefaultConfiguration();
//   } else {
//     strncpy(ssid, preferences.getString("ssid").c_str(), sizeof(ssid) - 1);
//     strncpy(password, preferences.getString("password").c_str(), sizeof(password) - 1);
//     strncpy(mqtt_server, preferences.getString("mqtt_server").c_str(), sizeof(mqtt_server) - 1);
//     strncpy(mqtt_username, preferences.getString("mqtt_username").c_str(), sizeof(mqtt_username) - 1);
//     strncpy(mqtt_password, preferences.getString("mqtt_password").c_str(), sizeof(mqtt_password) - 1);
//     mqtt_port = preferences.getInt("mqtt_port", default_mqtt_port);
//   }
  

//   preferences.end();  // Always end Preferences after use
// }

// void setDefaultConfiguration() {
//   // strncpy(ssid, default_ssid, sizeof(ssid));
//   // strncpy(password, default_password, sizeof(password));
//   // strncpy(mqtt_server, default_mqtt_server, sizeof(mqtt_server));
//   // strncpy(mqtt_username, default_mqtt_username, sizeof(mqtt_username));
//   // strncpy(mqtt_password, default_mqtt_password, sizeof(mqtt_password));
//   // mqtt_port = default_mqtt_port;
//   strncpy(ssid, default_ssid, sizeof(ssid) - 1);
//   strncpy(password, default_password, sizeof(password) - 1);
//   strncpy(mqtt_server, default_mqtt_server, sizeof(mqtt_server) - 1);
//   strncpy(mqtt_username, default_mqtt_username, sizeof(mqtt_username) - 1);
//   strncpy(mqtt_password, default_mqtt_password, sizeof(mqtt_password) - 1);
//   mqtt_port = default_mqtt_port;

// }

// void connectToWiFi() {
//   // Start the Wi-Fi Access Point or connect to a Wi-Fi network
//   if (WiFi.softAP(ssid, password)) {
//     startAccessPoint();
//   } else {
//     WiFi.begin(ssid, password); // Connect to Wi-Fi
//     while (WiFi.status() != WL_CONNECTED) {
//       delay(5000);
//       Serial.print(".");
//     }
//     Serial.println("Connected to Wi-Fi");
//   }
// }

// void startAccessPoint() {
//   // Set up the Access Point with a static IP address
//   WiFi.softAPConfig(local_IP, gateway, subnet);
//   WiFi.softAP(ssid, password);

//   // Print the IP address of the ESP32 in AP mode
//   Serial.println("Access Point Started");
//   Serial.print("ESP32 IP Address: ");
//   Serial.println(WiFi.softAPIP());  // This will display the static IP (192.168.4.1)
// }

// void handleReset() {
//   if (digitalRead(PIN23) == LOW) {
//     Serial.println("Reset triggered. Clearing configuration...");
//     preferences.clear();
//     preferences.end();
//     ESP.restart(); // Restart ESP32
//   }
// }

// void handleRoot() {
//   server.send(200, "text/html", "<h1>ESP32 Configuration Page</h1>");
// }

// void handleAction() {
//   String action = server.arg("action"); // Get the 'action' parameter from the URL
//   if (action == "ON") {
//     digitalWrite(PIN32, HIGH);  // Set GPIO to HIGH
//     server.send(200, "text/plain", "GPIO 32 set to HIGH");
//   } else if (action == "OFF") {
//     digitalWrite(PIN32, LOW);   // Set GPIO to LOW
//     server.send(200, "text/plain", "GPIO 32 set to LOW");
//   } else {
//     server.send(400, "text/plain", "Invalid action");
//   }
// }

// void mqttCallback(char* topic, byte* payload, unsigned int length) {
//   Serial.print("Message arrived on topic: ");
//   Serial.println(topic);

//   char buffer[length + 1];
//   memcpy(buffer, payload, length);
//   buffer[length] = '\0';

//   StaticJsonDocument<256> jsonDoc;
//   if (deserializeJson(jsonDoc, buffer)) {
//     Serial.println("Invalid JSON received");
//     return;
//   }

//   // Handle configuration update
//   if (strcmp(topic, "esp32/config") == 0) {
//     handleConfigurationUpdate(jsonDoc);
//   }

//   // Handle action commands
//   if (jsonDoc.containsKey("action")) {
//     handleActionCommand(jsonDoc["action"]);
//   }
// }

// void handleConfigurationUpdate(const StaticJsonDocument<256>& jsonDoc) {
//   strncpy(ssid, jsonDoc["ssid"] | ssid, sizeof(ssid));
//   strncpy(password, jsonDoc["password"] | password, sizeof(password));
//   strncpy(mqtt_server, jsonDoc["mqtt_server"] | mqtt_server, sizeof(mqtt_server));
//   strncpy(mqtt_username, jsonDoc["mqtt_username"] | mqtt_username, sizeof(mqtt_username));
//   strncpy(mqtt_password, jsonDoc["mqtt_password"] | mqtt_password, sizeof(mqtt_password));
//   mqtt_port = jsonDoc["mqtt_port"] | mqtt_port;

//   saveConfiguration();
//   Serial.println("Configuration updated and saved");

//   // Restart to apply new configuration
//   ESP.restart();
// }

// void saveConfiguration() {
//   preferences.putString("ssid", ssid);
//   preferences.putString("password", password);
//   preferences.putString("mqtt_server", mqtt_server);
//   preferences.putString("mqtt_username", mqtt_username);
//   preferences.putString("mqtt_password", mqtt_password);
//   preferences.putInt("mqtt_port", mqtt_port);
//   preferences.end();
// }

// void handleActionCommand(const char* action) {
//   StaticJsonDocument<200> responseJson;

//   if (action == nullptr) {
//     StaticJsonDocument<200> errorJson;
//     errorJson["status"] = "error";
//     errorJson["message"] = "Missing 'action' field in JSON";

//     char errorBuffer[256];
//     serializeJson(errorJson, errorBuffer);
//     client.publish("esp32/status", errorBuffer);

//     Serial.println("Missing 'action' field in JSON");
//     return;
//   }

//   if (strcmp(action, "ON") == 0) {
//     digitalWrite(PIN32, HIGH);
//     responseJson["status"] = "success";
//     responseJson["state"] = "ON";
//     responseJson["message"] = "GPIO 32 set to HIGH";
//     Serial.println("GPIO 32 set to HIGH");
//   } else if (strcmp(action, "OFF") == 0) {
//     digitalWrite(PIN32, LOW);
//     responseJson["status"] = "success";
//     responseJson["state"] = "OFF";
//     responseJson["message"] = "GPIO 32 set to LOW";
//     Serial.println("GPIO 32 set to LOW");
//   } else {
//     responseJson["status"] = "error";
//     responseJson["message"] = "Invalid action received";
//     Serial.println("Invalid action received");
//   }

//   char responseBuffer[256];
//   serializeJson(responseJson, responseBuffer);
//   client.publish("esp32/status", responseBuffer);
// }

// void reconnectMQTT() {
//   while (!client.connected()) {
//     Serial.println("Attempting MQTT connection...");
//     String clientId = "ESP32Client-" + String(random(0xffff), HEX);
//     if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
//       Serial.println("Connected to MQTT broker");
//       client.subscribe("esp32/command");
//       client.subscribe("esp32/config");
//     } else {
//       Serial.print("Failed to connect to MQTT. Retrying...");
//       Serial.print(" failed, rc=");
//       Serial.println(client.state());
//       delay(6000);
//     }
//   }
// }



#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <WebServer.h>

// GPIO Pins
#define PIN23 23
#define PIN32 32

// Default Configuration
const char* DEFAULT_SSID = "ESP32-Access-Point";
const char* DEFAULT_PASSWORD = "123456789";
const char* DEFAULT_MQTT_SERVER = "192.168.0.148";//127.0.0.1";
const char* DEFAULT_MQTT_USERNAME = "testuser";
const char* DEFAULT_MQTT_PASSWORD = "password";
const int DEFAULT_MQTT_PORT = 1883;

// Global Variables
// char ssid[32];
// char password[32];
char ssid[32] = "Providence";  // Initial value
char password[32] = "Provider123";
char mqtt_server[32];
char mqtt_username[32];
char mqtt_password[32];
int mqtt_port;

WiFiClient espClient;
PubSubClient mqttClient(espClient);
Preferences preferences;
WebServer server(80);

void setup() {
  Serial.begin(115200);
  pinMode(PIN32, OUTPUT);
  pinMode(PIN23, INPUT_PULLUP);

  // Initialize Preferences and Load Configuration
  preferences.begin("config", false);
  loadConfiguration();

  // Initialize Wi-Fi in Dual Mode (AP + STA)
  setupWiFiDualMode();

  // Initialize MQTT and HTTP Server
  setupMQTT();
  setupHTTPRoutes();

  server.begin();
  Serial.println("HTTP server started.");
}

void loop() {
  if (!mqttClient.connected()) reconnectMQTT();
  mqttClient.loop();
  server.handleClient();
  handleReset();
}

// Load Configuration
void loadConfiguration() {
  strncpy(ssid, preferences.getString("ssid", DEFAULT_SSID).c_str(), sizeof(ssid) - 1);
  strncpy(password, preferences.getString("password", DEFAULT_PASSWORD).c_str(), sizeof(password) - 1);
  strncpy(mqtt_server, preferences.getString("mqtt_server", DEFAULT_MQTT_SERVER).c_str(), sizeof(mqtt_server) - 1);
  strncpy(mqtt_username, preferences.getString("mqtt_username", DEFAULT_MQTT_USERNAME).c_str(), sizeof(mqtt_username) - 1);
  strncpy(mqtt_password, preferences.getString("mqtt_password", DEFAULT_MQTT_PASSWORD).c_str(), sizeof(mqtt_password) - 1);
  mqtt_port = preferences.getInt("mqtt_port", DEFAULT_MQTT_PORT);
  Serial.println("Configuration loaded.");
}

// Save Configuration
void saveConfiguration() {
  preferences.putString("ssid", ssid);
  preferences.putString("password", password);
  preferences.putString("mqtt_server", mqtt_server);
  preferences.putString("mqtt_username", mqtt_username);
  preferences.putString("mqtt_password", mqtt_password);
  preferences.putInt("mqtt_port", mqtt_port);
  Serial.println("Configuration saved.");
}

// Wi-Fi Dual Mode Setup
void setupWiFiDualMode() {
  WiFi.mode(WIFI_AP_STA);  // Dual Mode
  WiFi.begin(ssid, password);

  Serial.println("Connecting to Wi-Fi...");
  unsigned long startAttemptTime = millis();
  const unsigned long timeout = 20000;  // 20 seconds

  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < timeout) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to Wi-Fi:");
    Serial.println("IP Address (STA): " + WiFi.localIP().toString());
  } else {
    Serial.println("\nFailed to connect to Wi-Fi. Setting up Access Point...");
    WiFi.softAP(DEFAULT_SSID, DEFAULT_PASSWORD);
    Serial.println("Access Point IP Address: " + WiFi.softAPIP().toString());
  }
}

// MQTT Setup and Reconnection
void setupMQTT() {
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(mqttCallback);
}

void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.println("Attempting MQTT connection...");
    String clientId = "ESP32-" + String(random(0xffff), HEX);
    if (mqttClient.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("MQTT connected.");
      mqttClient.subscribe("esp32/command");
      mqttClient.subscribe("esp32/config");
    } else {
      Serial.print("MQTT connection failed, rc=");
      Serial.println(mqttClient.state());
      Serial.println("Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

// MQTT Callback
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  char buffer[length + 1];
  memcpy(buffer, payload, length);
  buffer[length] = '\0';

  StaticJsonDocument<256> jsonDoc;
  if (deserializeJson(jsonDoc, buffer)) {
    Serial.println("Invalid JSON received.");
    return;
  }

  if (strcmp(topic, "esp32/command") == 0) {
    handleActionCommand(jsonDoc["action"]);
  } else if (strcmp(topic, "esp32/config") == 0) {
    handleConfigurationUpdate(jsonDoc);
  }
}

// Handle Action Commands
void handleActionCommand(const char* action) {
  StaticJsonDocument<200> responseJson;

  if (!action) {
    createErrorResponse(responseJson, "Missing 'action' field");
  } else if (strcmp(action, "ON") == 0) {
    digitalWrite(PIN32, HIGH);
    createSuccessResponse(responseJson, "ON", "GPIO 32 set to HIGH");
  } else if (strcmp(action, "OFF") == 0) {
    digitalWrite(PIN32, LOW);
    createSuccessResponse(responseJson, "OFF", "GPIO 32 set to LOW");
  } else {
    createErrorResponse(responseJson, "Invalid action received");
  }

  char responseBuffer[256];
  serializeJson(responseJson, responseBuffer);
  mqttClient.publish("esp32/status", responseBuffer);
}

// Handle Configuration Update
void handleConfigurationUpdate(const JsonDocument& jsonDoc) {
  if (jsonDoc.containsKey("ssid")) {
    strncpy(ssid, jsonDoc["ssid"], sizeof(ssid) - 1);
  }
  if (jsonDoc.containsKey("password")) {
    strncpy(password, jsonDoc["password"], sizeof(password) - 1);
  }
  if (jsonDoc.containsKey("mqtt_server")) {
    strncpy(mqtt_server, jsonDoc["mqtt_server"], sizeof(mqtt_server) - 1);
  }
  if (jsonDoc.containsKey("mqtt_username")) {
    strncpy(mqtt_username, jsonDoc["mqtt_username"], sizeof(mqtt_username) - 1);
  }
  if (jsonDoc.containsKey("mqtt_password")) {
    strncpy(mqtt_password, jsonDoc["mqtt_password"], sizeof(mqtt_password) - 1);
  }
  if (jsonDoc.containsKey("mqtt_port")) {
    mqtt_port = jsonDoc["mqtt_port"];
  }

  saveConfiguration();
  ESP.restart();
}

// HTTP Routes
void setupHTTPRoutes() {
  server.on("/", []() { server.send(200, "text/html", "ESP32 is running."); });
  server.on("/action", HTTP_POST, []() {
    StaticJsonDocument<256> jsonDoc;
    if (deserializeJson(jsonDoc, server.arg("plain"))) {
      server.send(400, "application/json", "{\"status\":\"error\"}");
    } else {
      handleActionCommand(jsonDoc["action"]);
      server.send(200, "application/json", "{\"status\":\"success\"}");
    }
  });
}

// Reset Handling
void handleReset() {
  if (digitalRead(PIN23) == LOW) {
    preferences.clear();
    ESP.restart();
  }
}

// Helper Functions
void createErrorResponse(JsonDocument& json, const char* message) {
  json["status"] = "error";
  json["message"] = message;
}

void createSuccessResponse(JsonDocument& json, const char* state, const char* message) {
  json["status"] = "success";
  json["state"] = state;
  json["message"] = message;
}
