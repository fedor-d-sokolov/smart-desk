#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <HardwareSerial.h>

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

#define XPT2046_IRQ 36   // T_IRQ
#define XPT2046_MOSI 32  // T_DIN
#define XPT2046_MISO 39  // T_OUT
#define XPT2046_CLK 25   // T_CLK
#define XPT2046_CS 33    // T_CS

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define FONT_SIZE 2


TFT_eSPI tft = TFT_eSPI();
SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);
HardwareSerial SerialPort2 (2);

void setup() {
  Serial.begin(115200);
  SerialPort2.begin(115200, SERIAL_8N1, 16, 17);

  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  // Установить поворот сенсорного экрана в ландшафтный режим
  // Примечание: на некоторых дисплеях сенсорный экран может быть перевернут, поэтому может потребоваться установить поворот в 3: touchscreen.setRotation(3);
  touchscreen.setRotation(1);

  // Начать использовать TFT дисплей
  tft.init();
  // Установить поворот TFT дисплея в ландшафтный режим
  tft.setRotation(1);

  // Очистить экран перед записью на него
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  
  // Установить координаты X и Y для центра дисплея
  int centerX = SCREEN_WIDTH / 2;
  int centerY = SCREEN_HEIGHT / 2;

  tft.drawCentreString("Touch screen to update data", centerX, centerY, FONT_SIZE);
}

void loop() {
  while (!SerialPort2.readBytes((byte*)&data, sizeof(data))) {
      Serial.println("Waiting...");
  }

  Serial.println("Temperature: " + String(data.temperature));
  Serial.println("Humidity: " + String(data.humidity));
  Serial.println("Distance: " + String(data.distance));

  Serial.println("Got data from another device!");

  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);

  int centerX = SCREEN_WIDTH / 2;
  int textY = 80;

  tft.drawCentreString("Temperature: " + String(data.temperature), centerX, textY, FONT_SIZE);
  textY += 20;

  tft.drawCentreString("Humidity: " + String(data.humidity), centerX, textY, FONT_SIZE);
  textY += 20;

  tft.drawCentreString("Distance: " + String(data.distance), centerX, textY, FONT_SIZE);
  textY += 20;

  delay(1000);
}
