#include "Task.h"

class TaskHold : public Task {
public:

    TaskHold* hold(uint8_t sensorIndex, uint8_t temp = 0, uint8_t minutes = 0, bool on = true);

    void begin();

    void loop();

    uint8_t remaining() override;
};
