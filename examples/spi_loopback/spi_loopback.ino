/* 
 *  Code written by Chia Jiun Wei ans Stefano Speretta
 *  This code shows how to use a blocking master SPI 
 *  connected to an interrupt-based slave SPI.
 */

 /*  PIN connection for a MSP432 LaunchPad board:
  *   
  *  P1.5 -> 1k resistor -> P3.5
  *  P1.6 -> 1k resistor -> P3.6
  *  P1.7 -> 1k resistor -> P3.7
  *  
  *  The 1k resistors are used to prevent pin damages during the development phases
  */

#include <DSPI.h>

#define BUFFER_LENGTH 8

DSPI master;      // used EUSCI_B0
DSPI slave(2);    // used EUSCI_B2

unsigned char masterTXbuffer[BUFFER_LENGTH];
unsigned char masterRXbuffer[BUFFER_LENGTH];
unsigned char  slaveRXbuffer[BUFFER_LENGTH];

uint8_t character = '0';
unsigned char slaveRXpointer = 0;
unsigned char slaveTXpointer = 0;

void setup()
{
  // initialize the UART
  Serial.begin(115200);

  delay(300);

  // Initialize SPI master
  master.begin();
  
  // Initialize SPI slave
  slave.setSlaveMode(); 
  slave.begin();
  slave.onReceive(receiveHandler);
  slave.onTransmit(transmitHandler);

  delay(300);
}

void send()
{
  Serial.print("MASTER TX: ");

  for (unsigned short i = 0; i < BUFFER_LENGTH; i++)
  {
    masterTXbuffer[i] = character;
    character++;
  }

  for (unsigned short i = 0; i < BUFFER_LENGTH; i++)
  {
    Serial.write(masterTXbuffer[i]);
    Serial.print(" ");
  }
  Serial.println();

  Serial.print("SLAVE  RX: ");
  for (unsigned short i = 0; i < BUFFER_LENGTH; i++)
  {
    masterRXbuffer[i] = master.transfer(masterTXbuffer[i]);
  }

  Serial.println();
  Serial.print("MASTER RX: ");
  for (unsigned short i = 0; i < BUFFER_LENGTH; i++)
  {
    Serial.write(masterRXbuffer[i]);
    Serial.print(" ");
  }
  Serial.println();
  Serial.println();
  
  // limit i within printable characters
  if (character > 'z')
  {
    character = '0';
  }
}

void loop()
{
  send();
  delay(500);
}

/* Receive Interrupt Handler */
void receiveHandler(uint8_t data)
{
  slaveRXbuffer[slaveRXpointer] = data;
  slaveRXpointer++;
  slaveRXpointer %= BUFFER_LENGTH;
  Serial.write(data);
  Serial.print(" ");
}

/* Transmit Interrupt Handler */
uint8_t transmitHandler( void )
{
  unsigned char t = slaveRXbuffer[slaveTXpointer];
  slaveTXpointer++;
  slaveTXpointer %= BUFFER_LENGTH;
  return t;
}

