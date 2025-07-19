//
// Created by jimmy on 19/07/2025.
//
#define PID_SAMPLE_TIME 1000
#define KP 300
#define KI 0.1
#define KD 200

#define T_BAKE  100.0
#define DURATION_BAKE (5*60000)

#include "Bake.h"

Bake::Bake(const int pin, hd44780_I2Cexp *screen, Adafruit_MAX31865 *thermocouple): temperature(0), output(0),
    setPoint(0),
    bakePID(&temperature, &output, &setPoint, KP, KI, KD, DIRECT) {
    ssrPIN = pin;
    lcd = screen;
    thermo = thermocouple;
}

void Bake::Stop() {
    isRunning = false;
}

bool Bake::IsRunning() const {
    return isRunning;
}


void Bake::Start() {
    isRunning = true;
    const unsigned long phaseStart = millis();

    while (isRunning) {
        lcd->setCursor(0, 0);
        const unsigned long elapsedPhase = millis() - phaseStart;

        if (elapsedPhase >= DURATION_BAKE) {
            digitalWrite(ssrPIN, LOW);
            lcd->print("Bake complete");
            delay(1000);
            isRunning = false;
            break;
        }
        setPoint = T_BAKE;

        lcd->print("Baking....");

        lcd->setCursor(0, 1);
        lcd->print("T:");
        lcd->print(temperature, 1);
        lcd->print("C S:");
        lcd->print(setPoint, 1);
        lcd->print(" ");

        bakePID.Compute();
        if (output > static_cast<double>((millis() - phaseStart) % 2000)) {
            digitalWrite(ssrPIN, HIGH);
        } else {
            digitalWrite(ssrPIN, LOW);
        }
    }
}
