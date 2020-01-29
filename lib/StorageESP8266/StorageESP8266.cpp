#include "StorageESP8266.h"

#define EEPROM_EMPTY_ADDRESS 0
#define EEPROM_SSID_ADDRESS 10
#define EEPROM_PASS_ADDRESS 60
#define EEPROM_MAXVOLUME_ADDRESS 100
#define EEPROM_MAXDISTANCE_ADDRESS 104
#define EEPROM_MINDISTANCE_ADDRESS 108
#define EEPROM_OFFSETSUP_ADDRESS 112
#define EEPROM_OFFSETINF_ADDRESS 116
#define EEPROM_TIMERMQTT_ADDRESS 120
#define EEPROM_SENSORAVG_ADDRESS 124
#define EEPROM_ENABLEMQTT_ADDRESS 125

#define EEPROM_ISNOTEMPTY 20

void storage::init()
{
    // Iniciar EEPROM con 512 KB
    EEPROM.begin(512);

    // Verifica que la memoria esta vacia
    if (EEPROM_ISNOTEMPTY != storage::isEmpty())
    {
        set_isNotEmpty();
        //Default values
        setWifi_ssid("");
        setWifi_pass("");
        setMaxVolume(1000);
        setMaxDistance(1000);
        setMinDistance(500);
        setOffsetSup(0);
        setOffsetInf(0);
        setTimerMqtt(10);
        setSensorAvg(false);
        setEnableMqtt(true);
    }
}

uint8_t storage::isEmpty()
{
    uint8_t empty_num = 0;
    EEPROM.get(EEPROM_EMPTY_ADDRESS, empty_num);
    return empty_num;
}

void storage::set_isNotEmpty()
{
    EEPROM.put(EEPROM_EMPTY_ADDRESS, EEPROM_ISNOTEMPTY);
    EEPROM.commit();
}

String storage::getWifi_ssid()
{
    String str;
    EEPROM.get(EEPROM_SSID_ADDRESS, str);
    return str;
}

String storage::getWifi_pass()
{
    String str;
    EEPROM.get(EEPROM_PASS_ADDRESS, str);
    return str;
}

unsigned int storage::getMaxVolume()
{
    unsigned int number;
    EEPROM.get(EEPROM_MAXVOLUME_ADDRESS, number);
    return number;
}

int storage::getMaxDistance()
{
    int number;
    EEPROM.get(EEPROM_MAXDISTANCE_ADDRESS, number);
    return number;
}

int storage::getMinDistance()
{
    int number;
    EEPROM.get(EEPROM_MINDISTANCE_ADDRESS, number);
    return number;
}

float storage::getOffsetSup()
{
    float  number;
    float offset;
    EEPROM.get(EEPROM_OFFSETSUP_ADDRESS, number);
    offset = (float) number/100; // conver to decimal
    return offset;
}

float storage::getOffsetInf()
{
    float number;
    float offset;
    EEPROM.get(EEPROM_OFFSETINF_ADDRESS, number);
    offset = (float) number/100;// conver to decimal
    return offset;
}

unsigned int storage::getTimerMqtt()
{
    unsigned int number;
    EEPROM.get(EEPROM_TIMERMQTT_ADDRESS, number);
    return number;
}

bool storage::getSensorAvg()
{
    bool bl;
    EEPROM.get(EEPROM_SENSORAVG_ADDRESS, bl);
    return bl;
}

bool storage::getEnableMqtt()
{
    bool bl;
    EEPROM.get(EEPROM_ENABLEMQTT_ADDRESS, bl);
    return bl;
}

//setters
void storage::setWifi_ssid(String str)
{
    EEPROM.put(EEPROM_SSID_ADDRESS, str);
    EEPROM.commit();
    Serial.println("EEPROM: Wifi SSID = " + str);
}

void storage::setWifi_pass(String str)
{
    EEPROM.put(EEPROM_PASS_ADDRESS, str);
    EEPROM.commit();
    Serial.println("EEPROM: Wifi PASS = " + str);
}

void storage::setMaxVolume(unsigned int number)
{
    EEPROM.put(EEPROM_MAXVOLUME_ADDRESS, number);
    EEPROM.commit();
    Serial.println("EEPROM: Max_Volume =  " + String(number));
}

void storage::setMaxDistance(int number)
{
    EEPROM.put(EEPROM_MAXDISTANCE_ADDRESS, number);
    EEPROM.commit();
    Serial.println("EEPROM: Max_distance =  " + String(number));
}
void storage::setMinDistance(int number)
{
    EEPROM.put(EEPROM_MINDISTANCE_ADDRESS, number);
    EEPROM.commit();
    Serial.println("EEPROM: Min_distance =  " + String(number));
}
void storage::setOffsetSup(float number)
{
    EEPROM.put(EEPROM_OFFSETSUP_ADDRESS, number);
    EEPROM.commit();
    Serial.println("EEPROM: Offset_Sup =  " + String(100 - number));
    Serial.println("EEPROM: Offset_Sup (real) =  " + String(number));
}
void storage::setOffsetInf(float number)
{
    EEPROM.put(EEPROM_OFFSETINF_ADDRESS, number);
    EEPROM.commit();
    Serial.println("EEPROM: Offset_Inf =  " + String(number));
}
void storage::setTimerMqtt(unsigned int number)
{
    EEPROM.put(EEPROM_TIMERMQTT_ADDRESS, number);
    EEPROM.commit();
    Serial.println("EEPROM: TimerMqtt =  " + String(number));
}
void storage::setSensorAvg(bool bl)
{
    EEPROM.put(EEPROM_SENSORAVG_ADDRESS, bl);
    EEPROM.commit();
    Serial.println("EEPROM: SensorAvg =  " + String(bl));
}
void storage::setEnableMqtt(bool bl)
{
    EEPROM.put(EEPROM_ENABLEMQTT_ADDRESS, bl);
    EEPROM.commit();
    Serial.println("EEPROM: EnableMqtt =  " + String(bl));
}