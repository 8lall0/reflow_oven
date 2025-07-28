#include "Heater.h"

#define RREF 430.0
#define RNOMINAL 100.0

Heater::Heater(const int pin, hd44780_I2Cexp *screen, Adafruit_MAX31865 *thermocouple,
               ClickEncoder *encoder): temperature(0), oldEncPos(-1), encPos(-1), finalTime(0), finalTemperature(30) {
    ssrPIN = pin;
    lcd = screen;
    thermo = thermocouple;
    this->encoder = encoder;
}

void Heater::Start() {
    while (true) {
        delay(100);
        lcd->setCursor(0, 0);
        lcd->print("Heating mode");
        lcd->setCursor(0, 1);
        lcd->print("> ");
        lcd->print(menuItems[menuIndex]);
        menuEncoder();

        const uint8_t buttonState = encoder->getButton();
        if (buttonState == ClickEncoder::Clicked) {
            switch (menuIndex) {
                case 0: {
                    setTemperature();
                    break;
                }
                case 1: {
                    setTime();
                    break;
                }
                case 2: {
                    startHeat();
                    break;
                }
                case 3: {
                    stop();
                    return;
                }
                default:
                    stop();
                    return;
            }
        }
    }
}

void Heater::setTemperature() {
    lcd->clear();
    while (true) {
        lcd->setCursor(0, 0);
        lcd->print("Set temp (C):");
        lcd->setCursor(0, 1);
        lcd->print("> ");
        lcd->print(finalTemperature);
        const uint8_t buttonState = encoder->getButton();
        if (buttonState == ClickEncoder::Clicked) {
            return;
        }
        temperatureEncoder();
        delay(100);
        lcd->clear();
    }
}

void Heater::setTime() {
    lcd->clear();
    while (true) {
        const int hour = static_cast<int>(finalTime / 60);
        const int minute = static_cast<int>(finalTime % 60);
        lcd->setCursor(0, 0);
        lcd->print("Set time (min):");
        lcd->setCursor(0, 1);
        lcd->print("> ");
        lcd->print(hour);
        lcd->print(":");
        lcd->print(minute);
        lcd->print(" ");

        const uint8_t buttonState = encoder->getButton();
        if (buttonState == ClickEncoder::Clicked) {
            return;
        }
        timeEncoder();
        delay(100);
        lcd->clear();
    }
}

void Heater::menuEncoder() {
    encPos += encoder->getValue();

    if (encPos > oldEncPos) {
        menuIndex++;
    } else if (encPos < oldEncPos) {
        menuIndex--;
    }
    oldEncPos = encPos;
    menuIndex = constrain(menuIndex, 0, menuLength - 1);
}

void Heater::timeEncoder() {
    encPos += encoder->getValue();

    if (encPos > oldEncPos) {
        finalTime += 10;
    } else if (encPos < oldEncPos) {
        finalTime -= 10;
    }
    oldEncPos = encPos;
    if (finalTime < 0) {
        finalTime = 0;
    }
}

void Heater::temperatureEncoder() {
    encPos += encoder->getValue();

    if (encPos > oldEncPos) {
        finalTemperature += 10;
    } else if (encPos < oldEncPos) {
        finalTemperature -= 10;
    }
    oldEncPos = encPos;

    finalTemperature = constrain(finalTemperature, 0, 300);
}

void Heater::getTemperature() {
    temperature = thermo->temperature(RNOMINAL, RREF);
}

void Heater::startHeat() {
    const unsigned long phaseStart = millis();
    lcd->clear();
    lcd->setCursor(0, 0);

    while (true) {
        const uint8_t buttonState = encoder->getButton();
        if (buttonState == ClickEncoder::Clicked) {
            digitalWrite(ssrPIN, LOW);
            this->stop();
            break;
        }
        getTemperature();
        if (isnan(temperature)) {
            lcd->clear();
            lcd->print("Thermocouple error");
            return;
        }

        const unsigned long elapsedPhase = millis() - phaseStart;

        if (elapsedPhase >= finalTime * 60 * 1000) {
            digitalWrite(ssrPIN, LOW);
            break;
        }

        if (temperature < finalTemperature) {
            digitalWrite(ssrPIN, HIGH);
        } else {
            digitalWrite(ssrPIN, LOW);
        }

        lcd->setCursor(0, 0);
        lcd->print("C: ");
        lcd->print(temperature, 1);
        lcd->print(" | ");
        lcd->print(finalTemperature, 1);


        const unsigned long elapsedMinute = elapsedPhase / (60000);
        const unsigned long elapsedHour = elapsedMinute / 60;
        const unsigned long toHour = finalTime / 60;
        const unsigned long toMinute = finalTime % 60;
        lcd->setCursor(0, 1);
        lcd->print("T: ");
        lcd->print(elapsedHour, 2);
        lcd->print(":");
        lcd->print(elapsedMinute, 2);
        lcd->print(" | ");
        lcd->print(toHour, 2);
        lcd->print(":");
        lcd->print(toMinute, 2);
        delay(100);
    }
}

void Heater::stop() {
    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->print("Stopping....");

    while (temperature > 50.0) {
        getTemperature();
        lcd->setCursor(0, 1);
        lcd->print("T:");
        lcd->print(temperature, 1);
        lcd->print("C");
    }
}
