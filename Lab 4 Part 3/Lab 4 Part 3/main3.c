/*
 * Partner(s) Name & E-mail: Brain 
 * My Email: candr018@ucr.edu
 * Lab Section: 027
 * Assignment: Lab #4 Exercise #3
 * 
 * I acknowledge all content contained herein, excluding given examples and temples code, is my original work
 */ 

#include <avr/io.h>

enum State_Machine {start, start_input, button_release, input_1, input_match, end_state} Keypad;
unsigned char temp_a;
unsigned char temp_b;
	
void Tick_Keypad()
{
	switch(Keypad)
	{
		//case 1 -- initialize case on startup
		case start:
			temp_b = 0x00;
			Keypad = start_input;
		break; 
		
		//case 2 -- only works if you are only pressing the # (A3) button
		//multiple buttons being pressed at the same time will not work
		case start_input:
			temp_b = 0x00;
			Keypad = temp_a == 0x04 ? button_release:start_input;
		break;
		
		//case 3 -- only works if you stop pressing # (A3) button along with
		//all other buttons
		case button_release:
			temp_b = 0x00;
			Keypad = temp_a == 0x00 ? input_1:button_release;
		break;
		
		//case 4 -- resets if Y (A1) is not pressed
		case input_1:
			temp_b = 0x00;
			if(temp_a == 0x02)
			{
				Keypad = input_match;
			}
			else if(temp_a == 0X00)
			{
				Keypad = input_1;
			}
			else
			{
				Keypad = start_input;
			}
		break;
		
		//case 5 -- door unlocked
		//door only locks if the lock button (A7) if pressed without other inputs
		case input_match:
			temp_b = 0x01;
			Keypad = end_state;
		break;
		
		case end_state:
			Keypad = end_state;
		break;
		
		default:
			Keypad = start;
			break;
	}
}

int main(void)
{
    DDRA = 0x00; PORTA = 0xFF; // Configure port A's 8 pins as inputs
    DDRB = 0xFF; PORTB = 0x00; // Configure port C's 8 pins as outputs
	Keypad = start;
	
    while (1) 
    {
		temp_a = PINA & 0x87; //allows for A7, A2, A1, and A0 to be inputs
		Tick_Keypad();
		PORTB = temp_b;
    }
}



