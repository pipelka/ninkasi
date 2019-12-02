#include "TaskMachine.h"
#include <EEPROM_Rotate.h>

extern EEPROM_Rotate EEPROM;

TaskMachine::TaskMachine(uint8_t queueCount, SensorBus* sensors, RelayBus* relais) : m_timer(500), m_queueCount(queueCount) {
    m_queue = new TaskQueue*[m_queueCount];
    m_sensors = sensors;
    m_relais = relais;

    for(int i = 0; i < m_queueCount; i++) {
        m_queue[i] = new TaskQueue(i, sensors, relais);
    }
}

TaskMachine::~TaskMachine() {
    clear();
}

void TaskMachine::begin() {
    // locate devices on the bus
    Serial.println("Locating sensor devices...");
    m_sensors->begin();

    Serial.print("Found ");
    Serial.print(m_sensors->getDeviceCount(), DEC);
    Serial.println(" devices.");

    // report parasite power requirements
    Serial.print("Parasite power is: "); 
    if (m_sensors->isParasitePowerMode()) Serial.println("ON");
    else Serial.println("OFF");

    Serial.println("Locating relais bus devices...");

    if(m_relais->setup()) {
        Serial.print(m_relais->getDeviceCount(), DEC);
        Serial.println(" relais cards detected");
    }
    else {
        Serial.println("Relais bus interface not found !");
        delay(1000);
        Serial.println("Resetting ...");
        delay(2000);
        ESP.reset();
    }

    m_relais->setPort(0);
}

void TaskMachine::loop() {
    m_sensors->loop();

    if(!m_timer.repeat() || !m_running) {
        return;
    }

    bool running = false;

    for(int i = 0; i < m_queueCount; i++) {
        running |= Q(i).loop();
    }

    if(!running) {
        m_relais->setPort(0);
        clear();
    }

    m_running = running;
}

TaskQueue& TaskMachine::Q(uint8_t i) {
    return *(m_queue[i]);
}

void TaskMachine::clear() {
    for(int i = 0; i < m_queueCount; i++) {
        m_queue[i]->clear();
    }
}

void TaskMachine::start() {
    m_running = true;
}

void TaskMachine::stop() {
    m_running = false;

    for(int i = 0; i < m_queueCount; i++) {
        Q(i).stop();
    }

    m_relais->delSingle((1 << m_queueCount) - 1);
}

float TaskMachine::getTempC(uint8_t index) {
    return m_sensors->getTemp(index);
}

uint8_t TaskMachine::getTargetTempC() {
    Task* t = Q(1).findNextTask();

    if(t == nullptr) {
        return 0;
    }

    return t->targetTempC();
}

uint8_t TaskMachine::getRemainingTime()  {
    Task* t = Q(1).findNextTask();

    if(t == nullptr) {
        return 0;
    }

    return t->remaining();
}

int TaskMachine::serialize(int addr) {
    Serial.print("write taskmachine operational status ... ");
    EEPROM.put(addr, m_running);
    Serial.println("done");
    
    addr += sizeof(m_running);

    for(int i = 0; i < m_queueCount; i++) {
        addr = m_queue[i]->serialize(addr);
    }

    return addr;
}

int TaskMachine::deserialize(int addr) {
    EEPROM.get(addr, m_running);
    addr += sizeof(m_running);

    for(int i = 0; i < m_queueCount; i++) {
        addr = m_queue[i]->deserialize(addr);
    }

    return addr;
}
