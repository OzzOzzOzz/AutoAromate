/*
  -----------------------
  ESPDASH - Basic Example
  -----------------------

  Skill Level: Intermediate

  In this example we will be creating a basic dashboard which consists 
  of some cards and then update them in realtime ( at 3s interval ).

  Github: https://github.com/ayushsharma82/ESP-DASH
  WiKi: https://ayushsharma82.github.io/ESP-DASH/

  Works with both ESP8266 & ESP32
*/

#include <Arduino.h>
#include "wifiConfig.h"
#include "calibrationConfig.h"
#if defined(ESP8266)
  /* ESP8266 Dependencies */
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
  #include <ESPAsyncWebServer.h>
#elif defined(ESP32)
  /* ESP32 Dependencies */
  #include <WiFi.h>
  #include <AsyncTCP.h>
  #include <ESPAsyncWebServer.h>
#endif
#include <ESPDash.h>


/* Your WiFi Credentials */
const char* ssid = _SSID_; // SSID
const char* password = _PASSWORD_; // Password

/* Start Webserver */
AsyncWebServer server(80);

/* Attach ESP-DASH to AsyncWebServer */
ESPDash dashboard(&server); 

/* 
  Dashboard Cards 
  Format - (Dashboard Instance, Card Type, Card Name, Card Symbol(optional) )
*/
Card light(&dashboard, GENERIC_CARD, "Light", "°%");
Card humidity(&dashboard, HUMIDITY_CARD, "Humidity", "%");


void setup() {
  Serial.begin(115200);

  pinMode(_LIGHT_PORT_, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(14, OUTPUT);

  /* Connect WiFi */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.printf("WiFi Failed!\n");
      return;
  }
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  /* Start AsyncWebServer */
  server.begin();
}

//uint16_t analogReadPort(int port) {
//  uint16_t value = 0;
//  digitalWrite(port, HIGH);
//  delay(100);
//  value = analogRead(A0);
//  digitalWrite(port, LOW);
//  return(value);
//}

uint16_t ReadLight() {
  float value = 0;
  digitalWrite(_LIGHT_PORT_, HIGH);
  delay(100);
  value = float(analogRead(A0));
  digitalWrite(_LIGHT_PORT_, LOW);
  return(((value - _MIN_LIGHT_VALUE) / (_MAX_LIGHT_VALUE - _MIN_LIGHT_VALUE)) * 100);
}

uint16_t ReadHumidity() {
  float value = 0;
  digitalWrite(_HUMIDITY_PORT_, HIGH);
  delay(100);
  value = float(analogRead(A0));
  Serial.println(value);
  digitalWrite(_LIGHT_PORT_, LOW);
  return(((value - _MIN_LIGHT_VALUE) / (_MAX_LIGHT_VALUE - _MIN_LIGHT_VALUE)) * 100);
}

void loop() {
  /* Update Card Values */
  light.update(ReadLight());
  humidity.update(ReadHumidity());

  /* Send Updates to our Dashboard (realtime) */
  dashboard.sendUpdates();

  /* 
    Delay is just for demonstration purposes in this example,
    Replace this code with 'millis interval' in your final project.
  */
  delay(200);
}
