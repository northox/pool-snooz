#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Ticker.h>

#define RELAY_PIN 5
#define LED_PIN 2

const char* ssid = "a";
const char* password = "b";
const char* mqttServer = "c";
const int mqttPort = 1883;
const char* mqttUser = "d";
const char* mqttPassword = "e";

WiFiClient espClient;
PubSubClient mqttClient(espClient);
Ticker watchdog;

void watchdogReset() {
  ESP.restart();
}

void setup() {
    Serial.begin(9600);
    Serial.println("Booting.");

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);
    
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, HIGH);

    watchdog.attach(60, watchdogReset);

    connectToWiFi();
    connectToMQTT();
    Serial.println("Setup completed.");
}

void loop() {
    digitalWrite(LED_PIN, LOW);
    delay(250);

    watchdog.detach();
    watchdog.attach(60, watchdogReset);

    if (WiFi.status() != WL_CONNECTED) {
        connectToWiFi();
    }
    if (!mqttClient.connected()) { 
        connectToMQTT();
    }
    mqttClient.loop();  
    digitalWrite(LED_PIN, HIGH);
    delay(1000);
}

void connectToWiFi() {
    Serial.print("WiFi... ");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("connected.");
}

void connectToMQTT() {
    mqttClient.setServer(mqttServer, mqttPort);
    mqttClient.setCallback(mqttCallback);
    reconnectMQTT();
}

void reconnectMQTT() {
    while (!mqttClient.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (mqttClient.connect("ESP8266Client", mqttUser, mqttPassword)) {
            Serial.println(" connected.");
            delay(500);
            mqttClient.subscribe("pool/pump/control");
        } else {
            Serial.print(" failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" try again in 5 seconds.");
            delay(5000);
        }
    }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    String message = "";
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    
    if (String(topic) == "pool/pump/control") {
        if (message == "ON") {
            flashLed();
            digitalWrite(RELAY_PIN, LOW);
            Serial.println("Relay ON.");
        } else {
            flashLed();
            digitalWrite(RELAY_PIN, HIGH);
            Serial.println("Relay OFF.");
        }
    }
}

void flashLed() {
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    delay(500);
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
}
