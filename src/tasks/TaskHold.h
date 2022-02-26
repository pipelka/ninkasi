#pragma once
#include "Task.h"

class TaskHold : public Task {
public:

    TaskHold* hold(uint8_t sensorIndex, uint8_t temp = 0, uint8_t minutes = 0, bool on = true);

    void begin() override;

    void loop() override;

    uint8_t remaining() const override;

protected:

    bool shouldStop();

    bool shouldStart();

    float temp() const;

private:

    float m_hysteresis{0.5};

    bool m_cooling{false};

};
