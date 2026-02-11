/****************************************************
 * SMART IRRIGATION SYSTEM – ESP8266 + BLYNK
 * Author: Kamlesh
 * Description:
 *  - Soil moisture–based automatic irrigation
 *  - Manual / Auto mode via Blynk
 *  - DHT11 temperature & humidity monitoring
 ****************************************************/

// -------- BLYNK CONFIG (USER MUST CHANGE) --------
#define BLYNK_TEMPLATE_ID   "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "SMART IRRIGATION WITH TEMP"
#define BLYNK_AUTH_TOKEN    "YOUR_AUTH_TOKEN"

// -------- WIFI CONFIG (USER MUST CHANGE) --------
char ssid[] = "YOUR_WIFI_NAME";
char pass[] = "YOUR_WIFI_PASSWORD";

// ------------------------------------------------

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>

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

// Auto / Manual mode switch
BLYNK_WRITE(VPIN_MODE) {
  autoMode = param.asInt();
}

// Manual pump control
BLYNK_WRITE(VPIN_RELAY) {
  if (!autoMode) {
    digitalWrite(RELAY_PIN, param.asInt() ? LOW : HIGH);
  }
}

// Soil moisture threshold slider
BLYNK_WRITE(VPIN_SOIL_TH) {
  soilThreshold = param.asInt();
}

// Read sensors & control pump
void irrigationTask() {

  // Soil moisture
  int soilValue = analogRead(SOIL_PIN);
  moisturePercent = map(soilValue, 1023, 0, 0, 100);
  Blynk.virtualWrite(VPIN_SOIL, moisturePercent);

  // DHT11 readings
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();

  if (!isnan(temperature) && !isnan(humidity)) {
    Blynk.virtualWrite(VPIN_TEMP, temperature);
    Blynk.virtualWrite(VPIN_HUM, humidity);
  }

  // AUTO IRRIGATION (soil-based logic)
  if (autoMode) {
    if (moisturePercent < soilThreshold) {
      digitalWrite(RELAY_PIN, LOW);   // Pump ON
    } else {
      digitalWrite(RELAY_PIN, HIGH);  // Pump OFF
    }
  }

  // Debug output
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

  // Run every 2 seconds
  timer.setInterval(2000L, irrigationTask);
}

void loop() {
  Blynk.run();
  timer.run();
}
