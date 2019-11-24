#include "Task.h"

void Task::begin() {
    m_startMillis = millis();
}

void Task::loop() {
    if(m_atom.targetTemp != 0) {
        float temp = m_sensors->getTemp(m_atom.sensor);

        Serial.print("T");
        Serial.print(m_atom.sensor + 1, DEC);
        Serial.print(" - ");
        Serial.print(temp);
        Serial.println("C");
    }
}

uint32_t Task::millisSinceStart() {
    return millis() - m_startMillis;
}

void Task::switchOn(bool on, bool force) {
    if(on == m_atom.sw_on && !force) {
        return;
    }

    m_atom.sw_on = on;

    if(m_atom.sw_on == 1) {
        m_relay->setSingle(1 << m_atom.sw);
    }
    else {
        m_relay->delSingle(1 << m_atom.sw);
    }
}

void Task::serialize() {
}

void Task::setTargetTempC(uint8_t tempC) {
        m_atom.targetTemp = tempC;
}

void Task::setHoldTime(uint8_t minutes) {
        m_atom.holdTime = minutes;
}

uint8_t Task::remaining() {
    return 0;
}
