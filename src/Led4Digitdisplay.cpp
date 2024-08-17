#include "Led4DigitDisplay.h"

uint8_t encodeDigit[10] = {231, 36, 179, 182, 116, 214, 215, 164, 247, 246};

CLed4DigitDisplay::CLed4DigitDisplay(uint8_t* segPins,  uint8_t* digitPins)
{
    for (uint8_t i=0; i<8; i++) this->segPins[i] = segPins[i];
    for (uint8_t i=0; i<4; i++) this->digitPins[i] = digitPins[i];
    value = 0;
    leadingzerosOn = false;
    displayOn = false;
    for (uint8_t i=0; i<4; i++) segmentBits[i] = 0;
    t_period = 4;
    t_last_update = 0;
    last_digit = 0;
    this->dp = 0;  // all decimal point off
};

void CLed4DigitDisplay::init()
{
    this->t_last_update = millis();
    for (uint8_t i=0; i<8; i++) pinMode(this->segPins[i], OUTPUT);
    for (uint8_t i=0; i<4; i++) pinMode(this->digitPins[i], OUTPUT);
};

void CLed4DigitDisplay::loop()
{
  unsigned long t_cur = millis();
  if (t_cur - t_last_update > t_period) {
    t_last_update = t_cur;
    digitalWrite(digitPins[last_digit], LOW);
    last_digit = (last_digit + 1) % 4;
    setSegmentBits(segmentBits[last_digit]);
    digitalWrite(digitPins[last_digit], HIGH);
  }
};

void CLed4DigitDisplay::setValue(uint16_t value)
{
    this->value = value;
    this->encodeSegmentbits();
};

void CLed4DigitDisplay::setLeadingzerosOnOff (bool on)
{
    this->leadingzerosOn = on;
    this->encodeSegmentbits();
};

void CLed4DigitDisplay::setDisplayOnOff(bool on)
{
    this->displayOn = on;
    this->encodeSegmentbits();
};

void CLed4DigitDisplay::setDP(uint8_t digit)
{
    this->dp = (1 << digit);
}

void CLed4DigitDisplay::encodeSegmentbits(void)
{
    if (!this->displayOn) {
        for (int i=0; i<4; i++) this->segmentBits[i] = 0; // all dark
    }
    else {
        uint16_t v = this->value;
        uint16_t digit[4];
        for (int i=0; i<4; i++) {
            digit[i] = v % 10;
            v = v / 10;
        }
        bool leadActive = true;
        for (int i=3; i>=0; i--) {
            if (digit[i] != 0 || i == 0) leadActive = false;
            if (!leadingzerosOn && leadActive) {
                this->segmentBits[i] = 0; // leading zero detected
            }
            else {
                this->segmentBits[i] = encodeDigit[digit[i]];
            }
            // check if decimal point is active
            if (this->dp & (1<<i)) {
                this->segmentBits[i] |= 8;
            }
        }
    };
//    Serial.printf("value: %3d, segBits: %3d %3d %3d %3d\n", this->value, this->segmentBits[3], this->segmentBits[2], this->segmentBits[1], this->segmentBits[0]);
};

void CLed4DigitDisplay::setSegmentBits(uint8_t segBits)
{
    for(int i=0; i<8; i++) {
        digitalWrite(segPins[i], ((segBits & (1u<<i)) ? LOW : HIGH));
    }
};

