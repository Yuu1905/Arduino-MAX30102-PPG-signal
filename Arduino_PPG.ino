#include <Wire.h>
#include "MAX30105.h"

MAX30105 sensor;

float baseline = 0;
float ppg = 0;

unsigned long lastBeat = 0;
int bpm = 0;

bool finger = false;
bool lastAbove = false;

void setup()
{
  Serial.begin(115200);
  Wire.begin();

  if (!sensor.begin(Wire, I2C_SPEED_FAST))
  {
    Serial.println("MAX30102 not found");
    while (1);
  }

  sensor.setup(
    25,     
    1,      
    2,
    100,    
    411,    
    4096    
  );

  sensor.setPulseAmplitudeRed(0x10);
  sensor.setPulseAmplitudeIR(0x20);
  sensor.setPulseAmplitudeGreen(0);

  delay(1000);
}

void loop()
{
  sensor.check();

  while (sensor.available())
  {
    long ir = sensor.getIR();
    sensor.nextSample();

    if (ir < 10000)
    {
      finger = false;
      bpm = 0;
      baseline = 0;
      ppg = 0;
      lastAbove = false;

      Serial.println("Please gently place your finger on the sensor");
      continue;
    }

    finger = true;

    if (baseline == 0)
      baseline = ir;

    baseline += 0.002 * (ir - baseline);

    float signal = ir - baseline;

    ppg += 0.35 * (signal - ppg);

    bool above = ppg > 200;

    if (above && !lastAbove)
    {
      unsigned long now = millis();

      if (lastBeat != 0)
      {
        unsigned long timeBetween = now - lastBeat;

        if (timeBetween > 300 && timeBetween < 2000)
        {
          bpm = 60000 / timeBetween;
        }
      }

      lastBeat = now;
    }

    lastAbove = above;

    Serial.print(ppg);
    Serial.print(",");
    Serial.println(bpm);
  }
}
