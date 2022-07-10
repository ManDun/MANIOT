#include <Arduino.h>
#include <ArduinoJson.h>
#include <MQTTClient.h>
#include <WiFiClientSecure.h>
#include "secrets.h"
#include "WiFi.h"
#include <Wire.h>
#include <SPI.h>
#include <DHT.h>
#include <PubSubClient.h>
#include <string>

#define DHTPIN 21
#define DHTTYPE DHT11
#define LIGHT_SENSOR_PIN 34
#define MOTION_SENSOR 4


// The MQTT topics that this device should publish/subscribe to
#define RPI_IOT_PUBLISH_TOPIC RPI_IOT_PUBLISH_TOPIC_THING
#define RPI_IOT_SUBSCRIBE_TOPIC RPI_IOT_SUBSCRIBE_TOPIC_THING
#define mqtt_username mqtt_username
#define mqtt_password mqtt_password

#define PORT 1883


DHT dht(DHTPIN, DHTTYPE);

void callback(char* topic, byte* payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

WiFiClient wifiClient = WiFiClient();
PubSubClient mqttClient(RPI_IOT_ENDPOINT, PORT, wifiClient); 
// MQTTClient mqttClient = MQTTClient(256);

float humidity;
float temp;
long lastMsg = 0;
int lightValue = 0;
String brightness;
int pinStateCurrent = LOW;
int pinStatePrevious = LOW;

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
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void connectMQTT()
{

  Serial.print("Attempting to connect to MQTT Server: ");
  if (mqttClient.connect("esp321", mqtt_username, mqtt_password)) {
    Serial.println("Connected to MQTT Broker!");
    Serial.setTimeout(2000);
    mqttClient.subscribe(RPI_IOT_SUBSCRIBE_TOPIC.c_str());
  }
  else {
    Serial.println("Connection to MQTT Broker failed...");
  }

  mqttClient.setServer(RPI_IOT_ENDPOINT, 1883);
  mqttClient.setCallback(callback);

}

// Publish Message to Thing
void publishMessage()
{

  // Initialise json object and print
  StaticJsonDocument<200> jsonDoc;
  char jsonBuffer[512];

  // JsonObject tempsensor = jsonDoc.createNestedObject("TempSensorThing");
  // tempsensor["time"] = millis();
  // tempsensor["humidity"] = humidity;
  // tempsensor["temperature"] = temp;

  // jsonDoc["time"] = millis();

  double scale = 0.01;  // i.e. round to nearest one-hundreth
  double tempf = (int)(temp / scale) * scale;
  
  jsonDoc["temperature"] = tempf;
  jsonDoc["humidity"] = humidity;
  jsonDoc["light"] = lightValue;

  // jsonDoc ["message"] = "Hello, this is transmitting from the ESP321";

  serializeJsonPretty(jsonDoc, jsonBuffer);
  Serial.println("");
  Serial.print("Publishing to " + RPI_IOT_PUBLISH_TOPIC + ": ");
  Serial.println(jsonBuffer);

  

  // Publish json to RPI IoT Core
  if (mqttClient.publish(RPI_IOT_PUBLISH_TOPIC.c_str(), jsonBuffer)){
    Serial.println("Data sent!! ");
  }

  else {
    Serial.println("Temperature failed to send. Reconnecting to MQTT Broker and trying again");
    mqttClient.connect("esp321", mqtt_username, mqtt_password);
    delay(10); // This delay ensures that client.publish doesn't clash with the client.connect call
    if(mqttClient.publish(RPI_IOT_PUBLISH_TOPIC.c_str(), jsonBuffer)){
      Serial.println("Temperature sent");
    }
  }

  mqttClient.disconnect();
}

// Handle message from RPI IoT Core


  // Parse the incoming JSON
  // StaticJsonDocument<200> jsonDoc;
  // deserializeJson(jsonDoc, payload);

  //const bool LED = jsonDoc["LED"].as<bool>();

  // Decide to turn LED on or off
  // if (payload == "SUCCESS") {
  //   Serial.print("LED STATE: ");
  //   Serial.println(payload);
  //   digitalWrite(2, HIGH); 
  // } else {
  //   Serial.print("LED STATE: ");
  //   Serial.println(payload);
  //   digitalWrite(2, LOW); 
  // }


// Connect to the RPI MQTT message broker
// void connectRPIIoTCore()
// {
//   // Create a message handler
//   mqttClient.onMessage(messageHandler);

//   // Configure WiFiClientSecure to use the RPI IoT device credentials
//   // wifiClient.setCACert(RPI_CERT_CA);
//   // wifiClient.setCertificate(RPI_CERT_CRT);
//   // wifiClient.setPrivateKey(RPI_CERT_PRIVATE);

//   // Connect to the MQTT broker on RPI
//   Serial.print("Attempting to connect to RPI IoT Core message broker at mqtt:\\\\");
//   Serial.print(RPI_IOT_ENDPOINT);
//   Serial.print(":");
//   Serial.println(PORT);

//   // Connect to RPI MQTT message broker
//   // Retries every 500ms
//   mqttClient.begin(RPI_IOT_ENDPOINT, PORT, wifiClient);
//   while (!mqttClient.connect(THINGNAME.c_str())) {
//     Serial.print("Failed to connect to RPI IoT Core. Error code = ");
//     Serial.print(mqttClient.lastError());
//     Serial.println(". Retrying...");
//     delay(500);
//   }
//   Serial.println("Connected to RPI IoT Core!");

//   // Subscribe to the topic on RPI IoT
//   mqttClient.subscribe(RPI_IOT_SUBSCRIBE_TOPIC.c_str());
// }

void check_light() {
  lightValue = analogRead(LIGHT_SENSOR_PIN);
  // We'll have a few threshholds, qualitatively determined

  Serial.println(lightValue);


  if (lightValue < 10) {
    Serial.println(" - Dark");
    brightness = "Dark";
    digitalWrite(2, HIGH); 
  } else if (lightValue < 200) {
    Serial.println(" - Dim");
    brightness = "Dim";
    digitalWrite(2, HIGH); 
  } else if (lightValue < 500) {
    Serial.println(" - Light");
    brightness = "Light";
    digitalWrite(2, LOW); 
  } else if (lightValue < 800) {
    Serial.println(" - Bright");
    brightness = "Bright";
    digitalWrite(2, LOW); 
  } else {
    Serial.println(" - Very bright");
    brightness = "Very bright";
    digitalWrite(2, LOW); 
  }

  delay(500);
}

void setup() {
  Serial.begin(9600);

  Serial.println("This is in the initial setup");


  pinMode(2, OUTPUT);
  pinMode(MOTION_SENSOR, INPUT);

  dht.begin();

  connectWifi();
  // connectRPIIoTCore();
  //connectMQTT();
}

void loop() {

  // Reconnection Code if disconnected from the MQTT Client/Broker
  // if (!mqttClient.connected()) {
  //   Serial.println("Device has disconnected from MQTT Broker, reconnecting...");
  //   // connectRPIIoTCore();
  // }
  // mqttClient.loop();
  pinStatePrevious = pinStateCurrent; // store old state
  pinStateCurrent = digitalRead(MOTION_SENSOR);   // read new state

  if (pinStatePrevious == LOW && pinStateCurrent == HIGH) {   // pin state change: LOW -> HIGH
    Serial.println("Motion detected!");
    // TODO: turn on alarm, light or activate a device ... here
  }
  else
  if (pinStatePrevious == HIGH && pinStateCurrent == LOW) {   // pin state change: HIGH -> LOW
    Serial.println("Motion stopped!");
    // TODO: turn off alarm, light or deactivate a device ... here
  }

  long now = millis();

  if (now - lastMsg > 30000) {
    lastMsg = now;

    humidity = dht.readHumidity();
    temp = dht.readTemperature();

    check_light();

    connectMQTT();
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