#include <GyverOLED.h>                     // Подключаем библиотеку GyverOLED
#include <math.h>
#include <ESP8266WiFi.h>                   // Подключаем библиотеку ESP8266WiFi
#include <Wire.h>                          // Подключаем библиотеку Wire
#include <SparkFun_Qwiic_Humidity_AHT20.h> // Подключаем библиотеку на датчик AHT20
#include "ThingSpeak.h"                    // Подключаем библиотеку ThingSpeak
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>                    // Библиотека для OTA-прошивки
AHT20 humiditySensor;

#define BUTTON 14                          // Нужно замкнуть пин D5 и GND при включении, для ОТА прошивки

const char *ssid = "";         // Название WiFi сети
const char *password = "";         // Пароль от WiFi сети

unsigned long myChannelNumber[] = {, , };                              // Номера канала на thingspeak.com
const char *myWriteAPIKey[] = {"", "", ""}; // API ключи на thingspeak.com
const char *HostName[] = {"SmallRoomTermo", "BigRoomTermo", "KitchenTermo"};                // Имена устройств при OTA прошивке
int room = 2;                                                                               // 0 - Маленькая комната, 1 - Большая комната, 2 - Кухня

WiFiClient client;

GyverOLED<SSD1306_128x32, OLED_NO_BUFFER> oled; // Инициализация дисплея

int sleeptime = 20 * 60e6; // Время сна (минут)

float volt; // Переменная для считывания показаний с аналогового пина
int volts;  // Перевод значиния с аналогового пина в %
boolean ota = false;
int ledState = LOW;

unsigned long previousMillis = 0;
const long interval = 1000;

void setup()
{
  pinMode(A0, INPUT); // Устанавливаем пин А0 как вход
  pinMode(BUTTON, INPUT_PULLUP);
  volt = (analogRead(A0) / 1024.0) * 4.2;         // Считываем напряжение батарейки
  volts = map(analogRead(A0), 796, 1024, 0, 100); // Переводим вольтаж в %
  if (volts >= 20) // Если батарея заряжена, то проводим инициализацию
  {
    Wire.begin();               // Инициализация I2C шины
    WiFi.mode(WIFI_STA);        // Инициализация WiFi
    WiFi.begin(ssid, password); // Подключиться к Вашей локальной WiFi сети
    humiditySensor.begin();     // Инициализация температурного датчика
    ThingSpeak.begin(client);   // Инициализация ThingSpeak
  }
  if (digitalRead(BUTTON) == 0)
  {
    pinMode(LED_BUILTIN, OUTPUT);
    ota = true;
    WiFi.config(IPAddress(192, 168, 1, 222), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0), IPAddress(192, 168, 1, 1));
    ArduinoOTA.setHostname(HostName[room]); // Задаем имя сетевого порта
    ArduinoOTA.begin();                     // Инициализируем OTA
    while (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
      delay(5000);
      ESP.restart();
    }
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else
  {
    ota = false;
  }
}

void loop()
{
  if (ota)
  {
    ArduinoOTA.handle(); // Моргаем светодиодом и ждём прошивку
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval)
    {
      previousMillis = currentMillis;
      digitalWrite(LED_BUILTIN, ledState);
      ledState = !ledState; // инвертировать флаг
    }
  }
  else
  {
    if (ESP.getResetReason() == String("Power On")) // Включаем экран если включение произошло по кнопке
    {
      oled.setPower(1); // Вкл дисплей
      oled.init();      // Инициализация дисплея
      if (volts <= 20)  // Показываем что батарея разряжена
      {
        for (int a = 0; a < 5; a++)
        {
          oled.setContrast(0);
          oled.clear();                               // Очищаем дисплей
          oled.home();                                // Устанавливаем курсор в 0, 0
          oled.roundRect(0, 0, 127, 31, OLED_STROKE); // Рисуем скруглённый прямоугольник
          oled.setCursor(10, 1);                      // Устанавливаем курсор в (пиксель X, строка Y)
          oled.setScale(2);                           // Устанавливаем масштаб шрифта
          oled.print("Зарядите");                     // Печатаем указания на дисплее
          drawBattery(volts);                         // Рисуем батарейку на дисплее
          delay(500);
        }
        oled.setPower(0); // Выкл дисплей
      }
      else
      {
        oled.setContrast(255);
        printTemperature(); // Показываем температуру
        delay(5000);
        printHumidity(); // Показываем влажность
        delay(5000);
        oled.setPower(0); // Выкл дисплей
        writeServer();
      }
    }
    else
    {
      writeServer();
    }
    WiFi.disconnect();        // обрываем WIFI соединения
    WiFi.softAPdisconnect();  // отключаем точку доступа(если она была)
    WiFi.mode(WIFI_OFF);      // отключаем WIFI
    ESP.deepSleep(sleeptime); // Глубокий сон
  }
}

void printTemperature()
{
  oled.clear();                               // Очищаем дисплей
  oled.home();                                // Устанавливаем курсор в 0, 0
  oled.roundRect(0, 0, 127, 31, OLED_STROKE); // Рисуем скруглённый прямоугольник
  oled.setCursor(25, 1);                      // Устанавливаем курсор в (пиксель X, строка Y)
  oled.setScale(2);                           // Устанавливаем масштаб шрифта

  // Отбрасываем сотые доли значений
  String temperature = String(humiditySensor.getTemperature());
  temperature = temperature.substring(0, temperature.length() - 1);

  oled.print(temperature);
  oled.print(" *C");  // Печатаем показания на дисплее
  drawBattery(volts); // Рисуем батарейку на дисплее
}

void printHumidity()
{
  oled.clear();                               // Очищаем дисплей
  oled.home();                                // Устанавливаем курсор в 0, 0
  oled.roundRect(0, 0, 127, 31, OLED_STROKE); // Рисуем скруглённый прямоугольник
  oled.setCursor(18, 1);                      // Устанавливаем курсор в (пиксель X, строка Y)
  oled.setScale(2);                           // Устанавливаем масштаб шрифта

  // Отбрасываем сотые доли значений
  String humidity = String(humiditySensor.getHumidity());
  humidity = humidity.substring(0, humidity.length() - 1);

  oled.print(humidity);
  oled.print("% rH"); // Печатаем показания на дисплее
  drawBattery(volts); // Рисуем батарейку на дисплее
}

void drawBattery(byte percent)
{
  oled.setCursorXY(110, 24);
  oled.drawByte(0b00111100);
  oled.drawByte(0b00111100);
  oled.drawByte(0b11111111);
  for (byte i = 0; i < 100 / 8; i++)
  {
    if (i < (100 - percent) / 8)
      oled.drawByte(0b10000001);
    else
      oled.drawByte(0b11111111);
  }
  oled.drawByte(0b11111111);
}

void writeServer()
{
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100); // Проверяем, выполнено ли подключение к WiFi сети
  }
  ThingSpeak.setField(1, humiditySensor.getTemperature());                    // Устанавливаем значения для полей
  ThingSpeak.setField(2, humiditySensor.getHumidity());                       // Устанавливаем значения для полей
  ThingSpeak.setField(3, volt);                                               // Устанавливаем значения для полей
  int x = ThingSpeak.writeFields(myChannelNumber[room], myWriteAPIKey[room]); // Записываем значения для полей
}
