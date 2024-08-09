#include "myDHT11.h"

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
    this->state = 0;
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
};

float CMyDHT11::readTemperature(bool S, bool force)
{
    this->read();
    return 28.4; // dummy
};

float CMyDHT11::readHumidity(bool force)
{
    return 0.0; // dummy
};

void CMyDHT11::read()
{
    if (this->state != 0) return;

    // Go into high impedence state to let pull-up raise data line level and
    // start the reading process.
    pinMode(pin, INPUT_PULLUP);
    delay(1);

    // First set data line low for a period according to sensor type
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    delay(20); // data sheet says at least 18ms, 20ms just to be safe

    tidx = 0;
    this->state = 1;
    this->tStartMeasurement = millis();
    this->dataValid = false;
    pinMode(pin, INPUT_PULLUP);
};

void CMyDHT11::loop()
{
    unsigned long t_current = millis();
    if ((this->state != 0) && (t_current - this->tStartMeasurement > 10)) {
        int tidx2 = tidx;
        tidx = 256; // stop ISR

        // save now to work on t ...
        // calculate deltas and pass them to decoder
        unsigned long dtbf[256];
        for (int i=0; i<tidx2-1; i++) dtbf[i] = t[i+1]-t[i];
        this->dataValid = this->checkAndDecode(dtbf, tidx2-1);

        // ... til here
        this->state = 0; // a new read cycle may start now
       
    }
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
    return true; // TODO
};

bool CMyDHT11::checkAndDecode(unsigned long *dt, int len)
{
    Serial.printf("len=%d\n", len);
    for (int i=0; i<len; i++) Serial.printf("%d ", dt[i]);
    Serial.printf("\n");

    int idx0 = 0;
    while (idx0 + 80 < len)
    {
        if (valid50msSequence(idx0, dt, len)) {
            Serial.printf("valid50msSequence at idx0=%d\n", idx0);
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
