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
 
/**** CONSTRUCTORS Default ****/
DSPI::DSPI() 
{	//MSP432 launchpad used EUSCI_B0_SPI as default 
    this->module = EUSCI_B0_SPI_BASE; //default settings, base address found in msp432p401r.h
}

/**** CONSTRUCTORS User Defined****/
DSPI::DSPI(uint8_t mod)
{
	 switch (mod) 
    {	//base address found in msp432p401r.h, only EUSCI_B was used       
		case 0:
        	this->module = EUSCI_B0_SPI_BASE;
            break;

        case 1:
            this->module = EUSCI_B1_SPI_BASE;
            break;
            
        case 2:
            this->module = EUSCI_B2_SPI_BASE;
            break;
            
        case 3:
            this->module = EUSCI_B3_SPI_BASE;
            break;
    }
}

/**** Begin SPI as Master ****/
void DSPI::begin()
{	
	SPI_disableModule(this->module);	//disable SPI operation for configuration settings, SPI_disableModule() found in driver library, spi.c
	
	//Default Configuration
	MasterConfig.selectClockSource 		= EUSCI_SPI_CLOCKSOURCE_SMCLK;								// SMCLK Clock Source, macro found in spi.h
	MasterConfig.clockSourceFrequency 	= MAP_CS_getSMCLK();										// MAP_CS_getSMCLK() function found in rom_map.h
	MasterConfig.desiredSpiClock 		= 1000000;													// 1MHz		
	MasterConfig.msbFirst 				= EUSCI_B_SPI_MSB_FIRST;									// MSB first, macro found in spi.h
	MasterConfig.clockPhase 			= EUSCI_B_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT;	// Phase, macro found in spi.h
	MasterConfig.clockPolarity			= EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_LOW;					// low polarity, macro found in spi.h
	MasterConfig.spiMode 				= EUSCI_B_SPI_4PIN_UCxSTE_ACTIVE_HIGH;                       // 4Wire SPI Mode with active high, macro found in spi.h	
	
	_initMain();	//SPI pin init
	
	SPI_initMaster(this->module, &MasterConfig);	//function found in spi.c
	
	SPI_enableModule(this->module);		//enable SPI operation, SPI_enableModule() found in driver library, spi.c	
}

/**** Read and write 1 byte of data ****/
char DSPI::transfer(char data)
{		
	SPI_transmitData(this->module, data);		//function found in spi.c
	
	while (!(SPI_getInterruptStatus(this->module,UCRXIE)));		//function can be found in spi.c
	return SPI_receiveData(this->module);		//function found in spi.c
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
		break;
		
		case EUSCI_B1_SPI_BASE:
		
		modulePort = EUSCI_B1_PORT;
		modulePins = EUSCI_B1_PINS;
		break;
		
		case EUSCI_B2_SPI_BASE:
		
		modulePort = EUSCI_B2_PORT;
		modulePins = EUSCI_B2_PINS;
		break;
		
		case EUSCI_B3_SPI_BASE:
		
		modulePort = EUSCI_B3_PORT;
		modulePins = EUSCI_B3_PINS;
		break;		
	}	
	GPIO_setAsPeripheralModuleFunctionInputPin(modulePort, modulePins, GPIO_PRIMARY_MODULE_FUNCTION);	//function found in gpio.c, GPIO_PRIMARY_MODULE_FUNCTION found in gpio.h
}
