// #include <Preferences.h>

// Preferences preferences;
// bool needsRestart = false;

// #define LED_BUILTIN 2  // Default pin for the built-in LED on most ESP32 boards

// constexpr char DEFAULT_SSID[] = "";
// constexpr char DEFAULT_PASSWORD[] = "";
// constexpr char DEFAULT_MQTT_SERVER[] = "";
// constexpr char DEFAULT_MQTT_USERNAME[] = "";
// constexpr char DEFAULT_MQTT_PASSWORD[] = "";  
// constexpr int DEFAULT_MQTT_PORT = 1883;

// char ssid[32] = "";
// char password[32] = "";
// char mqtt_server[32] = "";
// char mqtt_username[32] = "";
// char mqtt_password[32] = "";
// int mqtt_port = DEFAULT_MQTT_PORT;

// // Function Prototypes
// void saveConfiguration();

// void setup() {
//     Serial.begin(9600);
//     pinMode(2, OUTPUT);  // Use GPIO 2 for the built-in LED   
    
//      // Clear preferences and Wi-Fi credentials if not already done
//     preferences.clear();
//     Serial.println("Preferences cleared");

//             // Reset configuration to default values
//     strncpy(ssid, DEFAULT_SSID, sizeof(ssid) - 1);
//     strncpy(password, DEFAULT_PASSWORD, sizeof(password) - 1);
//     strncpy(mqtt_server, DEFAULT_MQTT_SERVER, sizeof(mqtt_server) - 1);
//     strncpy(mqtt_username, DEFAULT_MQTT_USERNAME, sizeof(mqtt_username) - 1);
//     strncpy(mqtt_password, DEFAULT_MQTT_PASSWORD, sizeof(mqtt_password) - 1);
//     mqtt_port = DEFAULT_MQTT_PORT;

//     // Save defaults back to preferences
//     saveConfiguration();

//     Serial.println("Configuration reset to default values.");


//     // Re-initialize preferences and continue setup after restart
//     // preferences.begin("config", false);
//     // loadConfiguration();

//     // if (strlen(ssid) == 0 || strlen(password) == 0) {
//     //     setupWiFiAP(); // Start in configuration mode
//     // } else {
//     //     connectToWiFi(); // Connect using stored credentials
//     // }

//     // setupHTTPRoutes();
//     // server.begin();
//     // Serial.println("HTTP server started.");
// }

// void loop() {
//   Serial.println("Turning the LED ON");
//   digitalWrite(2, HIGH);   // Turn the LED on
  
//   delay(2000);             // Wait for 2 second
//   Serial.println("Turning the LED OFF");
//   digitalWrite(2, LOW);    // Turn the LED off
//   delay(2000);   
// }

// void saveConfiguration() {
//     preferences.putString("ssid", ssid);
//     preferences.putString("password", password);
//     preferences.putString("mqtt_server", mqtt_server);
//     preferences.putString("mqtt_username", mqtt_username);
//     preferences.putString("mqtt_password", mqtt_password);
//     preferences.putInt("mqtt_port", mqtt_port);
//     Serial.println("Configuration saved.");
// }



#include <Preferences.h>

Preferences preferences;
bool needsRestart = false;

#define LED_BUILTIN 32  // Default pin for the built-in LED on most ESP32 boards

constexpr char DEFAULT_SSID[] = "";
constexpr char DEFAULT_PASSWORD[] = "";
constexpr char DEFAULT_MQTT_SERVER[] = "";
constexpr char DEFAULT_MQTT_USERNAME[] = "";
constexpr char DEFAULT_MQTT_PASSWORD[] = "";
constexpr int DEFAULT_MQTT_PORT = 1883;

char ssid[32] = "";
char password[32] = "";
char mqtt_server[32] = "";
char mqtt_username[32] = "";
char mqtt_password[32] = "";
int mqtt_port = DEFAULT_MQTT_PORT;

constexpr int MAX_DEVICES = 4;

// Function Prototypes
void saveConfiguration();
void loadConfiguration();
void clearAllDevicesState();

// void setup() {
//     Serial.begin(115200);
//     Serial.flush();
//     delay(100); // Allow time for Serial to initialize
//     Serial.println("========== Setup Start ==========");

//     pinMode(LED_BUILTIN, OUTPUT);

//     // Initialize preferences
//     preferences.begin("config", false);

//     // Clear preferences
//     Serial.println("Clearing preferences...");
//     // preferences.clear();
//     if (preferences.clear()) {
//     Serial.println("Preferences cleared successfully.");
//     } else {
//         Serial.println("Failed to clear preferences.");
//     }

//     // Reset configuration to default values
//     Serial.println("Resetting configuration to defaults...");
//     strncpy(ssid, DEFAULT_SSID, sizeof(ssid) - 1);
//     strncpy(password, DEFAULT_PASSWORD, sizeof(password) - 1);
//     strncpy(mqtt_server, DEFAULT_MQTT_SERVER, sizeof(mqtt_server) - 1);
//     strncpy(mqtt_username, DEFAULT_MQTT_USERNAME, sizeof(mqtt_username) - 1);
//     strncpy(mqtt_password, DEFAULT_MQTT_PASSWORD, sizeof(mqtt_password) - 1);
//     mqtt_port = DEFAULT_MQTT_PORT;

//     // Save defaults back to preferences
//     saveConfiguration();

//     Serial.println("Configuration reset to default values.");
        
//     preferences.end();  // Add this line

//     Serial.println("Setup complete.");
// }

void setup() {
    Serial.begin(115200);
    Serial.flush();
    delay(100);
    Serial.println("========== Setup Start ==========");

    pinMode(LED_BUILTIN, OUTPUT);

    // Clear main configuration
    preferences.begin("config", false);
    if (preferences.clear()) {
        Serial.println("Main preferences cleared successfully.");
    } else {
        Serial.println("Failed to clear main preferences.");
    }
    preferences.end();

    // Clear all device states
    clearAllDevicesState();

    // Now set up the configuration
    preferences.begin("config", false);
    // Reset configuration to default values
    Serial.println("Resetting configuration to defaults...");
    strncpy(ssid, DEFAULT_SSID, sizeof(ssid) - 1);
    strncpy(password, DEFAULT_PASSWORD, sizeof(password) - 1);
    strncpy(mqtt_server, DEFAULT_MQTT_SERVER, sizeof(mqtt_server) - 1);
    strncpy(mqtt_username, DEFAULT_MQTT_USERNAME, sizeof(mqtt_username) - 1);
    strncpy(mqtt_password, DEFAULT_MQTT_PASSWORD, sizeof(mqtt_password) - 1);
    mqtt_port = DEFAULT_MQTT_PORT;

    // Save defaults back to preferences
    saveConfiguration();
    preferences.end();
    
    Serial.println("Setup complete.");
}    



void loop() {
    Serial.println("Turning the LED ON");
    digitalWrite(LED_BUILTIN, HIGH);   // Turn the LED on

    delay(2000);             // Wait for 2 seconds
    Serial.println("Turning the LED OFF");
    digitalWrite(LED_BUILTIN, LOW);    // Turn the LED off
    delay(2000);
}

// void saveConfiguration() {
//     preferences.putString("ssid", ssid);
//     preferences.putString("password", password);
//     preferences.putString("mqtt_server", mqtt_server);
//     preferences.putString("mqtt_username", mqtt_username);
//     preferences.putString("mqtt_password", mqtt_password);
//     preferences.putInt("mqtt_port", mqtt_port);
//     Serial.println("Configuration saved.");
// }

void saveConfiguration() {
    size_t written = 0;
    written += preferences.putString("ssid", ssid);
    written += preferences.putString("password", password);
    written += preferences.putString("mqtt_server", mqtt_server);
    written += preferences.putString("mqtt_username", mqtt_username);
    written += preferences.putString("mqtt_password", mqtt_password);
    written += preferences.putInt("mqtt_port", mqtt_port);
    if (written > 0) {
        Serial.println("Configuration saved successfully.");
    } else {
        Serial.println("Failed to save configuration.");
    }
}

// void loadConfiguration() {
//     // Load configuration from preferences
//     preferences.getString("ssid", ssid, sizeof(ssid));
//     preferences.getString("password", password, sizeof(password));
//     preferences.getString("mqtt_server", mqtt_server, sizeof(mqtt_server));
//     preferences.getString("mqtt_username", mqtt_username, sizeof(mqtt_username));
//     preferences.getString("mqtt_password", mqtt_password, sizeof(mqtt_password));
//     mqtt_port = preferences.getInt("mqtt_port", DEFAULT_MQTT_PORT);
//     Serial.println("Configuration loaded.");
// }

void loadConfiguration() {
    struct ConfigItem {
        const char* key;
        char* value;
        const char* defaultValue;
        size_t maxSize;
    };

    ConfigItem stringConfigs[] = {
        {"ssid", ssid, DEFAULT_SSID, sizeof(ssid)},
        {"password", password, DEFAULT_PASSWORD, sizeof(password)},
        {"mqtt_server", mqtt_server, DEFAULT_MQTT_SERVER, sizeof(mqtt_server)},
        {"mqtt_username", mqtt_username, DEFAULT_MQTT_USERNAME, sizeof(mqtt_username)},
        {"mqtt_password", mqtt_password, DEFAULT_MQTT_PASSWORD, sizeof(mqtt_password)}
    };

    bool success = true;

    // Handle string configurations
    for (const auto& config : stringConfigs) {
        if (preferences.getString(config.key, config.value, config.maxSize) == 0) {
            Serial.printf("Failed to load %s, using default\n", config.key);
            strncpy(config.value, config.defaultValue, config.maxSize - 1);
            success = false;
        }
    }

    // Handle MQTT port separately since it's an int
    mqtt_port = preferences.getInt("mqtt_port", DEFAULT_MQTT_PORT);
    if (mqtt_port == 0) {
        Serial.println("Failed to load MQTT port, using default");
        mqtt_port = DEFAULT_MQTT_PORT;
        success = false;
    }

    Serial.println(success ? "Configuration loaded successfully." 
                         : "Some configuration values were reset to defaults.");
}

void clearAllDevicesState() {
    for (int deviceIndex = 0; deviceIndex < MAX_DEVICES; deviceIndex++) {
        String namespaceStr = "dev_" + String(deviceIndex);
        preferences.begin(namespaceStr.c_str(), false);
        if (preferences.clear()) {
            Serial.printf("Device %d preferences cleared successfully\n", deviceIndex);
        } else {
            Serial.printf("Failed to clear preferences for device %d\n", deviceIndex);
        }
        preferences.end();
    }
}
