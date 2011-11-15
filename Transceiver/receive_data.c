/* ECE 3140 - Embedded Systems, Cornell University
 * Lab 5 - Local Receiver Code for Remote Weather Station
 * Jeremy Blum and Matt Newberg - May 2011
 */
 
#include "bsp.h"
#include "mrfi.h"
#include "mrfi_spi.h"
#include "uart.h"
#include "msp430x22x4.h"

void sleep(unsigned int count);

#define RECEIVE_LED 0x01;
#define GREEN_LED   0x02;

main()
{
	WDTCTL = WDTPW + WDTHOLD;             // Stop watchdog timer
	BSP_Init();
	uart_default_init();
	MRFI_Init();
	MRFI_WakeUp();
	__bis_SR_register(GIE);
	
	//Set a filter address for the radio. This should correspond to what the TX is sending to.
	{
		uint8_t 	address[] = {0x10,0x10,0x10,0x10};
		unsigned char status;
		
		status = MRFI_SetRxAddrFilter(address);
		
		if( status == 1)
		{
			P1OUT = 0xFF;
			while(1);
		}
	}
	
	MRFI_RxOn();
	
	//Bit 0 is output - LED.
	P1DIR |= RECEIVE_LED;
	P1OUT |= RECEIVE_LED;
	P1OUT |= GREEN_LED;
	
	__bis_SR_register(GIE);
	
	//Main loop simply toggles the green LED - just for fun.
	while(1)
	{
		//sleep.
		sleep(60000);
		
		P1OUT ^= GREEN_LED;
	}
}



//ISR for reception. This function is called by the driver
//when fresh data is received.
void MRFI_RxCompleteISR(void)
{
	char msg[11];
	mrfiPacket_t	packet;
	
	//Read the data

	MRFI_Receive(&packet);
	
	//This grabs out nine bits of data, sticks a '.' at the end, and null terminates it.
	msg[0] = packet.frame[9];
	msg[1] = packet.frame[10];
	msg[2] = packet.frame[11];
	msg[3] = packet.frame[12];
	msg[4] = packet.frame[13];
	msg[5] = packet.frame[14];
	msg[6] = packet.frame[15];
	msg[7] = packet.frame[16];
	msg[8] = packet.frame[17];
	msg[9] = '.';
	msg[10] = '\0';
	
	//The data is then sent via UART to the Computer's Serial Interface
	uartTxString(msg);
	
	
	//Signal that data has arrived.
	P1OUT ^= RECEIVE_LED;
}


void sleep(unsigned int count)
{
	while(count > 0)
	{
		count--;
		__no_operation();
		__no_operation();
		__no_operation();
	}
}


/******************************************************************************
	Cornell University
	Computer Systems Lab
	ECE 3140 UART common functions
******************************************************************************/

/*
 * Initializes the UART interface, it assumes that the clock is set to 8Mhz and it uses this data to set the correct baud rate
 */
void uart_default_init() {
	
  //Calibrating clocks 
  DCOCTL = CALDCO_8MHZ;  				//Load 8MHz constants  (DCO FREQUENCY SET TO 8MHZ)
  BCSCTL1 = CALBC1_8MHZ; 				//(BASIC CLOCK SYSTEM CALIBRATED TO 8MHZ) --- THIS CAN BE SET TO 8, 12 OR 16
  
  //UART init
  P3SEL |= 0x30;
  UCA0CTL1 |= UCSSEL_2;                     // CLK = ACLK
  UCA0BR0 = 0x41;                           // 32kHz/9600 = 3.41
  UCA0BR1 = 0x03;                           //
  UCA0MCTL = UCBRS1 + UCBRS0;               // Modulation UCBRSx = 3
  UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
  IE2 |= UCA0RXIE;                          // Enable USCI_A0 (UART) RX interrupt
}

/*
 * Transmit a string through the UART interface
 * 
 */ 
void uartTx(char c) {
    while(!(IFG2 & UCA0TXIFG)); // wait for tx buf empty	
    UCA0TXBUF = c;
}
/*
 * Transmits a null-terminated string throught the UART interface
 */
void uartTxString(char *p) {
	while (*p) {
		if(*p == '\n') uartTx('\r');
		uartTx(*p++);
	}
}

