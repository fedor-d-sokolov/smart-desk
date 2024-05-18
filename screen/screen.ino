#include <FS.h>
#include <SPI.h>
#include "Free_Fonts.h" 
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <TFT_eWidget.h> 

#define XPT2046_IRQ 36   // T_IRQ
#define XPT2046_MOSI 32  // T_DIN
#define XPT2046_MISO 39  // T_OUT
#define XPT2046_CLK 25   // T_CLK
#define XPT2046_CS 33    // T_CS

#define BUTTON_W 100
#define BUTTON_H 40

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define FONT_SIZE 2

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

static int previousMode = 999;
int mode = 0;
Data data;
HardwareSerial SerialPort2 (2);
TFT_eSPI tft = TFT_eSPI();
SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

ButtonWidget btn_air = ButtonWidget(&tft);
ButtonWidget btn_led = ButtonWidget(&tft);
ButtonWidget btn_time_spent = ButtonWidget(&tft);
ButtonWidget btn_music = ButtonWidget(&tft);

ButtonWidget* btn[] = {&btn_air , &btn_led, &btn_time_spent, &btn_music};;
uint8_t buttonCount = sizeof(btn) / sizeof(btn[0]);

void btn_air_press(void) {
  if (btn_air.justPressed()) {
    Serial.println("btn_air just got pressed");
    mode = 0;
    btn_air.drawSmoothButton(true);
  }
}

void btn_led_press(void) {
  if (btn_led.justPressed()) {
    Serial.println("btn_led just got pressed");
    mode = 1;
    btn_led.drawSmoothButton(true);
  }
}

void btn_time_spent_press(void) {
  if (btn_time_spent.justPressed()) {
    Serial.println("btn_time_spent just got pressed");
    mode = 2;
    btn_time_spent.drawSmoothButton(true);
  }
}

void btn_music_press(void) {
  if (btn_music.justPressed()) {
    Serial.println("btn_music just got pressed");
    mode = 3;
    btn_music.drawSmoothButton(true);
  }
}

void btn_air_release(void) {
  if (btn_air.justReleased()) {
    Serial.println("btn_air just got released");
    btn_air.drawSmoothButton(false);
  }
}

void btn_led_release(void) {
  if (btn_led.justReleased()) {
    Serial.println("btn_led just got released");
    btn_led.drawSmoothButton(false);
  }
}

void btn_time_spent_release(void) {
  if (btn_time_spent.justReleased()) {
    Serial.println("btn_time_spent just got released");
    btn_time_spent.drawSmoothButton(false);
  }
}

void btn_music_release(void) {
  if (btn_music.justReleased()) {
    Serial.println("btn_music just got released");
    btn_music.drawSmoothButton(false);
  }
}

void initButtons() {
  int x, y;
  x = 10;

  y = 10;
  btn_air.initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_BLACK, TFT_WHITE, "Air", 1);
  btn_air.setPressAction(btn_air_press);
  btn_air.setReleaseAction(btn_air_release);
  btn_air.drawSmoothButton(false, 3, TFT_BLACK);

  y += 60;
  btn_led.initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_BLACK, TFT_WHITE, "LED", 1);
  btn_led.setPressAction(btn_led_press);
  btn_led.setReleaseAction(btn_led_release);
  btn_led.drawSmoothButton(false, 3, TFT_BLACK);

  y += 60;
  btn_time_spent.initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_BLACK, TFT_WHITE, "Work time", 1);
  btn_time_spent.setPressAction(btn_time_spent_press);
  btn_time_spent.setReleaseAction(btn_time_spent_release);
  btn_time_spent.drawSmoothButton(false, 3, TFT_BLACK);

  y += 60;
  btn_music.initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_BLACK, TFT_WHITE, "Music", 1);
  btn_music.setPressAction(btn_music_press);
  btn_music.setReleaseAction(btn_music_release);
  btn_music.drawSmoothButton(false, 3, TFT_BLACK);
}

void setup() {
  Serial.begin(115200);
  SerialPort2.begin(115200, SERIAL_8N1, 16, 17);

  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  touchscreen.setRotation(1);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawLine(120, 0, 120, 240, TFT_WHITE);
  initButtons();

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

void loop() {
  static uint32_t scanTime = millis();
  uint16_t t_x = 9999, t_y = 9999;

  if (millis() - scanTime >= 50) {
    bool pressed = touchscreen.tirqTouched() && touchscreen.touched();

    if (pressed)
    {
      TS_Point p = touchscreen.getPoint();
      t_x = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
      t_y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);
    }

    scanTime = millis();
    for (uint8_t b = 0; b < buttonCount; b++) {
      if (pressed) {
        if (btn[b]->contains(t_x, t_y)) {
          btn[b]->press(true);
          btn[b]->pressAction();
        }
      }
      else {
        btn[b]->press(false);
        btn[b]->releaseAction();
      }
    }

    delay(100);
  }

  if (previousMode != mode)
  {
    Serial.println("Mode changed");
    tft.fillScreen(TFT_BLACK);
    tft.drawLine(120, 0, 120, 240, TFT_WHITE);
    initButtons();
    int center_x = (SCREEN_WIDTH / 2) + 45;
    int text_y  = 80;
    if (mode == 0)
    {
      tft.drawCentreString("Temperature: " + String(data.temperature), center_x, text_y, FONT_SIZE);
      text_y += 20;

      tft.drawCentreString("Humidity: " + String(data.humidity), center_x, text_y, FONT_SIZE);
      text_y += 20;
    }
  }

  previousMode = mode;
}

void loop2 (void* pvParameters) {
  while (true)
  {
    while (!SerialPort2.readBytes((byte*)&data, sizeof(data)))
    {
      
    }
    Serial.println("Got sensor data");
    Serial.println("Temperature: " + String(data.temperature));
    Serial.println("Humidity: " + String(data.humidity));
    Serial.println("Distance: " + String(data.distance));
  }
}
