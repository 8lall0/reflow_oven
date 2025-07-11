#include <Arduino.h>
#include <PID_v1.h>
#include <Adafruit_MAX31865.h>
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include <ClickEncoder.h>
#include <TimerOne.h>

#include "const.h"


hd44780_I2Cexp lcd;
auto thermo = Adafruit_MAX31865(THERMO_CS, THERMO_MOSI, THERMO_MISO, THERMO_CLK);
double temperature, output, setPoint;
PID myPID(&temperature, &output, &setPoint, KP, KI, KD, DIRECT);
ClickEncoder encoder(ENC_ROT_CLK, ENC_ROT_DT, ENC_ROT_BTN, 4);

unsigned long processStart = 0;
unsigned long phaseStart = 0;
int phase = PHASE_IDLE;

void startReflow();

void haltReflow();

void readEncoder();

void printMenu();

void timerIsr();

int menuIndex = 0;

int16_t oldEncPos, encPos;
uint8_t buttonState;

bool isAbout = false;
// Menu items
constexpr int menuLength = 3;
String menuItems[menuLength] = {
    "Start reflow",
    "Bake",
    "About",
};

void setup() {
    Serial.begin(9600);
    thermo.begin(MAX31865_3WIRE);
    pinMode(SSR_PIN, OUTPUT);
    digitalWrite(SSR_PIN, LOW);

    myPID.SetOutputLimits(0, 2000); // window size
    myPID.SetSampleTime(PID_SAMPLE_TIME);
    myPID.SetMode(AUTOMATIC);

    lcd.begin(LCD_WIDTH, LCD_HEIGHT);
    Timer1.initialize(1000);
    Timer1.attachInterrupt(timerIsr);
    oldEncPos = -1;
    encPos = -1;
    encoder.setAccelerationEnabled(false);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Loading...");
    Serial.println("Ready");
    delay(1000);
}

void loop() {
    readEncoder();
    buttonState = encoder.getButton();

    if (buttonState != 0) {
        switch (buttonState) {
            case ClickEncoder::Open: //0
                break;

            case ClickEncoder::Closed: //1
                break;

            case ClickEncoder::Pressed: //2
                break;

            case ClickEncoder::Held: //3
                break;

            case ClickEncoder::Released: //4
                break;

            case ClickEncoder::Clicked: //5
                switch (menuIndex) {
                    case 0: {
                        if (phase != PHASE_IDLE) {
                            haltReflow();
                        } else if (phase == PHASE_IDLE) {
                            startReflow();
                        }
                        break;
                    }

                    case 1: {
                        if (phase == PHASE_IDLE) {
                            phase = PHASE_BAKE;
                        } else {
                            phase = PHASE_IDLE;
                        }
                        break;
                    }
                    case 2: {
                        if (!isAbout) {
                            lcd.clear();
                            lcd.setCursor(0, 0);
                            lcd.print("Reflowduino v1.0");
                            lcd.setCursor(0, 1);
                            lcd.print("Written by 8lall0");
                            isAbout = true;
                        } else {
                            phase = PHASE_IDLE;
                            isAbout = false;
                        }

                        break;
                    }
                    default:
                        break;
                }
                break;

            case ClickEncoder::DoubleClicked: //6
                break;
        }
    }

    if (phase == PHASE_IDLE) {
        printMenu();
    }
    temperature = thermo.temperature(RNOMINAL, RREF);
    if (isnan(temperature)) {
        lcd.clear();
        lcd.print("Thermocouple error");
        Serial.println("Thermocouple error");
        return;
    }

    const unsigned long now = millis();
    const unsigned long elapsedPhase = now - phaseStart;

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
        case PHASE_BAKE: {
            if (elapsedPhase >= DURATION_BAKE) {
                phase = PHASE_IDLE;
                digitalWrite(SSR_PIN, LOW);
                lcd.print("Bake complete ");
                if (DEBUG) {
                    Serial.println("Bake complete.");
                }
                delay(1000);
                break;
            }
            lcd.print("Baking       ");
            setPoint = T_BAKE;
            break;
        }
        case PHASE_DONE: {
            lcd.setCursor(0, 0);
            lcd.print("Done       ");
            delay(3000);
            phase = PHASE_IDLE;
        }
        case PHASE_IDLE:
        default: {
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

    if (phase != PHASE_IDLE) {
        // Display
        lcd.setCursor(0, 1);
        lcd.print("T:");
        lcd.print(temperature, 1);
        lcd.print("C S:");
        lcd.print(setPoint, 1);
        lcd.print(" ");
    }

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

void printMenu() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Reflowduino ");
    lcd.print(VERSION);
    lcd.setCursor(0, 1);
    lcd.print("> ");
    lcd.print(menuItems[menuIndex]);
}

void startReflow() {
    lcd.clear();
    lcd.home();
    lcd.print("Reflow started");
    Serial.println("Reflow process started.");
    delay(2000);
    processStart = millis();
    phaseStart = processStart;
    phase = PHASE_PREHEAT;
}

void haltReflow() {
    phase = PHASE_IDLE;
}

void readEncoder() {
    if (phase != PHASE_IDLE) {
        return;
    }
    encPos += encoder.getValue();

    if (encPos > oldEncPos) {
        menuIndex++;
    } else if (encPos < oldEncPos) {
        menuIndex--;
    }
    oldEncPos = encPos;
    menuIndex = constrain(menuIndex, 0, menuLength - 1);
}

void timerIsr() {
    encoder.service();
}
