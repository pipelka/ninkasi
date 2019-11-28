#include "Arduino.h"
#include "RelayBus.h"

#ifdef DEBUG
    #if defined(ESP8266)
    #define LOG(...) Serial.print("[RELCTRL] "); Serial.printf(__VA_ARGS__); Serial.println()
    #endif
    #ifdef __AVR__
    char _logData[128];
    #define LOG(...) Serial.print("[RELCTRL] "); snprintf(_logData, sizeof(_logData), __VA_ARGS__); Serial.println(_logData)
    #endif
#else
#define LOG(...)
#endif

RelayBus::RelayBus(int rx, int tx) : m_serial(rx, tx) {
}

bool RelayBus::setup() {
    LOG("SETUP");
    m_serial.begin(19200);

    while(m_serial.available()) {
        m_serial.read();
        delay(4);
    }

    for(int i = 0; i < 4; i++) {
        m_serial.write((char)1);
        delay(10);
        if(m_serial.available() > 3) {
            break;
        }
    }

    Frame response{};

    if(!receiveResponse(1, response)) {
        return false;
    }

    while(receiveResponse(1, response, true)) {
        if(response[0] == 1) {
            m_deviceCount = response[1] -1;
            break;
        }
    }

    setPort(0);
    return m_deviceCount != 0;
}

bool RelayBus::getPort(uint8_t& data, uint8_t addr) {
    LOG("GETPORT %i", addr);
    if(!get(2, data, addr)) {
        return false;
    }

    setCache(data, addr);
    return true;
}

bool RelayBus::setPort(uint8_t data, uint8_t addr) {
    LOG("SETPORT %i, %i", data, addr);
    if(!set(3, data, addr)) {
        return false;
    }

    setCache(data, addr);
    return true;
}

bool RelayBus::getOption(uint8_t& data, uint8_t addr) {
    LOG("GETOPTION %i", addr);
    return get(4, data, addr);
}

bool RelayBus::setOption(uint8_t data, uint8_t addr) {
    LOG("SETOPTION %i, %i", data, addr);
    return set(5, data, addr);
}

bool RelayBus::setSingle(uint8_t data, uint8_t addr) {
    LOG("SETSINGLE %i, %i", data, addr);
    if(!set(6, data, addr)) {
        return false;
    }

    uint8_t d = getCache(addr);
    setCache(d | data, addr);

    return true;
}

bool RelayBus::delSingle(uint8_t data, uint8_t addr) {
    LOG("DELSINGLE %i, %i", data, addr);
    if(!set(7, data, addr)) {
        return false;
    }

    uint8_t d = getCache(addr);
    setCache(d & (data ^ 0xFF), addr);

    return true;
}

bool RelayBus::toggle(uint8_t data, uint8_t addr) {
    LOG("TOGGLE %i, %i", data, addr);
    return set(8, data, addr);
}

bool RelayBus::sendFrame(uint8_t cmd, uint8_t data, uint8_t addr) {
    uint8_t checksum = ((cmd ^ addr) ^ data);
    LOG(">> SEND (%i, %i, %i, %i)", cmd, addr, data, checksum);

    // assemble frame
    Frame frame; //{cmd, addr, data, checksum};

    frame[0] = cmd;
    frame[1] = addr;
    frame[2] = data;
    frame[3] = checksum;

    size_t written = 0;
    // write frame
    for(size_t i = 0; i < sizeof(frame); i++) {
        written += m_serial.write(frame[i]);
    }
    LOG("%i bytes written", (int)written);

    return written == sizeof(frame);
}

bool RelayBus::receiveResponse(uint8_t cmd, Frame frame, bool ignoreError) {
    LOG("WAIT FOR RESPONSE");

    // wait for response
    for(int i = 0; i < 10 && m_serial.available() != 4; i++) {
        delay(100);
    }

    // no response
    /*if(m_serial.available() == 0) {
        LOG("TIMEOUT NO RESPONSE");
        return false;
    }*/

    // read response frame
    for(unsigned int i = 0; i < sizeof(Frame); i++) {
        int c = m_serial.read();
        delay(4);
        
        if(c == -1) {
            LOG("ERROR: READING RESPONSE");
            return false;
        }

        frame[i] = (uint8_t)c & 0xFF;
    }

    LOG("<< RECV(%i, %i, %i, %i)", frame[0], frame[1], frame[2], frame[3]);

    if(ignoreError) {
        return true;
    }

    // response cmd
    if(frame[0] != (cmd ^ 0xFF)) {
        LOG("ERROR: WRONG COMMAND IN RESPONSE");
        return false;
    }

    // checksum of response
    uint8_t checkSum = frame[0] ^ frame[1] ^ frame[2];

    if(checkSum == frame[3]) {
        LOG("OK");
        return true;
    }

    LOG("ERROR: WRONG CHECKSUM");
    return false;
}

bool RelayBus::get(uint8_t cmd, uint8_t& data, uint8_t addr) {
    if(!sendFrame(cmd, data, addr)) {
        return false;
    }

    Frame response{};
    if(!receiveResponse(cmd, response)) {
        return false;
    }

    return response[2];
}

bool RelayBus::set(uint8_t cmd, uint8_t data, uint8_t addr) {
    if(!sendFrame(cmd, data, addr)) {
        return false;
    }

    Frame response{};
    return receiveResponse(cmd, response);
}

uint8_t RelayBus::getCache(uint8_t addr) {
    if(addr < 1 || addr > sizeof(m_portCache)) {
        return 0;
    }

    return m_portCache[addr - 1];
}

void RelayBus::setCache(uint8_t data, uint8_t addr) {
    if(addr < 1 || addr > sizeof(m_portCache)) {
        return;
    }

    m_portCache[addr - 1] = data;
}

bool RelayBus::isRelayOn(uint8_t relay) {
    return ((getCache() & bit(relay)) == bit(relay));
}

void RelayBus::relaisTest() {
    // relais test sequence
  setPort(0x00);
  delay(2000);

  setPort(0x01);
  delay(2000);

  setPort(0x02);
  delay(2000);

  setPort(0x04);
  delay(2000);

  setPort(0x08);
  delay(2000);

  setPort(0x10);
  delay(2000);

  setPort(0x20);
  delay(2000);

  setPort(0x40);
  delay(2000);

  setPort(0x80);
  delay(2000);

  setPort(0xFF);
}
