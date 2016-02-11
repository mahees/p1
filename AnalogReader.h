#ifndef AnalogReader_h
#define AnalogReader_h

#include "Arduino.h"

class AnalogReader
{
  public:
    //requires a vcc pin so that we dont have it on unnecessarily and damage the sensor due to electrolysis
    AnalogReader(int sensorPin, int vccPin);
    int takeReading(int readNAdditionalTimes, int msDelayBetweenReads);
  private:
    int _sensorPin;
    int _vccPin;
};

#endif

AnalogReader::AnalogReader(int sensorPin, int vccPin)
{
  
  _sensorPin = sensorPin;
  _vccPin = vccPin;

  pinMode(vccPin, OUTPUT);
  digitalWrite(vccPin, LOW);
}

int AnalogReader::takeReading(int readNAdditionalTimes, int msDelayBetweenReads)
{
  if (msDelayBetweenReads == 0) {
    msDelayBetweenReads = 100;
  }
  digitalWrite(_vccPin, HIGH);
  delay(200);

  int sensorValue = analogRead(_sensorPin);

  for (int i = 0; i < readNAdditionalTimes; i++) {
    delay(msDelayBetweenReads);
    sensorValue += analogRead(_sensorPin);
  }

  digitalWrite(_vccPin, LOW);

  return sensorValue / (1 + readNAdditionalTimes);
}

