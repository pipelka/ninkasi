#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <WiFiManager.h>

#include <Arduino.h>
#include <OneWire.h>
#include <Ticker.h> 

#include "RelayBus.h"
#include "SensorBus.h"
#include "tasks/TaskMachine.h"
#include "vpins.h"

// PIN DEFINITIOMS

#define PIN_RELAIS_RX 12 // D6
#define PIN_RELAIS_TX 13 // D7
#define PIN_ONEWIRE 5 // D1
#define PIN_LED 0 // D3

RelayBus relayBus(PIN_RELAIS_RX, PIN_RELAIS_TX);
OneWire oneWire(PIN_ONEWIRE);
SensorBus sensors(&oneWire);
TaskMachine M(4, &sensors, &relayBus);
BlynkTimer timer;
Ticker timerLed;
WiFiManager wifiManager;

// WiFi AP credentials.
char ssid[] = "ninkasi";
char pass[] = "ninkasi";

// Blynk Project Settings
char auth[] = "2e265805660d4814842adf80ccd7690a";
const char* domain = "192.168.16.10";
uint16_t port = 8080;

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
      uint8_t pos = step * 2;
      bool duration = (request.pin % 2 == 1);

      for(uint8_t q = 0; q < 2; q++) {
        Task* taskRamp = M.Q(q)[pos];
        Task* taskHold = M.Q(q)[pos + 1];

        if(duration) {
          taskHold->setHoldTime(value);
        }
        else {
          taskRamp->setTargetTempC(value);
          taskHold->setTargetTempC(value);
        }
      }
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
};

BLYNK_READ(VPIN_START_BTN) {
  Blynk.virtualWrite(VPIN_START_BTN, M.running() ? 1 : 0);
}

BLYNK_WRITE(VPIN_START_BTN) {
  bool on = (param.asInt() == 1);

  if(on) {
    M.start();
  }
  else {
    M.stop();
  }
}

void sendBlynkData() {
  Blynk.virtualWrite(VPIN_START_BTN, M.running());

  Blynk.virtualWrite(VPIN_TEMP1, M.getTempC(0));
  Blynk.virtualWrite(VPIN_TEMP2, M.getTempC(1));
  Blynk.virtualWrite(VPIN_TARGET_TEMP, M.getTargetTempC());

  auto t = M.getRemainingTime();
  Blynk.virtualWrite(VPIN_REMAINING_TIME, t);
}

void statusLed() {
  if(!M.running()) {
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

  M.begin();

  for(int i = 0; i < 6; i++) {
    uint8_t pos =  i* 2;
    M.Q(0).setTask<TaskRamp>(pos)->ramp(0);
    M.Q(0).setTask<TaskHold>(pos + 1)->hold(0);
    M.Q(1).setTask<TaskRamp>(pos)->ramp(0);
    M.Q(1).setTask<TaskHold>(pos + 1)->hold(0);
  }

  timer.setInterval(5000L, sendBlynkData);

  timerLed.attach(0.5, statusLed);
}

void loop() {
  Blynk.run();
  timer.run();
  M.loop();
}
