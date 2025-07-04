#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define WIFI_SSID     "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

#define MQTT_SERVER   "YOUR_MQTT_BROKER_ADDRESS"
#define MQTT_PORT     1883

#define DHTPIN        4 
#define DHTTYPE       DHT22

const char* mqtt_topic = "schule/aks/1-stock/serverraum/server-rack/temperatur";

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

unsigned long lastMsg = 0;
const long interval = 30000;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected, IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client_Group6")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  setup_wifi();
  client.setServer(MQTT_SERVER, MQTT_PORT);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;

    float t = dht.readTemperature();
    if (isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    char payload[128];
    snprintf(payload, sizeof(payload),
      "{\"location\":\"Alfons-Kern-Schule\",\"room\":\"Serverraum\",\"value\":%.2f,\"unit\":\"°C\",\"timestamp\":%lu}",
      t, (unsigned long)time(NULL));

    Serial.print("Publishing: ");
    Serial.println(payload);

    client.publish(mqtt_topic, payload, true);
  }
}
