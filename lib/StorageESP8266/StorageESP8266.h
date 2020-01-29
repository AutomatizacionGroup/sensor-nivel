#include <EEPROM.h>
#include <Arduino.h>

namespace storage
{
    void init();

    uint8_t isEmpty();
    void set_isNotEmpty();

    String getWifi_ssid();
    String getWifi_pass();
    unsigned int getMaxVolume(); // L
    int getMaxDistance(); // mm
    int getMinDistance(); // mm
    float getOffsetSup(); // mm
    float getOffsetInf(); // mm
    unsigned int getTimerMqtt(); // Seconds
    bool getSensorAvg();
    bool getEnableMqtt();

    void setWifi_ssid(String);
    void setWifi_pass(String);
    void setMaxVolume(unsigned int); // L
    void setMaxDistance(int); // mm
    void setMinDistance(int); // mm
    void setOffsetSup(float); // percent %
    void setOffsetInf(float); // percent %
    void setTimerMqtt(unsigned int); // Seconds
    void setSensorAvg(bool);
    void setEnableMqtt(bool);
}