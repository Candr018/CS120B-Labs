/*
 * Lab 5 Test 1.c
 *
 * Created: 4/15/2019 8:35:08 AM
 * Author : User
 */ 
#include <avr/io.h>

int main(void)
{
	DDRB = 0xFF; PORTB = 0x00; // Configure port B's 8 pins as outputs
	while(1)
	{
		PORTB = 0x0F; // Writes port B's 8 pins with 00001111
	}
}


