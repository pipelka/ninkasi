#include "TaskRamp.h"

TaskRamp* TaskRamp::ramp(uint8_t sensorIndex, uint8_t targetTempC, bool on, bool greater) {
    m_atom.sensor = sensorIndex;
    m_atom.targetTemp = targetTempC;
    m_atom.greater = greater ? 1 : 0;
    m_atom.on = on ? 1 : 0;

    return this;
}

void TaskRamp::begin() {
    Task::begin();

    if(m_atom.on == 1) {
        m_relay->setSingle(1 << m_atom.sw);
    }
    else {
        m_relay->delSingle(1 << m_atom.sw);
    }
}

void TaskRamp::loop() {
    Task::loop();

    // check if we reached the target temperature
    float temp = m_sensors->getTemp(m_atom.sensor);

    if(m_atom.greater == 1 && temp >= (m_atom.targetTemp - 0.5)) {
        m_atom.done = 1;
    }
    if(m_atom.greater == 0 && temp <= m_atom.targetTemp) {
        m_atom.done = 1;
    }
}

void TaskRamp::setHoldTime(uint8_t minutes) {
    m_atom.holdTime = 0;
}
