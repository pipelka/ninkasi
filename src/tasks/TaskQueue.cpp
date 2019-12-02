#include "TaskQueue.h"
#include <EEPROM_Rotate.h>

extern EEPROM_Rotate EEPROM;

TaskQueue::TaskQueue(uint8_t sw, SensorBus* sensors, RelayBus* relay) {
    m_sw = sw;
    m_sensors = sensors;
    m_relay = relay;
}

TaskQueue::~TaskQueue() {
    clear();
}

void TaskQueue::clear() {
    for(int i = 0; i < 12; i++) {
        Task* task = m_tasks[i];
        if(task != nullptr) {
            task->clear();
        }
    }
}

Task* TaskQueue::findNextTask() {
    for(int i = 0; i < 12; i++) {
        Task* t = m_tasks[i];

        if(t == nullptr) {
            continue;
        }

        // skip empty or finished tasks
        if(t->done()) {
            continue;
        }

        if(t->empty()) {
            continue;
        }

        return t;
    }

    return nullptr;
}

void TaskQueue::stop() {
    // find next task
    Task* t = findNextTask();

    if(t == nullptr) {
        return;
    }

    t->stop();
}

bool TaskQueue::loop() {
    // find next task
    Task* t = findNextTask();

    if(t == nullptr) {
        return false;
    }

    Serial.print("Q");
    Serial.print((int)m_sw);
    Serial.print(" - ");

    // start task (if not started)
    if(!t->started()) {
        Serial.println("starting");
        t->begin();
        return true; 
    }

    // process task
    Serial.println("processing");
    t->loop();
    return true;
}

void TaskQueue::setSwitchRelay(uint8_t sw) {
    for(int i = 0; i < 12; i++) {
        Task* t = m_tasks[i];
        if(t != nullptr) {
            t->m_atom.sw = sw;
        }
    }
}

void TaskQueue::setSensorIndex(uint8_t index) {
    for(int i = 0; i < 12; i++) {
        Task* t = m_tasks[i];
        if(t != nullptr) {
            t->m_atom.sensor = index;
        }
    }
}

int TaskQueue::serialize(int addr) {
    Serial.print("write taskqueue relay configuration (relay ");
    Serial.print((int)m_sw);
    Serial.print(") ... ");

    EEPROM.put(addr, m_sw);
    Serial.println("done");

    addr += sizeof(m_sw);

    for(int i = 0; i < 12; i++) {
        Task* task = m_tasks[i];
        if(task != nullptr) {
            addr = task->serialize(addr);
        }
    }

    return addr;
}

int TaskQueue::deserialize(int addr) {
    EEPROM.get(addr, m_sw);
    addr += sizeof(m_sw);

    for(int i = 0; i < 12; i++) {
        Task* task = m_tasks[i];
        if(task != nullptr) {
            addr = m_tasks[i]->deserialize(addr);
        }
    }

    return addr;
}
