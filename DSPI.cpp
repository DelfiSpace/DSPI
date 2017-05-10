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
DSPI * instances[4];		//pointer of DSPI class, to access functions under DSPI class, _handleReceive

/**** CONSTRUCTORS Default ****/
DSPI::DSPI() 
{	//MSP432 launchpad used EUSCI_B0_SPI as default 
    this->module = EUSCI_B0_SPI_BASE; //default settings, base address found in msp432p401r.h
	this->mode = MASTER;
	instances[0] = this;
}

/**** CONSTRUCTORS User Defined****/
DSPI::DSPI(uint8_t mod)
{
	 switch (mod) 
    {	//base address found in msp432p401r.h, only EUSCI_B was used       
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
    MAP_SPI_disableModule(this->module);	//MAP_SPI_disableModule() found in driver library, rom_map.h
	
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
void DSPI::begin()		//follow Dwire
{	
	uint32_t status;		//interrupt flag status

	MAP_SPI_disableModule(this->module);	//disable SPI operation for configuration settings, MAP_SPI_disableModule() found in driver library, rom_map.h

	_initMain();	//SPI pin init
	
	if (mode == MASTER)
	{
		//Default Configuration
		MasterConfig.selectClockSource 		= EUSCI_SPI_CLOCKSOURCE_SMCLK;								// SMCLK Clock Source, macro found in spi.h
		MasterConfig.clockSourceFrequency 	= MAP_CS_getSMCLK();										// MAP_CS_getSMCLK() function found in rom_map.h
		MasterConfig.desiredSpiClock 		= 1000000;													// 1MHz		
		MasterConfig.msbFirst 				= EUSCI_B_SPI_MSB_FIRST;									// MSB first, macro found in spi.h
		MasterConfig.clockPhase 			= EUSCI_B_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT;	// Phase, macro found in spi.h
		MasterConfig.clockPolarity			= EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_LOW;					// low polarity, macro found in spi.h
		MasterConfig.spiMode 				= EUSCI_B_SPI_4PIN_UCxSTE_ACTIVE_HIGH;                       // 4Wire SPI Mode with active high, macro found in spi.h	
		
		MAP_SPI_initMaster(this->module, &MasterConfig);	//function found in rom_map.h	
		
		MAP_SPI_enableModule(this->module);		//enable SPI operation, MAP_SPI_enableModule() found in driver library, rom_map.h	
	}
	else
	{
		SlaveConfig.msbFirst				= EUSCI_B_SPI_MSB_FIRST;									// MSB first, macro found in spi.h
		SlaveConfig.clockPhase				= EUSCI_B_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT;	// Phase, macro found in spi.h
		SlaveConfig.clockPolarity			= EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_LOW;					// low polarity, macro found in spi.h
		SlaveConfig.spiMode					= EUSCI_B_SPI_3PIN;                       // 4Wire SPI Mode with active high, macro found in spi.h	
		
		MAP_SPI_initSlave(this->module, &SlaveConfig);							//function found in rom_map.h
		
		MAP_SPI_enableModule(this->module);		//enable SPI operation, MAP_SPI_enableModule() found in driver library, rom_map.h	
		
		//Interrupt initialisation
		status = MAP_SPI_getEnabledInterruptStatus(this->module);
		
		MAP_SPI_clearInterruptFlag(this->module, status);
		MAP_SPI_enableInterrupt(this->module, EUSCI_B_SPI_RECEIVE_INTERRUPT);	//function found in rom_map.h, macro found in spi.h	
		MAP_Interrupt_enableInterrupt( intModule );	//intModule found in _initMain
		MAP_Interrupt_enableMaster( );		//function found in rom_map.h
	}
}


/**** Read and write 1 byte of data ****/
char DSPI::transfer(char data)
{	
	while (!(MAP_SPI_getInterruptStatus(this->module, UCRXIFG))); //function can be found in rom_map.h, macro found in msp432p401r.h

	MAP_SPI_transmitData(this->module, data);		//function found in rom_map.h
	
	if(mode == MASTER)
	{	
		return MAP_SPI_receiveData(this->module);		//function found in rom_map.h
	}
	else
	{
		return 0;	//Slave does not expect a return
	}
}

/**** Slave RX Interrupt Handler ****/
void DSPI::onReceive( void (*islHandle)(void) ) 
{
    user_onReceive = islHandle;		//user_onReceive is a function declare in DSPI.h, user parse the Interrupt handler here from main sketches
}

/**
 * Internal process handling the rx buffers, and calling the user's interrupt handles
 */
void DSPI::_handleReceive() 
{
    // No need to do anything if there is no handler registered
    if (!user_onReceive)
        return;

	// call the user-defined receive handler
    user_onReceive();
}


/**** PRIVATE ****/
/**** Initialise SPI Pin Configuration based on EUSCI used ****/
void DSPI::_initMain( void )
{
	switch (module) 
    {	
		case EUSCI_B0_SPI_BASE:			//base address found in msp432p401r.h
		
		modulePort = EUSCI_B0_PORT;		//EUSCI_#_PORT and EUSCI_#_PINS found in dspi_msp432p401r.h
		modulePins = EUSCI_B0_PINS;
		
		intModule = INT_EUSCIB0;	//macro found in interrupt.h
		
		if (mode == SLAVE)
		{	//slave receive interrupt request handler
			MAP_SPI_registerInterrupt(this->module, EUSCIB0_IRQHandler);	//function found in rom_map.h
		}
		
		break;
		
		case EUSCI_B1_SPI_BASE:
		
		modulePort = EUSCI_B1_PORT;
		modulePins = EUSCI_B1_PINS;
		
		intModule = INT_EUSCIB1;
		
		if (mode == SLAVE)
		{
			MAP_SPI_registerInterrupt(this->module, EUSCIB1_IRQHandler);	//function found in rom_map.h
		}
		
		break;
		
		case EUSCI_B2_SPI_BASE:
		
		modulePort = EUSCI_B2_PORT;
		modulePins = EUSCI_B2_PINS;
		
		intModule = INT_EUSCIB2;
		
		if (mode == SLAVE)
		{
			MAP_SPI_registerInterrupt(this->module, EUSCIB2_IRQHandler);	//function found in rom_map.h
		}
		
		break;
		
		case EUSCI_B3_SPI_BASE:
		
		modulePort = EUSCI_B3_PORT;
		modulePins = EUSCI_B3_PINS;
		
		intModule = INT_EUSCIB3;
		
		if (mode == SLAVE)
		{
			MAP_SPI_registerInterrupt(this->module, EUSCIB3_IRQHandler);	//function found in rom_map.h
		}
		
		break;		
	}	
	MAP_GPIO_setAsPeripheralModuleFunctionInputPin(modulePort, modulePins, GPIO_PRIMARY_MODULE_FUNCTION);	//function found in gpio.c, GPIO_PRIMARY_MODULE_FUNCTION found in gpio.h
}

/**** ISR/IRQ Handles ****/
void EUSCIB0_IRQHandler( void ) 
{
    uint32_t status;
    status = MAP_SPI_getEnabledInterruptStatus(EUSCI_B0_SPI_BASE);		//function found in rom_map.h, this->module was not used as this function does not belong to class DSPI
    MAP_SPI_clearInterruptFlag(EUSCI_B0_SPI_BASE, status);				//function found in rom_map.h
	
	instances[0]->_handleReceive();
}

void EUSCIB1_IRQHandler( void ) 
{
    uint32_t status;

    status = MAP_SPI_getEnabledInterruptStatus(EUSCI_B1_SPI_BASE);
    MAP_SPI_clearInterruptFlag(EUSCI_B1_SPI_BASE, status);

	instances[1]->_handleReceive();

}

void EUSCIB2_IRQHandler( void ) 
{
    uint32_t status;

    status = MAP_SPI_getEnabledInterruptStatus(EUSCI_B2_SPI_BASE);
    MAP_SPI_clearInterruptFlag(EUSCI_B2_SPI_BASE, status);

	instances[2]->_handleReceive();
}

void EUSCIB3_IRQHandler( void ) 
{
    uint32_t status;

    status = MAP_SPI_getEnabledInterruptStatus(EUSCI_B3_SPI_BASE);
    MAP_SPI_clearInterruptFlag(EUSCI_B3_SPI_BASE, status);

	instances[3]->_handleReceive();
}
