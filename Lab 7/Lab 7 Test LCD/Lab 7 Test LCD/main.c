/*
 * Lab 7 Test LCD.c
 *
 * Created: 4/25/2019 4:45:57 PM
 * Author : User
 */ 

#include <avr/io.h>
#include "io.c"

int main(void)
{
	DDRC = 0xFF; PORTC = 0x00; // LCD data lines
	DDRD = 0xFF; PORTD = 0x00; // LCD control lines
	
	unsigned char tmpC = 0x01;
	// Initializes the LCD display
	LCD_init();
	
	// Starting at position 1 on the LCD screen, writes Hello World
	LCD_WriteData(tmpC = '0');
	
	while(1) {continue;}
}


