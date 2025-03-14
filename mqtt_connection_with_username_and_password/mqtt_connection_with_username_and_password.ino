#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

// WiFi credentials
#define WIFI_SSID "Providence"
#define WIFI_PASSWORD "Provider123"

// HiveMQ Cloud / MQTT Broker details
#define MQTT_HOST "461dfa1a28584cac941b461688592262.s1.eu.hivemq.cloud"
#define MQTT_PORT 8883  // Secure MQTT over TLS
#define MQTT_USERNAME "homeautomation"
#define MQTT_PASSWORD "Homeautomation1"
#define MQTT_TOPIC "test/topic"

// WiFi and MQTT clients
WiFiClientSecure espClient;
PubSubClient mqtt(espClient);

// WiFi Connection
void setupWiFi() {
    Serial.println();
    Serial.print("Connecting to WiFi: ");
    Serial.println(WIFI_SSID);
    
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }

    Serial.println("\nWiFi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

// Callback function for incoming MQTT messages
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message received on topic: ");
    Serial.print(topic);
    Serial.print(" - ");
    
    for (unsigned int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
}

// MQTT Connection
void connectMQTT() {
    Serial.print("Connecting to MQTT Broker: ");
    Serial.println(MQTT_HOST);

    mqtt.setServer(MQTT_HOST, MQTT_PORT);
    mqtt.setCallback(mqttCallback);
    espClient.setInsecure(); // Use insecure mode for testing (Remove for production with SSL certificate)

    while (!mqtt.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (mqtt.connect("ESP32Client", MQTT_USERNAME, MQTT_PASSWORD)) {
            Serial.println("Connected to MQTT!");
            // mqtt.subscribe(MQTT_TOPIC); // Subscribe to the topic
        } else {
            Serial.print("Failed, rc=");
            Serial.print(mqtt.state());
            Serial.println(" Retrying in 6 seconds...");
            delay(6000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    setupWiFi();
    connectMQTT();
}

void loop() {
    if (!mqtt.connected()) {
        connectMQTT();
    }
    mqtt.loop();  // Handle MQTT communication

    // Publish a test message every 10 seconds
    static unsigned long lastMsg = 0;
    if (millis() - lastMsg > 10000) {
        lastMsg = millis();
        String msg = "Hello from ESP32!";
        Serial.print("Publishing message: ");
        Serial.println(msg);
        mqtt.publish(MQTT_TOPIC, msg.c_str());
    }
}

