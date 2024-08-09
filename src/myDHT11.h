#include <Arduino.h>

class CMyDHT11 {

public:
    enum TState {
        STATE_IDLE,         // ready for next measurement
        STATE_PROLOG,       // 1ms INPUT_PULLUP       
        STATE_OUTPUT_LOW,   // at least 18ms required
        STATE_READ,         // ISR records a timestamp whenever the data pin changes 
        STATE_EPILOG        // ensure gap to next 
    } _TState;

    CMyDHT11(uint8_t pin, uint8_t sensortype = 0);
    void begin();
    void startCyclicMeasurement();
    void stopCyclicMeasurement();
    bool isDataValid() { return dataValid; };
    float readTemperature(bool S = false, bool force = false);
    float readHumidity(bool force = false);
    void loop ();

private:
    uint8_t pin;
    uint8_t data[4];
    bool dataValid; // whether last measurement was successful
    TState state; // 0=idle, 1=measuring
    bool cyclicMeasurementOn;
    unsigned long tStartMeasurement;
    unsigned long t_curStateEntered;
    
    bool checkAndDecode(unsigned long *dt, int len);
};
