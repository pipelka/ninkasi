#pragma once
#include <stdint.h>
#include <Arduino.h>
#include "RelayBus.h"
#include "SensorBus.h"

struct TaskAtom {
    TaskAtom() {
        type = 0;
        sensor = 0;
        greater = 1;
        on = 1;
        sw = 0;
        done = 0;
        empty = 1;
        elapsed = 0;
    }
    uint8_t type:3;
    uint8_t sensor:3;
    uint8_t greater:1;
    uint8_t on:1;
    uint8_t sw:3;
    uint8_t sw_on:1;
    uint8_t empty:1;
    uint8_t reserved:2;
    uint8_t done:1;
    uint8_t elapsed{0};
    uint8_t holdTime{0};
    uint8_t targetTemp{0};
};

class Task {
public:

    Task() = default;

    virtual ~Task() = default;

    virtual void begin();

    virtual void loop();

    virtual void clear();

    uint32_t millisSinceStart();

    inline uint8_t type() {
        return m_atom.type;
    }

    inline uint8_t sensor() {
        return m_atom.sensor;
    }

    inline uint8_t on() {
        return m_atom.on;
    }

    inline uint8_t targetTempC() {
        return m_atom.targetTemp;
    }
    
    inline uint8_t holdTime() {
        return m_atom.holdTime;
    }

    inline bool empty() {
        return (m_atom.empty == 1);
    }

    inline bool started() {
        return (m_startMillis != 0);
    }

    inline bool done() {
        return (m_atom.done == 1);
    }

    inline bool isSwitchOn() {
        return (m_atom.sw_on);
    }

    virtual uint8_t remaining();

    inline void stop() {
        m_startMillis = 0;
    }

    inline uint8_t elapsed() {
        return m_atom.elapsed;
    }

    void setTargetTempC(uint8_t tempC);

    virtual void setHoldTime(uint8_t minutes);

    void switchOn(bool on, bool force = false);

    void serialize();

protected:

    SensorBus* m_sensors{nullptr};

    RelayBus* m_relay{nullptr};

    uint32_t m_startMillis{0};

    struct TaskAtom m_atom;

    friend class TaskQueue;
};

