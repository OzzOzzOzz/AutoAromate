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

Card currentLightDisplay(&dashboard, PROGRESS_CARD, "Light duration for today", "%", 0, 100);
Card lightDisplay(&dashboard, GENERIC_CARD, "Light", "%");
Card humidityDisplay(&dashboard, HUMIDITY_CARD, "Humidity", "%");
Card pumpSystemButton(&dashboard, BUTTON_CARD, "Activate pump system");
Card wantedHumidityLevelSlider(&dashboard, SLIDER_CARD, "Wanted humidity", "%", 20, 80);

//  ============================

bool pumpSystemActive = false;
bool isLightTurnedOn = false;

int wantedLightDuration = _DEFAULT_LIGHT_DURATION_;
int wantedHumidityLevel = _DEFAULT_WANTED_HUMIDITY_LEVEL_;

int pumpWateringDuration = _DEFAULT_PUMP_WATER_DURATION_;

int currentLightDuration = 0;
int humidityLevel = 0;
int lightLevel = 0;
int minimumNaturalLightLevel = _DEFAULT_MINIMUM_NATURAL_LIGHT_LEVEL_;

//  ============================

auto timer = timer_create_default();

//  ============================

uint16_t ReadLight() {
  float value = 0;
  digitalWrite(_LIGHT_SENSOR_PORT_, HIGH);
  delay(100);
  value = float(analogRead(A0));
  digitalWrite(_LIGHT_SENSOR_PORT_, LOW);
  lightLevel = ((value - _MIN_LIGHT_VALUE_) / (_MAX_LIGHT_VALUE_ - _MIN_LIGHT_VALUE_)) * 100;
  return(lightLevel);
}

uint16_t ReadHumidity() {
  float value = 0;
  digitalWrite(_HUMIDITY_SENSOR_PORT_, HIGH);
  delay(100);
  value = float(analogRead(A0));
  digitalWrite(_HUMIDITY_SENSOR_PORT_, LOW);
  humidityLevel = ((value - _MIN_HUMIDITY_VALUE_) / (_MAX_HUMIDITY_VALUE_ - _MIN_HUMIDITY_VALUE_)) * 100;
  return(humidityLevel);
}

void countLightDuration() {
  if (isLightTurnedOn == true) {
    currentLightDuration += _CHECK_REFRESH_RATE_;
    currentLightDisplay.update(calcCurrentLightDurationPercentage());
  } else if (lightLevel > minimumNaturalLightLevel) {
    currentLightDuration += _CHECK_REFRESH_RATE_;
    currentLightDisplay.update(calcCurrentLightDurationPercentage());
  }
}

float calcCurrentLightDurationPercentage() {
  return ((float(currentLightDuration) / (wantedLightDuration * 3600000)) * 100);
}

void setLightState(bool newState) {
  isLightTurnedOn = newState;
  if (isLightTurnedOn) {
    digitalWrite(_LIGHT_PORT_, HIGH);
  } else {
    digitalWrite(_LIGHT_PORT_, LOW);
  }
}

void handleLightSystem() {
  countLightDuration();
  
  if (isLightTurnedOn == false) {
    if (currentLightDuration < (wantedLightDuration * 3600000)) {
      if (lightLevel < minimumNaturalLightLevel) {
        setLightState(true);
      }
    }
  } else {
    if (currentLightDuration > (wantedLightDuration * 3600000)) {
      setLightState(false);
    } else if (lightLevel > minimumNaturalLightLevel) {
      setLightState(false);
    }
  }
}

void handlePumpSystem() {
  if (pumpSystemActive == true) {
    if (humidityLevel < wantedHumidityLevel) {
      digitalWrite(_PUMP_PORT_, HIGH);
      delay(pumpWateringDuration);
      digitalWrite(_PUMP_PORT_, LOW);
    }
  }
}

bool check(void *) {
  lightDisplay.update(ReadLight());
  humidityDisplay.update(ReadHumidity());
  dashboard.sendUpdates();
  
  handleLightSystem();
  handlePumpSystem();
  return true;
}

void setup() {
  Serial.begin(115200);
  
  pinMode(_LIGHT_SENSOR_PORT_, OUTPUT);
  pinMode(_HUMIDITY_SENSOR_PORT_, OUTPUT);
  pinMode(_LIGHT_PORT_, OUTPUT);
  pinMode(_PUMP_PORT_, OUTPUT);

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
    Serial.println("pumpSystemButton: "+ String((pumpSystemActive)? "true" : "false"));
    pumpSystemButton.update(pumpSystemActive);
    dashboard.sendUpdates();
  });

  wantedHumidityLevelSlider.attachCallback([&](int value) {
    wantedHumidityLevel = value;
    Serial.println("[wantedHumidityLevelSlider]: "+ String(wantedHumidityLevel));
    wantedHumidityLevelSlider.update(wantedHumidityLevel);
    dashboard.sendUpdates();
  });

  wantedHumidityLevelSlider.update(wantedHumidityLevel);
  currentLightDisplay.update(calcCurrentLightDurationPercentage());
  
  timer.every(_CHECK_REFRESH_RATE_, check);
}

void loop() {
  timer.tick();
}
