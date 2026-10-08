// ENG5094 - Lab 3, Task 1
// Arduino UNO R4 WiFi + DHT11 + Blynk Cloud

#define BLYNK_PRINT Serial
#include "secrets.h"

#include <WiFiS3.h>
#include <BlynkSimpleWifi.h>
#include <DHT.h>

// DHT11 connection
#define DHTPIN A1
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;


// Read DHT11 and send data to Blynk
void sendSensor() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Check whether the sensor reading is valid
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT11 sensor!");
    return;
  }

  // Send readings to Blynk
  // These virtual pins MUST match your Blynk datastreams
  Blynk.virtualWrite(V1, humidity);
  Blynk.virtualWrite(V2, temperature);

  // Display readings in Serial Monitor
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.println("--------------------");
}


void setup() {
  Serial.begin(9600);

  dht.begin();

  // Connect Arduino to Wi-Fi and Blynk Cloud
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Read and send sensor data every second
  timer.setInterval(1000L, sendSensor);
}


void loop() {
  Blynk.run();
  timer.run();
}