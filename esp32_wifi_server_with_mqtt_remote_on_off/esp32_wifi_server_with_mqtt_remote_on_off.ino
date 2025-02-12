#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <WebServer.h>

// GPIO Pins
#define RESET_PIN 23
#define OUTPUT_PIN 32

// Default Configuration
constexpr char DEFAULT_SSID[] = "";//"ProvidenceTest";
constexpr char DEFAULT_PASSWORD[] ="";// "Provider123";
constexpr char DEFAULT_MQTT_SERVER[] = "192.168.0.148";
constexpr char DEFAULT_MQTT_USERNAME[] = "testuser";
constexpr char DEFAULT_MQTT_PASSWORD[] = "password";  
constexpr int DEFAULT_MQTT_PORT = 1883;

// Global Variables
// MTN_4G_E26E87
// 
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

    if (ssid[0] == '\0' || password[0] == '\0') {
    setupWiFiAP(); // Start in configuration mode
      } else {
    connectToWiFi(); // Connect using stored credentials
    }

    setupHTTPRoutes();
    server.begin();
    Serial.println("HTTP server started.");
}

void loop() {
    // Serial.printf("WiFi connection status: %d\n", WiFi.status());
    if (WiFi.status() == WL_CONNECTED) {
        if (!mqttClient.connected()) reconnectMQTT();
        mqttClient.loop();
    }
    server.handleClient();
    handleReset();
}

void loadConfiguration() {
    // Load configuration from preferences
    strncpy(ssid, preferences.getString("ssid", "").c_str(), sizeof(ssid) - 1);
    strncpy(password, preferences.getString("password", "").c_str(), sizeof(password) - 1);
    strncpy(mqtt_server, preferences.getString("mqtt_server", DEFAULT_MQTT_SERVER).c_str(), sizeof(mqtt_server) - 1);
    strncpy(mqtt_username, preferences.getString("mqtt_username", DEFAULT_MQTT_USERNAME).c_str(), sizeof(mqtt_username) - 1);
    strncpy(mqtt_password, preferences.getString("mqtt_password", DEFAULT_MQTT_PASSWORD).c_str(), sizeof(mqtt_password) - 1);
    mqtt_port = preferences.getInt("mqtt_port", DEFAULT_MQTT_PORT);

    // Print the current configuration to the console
    Serial.println("Configuration loaded:");
    Serial.print("SSID: ");
    Serial.println(ssid);
    Serial.print("Password: ");
    Serial.println(password);
    Serial.print("MQTT Server: ");
    Serial.println(mqtt_server);
    Serial.print("MQTT Username: ");
    Serial.println(mqtt_username);
    Serial.print("MQTT Password: ");
    Serial.println(mqtt_password);
    Serial.print("MQTT Port: ");
    Serial.println(mqtt_port);

    Serial.println("Configuration loaded.");
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

// Wi-Fi Access Point Mode Setup
void setupWiFiAP() {
    IPAddress apIP(192, 168, 4, 1);       // Static IP for the AP
    IPAddress gateway(192, 168, 4, 1);   // Gateway (same as AP IP)
    IPAddress subnet(255, 255, 255, 0);  // Subnet mask

    // Configure the static IP for the AP mode
    WiFi.softAPConfig(apIP, gateway, subnet);

    // Start the Access Point
    WiFi.mode(WIFI_AP);
    WiFi.softAP("ESP32_Config", "12345678");

    Serial.println("Started Wi-Fi AP for configuration:");
    Serial.print("SSID: ESP32_Config, IP Address: ");
    Serial.println(WiFi.softAPIP());  // Print the static IP address
}

void connectToWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    Serial.println("Connecting to Wi-Fi...");
    const unsigned long timeout = 20000; // 20 seconds timeout
    unsigned long startAttemptTime = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < timeout) {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected to Wi-Fi:");
        Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
    } else {
        Serial.println("\nFailed to connect to Wi-Fi.");
        setupWiFiAP();
    }
}

void setupMQTT() {
    mqttClient.setServer(mqtt_server, mqtt_port);
    mqttClient.setCallback(mqttCallback);
}

void reconnectMQTT() {
    // Retrieve MQTT credentials from preferences
    preferences.begin("mqtt", true); // Open preferences in read-only mode
    String mqtt_server = preferences.getString("mqtt_server", "");
    String mqtt_username = preferences.getString("mqtt_username", "");
    String mqtt_password = preferences.getString("mqtt_password", "");
    int mqtt_port = preferences.getInt("mqtt_port", 1883); // Default to port 1883
    preferences.end(); // Close preferences

    mqttClient.setServer(mqtt_server.c_str(), mqtt_port);
    mqttClient.setCallback(mqttCallback);

    while (!mqttClient.connected()) {
        Serial.println("Attempting MQTT connection...");
        // Generate a unique client ID
        String clientId = "ESP32-" + String(random(0xffff), HEX);

        // Attempt to connect
        if (mqttClient.connect(clientId.c_str(), mqtt_username.c_str(), mqtt_password.c_str())) {
            Serial.println("MQTT connected.");
            // Subscribe to topics
            mqttClient.subscribe("esp32/command");
            mqttClient.subscribe("esp32/config");
        } else {
            Serial.printf("MQTT connection failed, rc=%d. Retrying in 5 seconds...\n", mqttClient.state());
            delay(5000); // Wait before retrying
        }
    }
}



void setupHTTPRoutes() {
    server.on("/", HTTP_GET, []() {
        StaticJsonDocument<200> responseJson;
        createSuccessResponse(responseJson, "running", "ESP32 Web Server is running.");
        String responseString;
        serializeJson(responseJson, responseString);
        server.send(200, "application/json", responseString);
    });

    server.on("/config_with_payload", HTTP_POST, []() {
        if (server.hasArg("plain")) {
            StaticJsonDocument<256> jsonDoc;
            DeserializationError error = deserializeJson(jsonDoc, server.arg("plain"));

            if (error) {
                StaticJsonDocument<200> responseJson;
                createErrorResponse(responseJson, "Invalid JSON");
                String responseString;
                serializeJson(responseJson, responseString);
                server.send(400, "application/json", responseString);
                return;
            }

            handleConfigurationUpdate(jsonDoc);
            StaticJsonDocument<200> responseJson;
            createSuccessResponse(responseJson, "updated", "Configuration updated. Rebooting...");
            String responseString;
            serializeJson(responseJson, responseString);
            server.send(200, "application/json", responseString);
            delay(1000);
            ESP.restart();
        } else {
            StaticJsonDocument<200> responseJson;
            createErrorResponse(responseJson, "No data received");
            String responseString;
            serializeJson(responseJson, responseString);
            server.send(400, "application/json", responseString);
        }
    });

    // server.on("/config", HTTP_GET, []() {
    // if (server.args() > 0) {
    //     StaticJsonDocument<256> jsonDoc;

    //     // Map query parameters to the JSON document
    //     if (server.hasArg("ssid")) jsonDoc["ssid"] = server.arg("ssid");
    //     if (server.hasArg("password")) jsonDoc["password"] = server.arg("password");
    //     if (server.hasArg("mqtt_server")) jsonDoc["mqtt_server"] = server.arg("mqtt_server");
    //     if (server.hasArg("mqtt_port")) jsonDoc["mqtt_port"] = server.arg("mqtt_port").toInt();
    //     if (server.hasArg("mqtt_username")) jsonDoc["mqtt_username"] = server.arg("mqtt_username");
    //     if (server.hasArg("mqtt_password")) jsonDoc["mqtt_password"] = server.arg("mqtt_password");

    //     // Validate the JSON structure
    //     handleConfigurationUpdate(jsonDoc);

    //     StaticJsonDocument<200> responseJson;
    //     createSuccessResponse(responseJson, "updated", "Configuration updated. Rebooting...");
    //     String responseString;
    //     serializeJson(responseJson, responseString);
    //     server.send(200, "application/json", responseString);

    //     delay(1000);
    //     ESP.restart();
    // } else {
    //     StaticJsonDocument<200> responseJson;
    //     createErrorResponse(responseJson, "No query parameters received");
    //     String responseString;
    //     serializeJson(responseJson, responseString);
    //     server.send(400, "application/json", responseString);
    // }
    
// });

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

server.on("/current_config_former", HTTP_GET, []() {
    StaticJsonDocument<256> responseJson;
    responseJson["ssid"] = ssid;
    responseJson["password"] = password;
    responseJson["mqtt_server"] = mqtt_server;
    responseJson["mqtt_port"] = mqtt_port;
    responseJson["mqtt_username"] = mqtt_username;

    String responseString;
    serializeJson(responseJson, responseString);
    server.send(200, "application/json", responseString);
});

server.on("/current_config", HTTP_GET, []() {
    // Begin accessing preferences in read-only mode
    preferences.begin("config", true);

    // Retrieve values from preferences
    String ssid = preferences.getString("ssid", "default_ssid");
    String password = preferences.getString("password", "default_password");
    String mqtt_server = preferences.getString("mqtt_server", "default_server");
    int mqtt_port = preferences.getInt("mqtt_port", 1883); // Default MQTT port
    String mqtt_username = preferences.getString("mqtt_username", "default_user");

    preferences.end(); // Close preferences

    // Create JSON response
    StaticJsonDocument<256> responseJson;
    responseJson["ssid"] = ssid;
    responseJson["password"] = password;
    responseJson["mqtt_server"] = mqtt_server;
    responseJson["mqtt_port"] = mqtt_port;
    responseJson["mqtt_username"] = mqtt_username;

    // Serialize JSON response to string
    String responseString;
    serializeJson(responseJson, responseString);

    // Send response
    server.send(200, "application/json", responseString);
});


}

void handleReset() {
    if (digitalRead(RESET_PIN) == LOW) {
        Serial.println("Resetting settings...");
        preferences.clear();

    // Reset configuration to default values
    strncpy(ssid, DEFAULT_SSID, sizeof(ssid) - 1);
    strncpy(password, DEFAULT_PASSWORD, sizeof(password) - 1);
    strncpy(mqtt_server, DEFAULT_MQTT_SERVER, sizeof(mqtt_server) - 1);
    strncpy(mqtt_username, DEFAULT_MQTT_USERNAME, sizeof(mqtt_username) - 1);
    strncpy(mqtt_password, DEFAULT_MQTT_PASSWORD, sizeof(mqtt_password) - 1);
    mqtt_port = DEFAULT_MQTT_PORT;

    // Save defaults back to preferences
    saveConfiguration();

    Serial.println("Configuration reset to default values.");

        ESP.restart();
    }
}

void handleConfigurationUpdate(const StaticJsonDocument<256>& jsonDoc) {
    if (jsonDoc.containsKey("ssid") && jsonDoc.containsKey("password")) {
        strncpy(ssid, jsonDoc["ssid"], sizeof(ssid) - 1);
        strncpy(password, jsonDoc["password"], sizeof(password) - 1);
    }
    if (jsonDoc.containsKey("mqtt_server")) strncpy(mqtt_server, jsonDoc["mqtt_server"], sizeof(mqtt_server) - 1);
    if (jsonDoc.containsKey("mqtt_port")) mqtt_port = jsonDoc["mqtt_port"];
    if (jsonDoc.containsKey("mqtt_username")) strncpy(mqtt_username, jsonDoc["mqtt_username"], sizeof(mqtt_username) - 1);
    if (jsonDoc.containsKey("mqtt_password")) strncpy(mqtt_password, jsonDoc["mqtt_password"], sizeof(mqtt_password) - 1);

    saveConfiguration();
    Serial.println("Configuration updated.");
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

void mqttCallback(char* topic, byte* message, unsigned int length) {
    Serial.print("Message arrived on topic: ");
    Serial.println(topic);

    Serial.print("Message: ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)message[i]);
    }
    Serial.println();
}
