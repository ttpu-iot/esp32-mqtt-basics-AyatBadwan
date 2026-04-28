#include "Arduino.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// LED Pin definitions
#define RED_LED 26
#define GREEN_LED 27
#define BLUE_LED 14
#define YELLOW_LED 12

// WiFi credentials (for WOKWI simulation)
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// MQTT credentials
const char* mqtt_broker = "mqtt.iotserver.uz";
const int mqtt_port = 1883;
const char* mqtt_username = "userTTPU";
const char* mqtt_password = "mqttpass";
const char* client_id = "AyatBadwan-lab3-ex2";

// LED topics
const char* red_topic = "ttpu/iot/AyatBadwan/led/red";
const char* green_topic = "ttpu/iot/AyatBadwan/led/green";
const char* blue_topic = "ttpu/iot/AyatBadwan/led/blue";
const char* yellow_topic = "ttpu/iot/AyatBadwan/led/yellow";

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
    
    // Extract state field
    const char* state = doc["state"];
    
    if (state == nullptr) {
        Serial.println("[ERROR] Missing 'state' field in JSON");
        return;
    }
    
    // Determine which LED to control based on topic
    if (strcmp(topic, red_topic) == 0) {
        if (strcmp(state, "ON") == 0) {
            digitalWrite(RED_LED, HIGH);
            Serial.println("[LED] Red LED -> ON");
        } else if (strcmp(state, "OFF") == 0) {
            digitalWrite(RED_LED, LOW);
            Serial.println("[LED] Red LED -> OFF");
        } else {
            Serial.print("[ERROR] Invalid state: ");
            Serial.println(state);
        }
    }
    else if (strcmp(topic, green_topic) == 0) {
        if (strcmp(state, "ON") == 0) {
            digitalWrite(GREEN_LED, HIGH);
            Serial.println("[LED] Green LED -> ON");
        } else if (strcmp(state, "OFF") == 0) {
            digitalWrite(GREEN_LED, LOW);
            Serial.println("[LED] Green LED -> OFF");
        } else {
            Serial.print("[ERROR] Invalid state: ");
            Serial.println(state);
        }
    }
    else if (strcmp(topic, blue_topic) == 0) {
        if (strcmp(state, "ON") == 0) {
            digitalWrite(BLUE_LED, HIGH);
            Serial.println("[LED] Blue LED -> ON");
        } else if (strcmp(state, "OFF") == 0) {
            digitalWrite(BLUE_LED, LOW);
            Serial.println("[LED] Blue LED -> OFF");
        } else {
            Serial.print("[ERROR] Invalid state: ");
            Serial.println(state);
        }
    }
    else if (strcmp(topic, yellow_topic) == 0) {
        if (strcmp(state, "ON") == 0) {
            digitalWrite(YELLOW_LED, HIGH);
            Serial.println("[LED] Yellow LED -> ON");
        } else if (strcmp(state, "OFF") == 0) {
            digitalWrite(YELLOW_LED, LOW);
            Serial.println("[LED] Yellow LED -> OFF");
        } else {
            Serial.print("[ERROR] Invalid state: ");
            Serial.println(state);
        }
    }
}

void reconnect_mqtt() {
    while (!mqttClient.connected()) {
        Serial.print("Connecting to MQTT broker...");
        
        if (mqttClient.connect(client_id, mqtt_username, mqtt_password)) {
            Serial.println(" connected!");
            
            // Subscribe to all 4 LED topics
            mqttClient.subscribe(red_topic);
            mqttClient.subscribe(green_topic);
            mqttClient.subscribe(blue_topic);
            mqttClient.subscribe(yellow_topic);
            
            Serial.println("[MQTT] Subscribed to all LED topics:");
            Serial.println("  - ttpu/iot/AyatBadwan/led/red");
            Serial.println("  - ttpu/iot/AyatBadwan/led/green");
            Serial.println("  - ttpu/iot/AyatBadwan/led/blue");
            Serial.println("  - ttpu/iot/AyatBadwan/led/yellow");
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
    Serial.println("ESP32 MQTT Exercise 2 Starting...");
    Serial.println("MQTT Subscribe & Control LEDs");
    Serial.println("----------------------------------------");
    
    // Initialize LED pins
    pinMode(RED_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);
    pinMode(BLUE_LED, OUTPUT);
    pinMode(YELLOW_LED, OUTPUT);
    
    // Turn all LEDs OFF initially
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BLUE_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    
    Serial.println("[SETUP] LEDs initialized (all OFF)");
    
    // Setup WiFi
    setup_wifi();
    
    // Setup MQTT
    mqttClient.setServer(mqtt_broker, mqtt_port);
    mqttClient.setCallback(callback);
}

void loop() {
    if (!mqttClient.connected()) {
        reconnect_mqtt();
    }
    mqttClient.loop();
    
    delay(10);
}