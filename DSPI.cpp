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
 
 #include "DSPI.h"
 
 /* A reference list of DSPI DSPI_instancess */
DSPI * DSPI_instancess[4];		// pointer to the instantiated	DSPI classes

/**
 * The main (global) interrupt	handler
 * It ain't pretty, but using this as a macro should increase the performance tremendously
 */
#define IRQHANDLER(M) \
	{ \
		uint32_t status = MAP_SPI_getEnabledInterruptStatus( EUSCI_B## M ##_SPI_BASE ); \
		MAP_SPI_clearInterruptFlag( EUSCI_B## M ##_SPI_BASE, status ); \
		 \
		/* Transmit interrupt flag */ \
		if (status & UCTXIFG) \
		{ \
			MAP_SPI_transmitData( EUSCI_B## M ##_SPI_BASE, DSPI_instancess[M]->_handleTransmit() ); \
		} \
		 \
		/* Receive interrupt flag */ \
		if (status & UCRXIFG) \
		{ \
			DSPI_instancess[M]->_handleReceive( MAP_SPI_receiveData(EUSCI_B## M ##_SPI_BASE) ); \
		} \
	}

/**** ISR/IRQ Handles ****/
void EUSCIB0_IRQHandler_SPI( void )
{
	IRQHANDLER(0);
}

void EUSCIB1_IRQHandler_SPI( void )
{
	IRQHANDLER(1);
}

void EUSCIB2_IRQHandler_SPI( void )
{
	IRQHANDLER(2);
}

void EUSCIB3_IRQHandler_SPI( void )
{
	IRQHANDLER(3);
}

/**** CONSTRUCTORS Default ****/
DSPI::DSPI() 
{	//MSP432 launchpad used EUSCI_B0_SPI as default 
	this->module = EUSCI_B0_SPI_BASE;
	DSPI_instancess[0] = this;
}

/**** CONSTRUCTORS User Defined****/
DSPI::DSPI(uint8_t mod)
{
	 switch (mod) 
	{	   
		case 0:
			this->module = EUSCI_B0_SPI_BASE;
			DSPI_instancess[0] = this;
			break;

		case 1:
			this->module = EUSCI_B1_SPI_BASE;
			DSPI_instancess[1] = this;
			break;
			
		case 2:
			this->module = EUSCI_B2_SPI_BASE;
			DSPI_instancess[2] = this;
			break;
			
		case 3:
			this->module = EUSCI_B3_SPI_BASE;
			DSPI_instancess[3] = this;
			break;
	}
}

/**** DESTRUCTORS Reset the module ****/
DSPI::~DSPI() 
{
	MAP_SPI_disableModule(this->module);
	MAP_SPI_unregisterInterrupt(this->module);
	
	/* Deregister from the moduleMap */
	switch (module) 
	{
		case EUSCI_B0_SPI_BASE:
			DSPI_instancess[0] = 0;
			break;
			
		case EUSCI_B1_SPI_BASE:
			DSPI_instancess[1] = 0;
			break;
			
		case EUSCI_B2_SPI_BASE:
			DSPI_instancess[2] = 0;
			
			break;
		case EUSCI_B3_SPI_BASE:
			DSPI_instancess[3] = 0;
			break;
	}
}

void DSPI::initMaster( Mode mode, Order order, unsigned int speed )
{
    MAP_SPI_disableModule(this->module);    //disable SPI operation for configuration settings

    _initMain();    //SPI pins init

    eUSCI_SPI_MasterConfig config;

    // SPI Configuration
    config.selectClockSource    = EUSCI_SPI_CLOCKSOURCE_SMCLK;    // SMCLK Clock Source
    config.clockSourceFrequency = MAP_CS_getSMCLK();
    config.desiredSpiClock      = speed;
    config.msbFirst             = (order == MSBFirst) ? EUSCI_B_SPI_MSB_FIRST : EUSCI_B_SPI_LSB_FIRST;                                    // MSB first, macro found in spi.h
    config.spiMode                = EUSCI_B_SPI_3PIN;

    switch(mode)
    {
        case MODE0:
        default:
            config.clockPhase              = EUSCI_B_CTLW0_CKPH;
            config.clockPolarity           = 0;
            break;

        case MODE1:
            config.clockPhase              = 0;
            config.clockPolarity           = 0;
            break;

        case MODE2:
            config.clockPhase              = EUSCI_B_CTLW0_CKPH;
            config.clockPolarity           = EUSCI_B_CTLW0_CKPL;
            break;

        case MODE3:
            config.clockPhase              = 0;
            config.clockPolarity           = EUSCI_B_CTLW0_CKPL;
            break;
    }
    MAP_SPI_initMaster(this->module, &config);

    // enable SPI operation
    MAP_SPI_enableModule(this->module);
}

void DSPI::initSlave( Mode mode, Order order )
{
    MAP_SPI_disableModule(this->module);    //disable SPI operation for configuration settings

    _initMain();    //SPI pins init

    eUSCI_SPI_SlaveConfig config;

    // SPI Configuration
    config.msbFirst = (order == MSBFirst) ? EUSCI_B_SPI_MSB_FIRST : EUSCI_B_SPI_LSB_FIRST;
    config.spiMode  = EUSCI_B_SPI_3PIN;

    switch(mode)
    {
        case MODE0:
        default:
            config.clockPhase              = 0;
            config.clockPolarity           = 0;
            break;

        case MODE1:
            config.clockPhase              = EUSCI_B_CTLW0_CKPH;
            config.clockPolarity           = 0;
            break;

        case MODE2:
            config.clockPhase              = EUSCI_B_CTLW0_CKPH;
            config.clockPolarity           = EUSCI_B_CTLW0_CKPL;
            break;

        case MODE3:
            config.clockPhase              = 0;
            config.clockPolarity           = EUSCI_B_CTLW0_CKPL;
            break;
    }
    MAP_SPI_initSlave(this->module, &config);

    // enable SPI operation
    MAP_SPI_enableModule(this->module);
}

/**** Read and write 1 byte of data ****/
uint8_t DSPI::transfer(uint8_t data)
{	
	// transfer can only be used WITHOUT interrupts
	if ((user_onTransmit) || (user_onReceive))
	{
		return 0;
	}
	
	// ensure the transmitter is ready to transmit data
	while (!(MAP_SPI_getInterruptStatus(this->module, UCTXIFG)));
	MAP_SPI_transmitData(this->module, data);	
	
	// wait for a byte to be received	
	while (!(SPI_getInterruptStatus(this->module, UCRXIFG)));
	return MAP_SPI_receiveData(this->module);
}

/**** TX Interrupt Handler ****/
void DSPI::onTransmit( uint8_t (*islHandle)( void ) ) 
{
	user_onTransmit = islHandle;
	if ( islHandle )
	{
		// enable the transmit interrupt but do not clear the flag: this is done to ensure 
		// that the interrupt fires straight away so that the transmit buffer can be filled 
		// the first time
		MAP_SPI_enableInterrupt( this->module, EUSCI_B_SPI_TRANSMIT_INTERRUPT );
	}
	else
	{
		// disable transmit interrupt
		MAP_SPI_disableInterrupt( this->module, EUSCI_B_SPI_TRANSMIT_INTERRUPT) ;
	}
}

/**** RX Interrupt Handler ****/
void DSPI::onReceive( void (*islHandle)(uint8_t) ) 
{
	user_onReceive = islHandle;
	if ( islHandle )
	{
		// clear the receive interrupt to avoid spurious triggers the first time
		MAP_SPI_clearInterruptFlag( this->module, EUSCI_B_SPI_RECEIVE_INTERRUPT );
		// enable receive interrupt
		MAP_SPI_enableInterrupt( this->module, EUSCI_B_SPI_RECEIVE_INTERRUPT );
	}
	else
	{
		// disable receive interrupt
		MAP_SPI_disableInterrupt( this->module, EUSCI_B_SPI_RECEIVE_INTERRUPT );
	}	
}

/**** PRIVATE ****/
/**** Initialise SPI Pin Configuration based on EUSCI used ****/
void DSPI::_initMain( void )
{
	switch (module) 
	{	
		case EUSCI_B0_SPI_BASE:
		
		modulePort = EUSCI_B0_PORT;
		modulePins = EUSCI_B0_PINS;
		
		// transmit / receive interrupt request handler
		MAP_SPI_registerInterrupt(this->module, EUSCIB0_IRQHandler_SPI);
		
		break;
		
		case EUSCI_B1_SPI_BASE:
		
		modulePort = EUSCI_B1_PORT;
		modulePins = EUSCI_B1_PINS;
		
		// transmit / receive interrupt request handler
		MAP_SPI_registerInterrupt(this->module, EUSCIB1_IRQHandler_SPI);
		
		break;
		
		case EUSCI_B2_SPI_BASE:
		
		modulePort = EUSCI_B2_PORT;
		modulePins = EUSCI_B2_PINS;
		
		// transmit / receive interrupt request handler
		MAP_SPI_registerInterrupt(this->module, EUSCIB2_IRQHandler_SPI);
		
		break;
		
		case EUSCI_B3_SPI_BASE:
		
		modulePort = EUSCI_B3_PORT;
		modulePins = EUSCI_B3_PINS;
		
		// transmit / receive interrupt request handler
		MAP_SPI_registerInterrupt(this->module, EUSCIB3_IRQHandler_SPI);
		
		break;		
	}	
	MAP_GPIO_setAsPeripheralModuleFunctionInputPin(modulePort, modulePins, GPIO_PRIMARY_MODULE_FUNCTION);
}

/**
 * Internal process handling the tx, and calling the user's interrupt handles
 */
uint8_t DSPI::_handleTransmit( void ) 
{
	// do something only if there is a handler registered
	if (user_onTransmit)
	{
		// call the user-defined data transfer handler
		return user_onTransmit();
	}
	return 0;
}

/**
 * Internal process handling the rx, and calling the user's interrupt handles
 */
void DSPI::_handleReceive( uint8_t data ) 
{
	// do something only if there is a handler registered
	if (user_onReceive)
	{
		// call the user-defined data transfer handler
		user_onReceive(data);
	}
}
