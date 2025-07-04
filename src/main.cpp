#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// === CONFIGURATION ===
#define WIFI_SSID     "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

#define MQTT_SERVER   "YOUR_MQTT_BROKER_ADDRESS"
#define MQTT_PORT     1883 // Use 8883 for TLS if needed

#define DHTPIN        4      // GPIO Pin where your DHT22 is connected
#define DHTTYPE       DHT22  // or DHT11

// === MQTT TOPIC ===
const char* mqtt_topic = "schule/aks/1-stock/serverraum/server-rack/temperatur";

// === GLOBALS ===
WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

unsigned long lastMsg = 0;
const long interval = 30000; // 30 seconds

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
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32Client_Group6")) {
      Serial.println("connected");
      // No need to subscribe, this is a publisher
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

    // Optionally, add location/room etc. to payload
    char payload[128];
    snprintf(payload, sizeof(payload),
      "{\"location\":\"Alfons-Kern-Schule\",\"room\":\"Serverraum\",\"value\":%.2f,\"unit\":\"°C\",\"timestamp\":%lu}",
      t, (unsigned long)time(NULL));

    Serial.print("Publishing: ");
    Serial.println(payload);

    // Publish with QoS 1 (PubSubClient supports only QoS 0 and 1)
    client.publish(mqtt_topic, payload, true); // 'true' sets retain flag, optional
  }
}
