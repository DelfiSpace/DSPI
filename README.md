# DSPI (Dynamic SPI)
A simple library for handling SPI on the MSP432, trying to be similar to Energia's SPI library (originally from Arduino) but providing more advanced features.

## Introduction and Features

Although powerful, the Wire library included in [Energia](http://energia.nu/) (an Arduino port for TI microcontrollers) has been found to have certain limitations in its use. Delft Wire, or DWire for short (named after the Delft University of Technology), provides Energia developers looking for more advanced functionality when using the I2C bus standard in their projects.

Major features include:
- Full hardware-driven SPI (via the eUSCI modules)
- It is possible to select other eUSCI modules. 
- Ability to use multiple eUSCI modules at the same time.
- Full slave support: it is possible to run the microcontroller as a slave.
- Nearly identical interface as SPI's interface.

## Installation

The library can directly be used in Energia. Simply clone the repository or download the zip file, placing the root directory of the repository in your Energia user folder's 'libraries' folder. E.g. in Windows, this is typically found in **C:\Documents\Energia\libraries**. This library uses `driverlib`, which should come with the standard Energia installation. Nevertheless, make sure this library is accessible to the compiler.

DSPI should be able to compile with all generic toolchains for the MSP432.
