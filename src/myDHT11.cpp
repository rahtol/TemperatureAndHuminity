#include "myDHT11.h"

static const unsigned long t_stateTimeout[] = {0, 1, 20, 10, 11969};

volatile unsigned long t[256];
volatile int tidx = 256; // this value prevents ISR from doing anything

// ISR on data pin changed
void pinchange() {
    if (tidx < 256) {
        t[tidx++] = micros();
    };
}

CMyDHT11::CMyDHT11(uint8_t pin, uint8_t sensortype)
{
    this->pin = pin;
    this->state = STATE_IDLE;
    this->cyclicMeasurementOn = false;
    this->data[0] = 0;
    this->data[1] = 0;
    this->data[2] = 0;
    this->data[3] = 0;
    this->data[4] = 0;
    this->dataValid = false;
    
};

void CMyDHT11::begin()
{
    pinMode(this->pin, INPUT_PULLUP);
    attachInterrupt (digitalPinToInterrupt (this->pin), pinchange, CHANGE);
    this->startCyclicMeasurement();
};

void CMyDHT11::startCyclicMeasurement()
{
    this->cyclicMeasurementOn = true;

    if (this->state == STATE_IDLE) {
        this->t_curStateEntered = millis();
        this->state = STATE_PROLOG;
        pinMode(this->pin, INPUT_PULLUP);
    }
};

void CMyDHT11::stopCyclicMeasurement()
{
    this->cyclicMeasurementOn = false;
};


float CMyDHT11::readTemperature(bool S, bool force)
{
    float temperature = 999.9; // return data invalid  as default
    if (this->dataValid) {
        temperature = (float) data[2] + ((float) data[3]) / 10.0;
    }
    return temperature;
};

float CMyDHT11::readHumidity(bool force)
{
    return 0.0; // dummy
};

void CMyDHT11::loop()
{
    unsigned long t_current = millis();
    if ((this->state != STATE_IDLE) && (t_current - this->t_curStateEntered > t_stateTimeout[this->state]))
    {
        this->t_curStateEntered = t_current;

        switch (this->state) {
        
        case STATE_PROLOG:
        {
            this->state = STATE_OUTPUT_LOW;
            // set data line low for a period of at least 18ms according to DHT11 data sheet
            pinMode(pin, OUTPUT);
            digitalWrite(pin, LOW);
            break;
        }
        case STATE_OUTPUT_LOW:
        {
            this->state = STATE_READ;
            // data pin back to input mode, DHT11 reads high and will start to send
            // enable ISR to record timestamps
            this->dataValid = false;
            tidx = 0;
            pinMode(pin, INPUT_PULLUP);
            break;
        }
        case STATE_READ:
        { 
            this->state = STATE_EPILOG;

            int tidx2 = tidx;
            tidx = 256; // stop ISR

            // save now to work on t ...
            // calculate deltas and pass them to decoder
            unsigned long dtbf[256];
            for (int i=0; i<tidx2-1; i++) dtbf[i] = t[i+1]-t[i];
            this->dataValid = this->checkAndDecode(dtbf, tidx2-1);

            break;
        }
        case STATE_EPILOG:
        {
            if (this->cyclicMeasurementOn) {
                this->state = STATE_PROLOG;
            }
            else {
                this->state = STATE_IDLE;
            };
            break;
        }
        default:
            break;
        }; // of switch (this->state) ..
        
//        Serial.printf("state=%d\n", this->state);
    };
};

bool checkRange(unsigned long dt, unsigned long min, unsigned long max)
{
//    Serial.printf("checkRange:dt=%d, min=%d, max=%d\n", dt, min, max);
    return (min < dt) && (dt <= max);
};

bool valid50msSequence(int idx0, unsigned long *dt, int len)
{
    for (int idx=idx0; idx < idx0+80; idx +=2) {
        
        if (!checkRange(dt[idx], 38, 62)) return false;
    };
    return true;
};

bool validBitSequence(int idx0, unsigned long *dt, int len, uint8_t *data)
{
    for (int i=0; i < 6; i++) data[i] = 0;
    for (int idx=idx0+1, bitidx = 0; idx < idx0+80; idx +=2, bitidx++) {
        data[bitidx/8] <<= 1;
        if (checkRange(dt[idx], 14, 38)) {
            // valid LOW bit detected
            Serial.print("0");
        }
        else if (checkRange(dt[idx], 63, 82)) {
            // valid HIGH bit detected
            data[bitidx/8] |= 1;
            Serial.print("1");
        } 
        else {
            return false;
        }
    };
    Serial.printf("  data: %d %d %d %d %d\n", data[0], data[1], data[2], data[3], data[4]);
    return true;
};

bool checksumValid(uint8_t *data)
{
    return data[0] + data[1] + data[2] + data[3]  == data[4];
};

bool CMyDHT11::checkAndDecode(unsigned long *dt, int len)
{
/*
    Serial.printf("len=%d\n", len);
    for (int i=0; i<len; i++) Serial.printf("%d ", dt[i]);
    Serial.printf("\n");
*/

    int idx0 = 0;
    while (idx0 + 80 < len)
    {
        if (valid50msSequence(idx0, dt, len)) {
//            Serial.printf("valid50msSequence at idx0=%d\n", idx0);
            if (validBitSequence(idx0, dt, len, this->data)) {
                return checksumValid(this->data);
            }
            else {
                return false;
            }
        }; 
        idx0 += 1;
    };
    return false;
};
