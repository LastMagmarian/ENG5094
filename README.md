# This repo is for the code for our group project

## Requirements
### Software
- Arduino IDE
- [Blynk](https://blynk.cloud) and its associated [library]()
- [Adafruit DHT Library](https://github.com/adafruit/DHT-sensor-library)
- [MQ135 Library](https://github.com/NuclearPhoenixx/MQ135)
- A `secrets.h` file placed alongside `END5094.ino` with the following contents
```c
#pragma once

// Enter your Blynk device information
#define BLYNK_TEMPLATE_ID   "Your template ID"
#define BLYNK_TEMPLATE_NAME "Your template name"
#define BLYNK_AUTH_TOKEN    "Your Blynk auth token"

// Wi-Fi credentials
const char ssid[] = "Your SSID";
const char pass[] = "Your WiFi password";
```

  All libraries can be installed through the Arduino IDE.

### Hardware
- Arduino Uno R4
- 2 MQ135 Sensors
- 2 DHT11 Sensors
- 1 Buzzer
- 2 Servos
- Our custom 3D printed house
