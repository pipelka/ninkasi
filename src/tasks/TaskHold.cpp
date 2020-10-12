#include "TaskHold.h"

TaskHold* TaskHold::hold(uint8_t sensorIndex, uint8_t temp, uint8_t minutes, bool on) {
    m_atom.sensor = sensorIndex;
    m_atom.targetTemp = temp;
    m_atom.holdTime = minutes;
    m_atom.on = on ? 1 : 0;
    return this;
}

void TaskHold::begin() {
    Task::begin();

    if(m_atom.holdTime == 0) {
        m_atom.done = true;
        return;
    }

    switchOn(!m_atom.on, true);
}

void TaskHold::loop() {
    Task::loop();

    if(m_atom.done) {
        return;
    }

    if(m_atom.elapsed >= m_atom.holdTime) {
        switchOn(!m_atom.on, true);
        m_atom.done = true;
        return;
    }

    if(millisSinceStart() >= 60 * 1000) {
        m_atom.elapsed++;
        Task::begin();
    }

    Serial.print((int)remaining());
    Serial.println(" minutes left");

    float temp = this->temp();

    if(temp <= (float)m_atom.targetTemp - 1) {
        switchOn(m_atom.on);
    }

    if(temp >= (float)m_atom.targetTemp - 0.5) {
        switchOn(!m_atom.on);
    }
}

float TaskHold::temp() const {
    return m_sensors->getTemp(m_atom.sensor);
}

uint8_t TaskHold::remaining() const {
    return m_atom.holdTime - m_atom.elapsed;
}
