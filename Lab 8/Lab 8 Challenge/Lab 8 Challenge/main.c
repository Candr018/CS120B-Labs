/*
 * Lab 8 Challenge.c
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
enum States {init, led1, led2, led3, led4, led5, led6, reset} state;


unsigned char tempB;
unsigned short ADC_Max;
unsigned short Voltage_Supplied;
unsigned short lim_1;
unsigned short lim_2;
unsigned short lim_3;
unsigned short lim_4;
unsigned short lim_5;
unsigned short lim_6;

void tick() {
	
	unsigned char button = ~PINA & 0x01;
	
	switch (state) { //transitions
		case init:
		state = led1;
		break;
		
		case led1:
		if (Voltage_Supplied > lim_1)
		{
			state = led2;	
		}
		else
		{
			state = led1;
		}
		break;
		
		case led2:		
		if (Voltage_Supplied > lim_2)
		{
			state = led3;
		}
		else if(Voltage_Supplied > lim_1 && (Voltage_Supplied >! lim_2))
		{
			state = led2;
		}
		else
		{
			state = led1;
		}
		break;
		
		case led3:
		if (Voltage_Supplied > lim_2)
		{
			state = led3;
		}
		else if(Voltage_Supplied > lim_2)
		{
			state = led2;
		}
		else
		{
			state = led1;
		}
		break;
		
		case led4:
		if (Voltage_Supplied > lim_2)
		{
			state = led3;
		}
		else if(Voltage_Supplied > lim_2)
		{
			state = led2;
		}
		else
		{
			state = led1;
		}
		break;
		
		case led5:
		if (button)
		state = init;
		else
		state = reset;
		break;
		
		case led6:
		if()
		default:
		state = init;
	}
	
	switch (state) { //action
		case init:
			tempB = 0x01;
		break;
		
		case led1:
			tempB = 0x03;
		break;
		
		case led2:
			tempB = 0x02;
		break;
		
		case b2:
		tempB = 0x04;
		break;
		case stop:
		break;
		case reset:
		break;
		default:
		break;
		
	}
	
	PORTB = tempB;
	
}

int main(void)
{
	DDRA = 0xFE; PORTA = 0x01;
	DDRB = 0xFF; PORTB = 0x00;
	DDRD = 0xFF; PORTD = 0x00;	
    
	
	unsigned short ADC_Max;
	unsigned char temp_b;
	unsigned char temp_d;
	lim_1 = ADC_Max / 2;
	lim_2 = 2 * lim_1;
	lim_3 = 3 * lim_1;
	lim_4 = 4 * lim_1;
	lim_5 = 5 * lim_1;
	lim_6 = 6 * lim_1;
	
	while (1) 
    {
		// NOTE: Get ADC max from last part
		
		ADC_Max = 0x003F;
		Voltage_Supplied = ADC;
		temp_b = Voltage_Supplied & 0xFF;
		temp_d = Voltage_Supplied >> 8;
		
		if( Voltage_Supplied > ADC_Max / 2)
		{
			temp_b = 0x01;
		}
		else if( Voltage_Supplied < ADC_Max / 2)
		{
			temp_b = 0x00;
		}
		else
		{
			temp_b = 0x00;
		}
		PINB = temp_b;
    }
}
