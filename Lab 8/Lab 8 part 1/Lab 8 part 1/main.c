/*
 * Lab 8 part 1.c
 *
 * Created: 5/2/2019 4:11:24 PM
 * Author : User
 */ 

#include <avr/io.h>

void ADC_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
	// ADEN: setting this bit enables analog-to-digital conversion.
	// ADSC: setting this bit starts the first conversion.
	// ADATE: setting this bit enables auto-triggering. Since we are
	//        in Free Running Mode, a new conversion will trigger whenever
	//        the previous conversion completes.
}

int main(void)
{
	DDRA = 0xFE; PORTA = 0x01;
	DDRB = 0xFF; PORTB = 0x00;
	DDRD = 0xFF; PORTD = 0x00;	
    
	unsigned short Voltage_Supplied;
	
	unsigned char temp_b;
	unsigned char temp_d;
	
	while (1) 
    {
		Voltage_Supplied = ADC;
		temp_b = Voltage_Supplied & 0xFF;
		temp_d = Voltage_Supplied >> 8;
		
		PINB = temp_b;
		PIND = temp_d & 0x03;
    }
}

