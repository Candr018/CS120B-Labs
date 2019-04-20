/*
 *
 *
 * Partner(s) Name & E-mail: Brain 
 * My Email: candr018@ucr.edu
 * Lab Section: 027
 * Assignment: Lab #4 Exercise #1
 * 
 * I acknowledge all content contained herein, excluding given examples and temples code, is my original work
 */ 

#include <avr/io.h>

enum State_Machine {initalize, wait_buttons, decrement, increment, wait_release_1, wait_release_2, double_press} Buttons;
unsigned char temp_a;
unsigned char temp_c;
	
void Tick_Buttons()
{
	switch(Buttons)
	{
		//case 1
		case initalize:
			temp_c = 0x07;
			Buttons = wait_buttons;
			break; 
		
		//case 2	
		case wait_buttons:
			temp_c = temp_c;
			if(temp_a == 0x01 && temp_c < 0x09)
			{
				Buttons = increment;
			}
			else if(temp_a == 0x02 && temp_c > 0x00)
			{
				Buttons = decrement;
			}
			else if(temp_a == 0x03)
			{
				Buttons = double_press;
			}
			else
			{
				Buttons = wait_buttons;
			}
			break;
		
		//case 3
		case decrement:
			temp_c--;
			if(temp_a == 0x03)
			{
				Buttons = double_press;
			}
			else
			{
				Buttons = wait_release_2;
			}
			break;
		
		//case 4
		case increment:
			temp_c++;
			if(temp_a == 0x03)
			{
				Buttons = double_press;
			}
			else
			{
				Buttons = wait_release_1;
			}
			break;
		
		//case 5
		case wait_release_1:
		temp_c = temp_c;
		if(temp_a == 0x03)
		{
			Buttons = double_press;
		}
		else if(temp_a == 0x01)
		{
			Buttons = wait_release_1;
		}
		else
		{
			Buttons = wait_buttons;
		}
		break;
		
		//case 6
		case wait_release_2:
		temp_c = temp_c;
		if(temp_a == 0x03)
		{
			Buttons = double_press;
		}
		else if(temp_a == 0x02)
		{
			Buttons = wait_release_2;
		}
		else
		{
			Buttons = wait_buttons;
		}
		break;
		
		//case 7
		case double_press:
		temp_c = 0x00;
		Buttons = wait_buttons;
		break;
		
		default:
			Buttons = initalize;
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
	Buttons = initalize;
		unsigned char button_1 = 0x00;		//button for the first input
		unsigned char button_2 = 0x00;		//button for the second input
	
    while (1) 
    {
		button_1 = ~PINA & 0x01;
		button_2 = ~PINA & 0x02;
		temp_a = button_1 += button_2; 
		Tick_Buttons();
		PORTB = temp_c;
    }
}




