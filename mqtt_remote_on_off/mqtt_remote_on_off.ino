#include <WiFi.h>          // Include the Wi-Fi library for ESP32
#include <PubSubClient.h>  // Include the PubSubClient library for MQTT
#include <ArduinoJson.h>   // Include the ArduinoJson library for JSON parsing

#define PIN32 32  // Define GPIO pin for output
#define PIN33 33  // Define GPIO pin for input

const char* ssid = "Providence";        // Replace with your Wi-Fi SSID
const char* password = "Provider123";  // Replace with your Wi-Fi password

// MQTT server details
// const char* mqtt_server = "192.168.0.105"; // Replace with your MQTT broker address
const char* mqtt_server = "192.168.0.148"; // Replace with your MQTT broker address
const char* mqtt_username = "testuser";   // Replace with your MQTT username
const char* mqtt_password = "password";   // Replace with your MQTT password
const int mqtt_port = 1883;               // Replace with your MQTT port, typically 1883 for non-SSL

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(9600);         // Initialize serial communication
  pinMode(PIN32, OUTPUT);     // Set GPIO 32 as output
  pinMode(PIN33, INPUT);      // Set GPIO 33 as input

  // Connect to Wi-Fi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(5000);  // Wait 5 seconds and retry
    Serial.println("Connecting to WiFi...");
  }

  // Wi-Fi connected
  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Setup MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttCallback);
}

void loop() {
  // Reconnect to MQTT if disconnected
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();  // Maintain MQTT connection

    // Read and publish GPIO 33 state
  int pin32Value = digitalRead(PIN32);
  Serial.print("GPIO 32 reads: ");
  Serial.println(pin32Value == HIGH ? "HIGH" : "LOW");

  // Publish GPIO 33 status to MQTT topic
  String message = (pin32Value == HIGH) ? "HIGH" : "LOW";
  // client.publish("esp32/gpio32", message.c_str());

  delay(5000);  // Delay before next iteration
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

// Extract "action" from JSON
const char* action = jsonDoc["action"];
if (action) {
    StaticJsonDocument<200> responseJson; // Create a JSON document for the response

    if (strcmp(action, "ON") == 0) {
        digitalWrite(PIN32, HIGH); // Set GPIO 32 to HIGH
        responseJson["status"] = "success";
        responseJson["state"] = "ON";
        responseJson["message"] = "GPIO 32 set to HIGH";
        Serial.println("GPIO 32 set to HIGH");
    } else if (strcmp(action, "OFF") == 0) {
        digitalWrite(PIN32, LOW); // Set GPIO 32 to LOW
        responseJson["status"] = "success";
        responseJson["state"] = "OFF";
        responseJson["message"] = "GPIO 32 set to LOW";
        Serial.println("GPIO 32 set to LOW");
    } else {
        responseJson["status"] = "error";
        responseJson["message"] = "Invalid action received";
        Serial.println("Invalid action received");
    }

    // Serialize and publish the JSON object
    char responseBuffer[256];
    serializeJson(responseJson, responseBuffer);
    client.publish("esp32/status", responseBuffer);
} else {
    StaticJsonDocument<200> errorJson; // Create a JSON document for the error response
    errorJson["status"] = "error";
    errorJson["message"] = "Missing 'action' field in JSON";

    // Serialize and publish the JSON error
    char errorBuffer[256];
    serializeJson(errorJson, errorBuffer);
    client.publish("esp32/status", errorBuffer);

    Serial.println("Missing 'action' field in JSON");
}
}

// Function to reconnect to MQTT broker
void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    // Generate a unique client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");

      // Subscribe to the command topic
      client.subscribe("esp32/command");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 6 seconds");
      delay(6000);
    }
  }
}
