#include <Arduino.h>
#include "wifiConfig.h"
#include "calibrationConfig.h"
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPDash.h>
#include <arduino-timer.h>

//  ============================

/* Start Webserver */
AsyncWebServer server(80);

/* Attach ESP-DASH to AsyncWebServer */
ESPDash dashboard(&server); 

//  ============================

Card lightDisplay(&dashboard, GENERIC_CARD, "Light", "%");
Card humidityDisplay(&dashboard, HUMIDITY_CARD, "Humidity", "%");
Card pumpSystemButton(&dashboard, BUTTON_CARD, "Pump system button");
Card wantedHumidityLevelSlider(&dashboard, SLIDER_CARD, "Wanted humidity", "%", 20, 80);

//  ============================

bool pumpSystemActive = false;
int pumpWateringDuration = _DEFAULT_PUMP_WATER_DURATION_;
int wantedHumidityLevel = _DEFAULT_WANTED_HUMIDITY_LEVEL_;
int humidityLevel = 0;

//  ============================

auto timer = timer_create_default();

//  ============================

uint16_t ReadLight() {
  float value = 0;
  digitalWrite(_LIGHT_PORT_, HIGH);
  delay(100);
  value = float(analogRead(A0));
  digitalWrite(_LIGHT_PORT_, LOW);
  return(((value - _MIN_LIGHT_VALUE_) / (_MAX_LIGHT_VALUE_ - _MIN_LIGHT_VALUE_)) * 100);
}

uint16_t ReadHumidity() {
  float value = 0;
  digitalWrite(_HUMIDITY_PORT_, HIGH);
  delay(100);
  value = float(analogRead(A0));
  digitalWrite(_LIGHT_PORT_, LOW);
  humidityLevel = ((value - _MIN_LIGHT_VALUE_) / (_MAX_LIGHT_VALUE_ - _MIN_LIGHT_VALUE_)) * 100;
  return(humidityLevel);
}

void handlePumpSystem() {
  humidityDisplay.update(ReadHumidity());
  dashboard.sendUpdates();
}

void handleLightSystem() {
  lightDisplay.update(ReadLight());
  dashboard.sendUpdates();
}

bool check(void *) {
  Serial.println("checking");
  handlePumpSystem();
  handleLightSystem();
  return true;
}

void setup() {
  Serial.begin(115200);

  pinMode(_LIGHT_PORT_, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(14, OUTPUT);

  /* Connect WiFi */
  WiFi.mode(WIFI_STA);
  WiFi.begin(_SSID_, _PASSWORD_);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.printf("WiFi Failed!\n");
      return;
  }
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.begin();

  pumpSystemButton.attachCallback([&](bool value){
    pumpSystemActive = value;
    Serial.println("pumpSystemButton: "+String((pumpSystemActive)?"true":"false"));
    pumpSystemButton.update(pumpSystemActive);
    dashboard.sendUpdates();
  });

  wantedHumidityLevelSlider.attachCallback([&](int value) {
    wantedHumidityLevel = value;
    Serial.println("[wantedHumidityLevelSlider]: "+String(wantedHumidityLevel));
    wantedHumidityLevelSlider.update(wantedHumidityLevel);
    dashboard.sendUpdates();
  });

  timer.every(15000, check);
}

void loop() {
  timer.tick();
}
