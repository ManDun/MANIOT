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
#define AWS_IOT_PUBLISH_TOPIC AWS_IOT_PUBLISH_TOPIC_THING
#define AWS_IOT_SUBSCRIBE_TOPIC AWS_IOT_SUBSCRIBE_TOPIC_THING

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

// Publish Message to Thing
void publishMessage()
{

  // Initialise json object and print
  StaticJsonDocument<200> jsonDoc;
  char jsonBuffer[512];

  JsonObject tempsensor = jsonDoc.createNestedObject("TempSensorThing");
  tempsensor["time"] = millis();
  tempsensor["humidity"] = humidity;
  tempsensor["temperature"] = temp;

  jsonDoc ["message"] = "Hello, this is transmitting from the ESP32";
  jsonDoc ["team"] = TEAMNAME;

  serializeJsonPretty(jsonDoc, jsonBuffer);
  Serial.println("");
  Serial.print("Publishing to " + AWS_IOT_PUBLISH_TOPIC + ": ");
  Serial.println(jsonBuffer);

  Serial.println("Message has been sent at ");

  // Publish json to AWS IoT Core
  mqttClient.publish(AWS_IOT_PUBLISH_TOPIC.c_str(), jsonBuffer);
}

// Handle message from AWS IoT Core
void messageHandler(String &topic, String &payload)
{
  Serial.println("Incoming: " + topic + " - " + payload);

  // Parse the incoming JSON
  StaticJsonDocument<200> jsonDoc;
  deserializeJson(jsonDoc, payload);

  const bool LED = jsonDoc["LED"].as<bool>();

  // Decide to turn LED on or off
  if (LED) {
    Serial.print("LED STATE: ");
    Serial.println(LED);
    digitalWrite(2, HIGH); 
  } else if (!LED) {
    Serial.print("LED_STATE: ");
    Serial.println(LED);
    digitalWrite(2, LOW); 
  }
}

// Connect to the AWS MQTT message broker
void connectAWSIoTCore()
{
  // Create a message handler
  mqttClient.onMessage(messageHandler);

  // Configure WiFiClientSecure to use the AWS IoT device credentials
  wifiClient.setCACert(AWS_CERT_CA);
  wifiClient.setCertificate(AWS_CERT_CRT);
  wifiClient.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on AWS
  Serial.print("Attempting to connect to AWS IoT Core message broker at mqtt:\\\\");
  Serial.print(AWS_IOT_ENDPOINT);
  Serial.print(":");
  Serial.println(PORT);

  // Connect to AWS MQTT message broker
  // Retries every 500ms
  mqttClient.begin(AWS_IOT_ENDPOINT, PORT, wifiClient);
  while (!mqttClient.connect(THINGNAME.c_str())) {
    Serial.print("Failed to connect to AWS IoT Core. Error code = ");
    Serial.print(mqttClient.lastError());
    Serial.println(". Retrying...");
    delay(500);
  }
  Serial.println("Connected to AWS IoT Core!");

  // Subscribe to the topic on AWS IoT
  mqttClient.subscribe(AWS_IOT_SUBSCRIBE_TOPIC.c_str());
}

void setup() {
  Serial.begin(9600);

  Serial.println("This is in the initial setup");


  pinMode(2, OUTPUT);

  dht.begin();

  connectWifi();
  connectAWSIoTCore();
}

void loop() {

  // Reconnection Code if disconnected from the MQTT Client/Broker
  if (!mqttClient.connected()) {
    Serial.println("Device has disconnected from MQTT Broker, reconnecting...");
    connectAWSIoTCore();
  }
  mqttClient.loop();

  long now = millis();

  if (now - lastMsg > 30000) {
    lastMsg = now;

    humidity = dht.readHumidity();
    temp = dht.readTemperature();

    publishMessage();
  }


  // if (isnan(humidity) || isnan(temp) )  // Check if any reads failed and exit early (to try again).
  // {
  //   Serial.println(F("Failed to read from DHT sensor!"));
  //   return;
  // }

  // digitalWrite(2, HIGH);   // turn the LED on (HIGH is the voltage level)
  // delay(100);                       // wait for a second
  // digitalWrite(2, LOW);

  // Serial.print("Humidity: ");
  // Serial.print(humidity);
  // Serial.print("%  Temperature: ");
  // Serial.print(temp);
  // Serial.println("°C ");

  // delay(3000);

}