#include <Arduino.h>
#include <Adafruit_MAX31865.h>
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include <ClickEncoder.h>
#include <TimerOne.h>

#include "reflow/Reflow.h"
#include "heat/Heater.h"

#include "const.h"

hd44780_I2Cexp lcd;
auto thermo = Adafruit_MAX31865(THERMO_CS, THERMO_MOSI, THERMO_MISO, THERMO_CLK);
auto encoder = ClickEncoder(ENC_ROT_CLK, ENC_ROT_DT, ENC_ROT_BTN, 4);

auto reflow = Reflow(SSR_PIN, &lcd, &thermo, &encoder);
auto heat = Heater(SSR_PIN, &lcd, &thermo, &encoder);

void readEncoder();
void timerIsr();

void printMenu();
void printInfo();

int menuIndex = 0;

int16_t oldEncPos, encPos;

constexpr int menuLength = 3;
String menuItems[menuLength] = {
    "Start reflow",
    "Heat",
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
    lcd.clear();
}

void loop() {
    printMenu();
    readEncoder();
    const uint8_t buttonState = encoder.getButton();

    if (buttonState == ClickEncoder::Clicked) {
        switch (menuIndex) {
            case 0: {
                reflow.Start();
                break;
            }
            case 1: {
                heat.Start();
                break;
            }
            case 2: {
                printInfo();
                break;
            }
            default:
                break;
        }
        lcd.clear();
    }
    delay(100);
}

void printInfo() {
    uint8_t buttonState = encoder.getButton();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Reflowduino ");
    lcd.print(VERSION);
    lcd.setCursor(0, 1);
    lcd.print("By 8lall0");

    while (buttonState != ClickEncoder::Clicked) {
        buttonState = encoder.getButton();
    }
}

void printMenu() {
    char buffer[16];
    lcd.setCursor(0, 0);
    lcd.print("Reflowduino ");
    lcd.print(VERSION);
    lcd.setCursor(0, 1);
    lcd.print("> ");
    sprintf(buffer, "%-14s", menuItems[menuIndex].c_str());
    lcd.print(buffer);
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
