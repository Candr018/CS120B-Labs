/*
 * Partner(s) Name & E-mail: Brain 
 * My Email: candr018@ucr.edu
 * Lab Section: 027
 * Assignment: Lab #4 Exercise #1
 * 
 * I acknowledge all content contained herein, excluding given examples and temples code, is my original work
 */ 

#include <avr/io.h>

enum State_Machine {one, two, three, four, five, six, seven, eight} Keypad;
unsigned char temp_a;
unsigned char temp_b;
	
void Tick_Keypad()
{
	switch(Keypad)
	{
		//case 1
		case one:
			temp_b = 0x00;
			Keypad = two;
		break; 
		
		//case 2 -- only works if you are only pressing the # (A3) button
		//multiple buttons being pressed at the same time will not work
		case two:
			temp_b = 0x00;
			Keypad = temp_a == 0x04 ? three:two;
		break;
		
		//case 3 -- only works if you stop pressing # (A3) button along with
		//all other buttons
		case three:
			temp_b = 0x00;
			Keypad = temp_a == 0x00 ? four:three;
		break;
		
		//case 4 -- resets if Y (A1) is not pressed
		case four:
			temp_b = 0x00;
			if(temp_a == 0x02)
			{
				Keypad = five;
			}
			else if(temp_a == 0X00)
			{
				Keypad = four;
			}
			else
			{
				Keypad = two;
			}
		break;
		
		//case 5 -- door unlocked
		//door only locks if the lock button (A7) if pressed without other inputs
		case five:
			temp_b = 0x01;
			Keypad = six;
		break;
		
		//case 6
		case six:
			temp_b = 0x01;
			if(temp_a == 0x70 || temp_a == 0x71 || temp_a == 0x72 || temp_a == 0x73 || temp_a == 0x75 || temp_a == 0x76 || temp_a == 0x77)
			{
				Keypad = two;
			}
			else if(temp_a == 0x04)
			{
				Keypad = seven;
			}
			else
			{
				Keypad = six;
			}
		break;
		
		//case 7
		case seven:
			temp_b = 0x01;
			if(temp_a == 0x70 || temp_a == 0x71 || temp_a == 0x72 || temp_a == 0x73 || temp_a == 0x75 || temp_a == 0x76 || temp_a == 0x77)
			{
				Keypad = two;
			}
			else if(temp_a == 0x00)
			{
				Keypad = eight;
			}
			else
			{
				Keypad = seven;
			}
		break;
		
		//case 8
		case eight:
			temp_b = 0x01;
			if(temp_a == 0x70 || temp_a == 0x71 || temp_a == 0x72 || temp_a == 0x73 || temp_a == 0x75 || temp_a == 0x76 || temp_a == 0x77)
			{
				Keypad = two;
			}
			else if(temp_a == 0x02)
			{
				Keypad = one;
			}
			else
			{
				Keypad = eight;
			}
		break;
		
		default:
			Keypad = one;
			break;
	}
}

int main(void)
{
    DDRA = 0x00; PORTA = 0xFF; // Configure port A's 8 pins as inputs
    DDRB = 0xFF; PORTB = 0x00; // Configure port C's 8 pins as outputs
	Keypad = one;
	
    while (1) 
    {
		temp_a = PINA & 0x87; 
		Tick_Keypad();
		PORTB = temp_b;
    }
}



