#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <WiFiManager.h>
#include <EEPROM_Rotate.h>

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

#define BLYNK_GREEN     "#23C48E"
#define BLYNK_BLUE      "#04C0F8"
#define BLYNK_YELLOW    "#ED9D00"
#define BLYNK_RED       "#D3435C"
#define BLYNK_DARK_BLUE "#5F7CD8"

EEPROM_Rotate EEPROM;
RelayBus relayBus(PIN_RELAIS_RX, PIN_RELAIS_TX);
OneWire oneWire(PIN_ONEWIRE);
SensorBus sensors(&oneWire);
Ninkasi ninkasi(&sensors, &relayBus);
BlynkTimer timer;
Ticker timerLed;
Ticker timerSerialize;
WiFiManager wifiManager;
WidgetLED ledRunning(VPIN_LED_RUNNING);
WidgetLED ledHeating(VPIN_LED_HEATING);
WidgetLED ledImpeller(VPIN_LED_IMPELLER);
WidgetLED ledPump(VPIN_LED_PUMP);

uint32_t millisResetBtn = 0;
uint8_t version = '1';

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
void serialize(bool force = false);


BLYNK_CONNECTED() {
  Blynk.syncVirtual(
    VPIN_MASH_TEMP_STEP1,
    VPIN_MASH_TIME_STEP1,
    VPIN_MASH_TEMP_STEP2,
    VPIN_MASH_TIME_STEP2,
    VPIN_MASH_TEMP_STEP3,
    VPIN_MASH_TIME_STEP3,
    VPIN_MASH_TEMP_STEP4,
    VPIN_MASH_TIME_STEP4,
    VPIN_MASH_TEMP_STEP5,
    VPIN_MASH_TIME_STEP5,
    VPIN_MASH_TEMP_STEP6,
    VPIN_MASH_TIME_STEP6,
    VPIN_BOIL_TEMP,
    VPIN_BOIL_DURATION,
    VPIN_BOIL_OUTLET,
    VPIN_BOIL_TEMP_PROBE
  );

  updateBlynkLedAndButtonStatus();
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

  if(request.pin == VPIN_BTN_PUMP) {
    if(param.asInt() == 0) {
      relayBus.delSingle(bit(RELAY_PUMP));
    }
    else {
      relayBus.setSingle(bit(RELAY_PUMP));
    }
  }

  // set boil output switch
  if(request.pin == VPIN_BOIL_OUTLET) {
    ninkasi.setBoilSwitchRelay(param.asInt());
  }

  // set boil temp sensor index
  if(request.pin == VPIN_BOIL_TEMP_PROBE) {
    ninkasi.setBoilSensorIndex(param.asInt() - 1);
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
  serialize(true);
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
  serialize(true);
}

BLYNK_WRITE(VPIN_RESET_BTN) {
  // start reset counter (5000 ms)
  if(param.asInt() == 1 && millisResetBtn == 0) {
    millisResetBtn = millis();
  }

  // button released

  if(param.asInt() == 0) {
    Blynk.setProperty(VPIN_RESET_BTN, "onBackColor", BLYNK_YELLOW);
    millisResetBtn = 0;
  }
}

void sendRelayStatus(uint8_t pin, uint8_t relay, WidgetLED* led = nullptr) {
  bool on = relayBus.isRelayOn(relay);
  Blynk.virtualWrite(pin, on ? 1 : 0);

  if(led == nullptr) {
    return;
  }

  if(on) {
    led->on();
  }
  else {
    led->off();
  }
}

void updateBlynkLedAndButtonStatus() {
  // mash / boil running state
  Blynk.virtualWrite(VPIN_START_MASH_BTN, ninkasi.mashRunning());
  Blynk.virtualWrite(VPIN_START_BOIL_BTN, ninkasi.boilRunning());

  sendRelayStatus(VPIN_BTN_HEATING, RELAY_HEATING, &ledHeating);
  sendRelayStatus(VPIN_BTN_IMPELLER, RELAY_IMPELLER, &ledImpeller);
  sendRelayStatus(VPIN_BTN_AUX, RELAY_AUX);
  sendRelayStatus(VPIN_BTN_PUMP, RELAY_PUMP, &ledPump);

  sendRelayStatus(VPIN_RELAY_MAN1, RELAY_MAN1);
  sendRelayStatus(VPIN_RELAY_MAN2, RELAY_MAN2);
  sendRelayStatus(VPIN_RELAY_MAN3, RELAY_MAN3);
  sendRelayStatus(VPIN_RELAY_MAN4, RELAY_MAN4);

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

void serialize(bool force) {
    if(!ninkasi.running() && !force) {
        return;
    }

    EEPROM.put(0, version);
    ninkasi.serialize(1);
    EEPROM.commit();
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

  // read EEPROM data
  EEPROM.size(10);
  EEPROM.begin(512);
  uint8_t v = 0;
  EEPROM.get(0, v);

  if(v == version) {
    Serial.print("reading configuration from EEPROM (Version ");
    Serial.print((char)version);
    Serial.println(")");
    ninkasi.deserialize(1);
  }
 
  timer.setInterval(3000L, sendBlynkData);
  timerLed.attach(0.5, statusLed);
  timerSerialize.attach(10, serialize, false);
}

void loop() {
  // handle Blynk
  Blynk.run();
  timer.run();

  // handle ninkasi
  ninkasi.loop();

  // handle OTA
  ArduinoOTA.handle();

  // check reset button
  if(millisResetBtn != 0 && millis() - millisResetBtn >= 5000) {
    millisResetBtn = 0;
    Blynk.setProperty(VPIN_RESET_BTN, "onBackColor", BLYNK_GREEN);
    ninkasi.reset();
    serialize();
  }
}
