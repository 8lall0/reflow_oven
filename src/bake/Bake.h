#ifndef BAKE_H
#define BAKE_H

#include <PID_v1.h>
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include <Adafruit_MAX31865.h>
#include <ClickEncoder.h>

class Bake {
public:
    explicit Bake(int pin, hd44780_I2Cexp *screen, Adafruit_MAX31865 *thermocouple, ClickEncoder *encoder);

    void Start();

private:
    Adafruit_MAX31865 *thermo;
    hd44780_I2Cexp *lcd;
    double temperature;
    int ssrPIN;
    ClickEncoder *encoder;

    void stop();
};


#endif //BAKE_H
