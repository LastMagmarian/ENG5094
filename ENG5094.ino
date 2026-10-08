#define BLYNK_PRINT Serial
/*
  You must create this file yourself as no auth tokens should be committed to the repo.
  Must include #defines for BLYNK_TEMPLATE_ID, BLYNK_TEMPLATE_NAME and BLYNK_AUTH_TOKEN
  as well as the definitions of const char ssid[] and const char pass[].
*/
#include "secrets.h"

#include <WiFiS3.h>
#include <BlynkSimpleWifi.h>
#include <DHT.h>

// DHT11 connections
#define DHTA_PIN 2
#define DHTB_PIN 3
#define DHTTYPE DHT11

DHT dhta(DHTA_PIN, DHTTYPE);
DHT dhtb(DHTB_PIN, DHTTYPE);
BlynkTimer timer;


// Read DHT11 and send data to Blynk
void sendDHT(DHT &dht, int humidity_vpin, int temperature_vpin, const char* name = "DHT") {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Check whether the sensor reading is valid
  if (isnan(humidity) || isnan(temperature)) {
    Serial.print("Failed to read from DHT11 sensor (");
    Serial.print(name);
    Serial.println(")!");
    return;
  }

  // Send readings to Blynk
  Blynk.virtualWrite(humidity_vpin, humidity);
  Blynk.virtualWrite(temperature_vpin, temperature);

  // Display readings in Serial Monitor
  Serial.print(name);
  Serial.println(": ");
  Serial.print("  Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");

  Serial.print("  Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.println("--------------------");
}

void sendSensors() {
  sendDHT(dhta, V0, V1, "DHT A");
  sendDHT(dhtb, V2, V3, "DHT B");
}

void setup() {
  Serial.begin(9600);

  dhta.begin();
  dhtb.begin();

  // Connect Arduino to Wi-Fi and Blynk Cloud
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Read and send sensor data every second
  timer.setInterval(1000L, sendSensors);
}


void loop() {
  Blynk.run();
  timer.run();
}