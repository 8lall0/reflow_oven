#include <Arduino.h>
#include <Adafruit_MAX31865.h>
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include <ClickEncoder.h>
#include <TimerOne.h>

#include "reflow/Reflow.h"
#include "bake/Bake.h"

#include "const.h"


hd44780_I2Cexp lcd;
auto thermo = Adafruit_MAX31865(THERMO_CS, THERMO_MOSI, THERMO_MISO, THERMO_CLK);
double temperature, output, setPoint;
ClickEncoder encoder(ENC_ROT_CLK, ENC_ROT_DT, ENC_ROT_BTN, 4);

auto reflow = Reflow(SSR_PIN, &lcd, &thermo);
auto bake = Bake(SSR_PIN, &lcd, &thermo);

void readEncoder();

void printMenu();

void timerIsr();

int menuIndex = 0;

int16_t oldEncPos, encPos;
uint8_t buttonState;

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
    printMenu();
    readEncoder();
    buttonState = encoder.getButton();

    if (buttonState != 0 && buttonState == ClickEncoder::Clicked) {
        switch (menuIndex) {
            case 0: {
                reflow.Start();
                break;
            }

            case 1: {
                if (bake.IsRunning()) {
                    bake.Stop();
                } else {
                    bake.Start();
                }

                break;
            }
            case 2: {
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Reflowduino ");
                lcd.print(VERSION);
                lcd.setCursor(0, 1);
                lcd.print("By 8lall0");
                delay(5000);
                lcd.clear();
                break;
            }
            default:
                break;
        }
    }
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

void readEncoder() {
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
