#include <Wire.h>
#include "SHT31.h"

SHT31 sht31;

void setup() {
  Serial.begin(115200);
  Wire.begin();  // I2C starten (SDA, SCL: je nach Board)
  
  if (!sht31.begin(0x44)) {  // Standardadresse ist 0x44
    Serial.println("Sensor nicht gefunden!");
    while (1);
  }

  Serial.println("SHT31-Sensor initialisiert.");
}

void loop() {
  float temp = sht31.getTemperature();
  float hum = sht31.getHumidity();

  Serial.print("Temperatur: ");
  Serial.print(temp);
  Serial.println(" °C");

  Serial.print("Luftfeuchtigkeit: ");
  Serial.print(hum);
  Serial.println(" %");

  delay(2000);  // Alle 2 Sekunden auslesen
}