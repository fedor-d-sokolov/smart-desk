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

// Ширина и высота кнопки
#define BUTTON_W 100
#define BUTTON_H 40

// Ширина, высота экрана и размер шрифта
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define FONT_SIZE 2

#define NUM_LEDS 8
#define LED_PIN 19

// Структурка в которой мы получаем данные от датчиков с другого микроконтроллера
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

struct Button_data{
  bool vol_up_pressed = false;
  bool vol_down_pressed = false;
  bool scan_pressed = false;
  bool mode_pressed = false;
  bool rainbow = true;
};

int working_distance;
int work_time = 0;
static int previousMode = 999;
int mode = 0;

Data data;
Button_data button_data;
HardwareSerial SerialPort2 (2); // Нужен для коммуникации между микроконтроллеров
TFT_eSPI tft = TFT_eSPI();
SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

// Кнопочки
ButtonWidget btn_air = ButtonWidget(&tft);
ButtonWidget btn_led = ButtonWidget(&tft);
ButtonWidget btn_time_spent = ButtonWidget(&tft);
ButtonWidget btn_music = ButtonWidget(&tft);

ButtonWidget btn_vol_up = ButtonWidget(&tft);
ButtonWidget btn_vol_down = ButtonWidget(&tft);
ButtonWidget btn_scan = ButtonWidget(&tft);
ButtonWidget btn_mode = ButtonWidget(&tft);

ButtonWidget btn_calibrate = ButtonWidget(&tft);
ButtonWidget btn_rainbow = ButtonWidget(&tft);
ButtonWidget* btn[] = {&btn_air , &btn_led, &btn_time_spent, &btn_music, &btn_vol_up, &btn_vol_down, &btn_scan, &btn_mode, &btn_calibrate, &btn_rainbow};
uint8_t buttonCount = sizeof(btn) / sizeof(btn[0]);

// Дальше идут функции, которые вызываются при нажатии/отжатии 
// определенных кнопок. Для удобства я их назвал таким же именем
// как и сами кнопки и добавил _release и _press с конца для 
// отпускания и нажатия соответственно

// Небольшое пояснение: так как в библиотеке TFT_eWidget 
// нет функции для выключения кнопок на время, то даже когда они
// не отрисовываются, на них все еще можно нажать. Поэтому в кнопках каких-то
// отделов я вставил проверку на номер включенной менюшки
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

void btn_vol_up_press(){
  if (mode != 3) 
  {
    return;
  }

  if (btn_vol_up.justPressed()) {
    Serial.println("btn_vol_up just got pressed");
    
    button_data.vol_up_pressed = true;

    btn_vol_up.drawSmoothButton(true);
  }
}

void btn_vol_down_press(){
  if (mode != 3) 
  {
    return;
  }

  if (btn_vol_down.justPressed()) {
    Serial.println("btn_vol_down just got pressed");
    
    button_data.vol_down_pressed = true;
    
    btn_vol_down.drawSmoothButton(true);
  }
}

void btn_scan_press(){
  if (mode != 3) 
  {
    return;
  }

  if (btn_scan.justPressed()) {
    Serial.println("btn_scan just got pressed");
    
    button_data.scan_pressed = true;
    
    btn_scan.drawSmoothButton(true);
  }
}

void btn_mode_press(){
  if (mode != 3) 
  {
    return;
  }

  if (btn_mode.justPressed()) {
    Serial.println("btn_mode just got pressed");
    
    button_data.mode_pressed = true;
    
    btn_mode.drawSmoothButton(true);
  }
}

void btn_vol_up_release(void) {
  if (mode != 3) 
  {
    return;
  }

  if (btn_vol_up.justReleased()) {
    Serial.println("btn_vol_up just got released");

    button_data.vol_up_pressed = false;

    btn_vol_up.drawSmoothButton(false);
  }
}

void btn_vol_down_release(void) {
  if (mode != 3) 
  {
    return;
  }
  
  if (btn_vol_down.justReleased()) {
    Serial.println("btn_vol_down just got released");
    button_data.vol_down_pressed = false;
    btn_vol_down.drawSmoothButton(false);
  }
}

void btn_scan_release(void) {
  if (mode != 3) 
  {
    return;
  }

  if (btn_scan.justReleased()) {
    Serial.println("btn_scan just got released");
    btn_scan.drawSmoothButton(false);
  }
}

void btn_mode_release(void) {
  if (mode != 3) 
  {
    return;
  }

  if (btn_mode.justReleased()) {
    Serial.println("btn_mode just got released");
    btn_mode.drawSmoothButton(false);
  }
}

void btn_calibrate_press(void)
{
  if (mode != 2) 
  {
    return;
  }

  if (btn_calibrate.justPressed())
  {
    Serial.println("btn_calibrate just got pressed");
    
    working_distance = data.distance;
    Serial.println("New mark:" + String(working_distance));

    btn_calibrate.drawSmoothButton(true);
  }
}

void btn_calibrate_release(void)
{
  if (mode != 2) 
  {
    return;
  }

  if (btn_calibrate.justReleased()) {
    Serial.println("btn_calibrate just got released");
    btn_calibrate.drawSmoothButton(false);
  }
}

void btn_rainbow_press(void)
{
  if (mode != 1)
  {
    return;
  }

  if (btn_rainbow.justPressed())
  {
    button_data.rainbow = !button_data.rainbow;
    Serial.println("btn_rainbow just got pressed");
    btn_rainbow.drawSmoothButton(true);
  }
}

void btn_rainbow_release(void)
{
  if (mode != 1)
  {
    return;
  }

  if (btn_rainbow.justReleased())
  {
    Serial.println("btn_rainbow just got released");
    btn_rainbow.drawSmoothButton(false);
  }
}

void musicTab()
{
  int x, y;
  x = 160;
  y = 10;

  btn_vol_down.initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_BLACK, TFT_WHITE, "VOL-", 1);
  btn_vol_down.setPressAction(btn_vol_down_press);
  btn_vol_down.setReleaseAction(btn_vol_down_release);
  btn_vol_down.drawSmoothButton(false, 3, TFT_BLACK);
  y += 60;

  btn_vol_up.initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_BLACK, TFT_WHITE, "VOL+", 1);
  btn_vol_up.setPressAction(btn_vol_up_press);
  btn_vol_up.setReleaseAction(btn_vol_up_release);
  btn_vol_up.drawSmoothButton(false, 3, TFT_BLACK);
  y += 60;

  btn_scan.initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_BLACK, TFT_WHITE, "SCAN", 1);
  btn_scan.setPressAction(btn_scan_press);
  btn_scan.setReleaseAction(btn_scan_release);
  btn_scan.drawSmoothButton(false, 3, TFT_BLACK);
  y += 60;

  btn_mode.initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_BLACK, TFT_WHITE, "MODE", 1);
  btn_mode.setPressAction(btn_mode_press);
  btn_mode.setReleaseAction(btn_mode_release);
  btn_mode.drawSmoothButton(false, 3, TFT_BLACK);
  y += 60;
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

  if (data.distance <= working_distance + 50)
  {
    work_time += millis() - scanTime;
  }

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

      if (data.eco2 == 0)
      {
        tft.drawCentreString("No data from AQS", center_x, text_y, FONT_SIZE);
        text_y += 20;
        tft.drawCentreString("Wait 1h", center_x, text_y, FONT_SIZE);
        text_y += 20;
      }
      else
      {
        tft.drawCentreString("Equivalent CO2: " + String(data.eco2), center_x, text_y, FONT_SIZE);
        text_y += 20;

        tft.drawCentreString("TVOC: " + String(data.tvoc), center_x, text_y, FONT_SIZE);
        text_y += 20;

        tft.drawCentreString("Air quality index: " + String(data.aqi), center_x, text_y, FONT_SIZE);
        text_y += 20;
      }
    }

    if (mode == 1)
    {
      int x, y;
      x = 160;
      y = 100;

      btn_rainbow.initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_BLACK, TFT_WHITE, "Rainbow", 1);
      btn_rainbow.setPressAction(btn_rainbow_press);
      btn_rainbow.setReleaseAction(btn_rainbow_release);
      btn_rainbow.drawSmoothButton(false, 3, TFT_BLACK);
      y += 60;

      tft.drawCentreString("Click again to pick", center_x, text_y, FONT_SIZE);
      text_y += 20;
    }

    if (mode == 2)
    {
      int x, y;
      x = 160;
      y = 10;

      btn_calibrate.initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_BLACK, TFT_WHITE, "Calibrate", 1);
      btn_calibrate.setPressAction(btn_calibrate_press);
      btn_calibrate.setReleaseAction(btn_calibrate_release);
      btn_calibrate.drawSmoothButton(false, 3, TFT_BLACK);
      y += 60;

      tft.drawCentreString("Work time (min): " + String(work_time / 60000 ), center_x, text_y, FONT_SIZE);
      text_y += 20;
      
    }

    if (mode == 3)
    {
      musicTab();
    }
  }

  previousMode = mode;
}

void loop2 (void* pvParameters) {
  while (true)
  {
    while (!SerialPort2.readBytes((byte*)&data, sizeof(data)))
    {
      Serial.println("Sending data...");
      if (SerialPort2.write((byte*)&button_data, sizeof(button_data)))
      {
        button_data.scan_pressed = false;
        button_data.mode_pressed = false;
      }
      delay(10);
    }
    Serial.println("Got sensor data");
    Serial.println("Temperature: " + String(data.temperature));
    Serial.println("Humidity: " + String(data.humidity));
    Serial.println("Distance: " + String(data.distance));
  }
}
