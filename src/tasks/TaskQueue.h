#pragma once
#include <stdint.h>
#include "Task.h"
#include "RelayBus.h"
#include "SensorBus.h"

class TaskQueue {
public:

    TaskQueue(uint8_t sw, SensorBus* sensors, RelayBus* relay);
    
    virtual ~TaskQueue();
    
    bool loop();

    void stop();

    template <class T = Task>
    T* setTask(uint8_t pos) {
        T* t = new T;
        m_tasks[pos] = t;
        
        t->m_sensors = m_sensors;
        t->m_relay = m_relay;
        t->m_atom.sw = m_sw;
        t->m_atom.empty = 0;

        return t;
    }

    inline Task* operator[](uint8_t pos) {
        return m_tasks[pos];
    }

    Task* findNextTask();

protected:

    void clear();

    friend class TaskMachine;

private:

    Task* m_tasks[12]{nullptr};
    uint8_t m_sw;
    SensorBus* m_sensors;
    RelayBus* m_relay;
};
