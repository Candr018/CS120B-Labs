/*
 * Lab 9 Part 1.c
 *
 * Created: 5/2/2019 5:02:30 PM
 * Author : User
 */ 

#include <avr/io.h>
void set_PWM(double frequency) {
	static double current_frequency; // Keeps track of the currently set frequency
	// Will only update the registers when the frequency changes, otherwise allows
	// music to play uninterrupted.
	if (frequency != current_frequency) {
		if (!frequency) { TCCR3B &= 0x08; } //stops timer/counter
		else { TCCR3B |= 0x03; } // resumes/continues timer/counter
		
		// prevents OCR3A from overflowing, using prescaler 64
		// 0.954 is smallest frequency that will not result in overflow
		if (frequency < 0.954) { OCR3A = 0xFFFF; }
		
		// prevents OCR3A from underflowing, using prescaler 64					// 31250 is largest frequency that will not result in underflow
		else if (frequency > 31250) { OCR3A = 0x0000; }
		
		// set OCR3A based on desired frequency
		else { OCR3A = (short)(8000000 / (128 * frequency)) - 1; }

		TCNT3 = 0; // resets counter
		current_frequency = frequency; // Updates the current frequency
	}
}

void PWM_on() {
	TCCR3A = (1 << COM3A0);
	// COM3A0: Toggle PB6 on compare match between counter and OCR3A
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
	// WGM32: When counter (TCNT3) matches OCR3A, reset counter
	// CS31 & CS30: Set a prescaler of 64
	set_PWM(0);
}

void PWM_off() {
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}


enum States {init, wait_input, freq1, freq2, freq3, multi_input, reset} next_state;

double input_frequency;
unsigned char button;

void tick() {
	
	unsigned char button = ~PINA & 0x07;
	
	switch (next_state)
	 { //transitions
		 
		case init:
		
			next_state = wait_input;
			
		break;
		
		case wait_input:
			if(button == 0x01)
			{
				next_state = freq1;
			}
			else if(button == 0x02)
			{
				next_state = freq2;
			}
			else if(button == 0x04)
			{
				next_state = freq3;
			}
			else if(button == 0x03 || button == 0x05 || button == 0x06 || button == 0x07)
			{
				next_state = multi_input;
			}
			else
			{
				next_state = wait_input;
			}
		break;
		
		case freq1:
		 if(button == 0x01)
		 {
			 next_state = freq1;
		 }
		 else
		 {
			 next_state = wait_input;
		 }
		break;
		
		case freq2:
		if(button == 0x02)
		{
			next_state = freq2;
		}
		else
		{
			next_state = wait_input;
		}
		break;
		
		case freq3:
		if(button == 0x04)
		{
			next_state = freq3;
		}
		else
		{
			next_state = wait_input;
		}
		break;
		
		case multi_input:
		next_state = wait_input;
		break;
		
		default:
		next_state = init;
	}
	
	switch (next_state) { //action
		case init:
		input_frequency = 0;
		break;
		
		case wait_input:
		input_frequency = 0;
		break;
		
		case freq1:
		input_frequency = 261.63;
		break;
		
		case freq2:
		input_frequency = 293.66;
		break;
		
		case freq3:
		input_frequency = 329.63;
		break;
		
		case multi_input:
		input_frequency = 0;
		break;
		
		default:
		input_frequency = 0;
		break;
		
	}
}

int main(void)
{
    DDRB = 0xFF; PORTB = 0x00;
	DDRA = 0x00; PORTA = 0xFF;
	
	next_state = init; 
	
	PWM_on();
    while (1) 
    {
		
		tick();
		set_PWM(input_frequency);
    }
}

