/* ECE 3140 - Embedded Systems, Cornell University
 * Lab 5 - Remote Weather Station Code
 * Jeremy Blum and Matt Newberg - May 2011
 */

#include "bsp.h"
#include "mrfi.h"
#include "radios/family1/mrfi_spi.h"

void sleep(unsigned int count);
#define RED_SEND_LED 		0x01 //Red

//Read from an Analog input
int analogRead(int channel)
{
	int result;
	ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON + ADC10IE + REF2_5V;
  	TACCR0 = 30;								// Delay to allow Ref to settle
 	TACCTL0 |= CCIE;                          	// Compare-mode interrupt
  	TACTL = TASSEL_2 + MC_1;                  	// Clock Setup
  	__bis_SR_register(CPUOFF + GIE);          	// LPM0, TA0_ISR allowed
  	TACCTL0 &= ~CCIE;                         	// Disable timer Interrupt
	
	// P1.1 -> Internal Temperature
	if (channel == 10)
	{
		ADC10CTL0 &= ~REF2_5V;					// 1.5V Reference	
		ADC10CTL1 = INCH_10 + ADC10DIV_3;		// Input Selct and Clock Div
		P1SEL = 0x02;							// Pin Select
		ADC10AE0 = 0x00;						// ADC Low Bit
		ADC10AE1 = 0x02;						// ADC High Bit (A10)
	}

	// P2.2 -> Light Sensor - A2
	if (channel == 2)
	{
		ADC10CTL0 |= REF2_5V;					// 2.5V Reference	
		ADC10CTL1 = INCH_2 + ADC10DIV_3;		// Input Selct and Clock Div
		P2SEL = 0x04;							// Pin Select
		ADC10AE0 = 0x04;						// ADC Low Bit (A2)
		ADC10AE1 = 0x00;						// ADC High Bit
	}
	// P2.3 -> Humidity Sensor - A3
	else if (channel == 3)
	{
		ADC10CTL0 |= REF2_5V;					// 2.5V Reference
		ADC10CTL1 = INCH_3 + ADC10DIV_3;		// Input Selct and Clock Div
		P2SEL = 0x08;							// Pin Select
		ADC10AE0 = 0x08;						// ADC Low Bit (A3)
		ADC10AE1 = 0x00;						// ADC High Bit
	}

	ADC10CTL0 |= ENC + ADC10SC;					// Start to sample
	__bis_SR_register(CPUOFF + GIE);        	// LPM0 w/ int
	result = ADC10MEM;							// Store Result
	if (result < 0) result = 0;					// Correct for potential Weirdness

	ADC10AE0 = 0;								// Reset Selection Bits
	ADC10AE1 = 0;
	return result;								// Return Result
}

// Converts a positive integer to a string for transmission
// Str = null-terminated string.
// *str = array that can hold the number of digits in limit + 1.
void itoa(unsigned int val, char *str, unsigned int limit)
{
	int temploc = 0;
	int digit = 0;
	int strloc = 0;
	char tempstr[5]; //16-bit number can't be more than 5 digits

	if(val>limit) val %= limit;
 
 	do
 	{
   		digit = val % 10;
   		tempstr[temploc++] = digit + '0'; //Use the '0' Ascii char to do some clever conversion
   		val /= 10;
 	}
 	while (val > 0);
  
 	// Put the computed digits in the output string
 	while(temploc>0)
 		str[strloc++] = tempstr[--temploc];
	str[strloc]=0;
}

//Main Execution of the program
main()
{
	int temp;
	int DegK;
	int light;
	int humidity;
	char c[4];
	
	
	mrfiPacket_t 	packet;
	unsigned char msg[9];
	
	BSP_Init();
	
	MRFI_Init();
	MRFI_WakeUp();
	
	//Disable Watchdog Timer
	WDTCTL = WDTPW + WDTHOLD;
	
	//Red LED P1.1
	P1DIR |= 0x01;
	P1OUT |= 0x00;
	
	//Analog Input Setup
	//P1.1 = A10 = Internal	= Temperature Sensor
	//P2.2 = A2  = Pin 5 	= Light Sensor
	//P2.3 = A3  = Pin 6 	= Humidity Sensor
	P1DIR &= ~0x02;	//P1.1 is Input
	P2DIR &= ~0x0C; //P2.2 & P2.3 are Inputs
	
	//Global Interrupt Enable
	__bis_SR_register(GIE);
	
	//Read Analog Sensors Forever!
	while(1)
	{
		//Read Light Sensor
		light = analogRead(2);
		light = (int) (light/1023.0*100.0);
		sleep(60000);
		//Read Humidity Sensor
		humidity = analogRead(3);
		humidity = (int) (((((humidity/1023.0)*2.5)-.45)*100.0)/1.5); //Derived from Datasheet Info
		sleep(60000);
		//ReadTemperature
		temp = analogRead(10); 
	    DegK = (int) (((temp - 673.0) * 423.0) / 1023.0) + 273.0;
	
		//Make all values into 3 digit strings.
		itoa(light, c, 999);
		if (light < 10)
		{
			msg[0] = '0';
			msg[1] = '0';
			msg[2] = c[0];
		}
		else if (light < 100)
		{
			msg[0] = '0';
			msg[1] = c[0];
			msg[2] = c[1];
		}
		else
		{
			msg[0] = c[0];
			msg[1] = c[1];
			msg[2] = c[2];
		}
		
		itoa(humidity, c, 999);
		if (humidity < 10)
		{
			msg[3] = '0';
			msg[4] = '0';
			msg[5] = c[0];
		}
		else if (humidity < 100)
		{
			msg[3] = '0';
			msg[4] = c[0];
			msg[5] = c[1];
		}
		else
		{
			msg[3] = c[0];
			msg[4] = c[1];
			msg[5] = c[2];
		}
		
		itoa(DegK, c, 999);
		if (DegK < 10)
		{
			msg[6] = '0';
			msg[7] = '0';
			msg[8] = c[0];
		}
		else if (DegK < 100)
		{
			msg[6] = '0';
			msg[7] = c[0];
			msg[8] = c[1];
		}
		else
		{
			msg[6] = c[0];
			msg[7] = c[1];
			msg[8] = c[2];
		}
		
		//Copy message into the packet.
		strcpy( &packet.frame[9] , msg );
		
		/** Enter source and destination addresses  **/
		//Source address here is useless, as this is a send-only application.
		memset( &packet.frame[1], 0x20, 4);
		memset( &packet.frame[5], 0x10, 4);
		
		// Enter length of packet.
		packet.frame[0] = 8 + strlen(msg); // 8-byte header, 20-byte payload.
		
		//Transmit 
		MRFI_Transmit(&packet , MRFI_TX_TYPE_FORCED);
		
		//Toggle led.
		P1OUT ^= RED_SEND_LED;
		
		//sleep.
		sleep(60000);
		sleep(60000);
		sleep(60000);
		sleep(60000);
		sleep(60000);
	}
	
}

void MRFI_RxCompleteISR(void)
{
	//Dummy function for a SEND-only application; 
	//still needs to be defined for the project to compile properly.
}


//A simple delay
void sleep(unsigned int count)
{
	int i;
	
	for (i = 0; i < 10; i++)
	{
		while(count > 0)
		{
			count--;
			
			__no_operation();
			__no_operation();
			__no_operation();
		}
	}
}


// ADC10 interrupt service routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
  __bic_SR_register_on_exit(CPUOFF);        // Clear CPUOFF bit from 0(SR)
}

#pragma vector=TIMERA0_VECTOR
__interrupt void TA0_ISR(void)
{
  TACTL = 0;                                // Clear Timer_A control registers
  __bic_SR_register_on_exit(CPUOFF);        // Clear CPUOFF bit from 0(SR)
}


