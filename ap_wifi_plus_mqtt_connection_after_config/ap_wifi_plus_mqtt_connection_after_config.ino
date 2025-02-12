#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <WebServer.h>

// GPIO Pins
#define RESET_PIN 23
#define OUTPUT_PIN 32

// Default Configuration
constexpr char DEFAULT_SSID[] = "";
constexpr char DEFAULT_PASSWORD[] = "";
constexpr char DEFAULT_MQTT_SERVER[] = "192.168.0.148";
constexpr char DEFAULT_MQTT_USERNAME[] = "testuser";
constexpr char DEFAULT_MQTT_PASSWORD[] = "password";
constexpr int DEFAULT_MQTT_PORT = 1883;

// Global Variables
char ssid[32] = "";
char password[32] = "";
char mqtt_server[32] = "";
char mqtt_username[32] = "";
char mqtt_password[32] = "";
int mqtt_port = DEFAULT_MQTT_PORT;

WiFiClient espClient;
PubSubClient mqttClient(espClient);
Preferences preferences;
WebServer server(80);

// Function Prototypes
void setupHTTPRoutes();
void handleReset();
void handleConfigurationUpdate(const StaticJsonDocument<256>& jsonDoc);
void loadConfiguration();
void saveConfiguration();
void setupWiFiAP();
void connectToWiFi();
void setupMQTT();
void reconnectMQTT();
void mqttCallback(char* topic, byte* payload, unsigned int length);
void createErrorResponse(StaticJsonDocument<200>& responseJson, const char* message);
void createSuccessResponse(StaticJsonDocument<200>& responseJson, const char* state, const char* message);

void setup() {
    Serial.begin(9600);
    pinMode(OUTPUT_PIN, OUTPUT);
    pinMode(RESET_PIN, INPUT_PULLUP);

    preferences.begin("config", false);
    loadConfiguration();

    ssid[0] == '\0' || password[0] == '\0' ? setupWiFiAP() : connectToWiFi();

    setupMQTT(); // Initialize MQTT connection with loaded configuration

    setupHTTPRoutes();
    server.begin();
    Serial.println("HTTP server started.");
}

void loop() {
    if (WiFi.status() == WL_CONNECTED) {
        if (!mqttClient.connected()) reconnectMQTT();
        mqttClient.loop();
    }
    server.handleClient();
    handleReset();
}

void loadConfiguration() {
    // Load configuration from preferences
    strncpy(ssid, preferences.getString("ssid", DEFAULT_SSID).c_str(), sizeof(ssid) - 1);
    ssid[sizeof(ssid) - 1] = '\0';
    strncpy(password, preferences.getString("password", DEFAULT_PASSWORD).c_str(), sizeof(password) - 1);
    password[sizeof(password) - 1] = '\0';
    strncpy(mqtt_server, preferences.getString("mqtt_server", DEFAULT_MQTT_SERVER).c_str(), sizeof(mqtt_server) - 1);
    mqtt_server[sizeof(mqtt_server) - 1] = '\0';
    strncpy(mqtt_username, preferences.getString("mqtt_username", DEFAULT_MQTT_USERNAME).c_str(), sizeof(mqtt_username) - 1);
    mqtt_username[sizeof(mqtt_username) - 1] = '\0';
    strncpy(mqtt_password, preferences.getString("mqtt_password", DEFAULT_MQTT_PASSWORD).c_str(), sizeof(mqtt_password) - 1);
    mqtt_password[sizeof(mqtt_password) - 1] = '\0';
    mqtt_port = preferences.getInt("mqtt_port", DEFAULT_MQTT_PORT);

    // Print the current configuration
    Serial.println("Configuration loaded:");
    Serial.printf("SSID: %s\nPassword: %s\nMQTT Server: %s\nMQTT Username: %s\nMQTT Password: %s\nMQTT Port: %d\n",
                  ssid, password, mqtt_server, mqtt_username, mqtt_password, mqtt_port);
}

void saveConfiguration() {
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    preferences.putString("mqtt_server", mqtt_server);
    preferences.putString("mqtt_username", mqtt_username);
    preferences.putString("mqtt_password", mqtt_password);
    preferences.putInt("mqtt_port", mqtt_port);
    Serial.println("Configuration saved.");
}

void setupWiFiAP() {
    WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
    WiFi.softAP("ESP32_Config", "12345678");
    Serial.printf("Started Wi-Fi AP:\nSSID: ESP32_Config, IP Address: %s\n", WiFi.softAPIP().toString().c_str());
}

void connectToWiFi() {
    WiFi.begin(ssid, password);
    Serial.println("Connecting to Wi-Fi...");
    unsigned long timeout = millis() + 20000;
    int attempts = 0;  // Counter for attempts
    
    while (WiFi.status() != WL_CONNECTED && millis() < timeout) {
        delay(500);
        Serial.print(".");
        attempts++;  // Increment attempt counter

// TODO:
// Remove the code below for security purpose, implement button for reset instead
         // If disconnected due to NO_AP_FOUND, break early
        if (WiFi.status() == WL_DISCONNECTED && attempts >= 20) {
                Serial.println("\nNo Access Point Found. Switching to AP mode...");
                setupWiFiAP();
                return;
        }
        
        // If 20 attempts are made and still not connected, switch to AP mode
        if (attempts >= 20) {
            Serial.println("\nFailed to connect after 20 attempts. Switching to AP mode...");
            setupWiFiAP();  // Switch to Access Point mode
            return;
        }

        // Remove the code above for security purpose, implement button for reset instead
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("\nConnected to Wi-Fi:\nIP Address: %s\n", WiFi.localIP().toString().c_str());
    } else {
        Serial.println("\nFailed to connect to Wi-Fi.");
        setupWiFiAP();  // Switch to Access Point mode if still not connected
    }
}


void setupMQTT() {
    mqttClient.setServer(mqtt_server, mqtt_port);
    mqttClient.setCallback(mqttCallback);
}

void reconnectMQTT() {
    while (!mqttClient.connected()) {
        Serial.println("Attempting MQTT connection...");
        if (mqttClient.connect("ESP32_Client", mqtt_username, mqtt_password)) {
            Serial.println("MQTT connected.");
            mqttClient.subscribe("esp32/command");
            mqttClient.subscribe("esp32/config");
        } else {
            Serial.printf("MQTT connection failed, rc=%d. Retrying in 5 seconds...\n", mqttClient.state());
            delay(5000);
        }
    }
}

void setupHTTPRoutes() {
  server.on("/", HTTP_GET, []() {
        StaticJsonDocument<200> responseJson;
        createSuccessResponse(responseJson, "running", "ESP32 Web Server is running.");
        String response;
        serializeJson(responseJson, response);
        server.send(200, "application/json", response);
    });

  server.on("/config", HTTP_GET, []() {
    // Define required fields
    const char* requiredFields[] = {"ssid", "password", "mqtt_server", "mqtt_port", "mqtt_username", "mqtt_password"};
    bool allFieldsPresent = true;
    String missingFields;

    // Check if all required fields are present
    for (const char* field : requiredFields) {
        if (!server.hasArg(field)) {
            allFieldsPresent = false;
            missingFields += String(field) + ", ";
        }
    }

    if (!allFieldsPresent) {
        // Remove trailing comma and space
        if (missingFields.length() > 2) {
            missingFields.remove(missingFields.length() - 2);
        }

        StaticJsonDocument<200> responseJson;
        Serial.println("Configuration Missing fields: " + missingFields);
        // createErrorResponse(responseJson, "Missing fields: " + missingFields);
        createErrorResponse(responseJson, "Configuration is missing some fields.");
        String responseString;
        serializeJson(responseJson, responseString);
        server.send(400, "application/json", responseString);
        return;
    }

    // Map query parameters to the JSON document
    StaticJsonDocument<256> jsonDoc;
    jsonDoc["ssid"] = server.arg("ssid");
    jsonDoc["password"] = server.arg("password");
    jsonDoc["mqtt_server"] = server.arg("mqtt_server");
    jsonDoc["mqtt_port"] = server.arg("mqtt_port").toInt();
    jsonDoc["mqtt_username"] = server.arg("mqtt_username");
    jsonDoc["mqtt_password"] = server.arg("mqtt_password");

    // Validate the JSON structure
    handleConfigurationUpdate(jsonDoc);

    StaticJsonDocument<200> responseJson;
    createSuccessResponse(responseJson, "updated", "Configuration updated. Rebooting...");
    String responseString;
    serializeJson(responseJson, responseString);
    server.send(200, "application/json", responseString);

    delay(1000);
    ESP.restart();
  });

  server.on("/update_config", HTTP_POST, []() {
        if (server.hasArg("plain")) {
            StaticJsonDocument<256> jsonDoc;
            DeserializationError error = deserializeJson(jsonDoc, server.arg("plain"));
            if (error) {
                StaticJsonDocument<200> responseJson;
                createErrorResponse(responseJson, "Invalid JSON");
                String response;
                serializeJson(responseJson, response);
                server.send(400, "application/json", response);
                return;
            }
            handleConfigurationUpdate(jsonDoc);
            StaticJsonDocument<200> responseJson;
            createSuccessResponse(responseJson, "updated", "Configuration updated. Rebooting...");
            String response;
            serializeJson(responseJson, response);
            server.send(200, "application/json", response);
            delay(1000);
            ESP.restart();
        } else {
            StaticJsonDocument<200> responseJson;
            createErrorResponse(responseJson, "No data received");
            String response;
            serializeJson(responseJson, response);
            server.send(400, "application/json", response);
        }
  });

}

void createErrorResponse(StaticJsonDocument<200>& responseJson, const char* message) {
    responseJson["status"] = "error";
    responseJson["message"] = message;
}

void createSuccessResponse(StaticJsonDocument<200>& responseJson, const char* state, const char* message) {
    responseJson["status"] = "success";
    responseJson["state"] = state;
    responseJson["message"] = message;
}

void handleConfigurationUpdate(const StaticJsonDocument<256>& jsonDoc) {
    if (jsonDoc.containsKey("ssid")) strncpy(ssid, jsonDoc["ssid"], sizeof(ssid) - 1);
    if (jsonDoc.containsKey("password")) strncpy(password, jsonDoc["password"], sizeof(password) - 1);
    if (jsonDoc.containsKey("mqtt_server")) strncpy(mqtt_server, jsonDoc["mqtt_server"], sizeof(mqtt_server) - 1);
    if (jsonDoc.containsKey("mqtt_username")) strncpy(mqtt_username, jsonDoc["mqtt_username"], sizeof(mqtt_username) - 1);
    if (jsonDoc.containsKey("mqtt_password")) strncpy(mqtt_password, jsonDoc["mqtt_password"], sizeof(mqtt_password) - 1);
    if (jsonDoc.containsKey("mqtt_port")) mqtt_port = jsonDoc["mqtt_port"];
    saveConfiguration();
    setupMQTT(); // Reinitialize MQTT with updated configuration
}

void handleReset() {
    if (digitalRead(RESET_PIN) == LOW) {
        Serial.println("Resetting configuration...");
        preferences.clear();
        ESP.restart();
    }
}

// MQTT callback function to handle incoming messages
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");

  // Copy and null-terminate the payload
  char buffer[length + 1];
  memcpy(buffer, payload, length);
  buffer[length] = '\0';
  Serial.println(buffer);

  // Parse the JSON payload
  StaticJsonDocument<256> jsonDoc;
  DeserializationError error = deserializeJson(jsonDoc, buffer);

  if (error) {
    Serial.print("Failed to parse JSON: ");
    Serial.println(error.c_str());
    return;
  }

  // Handle configuration updates
  if (strcmp(topic, "esp32/config") == 0) {
    // Validate the JSON structure
    handleConfigurationUpdate(jsonDoc);
    delay(1000);
    ESP.restart();
  } 
  // Handle GPIO commands
  else if (strcmp(topic, "esp32/command") == 0) {
    const char* action = jsonDoc["action"];

    if (action) {
      StaticJsonDocument<200> responseJson;

      if (strcmp(action, "ON") == 0) {
        digitalWrite(OUTPUT_PIN, HIGH); // Set GPIO pin to HIGH
        responseJson["status"] = "success";
        responseJson["state"] = "ON";
        responseJson["message"] = "GPIO set to HIGH";
        Serial.println("GPIO set to HIGH");
      } else if (strcmp(action, "OFF") == 0) {
        digitalWrite(OUTPUT_PIN, LOW); // Set GPIO pin to LOW
        responseJson["status"] = "success";
        responseJson["state"] = "OFF";
        responseJson["message"] = "GPIO set to LOW";
        Serial.println("GPIO set to LOW");
      } else {
        responseJson["status"] = "error";
        responseJson["message"] = "Invalid action received";
        Serial.println("Invalid action received");
      }

      // Serialize and publish the JSON response
      char responseBuffer[256];
      serializeJson(responseJson, responseBuffer);
      mqttClient.publish("esp32/status", responseBuffer);
    } else {
      StaticJsonDocument<200> errorJson;
      errorJson["status"] = "error";
      errorJson["message"] = "Missing 'action' field in JSON";

      char errorBuffer[256];
      serializeJson(errorJson, errorBuffer);
      mqttClient.publish("esp32/status", errorBuffer);

      Serial.println("Missing 'action' field in JSON");
    }
  }
}


