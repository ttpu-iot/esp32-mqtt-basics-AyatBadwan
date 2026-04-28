#include "Arduino.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// LED Pin definitions
#define RED_LED 26
#define GREEN_LED 27
#define BLUE_LED 14
#define YELLOW_LED 12

// Button pin
#define BUTTON_PIN 25

// I2C pins for LCD
#define I2C_SDA 21
#define I2C_SCL 22

// LCD address (usually 0x27)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// WiFi credentials (for WOKWI simulation)
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// MQTT credentials
const char* mqtt_broker = "mqtt.iotserver.uz";
const int mqtt_port = 1883;
const char* mqtt_username = "userTTPU";
const char* mqtt_password = "mqttpass";
const char* client_id = "AyatBadwan-lab3-ex3";

// LED topics
const char* red_topic = "ttpu/iot/AyatBadwan/led/red";
const char* green_topic = "ttpu/iot/AyatBadwan/led/green";
const char* blue_topic = "ttpu/iot/AyatBadwan/led/blue";
const char* yellow_topic = "ttpu/iot/AyatBadwan/led/yellow";

// Button event topic
const char* button_topic = "ttpu/iot/AyatBadwan/events/button";

// Display topic
const char* display_topic = "ttpu/iot/AyatBadwan/display";

// Button debouncing variables
int lastButtonState = LOW;
int currentButtonState = LOW;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// LCD display variables
String lastDisplayText = "";
bool lcdInitialized = false;

// MQTT client
WiFiClient espClient;
PubSubClient mqttClient(espClient);

void setup_wifi() {
    Serial.print("Connecting to WiFi");
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println();
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void setup_time() {
    Serial.print("Syncing NTP time for Tashkent (UTC+5)...");
    configTime(18000, 0, "pool.ntp.org", "time.nist.gov");
    
    struct tm timeinfo;
    int attempts = 0;
    while (!getLocalTime(&timeinfo) && attempts < 10) {
        delay(1000);
        attempts++;
        Serial.print(".");
    }
    
    if (attempts < 10) {
        Serial.println(" DONE!");
        char buffer[30];
        strftime(buffer, sizeof(buffer), "%d/%m %H:%M:%S", &timeinfo);
        Serial.print("Current Tashkent time: ");
        Serial.println(buffer);
    } else {
        Serial.println(" FAILED!");
    }
}

String get_formatted_time() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return "00/00 00:00:00";
    }
    
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%d/%m %H:%M:%S", &timeinfo);
    return String(buffer);
}

void update_lcd(String message) {
    // Truncate to 16 characters if needed
    if (message.length() > 16) {
        message = message.substring(0, 16);
    }
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(message);
    
    lcd.setCursor(0, 1);
    String timeStr = get_formatted_time();
    lcd.print(timeStr);
    
    Serial.print("[LCD] Updated - Line 1: ");
    Serial.print(message);
    Serial.print(" | Line 2: ");
    Serial.println(timeStr);
}

void callback(char* topic, byte* payload, unsigned int length) {
    // Convert payload to string
    String message;
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    
    Serial.print("[MQTT] Received on ");
    Serial.print(topic);
    Serial.print(": ");
    Serial.println(message);
    
    // Parse JSON
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, message);
    
    if (error) {
        Serial.print("[ERROR] JSON parsing failed: ");
        Serial.println(error.c_str());
        return;
    }
    
    // Handle LED control
    if (strcmp(topic, red_topic) == 0) {
        const char* state = doc["state"];
        if (state && strcmp(state, "ON") == 0) {
            digitalWrite(RED_LED, HIGH);
            Serial.println("[LED] Red LED -> ON");
        } else if (state && strcmp(state, "OFF") == 0) {
            digitalWrite(RED_LED, LOW);
            Serial.println("[LED] Red LED -> OFF");
        }
    }
    else if (strcmp(topic, green_topic) == 0) {
        const char* state = doc["state"];
        if (state && strcmp(state, "ON") == 0) {
            digitalWrite(GREEN_LED, HIGH);
            Serial.println("[LED] Green LED -> ON");
        } else if (state && strcmp(state, "OFF") == 0) {
            digitalWrite(GREEN_LED, LOW);
            Serial.println("[LED] Green LED -> OFF");
        }
    }
    else if (strcmp(topic, blue_topic) == 0) {
        const char* state = doc["state"];
        if (state && strcmp(state, "ON") == 0) {
            digitalWrite(BLUE_LED, HIGH);
            Serial.println("[LED] Blue LED -> ON");
        } else if (state && strcmp(state, "OFF") == 0) {
            digitalWrite(BLUE_LED, LOW);
            Serial.println("[LED] Blue LED -> OFF");
        }
    }
    else if (strcmp(topic, yellow_topic) == 0) {
        const char* state = doc["state"];
        if (state && strcmp(state, "ON") == 0) {
            digitalWrite(YELLOW_LED, HIGH);
            Serial.println("[LED] Yellow LED -> ON");
        } else if (state && strcmp(state, "OFF") == 0) {
            digitalWrite(YELLOW_LED, LOW);
            Serial.println("[LED] Yellow LED -> OFF");
        }
    }
    // Handle display messages
    else if (strcmp(topic, display_topic) == 0) {
        const char* text = doc["text"];
        if (text) {
            lastDisplayText = String(text);
            update_lcd(lastDisplayText);
            Serial.println("[LCD] Display message processed");
        } else {
            Serial.println("[ERROR] Missing 'text' field in display message");
        }
    }
}

void reconnect_mqtt() {
    while (!mqttClient.connected()) {
        Serial.print("Connecting to MQTT broker...");
        
        if (mqttClient.connect(client_id, mqtt_username, mqtt_password)) {
            Serial.println(" connected!");
            
            // Subscribe to all topics
            mqttClient.subscribe(red_topic);
            mqttClient.subscribe(green_topic);
            mqttClient.subscribe(blue_topic);
            mqttClient.subscribe(yellow_topic);
            mqttClient.subscribe(display_topic);
            
            Serial.println("[MQTT] Subscribed to topics:");
            Serial.println("  - ttpu/iot/AyatBadwan/led/red");
            Serial.println("  - ttpu/iot/AyatBadwan/led/green");
            Serial.println("  - ttpu/iot/AyatBadwan/led/blue");
            Serial.println("  - ttpu/iot/AyatBadwan/led/yellow");
            Serial.println("  - ttpu/iot/AyatBadwan/display");
        } else {
            Serial.print(" failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" retrying in 5 seconds");
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println("========================================");
    Serial.println("ESP32 MQTT Exercise 3 Starting...");
    Serial.println("Bi-directional MQTT + LCD Display");
    Serial.println("========================================");
    
    // Initialize LED pins
    pinMode(RED_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);
    pinMode(BLUE_LED, OUTPUT);
    pinMode(YELLOW_LED, OUTPUT);
    
    // Initialize button
    pinMode(BUTTON_PIN, INPUT_PULLDOWN);
    
    // Turn all LEDs OFF initially
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BLUE_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    
    Serial.println("[SETUP] LEDs initialized (all OFF)");
    Serial.println("[SETUP] Button initialized on D25");
    
    // Initialize LCD
    Wire.begin(I2C_SDA, I2C_SCL);
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Starting...");
    lcd.setCursor(0, 1);
    lcd.print("ESP32 MQTT Lab");
    Serial.println("[SETUP] LCD initialized");
    
    // Setup WiFi
    setup_wifi();
    
    // Setup NTP time
    setup_time();
    
    // Setup MQTT
    mqttClient.setServer(mqtt_broker, mqtt_port);
    mqttClient.setCallback(callback);
    
    // Update LCD with ready message
    delay(2000);
    update_lcd("ESP32 Ready!");
}

void loop() {
    if (!mqttClient.connected()) {
        reconnect_mqtt();
    }
    mqttClient.loop();
    
    unsigned long currentMillis = millis();
    
    // Publish button events on state change
    int reading = digitalRead(BUTTON_PIN);
    
    if (reading != lastButtonState) {
        lastDebounceTime = currentMillis;
    }
    
    if ((currentMillis - lastDebounceTime) > debounceDelay) {
        if (reading != currentButtonState) {
            currentButtonState = reading;
            
            // Button state changed - publish event
            const char* event = (currentButtonState == HIGH) ? "PRESSED" : "RELEASED";
            
            // Get current timestamp
            time_t now = time(nullptr);
            
            // Create JSON payload
            JsonDocument doc;
            doc["event"] = event;
            doc["timestamp"] = now;
            
            char buffer[256];
            serializeJson(doc, buffer);
            
            // Publish to MQTT
            boolean success = mqttClient.publish(button_topic, buffer);
            
            // Print to Serial Monitor
            Serial.println();
            Serial.print("[PUBLISH] Topic: ");
            Serial.println(button_topic);
            Serial.print("          Payload: ");
            Serial.println(buffer);
            if (success) {
                Serial.println("          Status: ✓ Published successfully");
            } else {
                Serial.println("          Status: ✗ Publish failed");
            }
        }
    }
    lastButtonState = reading;
    
    delay(10);
}