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

class DSPI
{
private: 
	/* MSP specific modules */
	uint32_t module;
	uint8_t modulePort;
	uint16_t modulePins;
	uint32_t intModule;
	
	/* Internal states */
	uint8_t (*user_onReceive)(uint8_t);
	
	eUSCI_SPI_MasterConfig MasterConfig;		//eUSCI_SPI_MasterConfig can be found in driver library, spi.h
	eUSCI_SPI_SlaveConfig SlaveConfig;			//eUSCI_SPI_SlaveConfig	 can be found in driver library, spi.h
	uint8_t mode;
	
	void _initMain( void ); 
	
	uint8_t _handleReceive(uint8_t);
	
public:
	DSPI();
	DSPI(uint8_t mod);
	~DSPI();
	
	void setMasterMode();
	void setSlaveMode();
	
	void begin();
	char transfer(char data);
	
	void onReceive(uint8_t(*islHandle)(uint8_t)); 

	/* Interrupt handlers: they are declared as friends to be accessible from outside 
	   but have access to class members */
	friend void EUSCIB0_IRQHandler( void );
	friend void EUSCIB1_IRQHandler( void );
	friend void EUSCIB2_IRQHandler( void );
	friend void EUSCIB3_IRQHandler( void );

protected:

};

#endif	/* DSPI_H_ */