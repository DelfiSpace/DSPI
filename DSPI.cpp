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
 
 #include <DSPI.h> 
 
 /* A reference list of DSPI instances */
DSPI * instances[4];		// pointer to the instantiated	DSPI classes

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
			MAP_SPI_transmitData( EUSCI_B## M ##_SPI_BASE, instances[M]->_handleTransmit() ); \
		} \
		 \
		/* Receive interrupt flag */ \
		if (status & UCRXIFG) \
		{ \
			instances[M]->_handleReceive( MAP_SPI_receiveData(EUSCI_B## M ##_SPI_BASE) ); \
		} \
	}

/**** ISR/IRQ Handles ****/
void EUSCIB0_IRQHandler( void ) 
{
	IRQHANDLER(0);
}

void EUSCIB1_IRQHandler( void ) 
{
	IRQHANDLER(1);
}

void EUSCIB2_IRQHandler( void ) 
{
	IRQHANDLER(2);
}

void EUSCIB3_IRQHandler( void ) 
{
	IRQHANDLER(3);
}

/**** CONSTRUCTORS Default ****/
DSPI::DSPI() 
{	//MSP432 launchpad used EUSCI_B0_SPI as default 
	this->module = EUSCI_B0_SPI_BASE;
	this->mode = MASTER;
	instances[0] = this;
}

/**** CONSTRUCTORS User Defined****/
DSPI::DSPI(uint8_t mod)
{
	 switch (mod) 
	{	   
		case 0:
			this->module = EUSCI_B0_SPI_BASE;
			instances[0] = this;
			break;

		case 1:
			this->module = EUSCI_B1_SPI_BASE;
			instances[1] = this;
			break;
			
		case 2:
			this->module = EUSCI_B2_SPI_BASE;
			instances[2] = this;
			break;
			
		case 3:
			this->module = EUSCI_B3_SPI_BASE;
			instances[3] = this;
			break;
	}
	this->mode = MASTER;
}

/**** DESTRUCTORS Reset the module ****/
DSPI::~DSPI() 
{
	MAP_SPI_disableModule(this->module);
	
	/* Deregister from the moduleMap */
	switch (module) 
	{
		case EUSCI_B0_SPI_BASE:
			instances[0] = 0;
			break;
			
		case EUSCI_B1_SPI_BASE:
			instances[1] = 0;
			break;
			
		case EUSCI_B2_SPI_BASE:
			instances[2] = 0;
			
			break;
		case EUSCI_B3_SPI_BASE:
			instances[3] = 0;
			break;
	}
}

/**** Set as Master User Defined****/
void DSPI::setMasterMode() 
{
	this->mode = MASTER;
}

/**** Set as Slave User Defined****/
void DSPI::setSlaveMode() 
{
	this->mode = SLAVE;
}


/**** Begin SPI as Master ****/
void DSPI::begin()
{	
	MAP_SPI_disableModule(this->module);	//disable SPI operation for configuration settings

	_initMain();	//SPI pins init
	
	if (mode == MASTER)
	{
		//Default Configuration
		MasterConfig.selectClockSource		= EUSCI_SPI_CLOCKSOURCE_SMCLK;								// SMCLK Clock Source, macro found in spi.h
		MasterConfig.clockSourceFrequency	= MAP_CS_getSMCLK();										// MAP_CS_getSMCLK() function found in rom_map.h
		MasterConfig.desiredSpiClock		= 1000000;													// 1MHz		
		MasterConfig.msbFirst				= EUSCI_B_SPI_MSB_FIRST;									// MSB first, macro found in spi.h
		MasterConfig.clockPhase				= EUSCI_B_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT;	// Phase, macro found in spi.h
		MasterConfig.clockPolarity			= EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_LOW;					// low polarity, macro found in spi.h
		MasterConfig.spiMode				= EUSCI_B_SPI_4PIN_UCxSTE_ACTIVE_HIGH;						 // 4Wire SPI Mode with active high, macro found in spi.h	
		
		MAP_SPI_initMaster(this->module, &MasterConfig);
		
		MAP_SPI_enableModule(this->module);		//enable SPI operation	
	}
	else
	{
		SlaveConfig.msbFirst				= EUSCI_B_SPI_MSB_FIRST;									// MSB first, macro found in spi.h
		SlaveConfig.clockPhase				= EUSCI_B_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT;	// Phase, macro found in spi.h
		SlaveConfig.clockPolarity			= EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_LOW;					// low polarity, macro found in spi.h
		SlaveConfig.spiMode					= EUSCI_B_SPI_3PIN;											// 4Wire SPI Mode with active high, macro found in spi.h	
		
		MAP_SPI_initSlave(this->module, &SlaveConfig);
		
		MAP_SPI_enableModule(this->module);		//enable SPI operation
		
		//Interrupt initialisation
		MAP_SPI_clearInterruptFlag(this->module, MAP_SPI_getEnabledInterruptStatus(this->module));
		MAP_SPI_enableInterrupt(this->module, EUSCI_B_SPI_RECEIVE_INTERRUPT | EUSCI_B_SPI_TRANSMIT_INTERRUPT);
		MAP_Interrupt_enableInterrupt( intModule ); 
		MAP_Interrupt_enableMaster( );
	}
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
}

/**** RX Interrupt Handler ****/
void DSPI::onReceive( void (*islHandle)(uint8_t) ) 
{
	user_onReceive = islHandle;
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
		
		intModule = INT_EUSCIB0;
		
		if (mode == SLAVE)
		{	//slave receive interrupt request handler
			MAP_SPI_registerInterrupt(this->module, EUSCIB0_IRQHandler);
		}
		
		break;
		
		case EUSCI_B1_SPI_BASE:
		
		modulePort = EUSCI_B1_PORT;
		modulePins = EUSCI_B1_PINS;
		
		intModule = INT_EUSCIB1;
		
		if (mode == SLAVE)
		{
			MAP_SPI_registerInterrupt(this->module, EUSCIB1_IRQHandler);
		}
		
		break;
		
		case EUSCI_B2_SPI_BASE:
		
		modulePort = EUSCI_B2_PORT;
		modulePins = EUSCI_B2_PINS;
		
		intModule = INT_EUSCIB2;
		
		if (mode == SLAVE)
		{
			MAP_SPI_registerInterrupt(this->module, EUSCIB2_IRQHandler);
		}
		
		break;
		
		case EUSCI_B3_SPI_BASE:
		
		modulePort = EUSCI_B3_PORT;
		modulePins = EUSCI_B3_PINS;
		
		intModule = INT_EUSCIB3;
		
		if (mode == SLAVE)
		{
			MAP_SPI_registerInterrupt(this->module, EUSCIB3_IRQHandler);
		}
		
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
		return user_onReceive(data);
	}
}