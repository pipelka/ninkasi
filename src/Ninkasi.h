#include "RelayBus.h"
#include "SensorBus.h"
#include "tasks/TaskMachine.h"

class Ninkasi {
public:

    Ninkasi(SensorBus* sensors, RelayBus* relay);

    virtual ~Ninkasi();

    void begin();

    void loop();

    void setMashStep(uint8_t step, float temp, uint8_t duration = -1);

    void setBoil(float temp, uint8_t duration = -1);

    inline bool mashRunning() {
        return m_mash.running();
    }

    inline bool boilRunning() {
        return m_boil.running();
    }

    bool isHeating();

    bool isImpellerRunning();

    void setBoilSwitchRelay(uint8_t sw);

    void setBoilSensorIndex(uint8_t index);

    void startMash();

    void startBoil();

    void stop();

    inline bool running() {
        return m_mash.running() || m_boil.running();
    }

    float getTargetTempC();

    uint8_t getRemainingTime();
    
    void reset();

private:

    SensorBus* m_sensors;
    RelayBus* m_relay;

    TaskMachine m_mash;
    TaskMachine m_boil;
};
