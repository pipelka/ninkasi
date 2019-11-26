#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <WiFiManager.h>

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <OneWire.h>
#include <Ticker.h> 

#include "RelayBus.h"
#include "SensorBus.h"
#include "Ninkasi.h"
#include "vpins.h"

// PIN DEFINITIOMS

#define PIN_RELAIS_RX 12 // D6
#define PIN_RELAIS_TX 13 // D7
#define PIN_ONEWIRE 5 // D1
#define PIN_LED 0 // D3

#define RELAY_IMPELLER 0
#define RELAY_HEATING 1
#define RELAY_AUX 2
#define RELAY_PUMP 3

RelayBus relayBus(PIN_RELAIS_RX, PIN_RELAIS_TX);
OneWire oneWire(PIN_ONEWIRE);
SensorBus sensors(&oneWire);
Ninkasi ninkasi(&sensors, &relayBus);
BlynkTimer timer;
Ticker timerLed;
WiFiManager wifiManager;
WidgetLED ledRunning(VPIN_LED_RUNNING);
WidgetLED ledHeating(VPIN_LED_HEATING);
WidgetLED ledImpeller(VPIN_LED_IMPELLER);
WidgetLED ledPump(VPIN_LED_PUMP);

// WiFi AP credentials.
char ssid[] = "ninkasi";
char pass[] = "ninkasi";

// Hostname needed by ArduinoOTA
const char* hostname = "ninkasi";

// Blynk Project Settings
char auth[] = "2e265805660d4814842adf80ccd7690a";
const char* domain = "192.168.16.10";
uint16_t port = 8080;

void updateBlynkLedAndButtonStatus();


BLYNK_CONNECTED() {
  Blynk.syncAll();
}

BLYNK_WRITE_DEFAULT() {
  Serial.print("PIN: ");
  Serial.print((int)request.pin, DEC);
  Serial.print(" = ");
  Serial.println(param.asInt());

  // mash steps
  if(request.pin >= VPIN_MASH_TEMP_STEP1 && request.pin <= VPIN_MASH_TIME_STEP6) {
      uint8_t value = (uint8_t)param.asInt();
      uint8_t step = request.pin / 2;
      bool duration = (request.pin % 2 == 1);

      if(duration) {
        ninkasi.setMashStep(step, -1, value);
      }
      else {
        ninkasi.setMashStep(step, value);
      }
  }

  // boil
  if(request.pin == VPIN_BOIL_TEMP) {
    ninkasi.setBoil(param.asInt(), -1);
  }
  else if(request.pin == VPIN_BOIL_DURATION) {
    ninkasi.setBoil(-1, param.asInt());
  }

  // manual relay switches
  if(request.pin >= VPIN_RELAY_MAN1 && request.pin <= VPIN_RELAY_MAN4) {
    uint8_t port = 4 + (request.pin - VPIN_RELAY_MAN1);

    if(param.asInt() == 0) {
      relayBus.delSingle(1 << port);
    }
    else {
      relayBus.setSingle(1 << port);
    }
  }

  // override switches
  if(request.pin == VPIN_BTN_IMPELLER) {
    if(param.asInt() == 0) {
      relayBus.delSingle(bit(RELAY_IMPELLER));
    }
    else {
      relayBus.setSingle(bit(RELAY_IMPELLER));
    }
  }

  if(request.pin == VPIN_BTN_HEATING) {
    if(param.asInt() == 0) {
      relayBus.delSingle(bit(RELAY_HEATING));
    }
    else {
      relayBus.setSingle(bit(RELAY_HEATING));
    }
  }

  if(request.pin == VPIN_BTN_AUX) {
    if(param.asInt() == 0) {
      relayBus.delSingle(bit(RELAY_AUX));
    }
    else {
      relayBus.setSingle(bit(RELAY_AUX));
    }
  }

  if(request.pin == VPIN_BTN_HEATING) {
    if(param.asInt() == 0) {
      relayBus.delSingle(bit(RELAY_PUMP));
    }
    else {
      relayBus.setSingle(bit(RELAY_PUMP));
    }
  }
};

BLYNK_WRITE(VPIN_START_MASH_BTN) {
  bool on = (param.asInt() == 1);

  if(on) {
    Blynk.virtualWrite(VPIN_START_BOIL_BTN, 0);
    ninkasi.startMash();
  }
  else {
    ninkasi.stop();
  }

  updateBlynkLedAndButtonStatus();
}

BLYNK_WRITE(VPIN_START_BOIL_BTN) {
  bool on = (param.asInt() == 1);

  if(on) {
    Blynk.virtualWrite(VPIN_START_MASH_BTN, 0);
    ninkasi.startBoil();
  }
  else {
    ninkasi.stop();
  }

  updateBlynkLedAndButtonStatus();
}

void updateBlynkLedAndButtonStatus() {
  Blynk.virtualWrite(VPIN_START_MASH_BTN, ninkasi.mashRunning());
  Blynk.virtualWrite(VPIN_START_BOIL_BTN, ninkasi.boilRunning());
  
  if(relayBus.isRelayOn(RELAY_IMPELLER)) {
    Blynk.virtualWrite(VPIN_BTN_IMPELLER, 1);
    ledImpeller.on();
  }
  else {
    Blynk.virtualWrite(VPIN_BTN_IMPELLER, 0);
    ledImpeller.off();
  }

  if(relayBus.isRelayOn(RELAY_HEATING)) {
    Blynk.virtualWrite(VPIN_BTN_HEATING, 1);
    ledHeating.on();
  }
  else {
    Blynk.virtualWrite(VPIN_BTN_HEATING, 0);
    ledHeating.off();
  }

  if(relayBus.isRelayOn(RELAY_PUMP)) {
    Blynk.virtualWrite(VPIN_BTN_PUMP, 1);
    ledPump.on();
  }
  else {
    Blynk.virtualWrite(VPIN_BTN_PUMP, 0);
    ledPump.off();
  }

  if(ninkasi.running()) {
    ledRunning.on();
  }
  else {
    ledRunning.off();
  }
}

void sendBlynkData() {
  Blynk.virtualWrite(VPIN_TEMP1, sensors.getTemp(0, true));
  Blynk.virtualWrite(VPIN_TEMP2, sensors.getTemp(1, true));

  Blynk.virtualWrite(VPIN_TARGET_TEMP, ninkasi.getTargetTempC());
  Blynk.virtualWrite(VPIN_REMAINING_TIME, ninkasi.getRemainingTime());

  updateBlynkLedAndButtonStatus();
}

void statusLed() {
  if(!ninkasi.running()) {
    digitalWrite(PIN_LED, LOW);
  }
  else {
    digitalWrite(PIN_LED, !digitalRead(PIN_LED));
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);

  wifiManager.autoConnect(ssid, pass);
  Blynk.config(auth, domain, port);

  ArduinoOTA.setHostname(hostname);
  ArduinoOTA.begin();

  ninkasi.begin();

  timer.setInterval(3000L, sendBlynkData);
  timerLed.attach(0.5, statusLed);
}

void loop() {
  // handle Blynk
  Blynk.run();
  timer.run();

  // handle ninkasi
  ninkasi.loop();

  // handle OTA
  ArduinoOTA.handle();
}
