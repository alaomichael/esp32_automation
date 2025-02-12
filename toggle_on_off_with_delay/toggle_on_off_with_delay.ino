// Formerly working code 1

// #define PIN32 32  // Define GPIO pin

// void setup() {
//   Serial.begin(9600);        // Initialize serial communication at 9600 baud
//   pinMode(PIN32, OUTPUT);    // Set GPIO 32 as output
// }

// void loop() {
//   digitalWrite(PIN32, HIGH); // Turn GPIO 32 ON
//   Serial.println("GPIO 32 is ON");
//   delay(1000);               // Wait for 1 second
  
//   digitalWrite(PIN32, LOW);  // Turn GPIO 32 OFF
//   Serial.println("GPIO 32 is OFF");
//   delay(1000);               // Wait for 1 second
// }


// Formerly working code 2
// #define PIN32 32  // Define GPIO pin for output
// #define PIN33 33  // Define GPIO pin for input

// void setup() {
//   Serial.begin(9600);         // Initialize serial communication at 9600 baud
//   pinMode(PIN32, OUTPUT);     // Set GPIO 32 as output
//   pinMode(PIN33, INPUT);      // Set GPIO 33 as input
// }

// void loop() {
//   // Write HIGH to PIN32
//   digitalWrite(PIN32, HIGH);
//   Serial.println("GPIO 32 is ON");
  
//   // Read the value on PIN33
//   int pin33Value = digitalRead(PIN33);
//   if (pin33Value == HIGH) {
//     Serial.println("GPIO 33 reads: HIGH");
//   } else {
//     Serial.println("GPIO 33 reads: LOW");
//   }

//   delay(1000);  // Wait for 1 second
  
//   // Write LOW to PIN32
//   digitalWrite(PIN32, LOW);
//   Serial.println("GPIO 32 is OFF");
  
//   // Read the value on PIN33
//   pin33Value = digitalRead(PIN33);
//   if (pin33Value == HIGH) {
//     Serial.println("GPIO 33 reads: HIGH");
//   } else {
//     Serial.println("GPIO 33 reads: LOW");
//   }

//   delay(1000);  // Wait for 1 second
// }



// Formerly working code 3
// #include <WiFi.h>  // Include the Wi-Fi library for ESP32

// #define PIN32 32  // Define GPIO pin for output
// #define PIN33 33  // Define GPIO pin for input

// const char* ssid = "Providence";     // Replace with your Wi-Fi SSID
// const char* password = "Provider123"; // Replace with your Wi-Fi password

// void setup() {
//   Serial.begin(9600);         // Initialize serial communication at 9600 baud
//   pinMode(PIN32, OUTPUT);     // Set GPIO 32 as output
//   pinMode(PIN33, INPUT);      // Set GPIO 33 as input
  
//   // Connect to Wi-Fi
//   Serial.println("Connecting to WiFi...");
//   WiFi.begin(ssid, password);

//   // Wait for the Wi-Fi to connect
//   while (WiFi.status() != WL_CONNECTED) {
//     delay(5000);  // Wait for 5 second
//     Serial.println("Connecting to WiFi...");
//   }

//   // Once connected, print the IP address
//   Serial.println("Connected to WiFi");
//   Serial.print("IP Address: ");
//   Serial.println(WiFi.localIP());
// }

// void loop() {
//   // Write HIGH to PIN32
//   digitalWrite(PIN32, HIGH);
//   Serial.println("GPIO 32 is ON");
  
//   // Read the value on PIN33
//   int pin33Value = digitalRead(PIN33);
//   if (pin33Value == HIGH) {
//     Serial.println("GPIO 33 reads: HIGH");
//   } else {
//     Serial.println("GPIO 33 reads: LOW");
//   }

//   delay(5000);  // Wait for 5 second
  
//   // Write LOW to PIN32
//   digitalWrite(PIN32, LOW);
//   Serial.println("GPIO 32 is OFF");
  
//   // Read the value on PIN33
//   pin33Value = digitalRead(PIN33);
//   if (pin33Value == HIGH) {
//     Serial.println("GPIO 33 reads: HIGH");
//   } else {
//     Serial.println("GPIO 33 reads: LOW");
//   }

//   delay(5000);  // Wait for 5 second
// }

// Formerly working code 3 End

// Formerly working code 4 Start

// #include <WiFi.h>          // Include the Wi-Fi library for ESP32
// #include <PubSubClient.h>   // Include the PubSubClient library for MQTT

// #define PIN32 32  // Define GPIO pin for output
// #define PIN33 33  // Define GPIO pin for input

// const char* ssid = "Providence";     // Replace with your Wi-Fi SSID
// const char* password = "Provider123"; // Replace with your Wi-Fi password

// // MQTT server details
// const char* mqtt_server = "192.168.0.105";  // Replace with your MQTT broker address
// const char* mqtt_username = "testuser";   // Replace with your MQTT username
// const char* mqtt_password = "password";   // Replace with your MQTT password
// const int mqtt_port = 1883;  // Replace with your MQTT port, typically 1883 for non-SSL

// WiFiClient espClient;
// PubSubClient client(espClient);

// void setup() {
//   Serial.begin(9600);         // Initialize serial communication at 9600 baud
//   pinMode(PIN32, OUTPUT);     // Set GPIO 32 as output
//   pinMode(PIN33, INPUT);      // Set GPIO 33 as input

//   // Connect to Wi-Fi
//   Serial.println("Connecting to WiFi...");
//   WiFi.begin(ssid, password);

//   // Wait for the Wi-Fi to connect
//   while (WiFi.status() != WL_CONNECTED) {
//     delay(5000);  // Wait for 5 seconds
//     Serial.println("Connecting to WiFi...");
//   }

//   // Once connected, print the IP address
//   Serial.println("Connected to WiFi");
//   Serial.print("IP Address: ");
//   Serial.println(WiFi.localIP());

//   // Setup MQTT
//   client.setServer(mqtt_server, mqtt_port);   // Set the MQTT server and port
//   client.setCallback(mqttCallback);            // Set the callback function for incoming messages
// }

// void loop() {
//   // Reconnect to MQTT if disconnected
//   if (!client.connected()) {
//     reconnectMQTT();
//   }
//   client.loop();  // Keep the MQTT connection alive

//   // Read the value on PIN33
//   int pin33Value = digitalRead(PIN33);
//   if (pin33Value == HIGH) {
//     Serial.println("GPIO 33 reads: HIGH");
//   } else {
//     Serial.println("GPIO 33 reads: LOW");
//   }

//   // Publish the GPIO 33 status to the MQTT topic
//   String message = (pin33Value == HIGH) ? "HIGH" : "LOW";
//   client.publish("esp32/gpio33", message.c_str()); // Publish the message to the topic "esp32/gpio33"

//   // Write HIGH to PIN32
//   digitalWrite(PIN32, HIGH);
//   Serial.println("GPIO 32 is ON");

//   delay(5000);  // Wait for 5 seconds

//   // Write LOW to PIN32
//   digitalWrite(PIN32, LOW);
//   Serial.println("GPIO 32 is OFF");

//   delay(5000);  // Wait for 5 seconds
// }

// // MQTT callback function to handle incoming messages
// void mqttCallback(char* topic, byte* payload, unsigned int length) {
//   Serial.print("Message arrived on topic: ");
//   Serial.print(topic);
//   Serial.print(". Message: ");
  
//   for (unsigned int i = 0; i < length; i++) {
//     Serial.print((char)payload[i]);
//   }
//   Serial.println();
// }

// // Function to reconnect to MQTT broker
// void reconnectMQTT() {
//   // Loop until we're connected
//   while (!client.connected()) {
//     Serial.print("Attempting MQTT connection...");
    
//     // Create a random client ID
//     String clientId = "ESP32Client-";
//     clientId += String(random(0xffff), HEX);

//     // Attempt to connect
//     if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
//       Serial.println("connected");
      
//       // Once connected, subscribe to the topic
//       client.subscribe("esp32/command");  // Replace with the topic you want to subscribe to
//     } else {
//       Serial.print("failed, rc=");
//       Serial.print(client.state());
//       Serial.println(" try again in 6 seconds");
//       delay(6000);  // Wait 6 seconds before retrying
//     }
//   }
// }


