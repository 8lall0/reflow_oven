#ifndef CONST_H
#define CONST_H

#ifndef DEBUG
#define DEBUG false
#endif

// Version
#define VERSION 0.99
// Phases
#define PHASE_IDLE 0
#define PHASE_PREHEAT 1
#define PHASE_SOAK 2
#define PHASE_REFLOW 3
#define PHASE_DWELL 4
#define PHASE_COOL 5
#define PHASE_DONE 6
#define PHASE_HALT 7
#define PHASE_BAKE 8

// Thermocouple
#define RREF 430.0
#define RNOMINAL 100.0

// Pins
#define THERMO_CS A0
#define THERMO_MOSI A1
#define THERMO_MISO A2
#define THERMO_CLK A3

// Screen
#define LCD_WIDTH 16
#define LCD_HEIGHT 2

// Heaters
#define SSR_ONE 13
#define SSR_TWO 12

// Rotary encoder
#define ENC_ROT_CLK 4
#define ENC_ROT_DT 3
#define ENC_ROT_BTN 2

// Menu


// SSR control pin
#define SSR_PIN SSR_ONE

// Phase durations (ms)
#define DURATION_PREHEAT 90000
#define DURATION_SOAK    90000
#define DURATION_REFLOW  30000
#define DURATION_DWELL   15000
#define DURATION_COOL    60000
#define DURATION_BAKE (5*60000)

// Temperature targets
#define T_ROOM   25.0
#define T_PREHEAT_END 150.0
#define T_SOAK_END    180.0
#define T_REFLOW_PEAK 245.0
#define T_COOL_END     50.0
#define T_BAKE  100.0

// PID settings (tune as needed)
#define PID_SAMPLE_TIME 1000
#define KP 300
#define KI 0.1
#define KD 200

#endif //CONST_H
