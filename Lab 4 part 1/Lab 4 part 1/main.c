/*
 * Partner(s) Name & E-mail: Brain 
 * My Email: candr018@ucr.edu
 * Lab Section: 027
 * Assignment: Lab #4 Exercise #1
 * 
 * I acknowledge all content contained herein, excluding given examples and temples code, is my original work
 */ 

#include <avr/io.h>

enum LED_States {led1_on, button_press_1, led2_on, button_press_2} LED_states;
unsigned char temp_a;
unsigned char temp_b;
	
void Tick_LED()
{
	switch(LED_states)
	{
		case led1_on:
			temp_b = 0x01;
			LED_states = temp_a == 0x01 ? button_press_1:led1_on;
			break; 
			
		case button_press_1:
			temp_b = 0x02;
			LED_states = temp_a == 0x00 ? led2_on:button_press_1;
			break;
		
		case led2_on:
			temp_b = 0x02;
			LED_states = temp_a == 0x01 ? button_press_2:led2_on;
			break;
		
		case button_press_2:
			temp_b = 0x01;
			LED_states = temp_a == 0x00 ? led1_on:button_press_2;
			break;
		
		default:
			temp_b = 0x01;
			LED_states = led1_on;
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
	LED_states = led1_on;
	
    while (1) 
    {
		temp_a = PINA & 0x01; 
		Tick_LED();
		PORTB = temp_b;
    }
}

