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

#define MASTER 0
#define SLAVE 1

#include <driverlib.h>

/* Device specific includes */
#include <inc/dspi_msp432p401r.h>

void EUSCIB0_IRQHandler( void );
void EUSCIB1_IRQHandler( void );
void EUSCIB2_IRQHandler( void );
void EUSCIB3_IRQHandler( void );

class DSPI
{
private: 
	/* MSP specific modules */
    uint32_t module;
	uint8_t modulePort;
    uint16_t modulePins;
	uint32_t intModule;
	
	/* Internal states */
	void (*user_onReceive)();
	
	eUSCI_SPI_MasterConfig MasterConfig;		//eUSCI_SPI_MasterConfig can be found in driver library, spi.h
	eUSCI_SPI_SlaveConfig SlaveConfig;			//eUSCI_SPI_SlaveConfig  can be found in driver library, spi.h
	uint8_t mode;
	
	void _initMain( void );	

	
public:
	DSPI();
	DSPI(uint8_t mod);
	~DSPI();
	
	void setMasterMode();
	void setSlaveMode();
	
	void begin();
	char transfer(char data);
	
	void onReceive(void(*islHandle)(void)); 
	void _handleReceive(); 				//public for EUSCIB#_IRQHandler function to access

protected:

};

#endif  /* DSPI_H_ */