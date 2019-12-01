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

    flush();

    for(int i = 0; i < 4; i++) {
        m_serial.write((char)1);
        delay(10);
        if(m_serial.available()) {
            break;
        }
    }

    if(!receiveResponse(1)) {
        return false;
    }

    while(receiveResponse(1, true)) {
        if(m_frame[0] == 1) {
            m_deviceCount = m_frame[1] -1;
            break;
        }
    }

    setPort(0);
    return m_deviceCount != 0;
}

void RelayBus::flush() {
    m_serial.flush();

    while(m_serial.available()) {
        m_serial.read();
        delay(4);
    }
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
    // flush receive buffer
    flush();

    uint8_t checksum = ((cmd ^ addr) ^ data);
    LOG(">> SEND (%i, %i, %i, %i)", cmd, addr, data, checksum);

    // assemble frame
    m_frame[0] = cmd;
    m_frame[1] = addr;
    m_frame[2] = data;
    m_frame[3] = checksum;

    size_t written = 0;
    size_t count = 0;

    // write frame
    while(written < sizeof(m_frame)) {
        // there is a command response waiting ?
        // something wrong is going on here -> exit
        if(m_serial.available() != 0) {
            return false;
        }

        // write byte of frame
        if(m_serial.write(m_frame[written]) == 1) {        
            written++;
            continue;
        }

        // retried 50 times to write -> exit
        if(count == 50) {
            return false;
        }

        // failed to write -> wait and retry
        delay(4);
        count++;
    }

    return true;
}

bool RelayBus::receiveResponse(uint8_t cmd, bool ignoreError) {
    LOG("WAIT FOR RESPONSE");

    // wait for response
    for(int i = 0; i < 10 && m_serial.available() <= 0; i++) {
        delay(100);
    }

    if(m_serial.available() <= 0) {
        return false;
    }

    // read response frame
    size_t i = 0;
    while(i < sizeof(Frame)) {
        if(m_serial.available() <= 0) {
            return false;
        }

        int c = m_serial.read();

        if(c == -1) {
            delay(10);
            continue;
        }

        m_frame[i] = (uint8_t)c & 0xFF;
        i++;
    }

    LOG("<< RECV(%i, %i, %i, %i)", m_frame[0], m_frame[1], m_frame[2], m_frame[3]);

    if(ignoreError) {
        return true;
    }

    // response cmd
    if(m_frame[0] != (cmd ^ 0xFF)) {
        LOG("ERROR: WRONG COMMAND IN RESPONSE");
        return false;
    }

    // checksum of response
    uint8_t checkSum = m_frame[0] ^ m_frame[1] ^ m_frame[2];

    if(checkSum == m_frame[3]) {
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

    if(!receiveResponse(cmd)) {
        return false;
    }

    return m_frame[2];
}

bool RelayBus::set(uint8_t cmd, uint8_t data, uint8_t addr) {
    if(!sendFrame(cmd, data, addr)) {
        return false;
    }

    return receiveResponse(cmd);
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
