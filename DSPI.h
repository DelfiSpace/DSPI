/*
 * Code written by Chia Jiun Wei @ 1 May 2017
 * <J.W.Chia@tudelft.nl>
 *
 * DSPI: a library to provide full hardware-driven SPI functionality
 * to the TI MSP432 family of microcontrollers. It is possible to use
 * this library in Energia (the Arduino port for MSP microcontrollers)
 * or in other toolchains.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License 
 * version 3, both as published by the Free Software Foundation.
 *
 */
 
#ifndef DSPI_H
#define DSPI_H

#include <driverlib.h>

/* Device specific includes */
#include <inc\dspi_msp432p401r.h>

class DSPI
{
private: 
	/* MSP specific modules */
    uint32_t module;
	uint8_t modulePort;
    uint16_t modulePins;
	
	/* Internal states */
	eUSCI_SPI_MasterConfig MasterConfig;		//eUSCI_SPI_MasterConfig can be found in driver library, spi.h
	
	void _initMain( void );
	
public:
	DSPI();
	DSPI(uint8_t mod);
	
	void begin();
	char transfer(char data);

protected:

};

#endif  /* DSPI_H_ */