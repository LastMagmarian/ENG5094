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

struct RoomData {
  float temperature = NAN;
  float humidity    = NAN;
  float ppm         = NAN;
};

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

    Room* connected = NULL;

    struct RoomData data;
    std::function<void()> process = [](){};

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

    bool sendDHT() {
      if (dht.has_value()) {
        data.humidity = dht.value().readHumidity();
        data.temperature = dht.value().readTemperature();

        // Check whether the sensor reading is valid
        if (isnan(data.humidity) || isnan(data.temperature)) {
          Serial.print("Failed to read from DHT11 sensor");
          return false;
        }

        // Send readings to Blynk
        Blynk.virtualWrite(humidity_vpin, data.humidity);
        Blynk.virtualWrite(temperature_vpin, data.temperature);

        // Display readings in Serial Monitor
        Serial.print("Temperature: ");
        Serial.print(data.temperature);
        Serial.println(" °C");

        Serial.print("Humidity: ");
        Serial.print(data.humidity);
        Serial.println(" %");
        
        return true;
      }
      return false;
    }

    void sendMQ135(std::optional<RoomData> dht_data = std::nullopt) {
      if (mq135.has_value()) {
        if (dht_data.has_value()) data.ppm = mq135.value().getCorrectedPPM(dht_data.value().temperature, dht_data.value().humidity);
        else data.ppm = mq135.value().getPPM();

        // Check whether the sensor reading is valid
        if (isnan(data.ppm)) {
          Serial.print("Failed to read from MQ135 sensor");
          return;
        }

        // Send readings to Blynk
        Blynk.virtualWrite(ppm_vpin, data.ppm);

        // Display readings in Serial Monitor
        Serial.print("PPM: ");
        Serial.print(data.ppm);
        Serial.println(" ppm");
      }
    }

    // If they are in the same room you can get improved accuracy from the MQ135
    void sendDHTAndMQ135() {
      if (sendDHT()) {
        sendMQ135(data);
      } else if (connected != NULL && !isnan(connected->data.temperature)) {
        sendMQ135(connected->data);
      } else sendMQ135();
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

    Room(const char* name, DHT dht, int humidity_vpin, int temperature_vpin, std::function<void()> process = [](){}) :
      Room(name, dht, humidity_vpin, temperature_vpin, std::nullopt, -1, -1, -1, -1, -1, -1, process) {};
    Room(const char* name, MQ135 mq135, int ppm_vpin, std::function<void()> process = [](){}) :
      Room(name, std::nullopt, -1, -1, mq135, ppm_vpin, -1, -1, -1, -1, -1, process) {};
    Room(const char* name, int led_pin, int led_vpin, std::function<void()> process = [](){}) :
      Room(name, std::nullopt, -1, -1, std::nullopt, -1, led_pin, led_vpin, -1, -1, -1, process) {};
    Room(const char* name, int servo_pin, int servo_vpin, const int init_servo_angle, std::function<void()> process = [](){}) :
      Room(name, std::nullopt, -1, -1, std::nullopt, -1, -1, -1, servo_pin, servo_vpin, init_servo_angle, process) {};

    Room(const char* name, DHT dht, int humidity_vpin, int temperature_vpin, int led_pin, int led_vpin, std::function<void()> process = [](){}) :
      Room(name, dht, humidity_vpin, temperature_vpin, std::nullopt, -1, led_pin, led_vpin, -1, -1, -1, process) {};
    Room(const char* name, MQ135 mq135, int ppm_vpin, int led_pin, int led_vpin, std::function<void()> process = [](){}) :
      Room(name, std::nullopt, -1, -1, mq135, ppm_vpin, led_pin, led_vpin, -1, -1, -1, process) {};
    Room(const char* name, int servo_pin, int servo_vpin, const int init_servo_angle, int led_pin, int led_vpin, std::function<void()> process = [](){}) :
      Room(name, std::nullopt, -1, -1, std::nullopt, -1, led_pin, led_vpin, servo_pin, servo_vpin, init_servo_angle, process) {};

    Room(const char* name, DHT dht, int humidity_vpin, int temperature_vpin, MQ135 mq135, int ppm_vpin, int led_pin, int led_vpin, std::function<void()> process = [](){}) :
      Room(name, dht, humidity_vpin, temperature_vpin, mq135, ppm_vpin, led_pin, led_vpin, -1, -1, -1, process) {};
    Room(const char* name, DHT dht, int humidity_vpin, int temperature_vpin, int servo_pin, int servo_vpin, const int init_servo_angle, int led_pin, int led_vpin, std::function<void()> process = [](){}) :
      Room(name, dht, humidity_vpin, temperature_vpin, std::nullopt, -1, led_pin, led_vpin, servo_pin, servo_vpin, init_servo_angle, process) {};

    Room(const char* name, std::optional<DHT> dht, int humidity_vpin, int temperature_vpin, std::optional<MQ135> mq135, int ppm_vpin, int led_pin, int led_vpin, int servo_pin, int servo_vpin, const int init_servo_angle, std::function<void()> process = [](){}) :
      name(name),
      dht(dht), humidity_vpin(humidity_vpin), temperature_vpin(temperature_vpin),
      mq135(mq135), ppm_vpin(ppm_vpin),
      led_pin(led_pin), led_vpin(led_vpin),
      servo_pin(servo_pin), servo_vpin(servo_vpin), init_servo_angle(init_servo_angle),
      process(process)
      {};
    private:
      Servo servo;
};

BlynkTimer timer;
// Define rooms here
Room rooms[4] {
  Room("Back Left", DHT(D2, DHTTYPE), V0, V1, D9, V24, 0, D6, V23, [](){
    // Do stuff with rooms[0] (Back Left) data here
  }),
  Room("Back Right", DHT(D4, DHTTYPE), V10, V11, D10, V34, 0, D7, V33, [](){
    // Do stuff with rooms[1] (Back Left) data here
  }),
  Room("Front Left", MQ135(A0), V2, D3, V3, [](){
    // Do stuff with rooms[2] (Front Left) data here
  }),
  Room("Front Right", MQ135(A1), V12, D5, V13, [](){
    // Do stuff with rooms[3] (Front Right) data here
  })
};

void onTimer() {
  for (Room room : rooms) { 
    room.sendData();
    room.process();

  }
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

BLYNK_CONNECTED() {
  for (Room room : rooms) {
    Blynk.syncVirtual(room.led_vpin, room.servo_vpin);
  }
  Blynk.syncVirtual(BUZZER_VPIN);
}

void setup() {
  // Setup connections between rooms
  rooms[0].connected = &rooms[2];
  rooms[1].connected = &rooms[3];
  rooms[2].connected = &rooms[0];
  rooms[3].connected = &rooms[1];

  Serial.begin(115200);

  for (Room room : rooms) room.begin();

  // Connect Arduino to Wi-Fi and Blynk Cloud
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Read and send sensor data every second
  timer.setInterval(1000L, onTimer);
}

void loop() {
  Blynk.run();
  timer.run();
}