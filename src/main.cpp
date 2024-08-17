#include <Arduino.h>
#include "Led4DigitDisplay.h"
#include "DHT.h"
#include "myDHT11.h"

#define DHTPIN 15     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11

unsigned long t_last;
uint16_t curval;
uint8_t segPins[] = {32, 33, 25, 26, 27, 13, 12, 14}; // segment pin order: E D C DP G B F A
uint8_t digitPins[] = {4, 5, 18, 19}; // digit pin order: 10^0 10^1 10^2 10^3
CLed4DigitDisplay display(segPins, digitPins);
CMyDHT11 dht(DHTPIN, DHTTYPE);


void setup() 
{
  Serial.begin(115200);
  Serial.printf("TemperatureAndHuminity, v1.01, 12.08.2024 16:12\n");

  curval = 0;
  display.init();
  display.setLeadingzerosOnOff (false);
  display.setDisplayOnOff(true);
  display.setValue(curval);

  dht.begin();

  t_last = millis();
}

static int count = 0;

void loop() 
{
  unsigned long t_cur = millis();
  dht.loop();

  if (t_cur - t_last > 1000) {
    t_last = t_cur;

    float h = 53.9; //dht.readHumidity();
    float t = dht.readTemperature();

    curval = (curval +1) % 10000;
    display.setDP(1);
    display.setValue((uint16_t)(t*10));
  }

































  display.loop();
};
