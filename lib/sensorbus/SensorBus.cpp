#include "SensorBus.h"

SensorBus::SensorBus(OneWire* oneWire) : m_sensors(oneWire) {
}

void SensorBus::begin() {
    m_sensors.begin();

    for(uint8_t i = 0; i < getDeviceCount(); i++) {
        if (!m_sensors.getAddress(m_thermometer[i], i)) {
            continue;
        }

        // set the resolution to 9 bit
        m_sensors.setResolution(m_thermometer[i], 9);
    }

    m_sensors.setWaitForConversion(false);
    m_sensors.setCheckForConversion(true);

    m_requesting = requestTemperatures();
}

void SensorBus::loop() {
    if(!m_requesting) {
        m_requesting = requestTemperatures();
        return;
    }

    unsigned long ms = m_sensors.millisToWaitForConversion(m_sensors.getResolution());

    if(millis() - m_lastTempRequest <= ms) {
        return;
    }

    if(m_sensors.isConversionComplete()) {
        for(int i = 0; i < getDeviceCount(); i++) {
            float temp = m_sensors.getTempC(m_thermometer[i]);

            // invalid temp - skip
            if(temp == DEVICE_DISCONNECTED_C) {
                continue;
            }

            // first temp - set
            if(m_temperature[i] == 0) {
                m_temperature[i] = temp;
                continue;
            }

            // sample temp by 2 values
            m_temperature[i] = (m_temperature[i] + temp) / 2;
        }

        m_requesting = false;
    }

}

bool SensorBus::requestTemperatures() {
    unsigned long now = millis();

    if(now - m_lastTempRequest < m_measureIntervalMs) {
        return false;
    }

    m_sensors.requestTemperatures();
    m_lastTempRequest = now;
    return true;
}

uint8_t SensorBus::getDeviceCount() {
    uint8_t count = m_sensors.getDeviceCount();
    return count < 8 ? count : 8;
}

float SensorBus::getTemp(uint8_t index, bool celsius) {
    if(index < 0 || index > 8) {
        return DEVICE_DISCONNECTED_C;
    }

    float C = m_temperature[index];
    return celsius ? C : DallasTemperature::toFahrenheit(C);
}

void SensorBus::setMeasureInterval(unsigned int intervalMs) {
    m_measureIntervalMs = intervalMs;
}

bool SensorBus::isParasitePowerMode() {
    return m_sensors.isParasitePowerMode();
}
