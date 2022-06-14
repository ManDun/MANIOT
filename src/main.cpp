#include <Arduino.h>
#include <ArduinoJson.h>
#include <MQTTClient.h>
#include <WiFiClientSecure.h>
#include "secrets.h"
#include "WiFi.h"
#include <Wire.h>
#include <SPI.h>
#include <DHT.h>

#define DHTPIN 5
#define DHTTYPE DHT11

// The MQTT topics that this device should publish/subscribe to
#define AWS_IOT_PUBLISH_TOPIC "esp32-manst/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32-manst/sub"

#define PORT 8883

DHT dht(DHTPIN, DHTTYPE);

WiFiClientSecure wifiClient = WiFiClientSecure();
MQTTClient mqttClient = MQTTClient(256);

float humidity;
float temp;
long lastMsg = 0;

void connectWifi()
{
  Serial.print("Attempting to connect to SSID: ");
  Serial.print(WIFI_SSID);

  // Connect to the specified Wi-Fi network
  // Retries every 500ms
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
}

void setup() {
  Serial.begin(9600);

  Serial.println("This is in the initial setup");


  pinMode(2, OUTPUT);

  dht.begin();

  connectWifi();
  //connectAWSIoTCore();
}

void loop() {

  humidity = dht.readHumidity();
  temp = dht.readTemperature();

  // if (isnan(humidity) || isnan(temp) )  // Check if any reads failed and exit early (to try again).
  // {
  //   Serial.println(F("Failed to read from DHT sensor!"));
  //   return;
  // }

  digitalWrite(2, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(100);                       // wait for a second
  digitalWrite(2, LOW);

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print("%  Temperature: ");
  Serial.print(temp);
  Serial.println("°C ");

  delay(3000);

}