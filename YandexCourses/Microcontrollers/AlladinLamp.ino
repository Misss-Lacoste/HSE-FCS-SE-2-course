#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define NEOPIXEL_PIN 13
#define NUMPIXELS 24

Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

const char *mqtt_broker = "m6.wqtt.ru";
const char *topic = "color";
const char *mqtt_username = "YOUR_USER_NAME";
const char *mqtt_password = "YOUR_PASSWORD";
const int mqtt_port = 19232;

WiFiClient espClient;
PubSubClient client(espClient);

void parseRGB(String input, int &r, int &g, int &b) {
  int firstComma = input.indexOf(',');
  int secondComma = input.indexOf(',', firstComma + 1);

  r = input.substring(0, firstComma).toInt();
  g = input.substring(firstComma + 1, secondComma).toInt();
  b = input.substring(secondComma + 1).toInt();
}

void connectMQTT()
{
    while (!client.connected())
    {
        Serial.print("Connecting to MQTT...");
        if (client.connect("ESP32Client", mqtt_username, mqtt_password))
        {
            Serial.println(" connected");
            client.subscribe(topic);
        }
        else
        {
            Serial.print(" failed, rc=");
            Serial.print(client.state());
            Serial.println(" retrying in 5 seconds");
            delay(5000);
        }
    }
}

void connectWiFi()
{
    Serial.print("Connecting to WiFi");
    WiFi.begin("Wokwi-GUEST", "", 6);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(100);
        Serial.print(".");
    }
    Serial.println("Connected!");
}

void callback(char *topic, byte *payload, unsigned int length)
{
    String message = "";
    for (int i = 0; i < length; i++)
    {
        message += (char)payload[i];  // Собираем пазл из байтов
    }

    int red,green,blue;
    parseRGB(message, red,green,blue);
    pixels.fill(pixels.Color(red,green,blue));
    pixels.show();
}

void setup()
{
    Serial.begin(115200);
    pixels.begin();

    connectWiFi();

    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);
    connectMQTT();
}

void loop()
{
    if (!client.connected())
    {
        connectMQTT();
    }
    client.loop();
}