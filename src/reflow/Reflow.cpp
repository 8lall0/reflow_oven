#include <Arduino.h>
#include "Reflow.h"

#define PHASE_IDLE 0
#define PHASE_PREHEAT 1
#define PHASE_SOAK 2
#define PHASE_REFLOW 3
#define PHASE_DWELL 4
#define PHASE_COOL 5
#define PHASE_DONE 6
#define PHASE_HALT 7
#define PHASE_BAKE 8

#define DURATION_PREHEAT 90000
#define DURATION_SOAK    90000
#define DURATION_REFLOW  30000
#define DURATION_DWELL   15000
#define DURATION_COOL    60000

#define PID_SAMPLE_TIME 1000
#define KP 300
#define KI 0.1
#define KD 200

// Temperature targets
#define T_ROOM   25.0
#define T_PREHEAT_END 150.0
#define T_SOAK_END    180.0
#define T_REFLOW_PEAK 245.0
#define T_COOL_END     50.0

#define RREF 430.0
#define RNOMINAL 100.0

Reflow::Reflow(const int pin, hd44780_I2Cexp *screen, Adafruit_MAX31865 *thermocouple, ClickEncoder *encoder): temperature(0), output(0),
    setPoint(0), reflowPID(PID(&temperature, &output, &setPoint, KP, KI, KD, DIRECT)) {
    reflowPID.SetOutputLimits(0, 2000); // window size
    reflowPID.SetSampleTime(PID_SAMPLE_TIME);
    reflowPID.SetMode(AUTOMATIC);
    ssrPIN = pin;
    lcd = screen;
    thermo = thermocouple;
    phase = PHASE_IDLE;
    this->encoder = encoder;
}

void Reflow::stop() {
    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->print("Stopping....");

    while (temperature > 50.0) {
        temperature = thermo->temperature(RNOMINAL, RREF);
        lcd->setCursor(0, 1);
        lcd->print("T:");
        lcd->print(temperature, 1);
        lcd->print("C S:");
        lcd->print(setPoint, 1);
        lcd->print(" ");
    }
}

void Reflow::Start() {
    unsigned long phaseStart = 0;

    phase = PHASE_PREHEAT;

    while (true) {
        const uint8_t buttonState = encoder->getButton();
        if (buttonState == ClickEncoder::Clicked || phase == PHASE_IDLE) {
            digitalWrite(ssrPIN, LOW);
            phase = PHASE_IDLE;
            this->stop();
            break;
        }

        const unsigned long now = millis();
        const unsigned long elapsedPhase = now - phaseStart;
        temperature = thermo->temperature(RNOMINAL, RREF);
        if (isnan(temperature)) {
            lcd->clear();
            lcd->print("Thermocouple error");
            return;
        }

        lcd->setCursor(0, 0);
        switch (phase) {
            case PHASE_PREHEAT: {
                // Preheat
                if (elapsedPhase >= DURATION_PREHEAT) {
                    phase = PHASE_SOAK;
                    phaseStart = now;
                    break;
                }
                lcd->print("Preheat...");
                setPoint = T_ROOM + (T_PREHEAT_END - T_ROOM) * (
                               static_cast<double>(elapsedPhase) / static_cast<double>(DURATION_PREHEAT));
                break;
            }
            case PHASE_SOAK: {
                // Soak
                if (elapsedPhase >= DURATION_SOAK) {
                    phase = PHASE_REFLOW;
                    phaseStart = now;
                    break;
                }
                lcd->print("Soak...");
                setPoint = T_PREHEAT_END + (T_SOAK_END - T_PREHEAT_END) * (
                               static_cast<double>(elapsedPhase) / static_cast<double>(DURATION_SOAK));
                break;
            }
            case PHASE_REFLOW: {
                // Reflow
                if (elapsedPhase >= DURATION_REFLOW) {
                    phase = PHASE_DWELL;
                    phaseStart = now;
                    break;
                }
                lcd->print("Reflow...");
                setPoint = T_SOAK_END + (T_REFLOW_PEAK - T_SOAK_END) * (
                               static_cast<double>(elapsedPhase) / static_cast<double>(DURATION_REFLOW));
                break;
            }
            case PHASE_DWELL: {
                // Dwell
                if (elapsedPhase >= DURATION_DWELL) {
                    phase = PHASE_COOL;
                    phaseStart = now;
                    break;
                }
                lcd->print("Peak Dwell...");
                setPoint = T_REFLOW_PEAK;
                break;
            }
            case PHASE_COOL: {
                // Cool
                if (elapsedPhase >= DURATION_COOL) {
                    phase = PHASE_DONE;
                    digitalWrite(ssrPIN, LOW);
                    break;
                }
                lcd->print("Cooling...");
                setPoint = T_REFLOW_PEAK - (T_REFLOW_PEAK - T_COOL_END) * (
                               static_cast<double>(elapsedPhase) / static_cast<double>(DURATION_COOL));
                break;
            }
            case PHASE_DONE: {
                lcd->print("Done...");
                digitalWrite(ssrPIN, LOW);
                phase = PHASE_IDLE;
                delay(3000);
                break;
            }
            default: {
                phase = PHASE_IDLE;
                break;
            }
        }
        reflowPID.Compute();
        if (output > static_cast<double>((millis() - phaseStart) % 2000)) {
            digitalWrite(ssrPIN, HIGH);
        } else {
            digitalWrite(ssrPIN, LOW);
        }

        lcd->setCursor(0, 1);
        lcd->print("T:");
        lcd->print(temperature, 1);
        lcd->print("C S:");
        lcd->print(setPoint, 1);
        lcd->print(" ");
        delay(100);
    }
}
