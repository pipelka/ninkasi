#pragma once
#include "TaskHold.h"

class TaskHoldStir : public TaskHold {
public:

    void loop() override;

    void setInterval(uint8_t interval);
    
protected:

    uint8_t m_stirInterval{7};

    uint8_t m_stirStart{0};
};
