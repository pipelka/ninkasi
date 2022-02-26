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

    switchOn(!m_atom.on, true); // TODO - do we need to force the switch ?
}

void TaskHold::loop() {
    Task::loop();

    if(m_atom.done) {
        return;
    }

    if(m_atom.elapsed >= m_atom.holdTime) {
        switchOn(!m_atom.on);
        m_atom.done = true;
        return;
    }

    Serial.print((int)remaining());
    Serial.println(" minutes left");

    if(shouldStart()) {
        switchOn(m_atom.on);
    }
    else if(shouldStop()) {
        switchOn(!m_atom.on);
    }
}

bool TaskHold::shouldStop() {
    if(m_cooling) {
        return (this->temp() <= (float)m_atom.targetTemp + m_hysteresis);
    }

    return (this->temp() >= (float)m_atom.targetTemp - m_hysteresis);
}

bool TaskHold::shouldStart() {
    if(m_cooling) {
        return (this->temp() >= (float)m_atom.targetTemp + m_hysteresis * 2);
    }

    return (this->temp() <= (float)m_atom.targetTemp - m_hysteresis * 2);
}

float TaskHold::temp() const {
    return m_sensors->getTemp(m_atom.sensor);
}

uint8_t TaskHold::remaining() const {
    return m_atom.holdTime - m_atom.elapsed;
}
