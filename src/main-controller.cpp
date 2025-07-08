#include <Arduino.h>
#include <PID_v1.h>
#include <Adafruit_MAX31865.h>
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>

// Replace these with your actual hardware pins and constants
#include "const.h"

// SSR control pin
#define SSR_PIN SSR_ONE

// Phase durations (ms)
#define DURATION_PREHEAT 90000
#define DURATION_SOAK    90000
#define DURATION_REFLOW  30000
#define DURATION_DWELL   15000
#define DURATION_COOL    60000

// Temperature targets
#define T_ROOM   25.0
#define T_PREHEAT_END 150.0
#define T_SOAK_END    180.0
#define T_REFLOW_PEAK 245.0
#define T_COOL_END     50.0

// PID settings (tune as needed)
#define PID_SAMPLE_TIME 1000
#define KP 300
#define KI 0.1
#define KD 200

hd44780_I2Cexp lcd;
auto thermo = Adafruit_MAX31865(THERMO_CS, THERMO_MOSI, THERMO_MISO, THERMO_CLK);

double temperature, output, setPoint;
PID myPID(&temperature, &output, &setPoint, KP, KI, KD, DIRECT);

unsigned long processStart = 0;
unsigned long phaseStart = 0;
int phase = PHASE_IDLE;

volatile byte buttonReleased = false;

void startReflow() {
    processStart = millis();
    phaseStart = processStart;
    phase = PHASE_PREHEAT;
    lcd.clear();
    lcd.home();
    lcd.print("Reflow started");
    if (DEBUG) {
        Serial.println("Reflow process started.");
    }
}

void haltReflow() {
    phase = PHASE_IDLE;
}

void buttonCallback() {
    buttonReleased = true;
}

void setup() {
    Serial.begin(9600);
    thermo.begin(MAX31865_3WIRE);
    pinMode(SSR_PIN, OUTPUT);
    digitalWrite(SSR_PIN, LOW);

    myPID.SetOutputLimits(0, 2000); // window size
    myPID.SetSampleTime(PID_SAMPLE_TIME);
    myPID.SetMode(AUTOMATIC);

    lcd.begin(LCD_WIDTH, LCD_HEIGHT);
    pinMode(ENC_ROT_BTN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(ENC_ROT_BTN),
                    buttonCallback,
                    FALLING);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Loading...");
    Serial.println("Ready");
    delay(1000);
}

void loop() {
    if (buttonReleased) {
        buttonReleased = false;
        if (phase == PHASE_IDLE) {
            startReflow();
        } else {
            haltReflow();
        }
        delay(1000);
    }
    temperature = thermo.temperature(RNOMINAL, RREF);
    if (isnan(temperature)) {
        lcd.clear();
        lcd.print("Thermocouple error");
        if (DEBUG) {
            Serial.println("Thermocouple error");
        }
        return;
    }

    const unsigned long now = millis();
    const unsigned long elapsedPhase = now - phaseStart;

    // Determine setPoint based on current phase
    lcd.setCursor(0, 0);
    switch (phase) {
        case PHASE_PREHEAT: {
            // Preheat
            if (elapsedPhase >= DURATION_PREHEAT) {
                phase = PHASE_SOAK;
                phaseStart = now;
                break;
            }
            setPoint = T_ROOM + (T_PREHEAT_END - T_ROOM) * (
                           static_cast<double>(elapsedPhase) / static_cast<double>(DURATION_PREHEAT));
            lcd.print("Preheat       ");
            break;
        }
        case PHASE_SOAK: {
            // Soak
            if (elapsedPhase >= DURATION_SOAK) {
                phase = PHASE_REFLOW;
                phaseStart = now;
                break;
            }
            setPoint = T_PREHEAT_END + (T_SOAK_END - T_PREHEAT_END) * (
                           static_cast<double>(elapsedPhase) / static_cast<double>(DURATION_SOAK));
            lcd.print("Soak          ");
            break;
        }
        case PHASE_REFLOW: {
            // Reflow
            if (elapsedPhase >= DURATION_REFLOW) {
                phase = PHASE_DWELL;
                phaseStart = now;
                break;
            }
            setPoint = T_SOAK_END + (T_REFLOW_PEAK - T_SOAK_END) * (
                           static_cast<double>(elapsedPhase) / static_cast<double>(DURATION_REFLOW));
            lcd.print("Reflow        ");
            break;
        }
        case PHASE_DWELL: {
            // Dwell
            if (elapsedPhase >= DURATION_DWELL) {
                phase = PHASE_COOL;
                phaseStart = now;
                break;
            }
            setPoint = T_REFLOW_PEAK;
            lcd.print("Peak Dwell    ");
            break;
        }
        case PHASE_COOL: {
            // Cool
            if (elapsedPhase >= DURATION_COOL) {
                phase = PHASE_DONE;
                digitalWrite(SSR_PIN, LOW);
                lcd.print("Cool complete ");
                if (DEBUG) {
                    Serial.println("Cool complete.");
                }
                break;
            }
            setPoint = T_REFLOW_PEAK - (T_REFLOW_PEAK - T_COOL_END) * (
                           static_cast<double>(elapsedPhase) / static_cast<double>(DURATION_COOL));
            lcd.print("Cooling       ");
            break;
        }
        case PHASE_HALT: {
            if (elapsedPhase >= DURATION_COOL) {
                phase = PHASE_IDLE;
                digitalWrite(SSR_PIN, LOW);
                lcd.print("Halt complete ");
                if (DEBUG) {
                    Serial.println("Halt complete.");
                }
                delay(1000);
                break;
            }
            setPoint = T_REFLOW_PEAK - (T_REFLOW_PEAK - T_COOL_END) * (
                           static_cast<double>(elapsedPhase) / static_cast<double>(DURATION_COOL));
            lcd.print("Halting       ");
            break;
        }
        case PHASE_DONE: {
            lcd.setCursor(0, 0);
            lcd.print("Done       ");
        }
        case PHASE_IDLE:
        default: {
            lcd.clear();
            lcd.print("Reflowduino");
            digitalWrite(SSR_PIN, LOW);
            break;
        }
    }

    if (phase != PHASE_IDLE) {
        myPID.Compute();
        if (output > static_cast<double>((millis() - phaseStart) % 2000)) {
            digitalWrite(SSR_PIN, HIGH);
        } else {
            digitalWrite(SSR_PIN, LOW);
        }
    }


    // Display
    lcd.setCursor(0, 1);
    lcd.print("T:");
    lcd.print(temperature, 1);
    lcd.print("C S:");
    lcd.print(setPoint, 1);
    lcd.print(" ");


    Serial.print("Phase: ");
    Serial.print(phase);
    Serial.print(" | Temp: ");
    Serial.print(temperature);
    Serial.print(" | SetPoint: ");
    Serial.print(setPoint);
    Serial.print(" | Output: ");
    Serial.println(output);

    delay(100);
}
