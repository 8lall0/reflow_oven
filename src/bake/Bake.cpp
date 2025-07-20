#define T_BAKE  100.0
#define DURATION_BAKE (5*60000)

#define RREF 430.0
#define RNOMINAL 100.0

#include "Bake.h"

Bake::Bake(const int pin, hd44780_I2Cexp *screen, Adafruit_MAX31865 *thermocouple,
           ClickEncoder *encoder): temperature(0) {
    ssrPIN = pin;
    lcd = screen;
    thermo = thermocouple;
    this->encoder = encoder;
}

void Bake::stop() {
    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->print("Stopping....");

    while (temperature > 50.0) {
        temperature = thermo->temperature(RNOMINAL, RREF);
        lcd->setCursor(0, 1);
        lcd->print("T:");
        lcd->print(temperature, 1);
        lcd->print("C");
    }
}

void Bake::Start() {
    const unsigned long phaseStart = millis();
    lcd->clear();

    while (true) {
        const uint8_t buttonState = encoder->getButton();
        if (buttonState == ClickEncoder::Clicked) {
            digitalWrite(ssrPIN, LOW);
            this->stop();
            break;
        }
        temperature = thermo->temperature(RNOMINAL, RREF);
        if (isnan(temperature)) {
            lcd->clear();
            lcd->print("Thermocouple error");
            return;
        }

        lcd->setCursor(0, 0);
        const unsigned long elapsedPhase = millis() - phaseStart;

        if (elapsedPhase >= DURATION_BAKE) {
            digitalWrite(ssrPIN, LOW);
            lcd->print("Bake complete");
            delay(1000);
            break;
        }

        if (temperature < T_BAKE) {
            digitalWrite(ssrPIN, HIGH);
        } else {
            digitalWrite(ssrPIN, LOW);
        }

        lcd->print("Baking....");

        lcd->setCursor(0, 1);
        lcd->print("T:");
        lcd->print(temperature, 1);
        lcd->print("C");
        delay(100);
    }
}
