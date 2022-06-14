#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <DHT.h>

DHT my_sensor(5, DHT11);

void setup() {
  // put your setup code here, to run once:
  //Check
  pinMode(2, OUTPUT);
  Serial.begin(9600);
  my_sensor.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print("Humidity: ");
  Serial.print(my_sensor.readHumidity());
  Serial.print("Temperature: ");
  Serial.print(my_sensor.readTemperature());
  Serial.println("#");

  digitalWrite(2, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(100);                       // wait for a second
  digitalWrite(2, LOW);    // turn the LED off by making the voltage LOW 

  delay(3000);
}