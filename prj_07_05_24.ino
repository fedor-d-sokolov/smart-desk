#include "ScioSense_ENS160.h"
#include <AHTxx.h>

#define MODE_PIN 13
#define VOL_UP_PIN 12
#define SCAN_PIN 14 
#define VOL_DOWN_PIN 27

// MODE - фиолетовый проводок - 13 пин
// VOL+ - серый проводок - 12 пин
// SCAN - синий проводок - 14 пин
// VOL- - оранжевый проводок - 27 пин

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
  pinMode(MODE_PIN, INPUT);
  pinMode(VOL_UP_PIN, INPUT);
  pinMode(SCAN_PIN, INPUT);
  pinMode(VOL_DOWN_PIN, INPUT);

  Serial.begin(9600);
  if (!ens160.begin()) {
    Serial.println("Error while starting ENS160");
  }

  if (!aht20.begin()) {
    Serial.println("Error while starting AHT20");
  }

  Serial.println("MENU");
  Serial.println("1 - Display ENS160/AHT20 data");
  Serial.println("2 - Press 'mode' button");
  Serial.println("3 - Press 'scan' button");
  Serial.println("4 - Press 'vol+' button");
  Serial.println("5 - Press 'vol-' button");
}

void loop() {
  int user_input = Serial.parseInt();
  if (user_input == 1){
    ens160.measure(0);
    float aqi = ens160.getAQI();
    float tvoc = ens160.getTVOC();
    float eco2 = ens160.geteCO2();
    float hp0 = ens160.getHP0();
    float hp1 = ens160.getHP0();
    float hp2 = ens160.getHP0();
    float hp3 = ens160.getHP0();

    float temperature = aht20.readTemperature();
    float humidity = aht20.readHumidity();

    Serial.println("\nAHT20 DATA");
    Serial.println("Temperature: " + String(temperature));
    Serial.println("Humidity: " + String(humidity));
    Serial.println("\nENS160 DATA");
    Serial.println("aqi: " + String(aqi));
    Serial.println("tvoc: " + String(tvoc));
    Serial.println("eco2: " + String(eco2));
    Serial.println("hp0: " + String(hp0));
    Serial.println("hp1: " + String(hp1));
    Serial.println("hp2: " + String(hp2));
    Serial.println("hp3: " + String(hp3));
  }

  if (user_input == 2)
  {
    press_mode_button();
  }

  if (user_input == 3)
  {
    press_scan_button();
  }

  if (user_input == 4)
  {
    press_volup_button();
  }
    
  if (user_input == 5)
  {
    press_voldown_button();
  }
  
  delay(100);
}
