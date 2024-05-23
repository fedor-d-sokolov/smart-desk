#include "ScioSense_ENS160.h"
#include <AHTxx.h>
#include <hcsr04.h>
#include "FastLED.h"
#include <HardwareSerial.h>

HardwareSerial SerialPort2 (2);

struct Button_data{
  bool vol_up_pressed = false;
  bool vol_down_pressed = false;
  bool scan_pressed = false;
  bool mode_pressed = false;
  bool rainbow = true;
};

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
Button_data button_data;

#define ECHO_PIN 2
#define TRIG_PIN 4
#define LED_PIN 25

#define NUM_LEDS 100

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
CRGB leds[NUM_LEDS];

void setup() {
  Serial.begin(115200);
  SerialPort2.begin(115200, SERIAL_8N1, 16, 17);

  FastLED.addLeds<WS2811, LED_PIN, GRB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(50);

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

  xTaskCreatePinnedToCore (
    loop2,
    "loop2",
    1000,
    NULL,
    0,
    NULL,
    0
  );
}

static uint32_t scanTime;
static bool mode_pressed = false;
static bool scan_pressed = false;

void loop() {
  ens160.measure(0);

  if (button_data.rainbow)
  {
    static byte counter = 0;
    for (int i = 0; i < NUM_LEDS; i++ ) {
      leds[i] = CHSV(counter + i * 2, 255, 255);
    }
    counter++;

    FastLED.show();
  }

  if (millis() - scanTime > 10000)
  {
    scanTime = millis();
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
  }

  static bool vol_up_button_mode = false;
  static bool vol_down_button_mode = false;
  if (button_data.mode_pressed && !mode_pressed)
  {
    mode_pressed = true;
    pinMode(MODE_PIN, OUTPUT);
    delay(50);
    pinMode(MODE_PIN, INPUT);
  }
  if (button_data.scan_pressed && !scan_pressed)
  {
    scan_pressed = true;
    pinMode(SCAN_PIN, OUTPUT);
    delay(50);
    pinMode(SCAN_PIN, INPUT);
  }

  if (button_data.vol_up_pressed && !vol_up_button_mode)
  {
    pinMode(VOL_UP_PIN, OUTPUT);
    vol_up_button_mode = true;
  }
  if (button_data.vol_down_pressed && !vol_down_button_mode)
  {
    pinMode(VOL_DOWN_PIN, OUTPUT);
    vol_down_button_mode = true;
  }

  if (!button_data.vol_up_pressed && vol_up_button_mode)
  {
    pinMode(VOL_UP_PIN, INPUT);
    vol_up_button_mode = false;
  }
  if (!button_data.vol_down_pressed && vol_down_button_mode)
  {
    pinMode(VOL_DOWN_PIN, INPUT);
    vol_down_button_mode = false;
  }
  
}

void loop2 (void* pvParameters) {
  while (true)
  {
    while (!SerialPort2.readBytes((byte*)&button_data, sizeof(button_data)))
    {
      Serial.println("Waiting....");
    }
    scan_pressed = false;
    mode_pressed = false;
    Serial.println("Got button data");
    Serial.println("mode: " + String(button_data.mode_pressed));
    Serial.println("scan: " + String(button_data.scan_pressed));
  }
}
