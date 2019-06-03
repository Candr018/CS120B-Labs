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
	
	// Initializes the LCD display
	LCD_init();
	
	// Starting at position 1 on the LCD screen, writes Hello World
	//LCD_DisplayString(1, "Hello World");
	LCD_ClearScreen();
	while(1) {continue;}
}


