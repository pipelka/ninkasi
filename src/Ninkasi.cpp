#include "Ninkasi.h"
#include "vpins.h"
#include <EEPROM_Rotate.h>

extern EEPROM_Rotate EEPROM;

Ninkasi::Ninkasi(SensorBus* sensors, RelayBus* relay) : 
m_sensors(sensors), 
m_relay(relay), 
m_mash(4, m_sensors, m_relay), 
m_boil(4, m_sensors, m_relay) {
}

Ninkasi::~Ninkasi() {
}

void Ninkasi::begin() {
    // ninkasi machine definitions

    // mash steps
    for(int i = 0; i < 6; i++) {
        uint8_t pos =  i* 2;
        m_mash.Q(RELAY_IMPELLER).setTask<TaskRamp>(pos)->ramp(TEMP_PROBE1);
        m_mash.Q(RELAY_HEATING).setTask<TaskRamp>(pos)->ramp(TEMP_PROBE1);
        m_mash.Q(RELAY_IMPELLER).setTask<TaskHoldStir>(pos + 1)->hold(TEMP_PROBE1);
        m_mash.Q(RELAY_HEATING).setTask<TaskHold>(pos + 1)->hold(TEMP_PROBE1);
    }

    // boil machine step
    m_boil.Q(RELAY_BOIL).setTask<TaskRamp>(0)->ramp(TEMP_PROBE2);
    m_boil.Q(RELAY_BOIL).setTask<TaskHold>(1)->hold(TEMP_PROBE2);

    m_mash.begin();
    m_boil.begin();
}

void Ninkasi::loop() {
    m_mash.loop();
    m_boil.loop();
}

void Ninkasi::setMashStep(uint8_t step, float temp, int duration) {
    uint8_t pos = step * 2;

    for(uint8_t q = 0; q < 2; q++) {
        Task* taskRamp = m_mash.Q(q)[pos];
        Task* taskHold = m_mash.Q(q)[pos + 1];

        if(temp > -1) {
            taskRamp->setTargetTempC(temp);
            taskHold->setTargetTempC(temp);
        }

        if(duration > -1) {
            taskHold->setHoldTime(duration);
        }
    }
}

void Ninkasi::setBoil(float temp, int duration) {
    if(duration != -1) {
        m_boil.Q(RELAY_BOIL)[1]->setHoldTime(duration);
    }

    if(temp != -1) {
        m_boil.Q(RELAY_BOIL)[0]->setTargetTempC(temp);
        m_boil.Q(RELAY_BOIL)[1]->setTargetTempC(temp);
    }
}

void Ninkasi::setBoilSwitchRelay(uint8_t sw) {
    m_boil.Q(RELAY_BOIL).setSwitchRelay(sw);
}

void Ninkasi::setBoilSensorIndex(uint8_t index) {
    m_boil.Q(RELAY_BOIL).setSensorIndex(index);
}

void Ninkasi::startMash() {
    if(boilRunning()) {
        m_boil.stop();
    }
    m_mash.start();
}

void Ninkasi::startBoil() {
    if(mashRunning()) {
        m_mash.stop();
    }
    m_boil.start();
}

void Ninkasi::stop() {
    if(mashRunning()) {
        m_mash.stop();
    }
    if(boilRunning()) {
        m_boil.stop();
    }
}

float Ninkasi::getTargetTempC() {
    if(mashRunning()) {
        return m_mash.getTargetTempC();
    }

    if(boilRunning()) {
        return m_boil.getTargetTempC();
    }

    return 0;
}

uint8_t Ninkasi::getRemainingTime() {
    if(mashRunning()) {
        return m_mash.getRemainingTime();
    }

    if(boilRunning()) {
        return m_boil.getRemainingTime();
    }

    return 0;
}

bool Ninkasi::isHeating() {
    uint8_t d = m_relay->getCache();
    return d & 1;
}

bool Ninkasi::isImpellerRunning() {
    uint8_t d = m_relay->getCache();
    return d & 2;
}

void Ninkasi::reset() {
    stop();
    m_mash.clear();
    m_boil.clear();
}

int Ninkasi::serialize(int addr) {
    Serial.println("serialiting system status ...");
    uint8_t port = m_relay->getCache();

    Serial.print("write relay status ... ");
    EEPROM.put(addr, port);
    Serial.println("done");

    addr += sizeof(port);

    addr = m_mash.serialize(addr);
    return m_boil.serialize(addr);
}

int Ninkasi::deserialize(int addr) {
    uint8_t port = 0;
    EEPROM.get(addr,port);
    addr += sizeof(port);

    m_relay->setPort(port);

    addr = m_mash.deserialize(addr);
    return m_boil.deserialize(addr);
}
