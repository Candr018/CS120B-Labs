/*
 * Partner(s) Name & E-mail: Brain 
 * My Email: candr018@ucr.edu
 * Lab Section: 027
 * Assignment: Lab #5 Challenge
 * 
 * I acknowledge all content contained herein, excluding given examples and temples code, is my original work
 */

#include <avr/io.h>

enum State_Machine {initalize, wait_button, lights_1, lights_2, lights_3, lights_4, wait_release} christmas_lights;
unsigned char temp_a;
unsigned char temp_c;

void Tick_Christmas()
{
	switch(christmas_lights)
	{
		//case 1
		case initalize:
		temp_c = 0x00;
		christmas_lights = wait_button;
		break;
		
		//case 2
		case wait_button:
		temp_c = temp_c;
		if(temp_a == 0x01 && temp_c == 0x00)
		{
			christmas_lights = lights_1;
		}
		else if(temp_a == 0x01 && temp_c == 0x21)
		{
			christmas_lights = lights_2;
		}
		else if(temp_a == 0x01 && temp_c == 0x12)
		{
			christmas_lights = lights_3;
		}
		else if(temp_a == 0x01 && temp_c == 0x0C)
		{
			christmas_lights = lights_4;
		}
		else if(temp_a == 0x01 && temp_c == 0x3F)
		{
			christmas_lights = initalize;
		}
		else
		{
			christmas_lights = wait_button;
		}
		break;
		
		case lights_1:
		temp_c = 0x21;
	    christmas_lights = wait_release;
		break;
		
		case lights_2:
		temp_c = 0x12;
		christmas_lights = wait_release;
		break;
		
		case lights_3:
		temp_c = 0x0C;
		christmas_lights = wait_release;
		break;
		
		case lights_4:
		temp_c = 0x3F;
		christmas_lights = wait_release;
		break;
		
		case wait_release:
		temp_c = temp_c;
		if(temp_a == 0x01)
		{
			christmas_lights = wait_release;
		}
		else
		{
			christmas_lights = wait_button;
		}
		break;
		
		default:
		christmas_lights = initalize;
		break;
	}
}

unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b) {
	return (b ? x | (0x01 << k) : x & ~(0x01 << k));
}
unsigned char GetBit(unsigned char x, unsigned char k) {
	return ((x & (0x01 << k)) != 0);
}

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF; // Configure port A's 8 pins as inputs
	DDRB = 0xFF; PORTB = 0x00; // Configure port C's 8 pins as outputs
	christmas_lights = initalize;
	unsigned char button_1 = 0x00;		//button for the first input
	
	while (1)
	{
		button_1 = PINA & 0x01;

		temp_a = button_1;
		Tick_Christmas();
		PORTB = temp_c;
	}
}






