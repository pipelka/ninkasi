#include "TaskRamp.h"

TaskRamp* TaskRamp::ramp(uint8_t sensorIndex, uint8_t targetTempC, bool on, bool greater) {
    m_atom.sensor = sensorIndex;
    m_atom.targetTemp = targetTempC;
    m_atom.greater = greater ? 1 : 0;
    m_atom.on = on ? 1 : 0;

    return this;
}

bool TaskRamp::isTempReached() {
    // check if we reached the target temperature
    float temp = m_sensors->getTemp(m_atom.sensor);

    if(m_atom.greater == 1 && temp >= (m_atom.targetTemp - 0.5)) {
        return true;
    }

    if(m_atom.greater == 0 && temp <= m_atom.targetTemp) {
        return true;
    }

    return false;
}

void TaskRamp::begin() {
    Task::begin();

    if(!isTempReached()) {
        switchOn(m_atom.on, true);
    }
}

void TaskRamp::loop() {
    Task::loop();

    // skip if we're done
    if(m_atom.done) {
        return;
    }

    // we're done where temp is reached
    m_atom.done = isTempReached() ? 1 : 0;

    // switch off when done
    if(m_atom.done) {
        switchOn(!m_atom.on, true);
    }
}

void TaskRamp::setHoldTime(uint8_t minutes) {
    m_atom.holdTime = 0;
}
