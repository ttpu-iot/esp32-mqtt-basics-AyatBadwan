#include "Arduino.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>

// Pin definitions
#define RED_LED 26
#define GREEN_LED 27
#define BLUE_LED 14
#define YELLOW_LED 12
#define BUTTON_PIN 25
#define LIGHT_SENSOR 33

// WiFi credentials (for WOKWI simulation)
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// MQTT credentials
const char* mqtt_broker = "mqtt.iotserver.uz";
const int mqtt_port = 1883;
const char* mqtt_username = "userTTPU";
const char* mqtt_password = "mqttpass";
const char* client_id = "AyatBadwan-lab3";

// Topics (using correct format: ttpu/iot/studentname/)
const char* light_topic = "ttpu/iot/AyatBadwan/sensors/light";
const char* button_topic = "ttpu/iot/AyatBadwan/events/button";

// Button debouncing variables
int lastButtonState = LOW;
int currentButtonState = LOW;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// Timing for periodic publish
unsigned long lastPublishTime = 0;
const unsigned long publishInterval = 5000;  // 5 seconds

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

void reconnect_mqtt() {
    while (!mqttClient.connected()) {
        Serial.print("Connecting to MQTT broker...");
        
        if (mqttClient.connect(client_id, mqtt_username, mqtt_password)) {
            Serial.println(" connected!");
            Serial.print("Connected to: ");
            Serial.println(mqtt_broker);
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
    Serial.println("ESP32 MQTT Exercise 1 Starting...");
    
    // Initialize LED pins (not used in this exercise but required)
    pinMode(RED_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);
    pinMode(BLUE_LED, OUTPUT);
    pinMode(YELLOW_LED, OUTPUT);
    
    // Initialize button
    pinMode(BUTTON_PIN, INPUT_PULLDOWN);
    
    // Initialize light sensor
    pinMode(LIGHT_SENSOR, INPUT);
    
    // Turn all LEDs OFF
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BLUE_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    
    // Setup WiFi
    setup_wifi();
    
    // Setup MQTT
    mqttClient.setServer(mqtt_broker, mqtt_port);
    
    // Initialize time for timestamps
    configTime(0, 0, "pool.ntp.org");
}

void loop() {
    // Ensure MQTT is connected
    if (!mqttClient.connected()) {
        reconnect_mqtt();
    }
    mqttClient.loop();
    
    unsigned long currentMillis = millis();
    
    // TASK 1: Publish light sensor every 5 seconds
    if (currentMillis - lastPublishTime >= publishInterval) {
        lastPublishTime = currentMillis;
        
        // Read light sensor
        int lightValue = analogRead(LIGHT_SENSOR);
        
        // Get current timestamp
        time_t now = time(nullptr);
        
        // Create JSON payload
        JsonDocument doc;
        doc["light"] = lightValue;
        doc["timestamp"] = now;
        
        char buffer[256];
        serializeJson(doc, buffer);
        
        // Publish to MQTT
        boolean success = mqttClient.publish(light_topic, buffer);
        
        // Print to Serial Monitor
        Serial.println();
        Serial.print("[PUBLISH] Topic: ");
        Serial.println(light_topic);
        Serial.print("          Payload: ");
        Serial.println(buffer);
        if (success) {
            Serial.println("          Status: ✓ Published successfully");
        } else {
            Serial.println("          Status: ✗ Publish failed");
        }
    }
    
    // TASK 2: Publish button events on state change
    int reading = digitalRead(BUTTON_PIN);
    
    // Button debouncing logic
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