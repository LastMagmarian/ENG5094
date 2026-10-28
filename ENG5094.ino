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
#include <functional> // Since you have used std::function you should also inlcude this - Youssef 

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
      }      // i Hvae removed Blynk.virtualWrite(servo_vpin, init_servo_angle); as room.begin() runs before Blynk.begin(), so it attempts to send that value before the Arduino connects to Blynk. - Youssef
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


/*Youssef's edits and contribution to the code */



const float TEMP_ON  = 28.0;
const float TEMP_OFF = 27.0;

const float HUMIDITY_ON  = 75.0;
const float HUMIDITY_OFF = 70.0;

// Example MQ135 thresholds: adjust after calibration/testing.
const float AIR_ON  = 1000.0;
const float AIR_OFF = 800.0;

// Check these angles against your actual window mechanism.
const int WINDOW_CLOSED = 0;
const int WINDOW_OPEN   = 90;

const int ALARM_FREQUENCY = 2000;

// Index 0 = left side; index 1 = right side.
bool temperatureHigh[2] = {false, false};
bool humidityHigh[2]    = {false, false};
bool airPoor[2]         = {false, false};
bool previousAlarm[2]   = {false, false};


// Update one condition using separate ON and OFF thresholds.
void updateCondition(float reading, float onThreshold,
                     float offThreshold, bool &condition) {
  // An invalid reading does not clear an existing alarm.
  if (isnan(reading) || isinf(reading)) {
    return;
  }

  if (reading >= onThreshold) {
    condition = true;
  } else if (reading <= offThreshold) {
    condition = false;
  }
}


// Read the stored data and control both sides of the house.
void updateAutomaticControl(bool forceDashboardUpdate = false) {
  bool anyAlarm = false;

  for (int side = 0; side < 2; side++) {
    // Left: rooms[0] and rooms[2].
    // Right: rooms[1] and rooms[3].
    Room &backRoom  = rooms[side];
    Room &frontRoom = rooms[side + 2];

    updateCondition(backRoom.data.temperature,
                    TEMP_ON, TEMP_OFF,
                    temperatureHigh[side]);

    updateCondition(backRoom.data.humidity,
                    HUMIDITY_ON, HUMIDITY_OFF,
                    humidityHigh[side]);

    updateCondition(frontRoom.data.ppm,
                    AIR_ON, AIR_OFF,
                    airPoor[side]);

    // Any one condition activates this side.
    bool alarm = temperatureHigh[side]
                 || humidityHigh[side]
                 || airPoor[side];

    int ledState = alarm ? HIGH : LOW;
    int windowAngle = alarm ? WINDOW_OPEN : WINDOW_CLOSED;

    backRoom.setLED(ledState);
    frontRoom.setLED(ledState);
    backRoom.setServo(windowAngle);

    // Update the app when the output changes or reconnects.
    if (Blynk.connected()
        && (forceDashboardUpdate
            || alarm != previousAlarm[side])) {
      Blynk.virtualWrite(backRoom.led_vpin, alarm ? 1 : 0);
      Blynk.virtualWrite(frontRoom.led_vpin, alarm ? 1 : 0);
      Blynk.virtualWrite(backRoom.servo_vpin, windowAngle);
    }

    previousAlarm[side] = alarm;
    anyAlarm = anyAlarm || alarm;
  }

  // One shared buzzer: sound if either side needs an alarm.
  if (anyAlarm) {
    tone(BUZZER_PIN, ALARM_FREQUENCY);
  } else {
    noTone(BUZZER_PIN);
  }

  static bool previousBuzzer = false;

  if (Blynk.connected()
      && (forceDashboardUpdate || anyAlarm != previousBuzzer)) {
    Blynk.virtualWrite(
      BUZZER_VPIN,
      anyAlarm ? ALARM_FREQUENCY : 0
    );
  }

  previousBuzzer = anyAlarm;
}


// Read every sensor before making control decisions.
void onTimer() {
  for (Room &room : rooms) {
    room.sendData();
  }

  for (Room &room : rooms) {
    room.process();
  }

  updateAutomaticControl();
}


// Send the actual automatic output states to the app.
BLYNK_CONNECTED() {
  updateAutomaticControl(true);
}


void setup() {
  Serial.begin(115200);

  rooms[0].connected = &rooms[2];
  rooms[1].connected = &rooms[3];
  rooms[2].connected = &rooms[0];
  rooms[3].connected = &rooms[1];

  pinMode(BUZZER_PIN, OUTPUT);
  noTone(BUZZER_PIN);

  for (Room &room : rooms) {
    if (room.led_pin != -1) {
      pinMode(room.led_pin, OUTPUT);
      room.setLED(LOW);
    }

    room.begin();

    if (room.servo_pin != -1) {
      room.setServo(WINDOW_CLOSED);
    }
  }

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Read sensors and update controls every two seconds.
  timer.setInterval(2000L, onTimer);
}


void loop() {
  Blynk.run();
  timer.run();
}


