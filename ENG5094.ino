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
#include <MQ135.h>
#include <Servo.h>
#include <optional>

#define DHTTYPE     DHT11
#define BUZZER_VPIN V45
#define BUZZER_PIN  13

class Room {
  public:
    const char* name;

    std::optional<DHT> dht;
    int humidity_vpin = -1;
    int temperature_vpin = -1;

    std::optional<MQ135> mq135;
    int ppm_vpin = -1;

    int led_pin  = -1;
    int led_vpin = -1;

    // Using pins 9 and 10 is recommended as they become unusable for PWM if ANY pin is used for a servo.
    int servo_pin   = -1;
    int servo_vpin  = -1;
    const int init_servo_angle;

    void begin() {
      if (dht.has_value()) dht.value().begin();
      if (servo_pin != -1) {
        servo.attach(servo_pin);
        servo.write(init_servo_angle);
        Blynk.virtualWrite(servo_vpin, init_servo_angle);
      }
    }

    void sendData() {
      Serial.print(name);
      Serial.println(":");
      sendDHTAndMQ135();
      Serial.println();
    }

    void sendDHT() {
      if (dht.has_value()) {
        float humidity = dht.value().readHumidity();
        float temperature = dht.value().readTemperature();

        // Check whether the sensor reading is valid
        if (isnan(humidity) || isnan(temperature)) {
          Serial.print("Failed to read from DHT11 sensor");
          return;
        }

        // Send readings to Blynk
        Blynk.virtualWrite(humidity_vpin, humidity);
        Blynk.virtualWrite(temperature_vpin, temperature);

        // Display readings in Serial Monitor
        Serial.print("Temperature: ");
        Serial.print(temperature);
        Serial.println(" °C");

        Serial.print("Humidity: ");
        Serial.print(humidity);
        Serial.println(" %");
      }
    }

    void sendMQ135() {
      if (mq135.has_value()) {
        float ppm = mq135.value().getPPM();

        // Check whether the sensor reading is valid
        if (isnan(ppm)) {
          Serial.print("Failed to read from MQ135 sensor");
          return;
        }

        // Send readings to Blynk
        Blynk.virtualWrite(ppm_vpin, ppm);

        // Display readings in Serial Monitor
        Serial.print("PPM: ");
        Serial.print(ppm);
        Serial.println(" ppm");
      }
    }

    // If they are in the same room you can get improved accuracy from the MQ135
    void sendDHTAndMQ135() {
      if (dht.has_value() && mq135.has_value()) {
        float humidity = dht.value().readHumidity();
        float temperature = dht.value().readTemperature();

        // Check whether the sensor reading is valid
        if (isnan(humidity) || isnan(temperature)) {
          Serial.print("Failed to read from DHT11 sensor");
          return;
        }

        float ppm = mq135.value().getCorrectedPPM(temperature, humidity);

        // Check whether the sensor reading is valid
        if (isnan(ppm)) {
          Serial.print("Failed to read from MQ135 sensor");
          return;
        }

        // Send readings to Blynk
        Blynk.virtualWrite(humidity_vpin, humidity);
        Blynk.virtualWrite(temperature_vpin, temperature);
        Blynk.virtualWrite(ppm_vpin, ppm);

        // Display readings in Serial Monitor
        Serial.print("Temperature: ");
        Serial.print(temperature);
        Serial.println(" °C");

        Serial.print("Humidity: ");
        Serial.print(humidity);
        Serial.println(" %");

        Serial.print("PPM: ");
        Serial.print(ppm);
        Serial.println(" ppm");
      } else {
        sendDHT();
        sendMQ135();
      }
    }

    void setLED(int status) {
      if (led_pin != -1) {
        digitalWrite(led_pin, status);
      }
    }

    void setServo(int angle) {
      if (servo_pin != -1) {
        servo.write(angle);
      }
    }

    Room() : Room("") {};
    Room(const char* name) : name(name), init_servo_angle(-1) {};

    Room(const char* name, DHT dht, int humidity_vpin, int temperature_vpin) :
      Room(name, dht, humidity_vpin, temperature_vpin, std::nullopt, -1, -1, -1, -1, -1, -1) {};
    Room(const char* name, MQ135 mq135, int ppm_vpin) :
      Room(name, std::nullopt, -1, -1, mq135, ppm_vpin, -1, -1, -1, -1, -1) {};
    Room(const char* name, int led_pin, int led_vpin) :
      Room(name, std::nullopt, -1, -1, std::nullopt, -1, led_pin, led_vpin, -1, -1, -1) {};
    Room(const char* name, int servo_pin, int servo_vpin, const int init_servo_angle) :
      Room(name, std::nullopt, -1, -1, std::nullopt, -1, -1, -1, servo_pin, servo_vpin, init_servo_angle) {};

    Room(const char* name, DHT dht, int humidity_vpin, int temperature_vpin, int led_pin, int led_vpin) :
      Room(name, dht, humidity_vpin, temperature_vpin, std::nullopt, -1, led_pin, led_vpin, -1, -1, -1) {};
    Room(const char* name, MQ135 mq135, int ppm_vpin, int led_pin, int led_vpin) :
      Room(name, std::nullopt, -1, -1, mq135, ppm_vpin, led_pin, led_vpin, -1, -1, -1) {};
    Room(const char* name, int servo_pin, int servo_vpin, const int init_servo_angle, int led_pin, int led_vpin) :
      Room(name, std::nullopt, -1, -1, std::nullopt, -1, led_pin, led_vpin, servo_pin, servo_vpin, init_servo_angle) {};

    Room(const char* name, DHT dht, int humidity_vpin, int temperature_vpin, MQ135 mq135, int ppm_vpin, int led_pin, int led_vpin) :
      Room(name, dht, humidity_vpin, temperature_vpin, mq135, ppm_vpin, led_pin, led_vpin, -1, -1, -1) {};

    Room(const char* name, std::optional<DHT> dht, int humidity_vpin, int temperature_vpin, std::optional<MQ135> mq135, int ppm_vpin, int led_pin, int led_vpin, int servo_pin, int servo_vpin, const int init_servo_angle) :
      name(name),
      dht(dht), humidity_vpin(humidity_vpin), temperature_vpin(temperature_vpin),
      mq135(mq135), ppm_vpin(ppm_vpin),
      led_pin(led_pin), led_vpin(led_vpin),
      servo_pin(servo_pin), servo_vpin(servo_vpin), init_servo_angle(init_servo_angle)
      {};
    private:
      Servo servo;
};

BlynkTimer timer;
// Define rooms here
Room rooms[4] {
  Room("Front Left", DHT(D2, DHTTYPE), V0, V1, MQ135(A0), V2, D3, V3),
  Room("Front Right", DHT(D4, DHTTYPE), V10, V11, MQ135(A1), V12, D5, V13),
  Room("Back Left", D9, V24, 0, D6, V23),
  Room("Back Right", D10, V34, 0, D7, V33)
};

void sendData() {
  for (Room room : rooms) room.sendData();
}

// This is sadly the only way of getting data from blynk while keeping the led_vpin and servo_vpin in the Room class,
// as BLYNK_WRITE(...) {...} doesn't work with variables. At least its only 4 iterations per message.
BLYNK_WRITE_DEFAULT() {
  for (Room room : rooms) {
    if (request.pin == room.led_vpin) {
      room.setLED(param.asInt());
      break;
    } else if (request.pin == room.servo_vpin) {
      room.setServo(param.asInt());
      break;
    }
  }
}

BLYNK_WRITE(BUZZER_VPIN) {
  int frequency = param.asInt();
  if (frequency <= 0) {
    noTone(BUZZER_PIN);
  } else {
    tone(BUZZER_PIN, frequency);
  }
}

void setup() {
  Serial.begin(115200);

  for (Room room : rooms) room.begin();

  // Connect Arduino to Wi-Fi and Blynk Cloud
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Read and send sensor data every second
  timer.setInterval(1000L, sendData);
}

void loop() {
  Blynk.run();
  timer.run();
}