#include <M5Stack.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <M5UnitENV.h>

// WLAN und MQTT Broker
const char* ssid = "sssid";
const char* password = "wifi_password";
const char* mqtt_server = "broker.hivemq.com";

WiFiClient wifiClient;
PubSubClient client(wifiClient);

M5UnitENV envSensor;

unsigned long lastEnvRead = 0;
unsigned long lastSend = 0;
StaticJsonDocument<256> jsonDoc;

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Nachricht erhalten in Topic: ");
  Serial.println(topic);
  Serial.print("Payload: ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Versuche MQTT-Verbindung...");
    String clientId = "M5Stack-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("verbunden");
      client.subscribe("M5Stack");
    } else {
      Serial.print("Fehler, rc=");
      Serial.print(client.state());
      Serial.println(" Warte 5 Sekunden...");
      delay(5000);
    }
  }
}

void setup() {
  M5.begin();
  Serial.begin(115200);
  Wire.begin();

  envSensor.begin();

  WiFi.begin(ssid, password);
  Serial.print("Verbinde mit WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi verbunden, IP: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();

  // ENV alle 5 Sekunden lesen
  if (now - lastEnvRead > 5000) {
    lastEnvRead = now;
    float temperature = envSensor.temperature;
    float humidity = envSensor.humidity;
    float pressure = envSensor.pressure;

    Serial.printf("Temp: %.2f °C, Feuchte: %.2f %%, Druck: %.2f hPa\n",
                  temperature, humidity, pressure);
  }

  // JSON alle 60 Sekunden senden
  if (now - lastSend > 60000) {
    lastSend = now;

    jsonDoc["location"] = "Alfons-Kern-Schule";
    jsonDoc["room"] = "Serverraum";
    jsonDoc["temperature"] = envSensor.temperature;
    jsonDoc["humidity"] = envSensor.humidity;
    jsonDoc["pressure"] = envSensor.pressure;
    jsonDoc["timestamp"] = now / 1000;

    char buffer[256];
    serializeJson(jsonDoc, buffer);

    Serial.print("Sende JSON: ");
    Serial.println(buffer);

    client.publish("M5Stack", buffer);
  }
}