#include "TaskHoldStir.h"

void TaskHoldStir::loop() {
    // stir disabled ?
    if(m_stirInterval == 0) {
        TaskHold::loop();
        return;
    }

    uint8_t e = elapsed();

    // check if we should start the stir (every 'stir interval' minutes)
    if((e % m_stirInterval) == 0 && !isSwitchOn() && e > 0 && m_stirStart == 0) {
        m_stirStart = e;
        switchOn(on());
        return;
    }

    // stop stirring after a minute
    if(e >= m_stirStart + 1 && m_stirStart != 0) {
        m_stirStart = 0;

        if(shouldStop()) {
            switchOn(!on());
            return;
        }
    }

    // business as usual when not stirring
    if(m_stirStart == 0) {
        Serial.println("resting ...");
        TaskHold::loop();
        return;
    }

    Serial.println("stirring ...");
    Task::loop();
}

void TaskHoldStir::setInterval(uint8_t interval) {
    m_stirInterval = interval;
}
