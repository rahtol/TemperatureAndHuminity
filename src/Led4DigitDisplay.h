#include <Arduino.h>

class CLed4DigitDisplay {

public:
    CLed4DigitDisplay(uint8_t *segPins,  uint8_t *digitPins);
    void init(); // call once  from setup()
    void loop();  // call from loop()
    void setValue(uint16_t value);
    void setLeadingzerosOnOff (bool on);
    void setDisplayOnOff(bool on);
    void setDP(uint8_t digit); // from 0 (right or DP4) to 3 (left or DP1)

private:
    uint8_t segPins[8];
    uint8_t digitPins[4];
    uint16_t value;
    bool leadingzerosOn;
    bool displayOn;
    uint8_t segmentBits[4];
    unsigned long t_period;  // refresh period
    unsigned long t_last_update; // time of last refresh
    uint8_t last_digit;  // the currently active digit
    uint8_t dp; // a bitlist of length four, bit set equal DP on on corresponding digit
    void encodeSegmentbits(void);
    void setSegmentBits(uint8_t segBits);
};
