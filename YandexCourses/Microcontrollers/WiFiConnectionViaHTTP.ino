#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <Keypad.h>
#include <WiFi.h>
#include <HTTPClient.h>


#define ROWS 4
#define COLS 3
#define TFT_DC 21
#define TFT_CS 22

char keys[ROWS][COLS] = {
  { '1', '2', '3', },
  { '4', '5', '6',},
  { '7', '8', '9', },
  { '*', '0', '#', }
};
String data = "";
String fact = "";

uint8_t colPins[COLS] = { 25, 33,32 }; // Pins connected to C1, C2, C3, 
uint8_t rowPins[ROWS] = { 12, 14, 27, 26 }; // Pins connected to R1, R2, R3, R4

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

void sendAPIRequest(String number) {
  if ((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;
    String url = "http://numbersapi.com/" + number + "/math";
    http.begin(url);

    int httpCode = http.GET(); // Отправка GET-запроса
    if (httpCode > 0) {
      fact = http.getString();
    }
    http.end();
  }
}

void connectWiFi(){
  tft.print("Connecting to WiFi");
  WiFi.begin("Wokwi-GUEST", "", 6);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    tft.print(".");
  }
  tft.print("\nConnected!");
  tft.setCursor(0, 0);
  delay(1000);
  tft.fillScreen(ILI9341_BLACK);
}

void setup() {
  tft.begin();
  tft.setTextSize(2);
  tft.setCursor(0, 0);
  connectWiFi();
}


void loop() {
  char key = keypad.getKey();
  if (key != NO_KEY) {
    if (key == '#') {
      sendAPIRequest(data);
      tft.println("\n");
      tft.println(fact);
      data = "";
      fact = "";
    } else if (key == '*'){ 
      tft.fillScreen(ILI9341_BLACK);
      tft.setCursor(0, 0);
  } else {
    tft.print(key);
    data += key;
  }
}
}