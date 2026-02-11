#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include "secrets.h"   // <-- credentials are here

// Pins
#define SOIL_PIN   A0
#define RELAY_PIN  5      // D1
#define DHTPIN     4      // D2
#define DHTTYPE    DHT11

DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

// Variables
int moisturePercent = 0;
float temperature = 0;
float humidity = 0;

int soilThreshold = 40;   // %
bool autoMode = true;

// Blynk Virtual Pins
#define VPIN_SOIL     V1
#define VPIN_MODE     V2
#define VPIN_RELAY    V3
#define VPIN_TEMP     V4
#define VPIN_HUM      V5
#define VPIN_SOIL_TH  V6

// Auto / Manual switch
BLYNK_WRITE(VPIN_MODE) {
  autoMode = param.asInt();
}

// Manual pump control
BLYNK_WRITE(VPIN_RELAY) {
  if (!autoMode) {
    digitalWrite(RELAY_PIN, param.asInt() ? LOW : HIGH);
  }
}

// Soil threshold slider
BLYNK_WRITE(VPIN_SOIL_TH) {
  soilThreshold = param.asInt();
}

// Read sensors & control pump
void irrigationTask() {

  // Soil moisture
  int soilValue = analogRead(SOIL_PIN);
  moisturePercent = map(soilValue, 1023, 0, 0, 100);
  Blynk.virtualWrite(VPIN_SOIL, moisturePercent);

  // DHT11
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();

  if (!isnan(temperature) && !isnan(humidity)) {
    Blynk.virtualWrite(VPIN_TEMP, temperature);
    Blynk.virtualWrite(VPIN_HUM, humidity);
  }

  // AUTO IRRIGATION (soil-based)
  if (autoMode) {
    if (moisturePercent < soilThreshold) {
      digitalWrite(RELAY_PIN, LOW);   // Pump ON
    } else {
      digitalWrite(RELAY_PIN, HIGH);  // Pump OFF
    }
  }

  // Debug
  Serial.print("Soil: ");
  Serial.print(moisturePercent);
  Serial.print("% | Threshold: ");
  Serial.print(soilThreshold);
  Serial.print("% | Temp: ");
  Serial.print(temperature);
  Serial.print("C | Hum: ");
  Serial.println(humidity);
}

void setup() {
  Serial.begin(9600);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // Pump OFF

  dht.begin();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(2000L, irrigationTask);
}

void loop() {
  Blynk.run();
  timer.run();
}
