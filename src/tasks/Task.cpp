#include "Task.h"

void Task::begin() {
    m_startMillis = millis();
}

void Task::loop() {
    if(m_atom.done) {
        return;
    }

    if(m_atom.targetTemp != 0) {
        float temp = m_sensors->getTemp(m_atom.sensor);

        Serial.print("T");
        Serial.print(m_atom.sensor + 1, DEC);
        Serial.print(" - ");
        Serial.print(temp);
        Serial.println("C");
    }

    if(millisSinceStart() >= 60 * 1000) {
        m_atom.elapsed++;
        Task::begin();
    }
}

void Task::clear() {
    m_atom.done = false;
    m_atom.elapsed = 0;
    m_atom.sw_on = 0;
    m_startMillis = 0;
}

uint32_t Task::millisSinceStart() {
    return millis() - m_startMillis;
}

void Task::switchOn(bool on, bool force) {
    if(on == m_atom.sw_on && !force) {
        return;
    }

    bool done = false;
    int count = 0;

    while(!done) {
        if(on == 1) {
            done = m_relay->setSingle(1 << m_atom.sw);
        }
        else {
            done = m_relay->delSingle(1 << m_atom.sw);
        }

        count++;

        if(!done) {
            delay(1000);
            m_relay->setup();
        }

        if(!done && count == 10) {
            Serial.print("[ERROR] RelayBus NOT responding !!!");
            // TODO - send notification, switch off device, ...
            // just take every possible action to avoid an unknown relay
            // state (e.g. heating forever, ...)
            return;
        }
    }

    m_atom.sw_on = on;
}

int Task::serialize(int addr) {
    /*Serial.print("write task atom configuration @0x");
    Serial.print((int)addr, HEX);
    Serial.print(" ... ");

    EEPROM.put(addr, m_atom);

    Serial.println("done");
    return addr + sizeof(m_atom);*/
    return 0;
}

int Task::deserialize(int addr) {
    /*EEPROM.get(addr, m_atom);
    return addr + sizeof(m_atom);*/
    return 0;
}

void Task::setTargetTempC(uint8_t tempC) {
        m_atom.targetTemp = tempC;
}

void Task::setHoldTime(uint8_t minutes) {
        m_atom.holdTime = minutes;
}

uint8_t Task::remaining() const {
    return 0;
}
