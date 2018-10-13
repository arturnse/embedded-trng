# TRNG for an Embedded Platform

This is an Embedded TRNG for the **Tiva C Series TM4C123G Launchpad** development kit.

It is written for the Stellaris EK-TM4C123GXL ARM Cortex-M4F MCU.

It generates Random data using the crystals available in this platform:
* The internal crystal oscillator (MOSC), 16Mhz
* The external crystal oscillator (XTAL), 80Mhz (16Mhz * PLL)

The generator is higly customizable, and got excellent results when exposed to the [Dieharder](http://webhome.phy.duke.edu/~rgb/General/dieharder.php) battery of tests.

## Dependencies

This project was created in the [Code Compser Studio IDE](http://www.ti.com/tool/ccstudio). It can be used for other platforms and MCUs, but keep in mind it uses at least 2 oscillators for data generation.

This project uses generic data structures and definitions available in the [Embedded C API](https://github.com/arturnse/embedded-api).



