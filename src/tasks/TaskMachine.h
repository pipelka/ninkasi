#pragma once

#include "TaskQueue.h"
#include "TaskRamp.h"
#include "TaskHold.h"
#include "TaskHoldStir.h"

#include "RelayBus.h"
#include "SensorBus.h"
#include <neotimer.h>

class TaskMachine {
public:

    TaskMachine(uint8_t queueCount, SensorBus* sensors, RelayBus* relais);

    virtual ~TaskMachine();

    void begin();

    void loop();

    TaskQueue& Q(uint8_t i);

    void start();

    void stop();

    float getTempC(uint8_t index);

    uint8_t getTargetTempC();

    uint8_t getRemainingTime();
    
    inline bool running() {
        return m_running;
    }

    void clear();

    int serialize(int addr);

    int deserialize(int addr);

private:

    SensorBus* m_sensors;
    RelayBus* m_relais;
    Neotimer m_timer;
    TaskQueue** m_queue;

    bool m_running{false};
    uint8_t m_queueCount;
};
