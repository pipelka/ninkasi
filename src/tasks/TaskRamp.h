#pragma once
#include "Task.h"

class TaskRamp : public Task {
public:

    TaskRamp* ramp(uint8_t sensorIndex, uint8_t targetTempC = 0, bool on = true, bool greater = true);

    void begin();

    void loop();

    void setHoldTime(uint8_t minutes) override;

protected:

    bool isTempReached();

};
