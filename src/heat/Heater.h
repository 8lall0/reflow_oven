#ifndef HEAT_H
#define HEAT_H

#include <PID_v1.h>
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include <Adafruit_MAX31865.h>
#include <ClickEncoder.h>

class Heater {
public:
    explicit Heater(int pin, hd44780_I2Cexp *screen, Adafruit_MAX31865 *thermocouple, ClickEncoder *encoder);

    void Start();
private:
    Adafruit_MAX31865 *thermo;
    hd44780_I2Cexp *lcd;
    double temperature;
    int ssrPIN;
    ClickEncoder *encoder;
    PID heatPID;

    double output, setPoint;
    const int menuLength = 4;
    String menuItems[4] = {
        "Set Temp",
        "Set Time",
        "Start   ",
        "Exit    "
    };
    int menuIndex = 0;
    int16_t oldEncPos, encPos;

    unsigned long finalTime;
    double finalTemperature;

    void stop();
    void startHeat();
    void getTemperature();
    void menuEncoder();
    void setTemperature();
    void setTime();
    void timeEncoder();
    void temperatureEncoder();
};

#endif //HEAT_H
