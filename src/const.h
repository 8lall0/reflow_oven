#ifndef CONST_H
#define CONST_H

#ifndef DEBUG
#define DEBUG false
#endif

// Phases
#define PHASE_IDLE 0
#define PHASE_PREHEAT 1
#define PHASE_SOAK 2
#define PHASE_REFLOW 3
#define PHASE_DWELL 4
#define PHASE_COOL 5
#define PHASE_DONE 6
#define PHASE_HALT 7

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
#define ENC_ROT_A 4
#define ENC_ROT_B 3
#define ENC_ROT_BTN 2

#endif //CONST_H
