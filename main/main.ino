#include "ScioSense_ENS160.h"
#include <AHTxx.h>
#include <hcsr04.h>
#include <HardwareSerial.h>

HardwareSerial SerialPort2 (2);

struct Data {
  float aqi;
  float tvoc;
  float eco2;
  float hp0;
  float hp1;
  float hp2;
  float hp3;

  float temperature;
  float humidity;
  float distance;
};

Data data;

#define ECHO_PIN 2
#define TRIG_PIN 4

#define MODE_PIN 13
#define VOL_UP_PIN 12
#define SCAN_PIN 14 
#define VOL_DOWN_PIN 27

// HC-RSO4
// Trig - 4
// Echo - 2

// MODE - фиолетовый проводок - 13 пин
// VOL+ - серый проводок - 12 пин
// SCAN - синий проводок - 14 пин
// VOL- - оранжевый проводок - 27 пин

HCSR04 hcsr04(TRIG_PIN, ECHO_PIN, 20, 4000);
ScioSense_ENS160 ens160(ENS160_I2CADDR_1);
AHTxx aht20(AHTXX_ADDRESS_X38, AHT2x_SENSOR);

void press_mode_button(){
  pinMode(MODE_PIN, OUTPUT);
  delay(50);
  pinMode(MODE_PIN, INPUT);
}

void press_volup_button(){
  pinMode(VOL_UP_PIN, OUTPUT);
  delay(5000);
  pinMode(VOL_UP_PIN, INPUT);
}

void press_scan_button(){
  pinMode(SCAN_PIN, OUTPUT);
  delay(50);
  pinMode(SCAN_PIN, INPUT);
}

void press_voldown_button(){
  pinMode(VOL_DOWN_PIN, OUTPUT);
  delay(5000);
  pinMode(VOL_DOWN_PIN, INPUT);
}

void setup() {
  Serial.begin(115200);
  SerialPort2.begin(115200, SERIAL_8N1, 16, 17);

  pinMode(MODE_PIN, INPUT);
  pinMode(VOL_UP_PIN, INPUT);
  pinMode(SCAN_PIN, INPUT);
  pinMode(VOL_DOWN_PIN, INPUT);

  if (!ens160.begin()) {
    Serial.println("Error while starting ENS160");
  }

  if (!aht20.begin()) {
    Serial.println("Error while starting AHT20");
  }
}

void loop() {
  ens160.measure(0);

  data.aqi = ens160.getAQI();
  data.tvoc = ens160.getTVOC();
  data.eco2 = ens160.geteCO2();
  data.hp0 = ens160.getHP0();
  data.hp1 = ens160.getHP0();
  data.hp2 = ens160.getHP0();
  data.hp3 = ens160.getHP0();

  data.temperature = aht20.readTemperature();
  data.humidity = aht20.readHumidity();
  data.distance = hcsr04.distanceInMillimeters();

  while (!data.temperature) {
    data.humidity = aht20.readTemperature();
  }

  while (!data.humidity) {
    data.humidity = aht20.readHumidity();
  }

  while (!data.distance) {
    data.distance = hcsr04.distanceInMillimeters();
  }

  Serial.println("Temperature: " + String(data.temperature));
  Serial.println("Humidity: " + String(data.humidity));
  Serial.println("Distance: " + String(data.distance));

  SerialPort2.write((byte*)&data, sizeof(data));

  delay(10000);
}
