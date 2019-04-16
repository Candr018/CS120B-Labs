/*
 * Partner(s) Name & E-mail: Brain 
 * My Email: candr018@ucr.edu
 * Lab Section: 027
 * Assignment: Lab #5 Exercise #1
 * 
 * I acknowledge all content contained herein, excluding given examples and temples code, is my original work
 */ 

#include <avr/io.h>

// Bit-access function
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
	
	const unsigned char full_tank = 0x0D;		//value for full tank
	const unsigned char almost_full = 0x0A;		//value for almost full
	const unsigned char half_full = 0x07;		//value for half full tank
	const unsigned char almost_empty = 0x05;	//value for almost empty tank
	const unsigned char empty_tank = 0x03;		//value for empty tank
	
	unsigned char fuel_level = 0x00; // intermediate variable used to hold fuel level from input sensors
	unsigned char temp_total = 0x00; // intermediate variable to hold output
	//unsigned char fuel_light = 0x00; // intermediate variable to hold output for the fuel light
	
	unsigned char button_1 = 0x00;		//button for the first input
	unsigned char button_2 = 0x00;		//button for the second input
	unsigned char button_3 = 0x00;		//button for the third input
	unsigned char button_4 = 0x00;		//button for the fourth input
	while(1)
	{
		fuel_level = 0x00;
		button_1 = PINA & 0x01;
		button_2 = PINA & 0x02;
		button_3 = PINA & 0x04;
		button_4 = PINA & 0x08;
		fuel_level += button_1;
		fuel_level += button_2;
		fuel_level += button_3; 
		fuel_level += button_4;
		
		temp_total = 0x3F; //This corresponds to a full tank 0011 1111
		
		if(fuel_level < full_tank && fuel_level > almost_full)
		{
			temp_total = 0x3E;
		}
		else if(fuel_level < almost_full && fuel_level > half_full)
		{
			temp_total = 0x3C;
		}
		else if(fuel_level < half_full && fuel_level > almost_empty)
		{
			temp_total = 0x38;
		}
		else if(fuel_level < almost_empty && fuel_level > empty_tank)
		{
			temp_total = 0x70;
		}
		else if(fuel_level < empty_tank)
		{
			temp_total = 0x60;
		}
		else
		{
			temp_total = 0x3F;
		}
		PORTB = temp_total;
	}
} 
