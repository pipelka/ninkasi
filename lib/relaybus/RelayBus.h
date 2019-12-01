#pragma once
#include <Arduino.h>
#include <SoftwareSerial.h>

class RelayBus {
public:

    RelayBus(int rx, int tx);

    bool setup();

    inline int getDeviceCount() {
        return m_deviceCount;
    }

    bool getPort(uint8_t& data, uint8_t addr = 1);

    bool setPort(uint8_t data, uint8_t addr = 1);

    bool getOption(uint8_t& data, uint8_t addr = 1);

    bool setOption(uint8_t data, uint8_t addr = 1);

    bool setSingle(uint8_t data, uint8_t addr = 1);

    bool delSingle(uint8_t data, uint8_t addr = 1);

    bool toggle(uint8_t data, uint8_t addr = 1);

    void relaisTest();

    uint8_t getCache(uint8_t addr = 1);

    bool isRelayOn(uint8_t relay);

    void flush();

protected:

    typedef uint8_t Frame[4];

    bool sendFrame(uint8_t cmd, uint8_t data, uint8_t addr = 1);

    bool receiveResponse(uint8_t cmd, bool ignoreError = false);

    bool get(uint8_t cmd, uint8_t& data, uint8_t addr);

    bool set(uint8_t cmd, uint8_t data, uint8_t addr);

    void setCache(uint8_t data, uint8_t addr = 1);

private:

    Frame m_frame{0};

    int m_deviceCount{0};

    uint8_t m_portCache[8]{0};

    SoftwareSerial m_serial;
};
