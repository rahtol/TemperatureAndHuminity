#include <Arduino.h>

class CMyDHT11 {

public:
    CMyDHT11(uint8_t pin, uint8_t sensortype = 0);
    void begin();
    float readTemperature(bool S = false, bool force = false);
    float readHumidity(bool force = false);
    void loop ();

private:
    uint8_t pin;
    uint8_t data[4];
    bool dataValid; // whether last measurement was successful
    uint8_t state; // 0=idle, 1=measuring
    unsigned long tStartMeasurement;
    
    void read();
    bool checkAndDecode(unsigned long *dt, int len);
};
