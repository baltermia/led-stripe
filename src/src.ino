#include <Arduino.h>
#include <RotaryEncoder.h>

#define PIN_IN1 2
#define PIN_IN2 3

RotaryEncoder rotary(PIN_IN1, PIN_IN2, RotaryEncoder::LatchMode::TWO03);

// Range from 0 to 100
static int brightness = 100;

void setup()
{
  Serial.begin(115200);
  while (! Serial) ;
}

void loop()
{
  static int pos = 0;
  rotary.tick();

  int newPos = rotary.getPosition();
  if (pos != newPos) 
  {
    RotarySensor((int)rotary.getDirection());
    pos = newPos;
  }
}

void RotarySensor(int direction) 
{
  bool isAdd = direction == 1;
  
  if (isAdd && brightness < 100)
  {
    brightness++;
  }
  else if (!isAdd && brightness > 0) 
  {
    brightness--;
  }

  Serial.println(brightness);
}
