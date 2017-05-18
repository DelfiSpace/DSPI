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
	
	/* Internal states */
	uint8_t (*user_onTransmit)( void );
	void (*user_onReceive)( uint8_t );
	uint8_t mode;
	
	void _initMain( void ); 
	
	/* stub functions to handle interrupts */
	uint8_t _handleTransmit( void );
	void _handleReceive(uint8_t);
	
	/* Interrupt handlers: they are declared as friends to be accessible from outside 
	   the class (the interrupt engine) but have access to member functions */
	friend void EUSCIB0_IRQHandler( void );
	friend void EUSCIB1_IRQHandler( void );
	friend void EUSCIB2_IRQHandler( void );
	friend void EUSCIB3_IRQHandler( void );
	
public:
	DSPI();
	DSPI( uint8_t mod );
	~DSPI();
	
	void setMasterMode();
	void setSlaveMode();
	
	void begin();
	uint8_t transfer( uint8_t data );
	
	void onTransmit(uint8_t(*islHandle)( void )); 
	void onReceive(void(*islHandle)( uint8_t )); 

protected:

};

#endif	/* DSPI_H_ */