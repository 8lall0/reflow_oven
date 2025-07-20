# Reflowduino
A simple reflow oven made with cheap components and a simple arduino.

## Features
* LCD menu controllable via rotary encoder (both rotation and pressing button)
* Reflow profile set in code and controlled via PID
* Bake profile to remove moisture from your PCB (maybe also usable to dry your PLA filaments...)

## Requirements
### Software
This project uses [platformio](https://platformio.org) to facilitate library handling, so make sure you have a working installation of it.

This project is made with 220V AC current in mind, but it should work fine with 110V AC.

### Hardware
* A cheap-arse small electric oven
* Arduino Uno
* Whatever to generate 5v from AC
* hd44780 LCD with i2c interface
* Rotary encoder
* Solid State Relay (SSR)
* MAX31865 temperature sensor board with compatible thermocouple
* Insulating wool fabric
* Insulating reflecting paper
* Some aluminium sheets (**optional**)
* Heating elements, (**optional**, I made this choice in order to replace them easly, but if your oven already has quartz heating elements you can use them directly)

## Author
Savino Pio Liguori aka 8lall0

## License
This project is released under the MIT License.

## TODO
* Add hardware schematic and photos
* Add Aliexpress links for the hardware