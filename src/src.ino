#include <Arduino.h>
#include <RotaryEncoder.h>
#include <DHT.h>

#define ROTARY1 2
#define ROTARY2 3

#define DHTPIN 4
#define DHTTYPE DHT11

RotaryEncoder rotary(ROTARY1, ROTARY2, RotaryEncoder::LatchMode::TWO03);

DHT dht(DHTPIN, DHTTYPE);

// Range from 0 to 100
static int brightness = 100;

static float temperature;

bool checkTemperature = true;

void setup()
{
  Serial.begin(115200);
  Serial.begin(9600);
  while (! Serial) ;

  dht.begin();
}

void loop()
{
  RotarySensor();
  if (checkTemperature)
  {
    TemperatureSensor();
  }
}

void RotarySensor() 
{
  static int pos = 0;
  rotary.tick();

  int newPos = rotary.getPosition();

  if (pos == newPos) 
  {
    return;
  }

  bool isAdd = (int)rotary.getDirection() == 1;

  if (isAdd && brightness < 100)
  {
    brightness++;
  }
  else if (!isAdd && brightness > 0) 
  {
    brightness--;
  }

  Serial.println(brightness);    
  pos = newPos;
}

void TemperatureSensor() 
{
  float tmp = dht.readTemperature();

  if (isnan(tmp))
  {
    return;
  }
  
  if (tmp == temperature) 
  {
    return;
  }

  temperature = tmp;

  Serial.print(temperature);
  Serial.print(F("°C"));

  Serial.println();
}
