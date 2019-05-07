/*
 * Lab 9 part 2.c
 *
 * Created: 5/5/2019 3:04:06 PM
 * Author : Athena
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


enum States {init, wait_start, wait_input, freq1, freq2, freq3, freq4, freq5, freq6, freq7, freq8} next_state;

double input_frequency;
unsigned char button;
unsigned char button_released;
double C4 = 261.63;
double D4 = 293.66;
double E4 = 329.63;
double F4 = 349.23;
double G4 = 392.00;
double A4 = 440.00;
double B4 = 493.88;
double C5 = 523.25;

void tick() {

	
	switch (next_state)
	{ //transitions
		
		case init:
		next_state = wait_start;
		break;
		
		case wait_start:
		if(button == 0x01)
		{
			next_state = wait_input;
		}
		else
		{
			next_state = wait_start;
		}
		break;
		
		case wait_input:
			next_state = freq1;
		break;
		
		//----------------- Frequency 1 state ----------------------//
		
		case freq1:
		if(button == 0x04)	// increment NOTE: No decrement in frequency one state
			{
				next_state = freq2;
			}
		else if(button == 0x01) //reset 
			{
				next_state = wait_start;
			}
		else
			{
				next_state = freq1;
			}
		break;
		
		//----------------- Frequency 2 state ----------------------//
		
		case freq2:
		if (button_released == 0x00)
		{
		if(button == 0x04)	//increment
			{
				next_state = freq3;
			}
		else if(button == 0x02)  //decrement
			{
				next_state = freq1;
			}
		else if(button == 0x01) // reset 
			{
				next_state = wait_start;
			}
		else       //null transition
			{
				next_state = freq2;
			}
		}
		else
		{
			next_state = freq2;
		}
		break;
		
		//----------------- Frequency 3 state ----------------------//
		
		case freq3:
		if (button_released == 0x00)
		{
		if(button == 0x04)	//increment
			{
				next_state = freq4;
			}
		else if(button == 0x02)	//decrement
			{
				next_state = freq2;
			}
		else if(button == 0x01) //reset
			{
				next_state = wait_start;
			}
		else // null transition
			{
				next_state = freq3;
			}
		}
		else
		{
			next_state = freq3;
		}
		break;
		
		//----------------- Frequency 4 state ----------------------//
		
		case freq4:
		if (button_released == 0x00)
		{
		if(button == 0x04)	// increment
			{
				next_state = freq5;
			}
		else if(button == 0x02)	//decrement
			{
				next_state = freq3;
			}
		else if(button == 0x01)	//reset
			{
				next_state = wait_start;
			}
		else //null transition
			{
				next_state = freq4;
			}
		}
		else
		{
			next_state = freq4;
		}
		break;
		
		//----------------- Frequency 5 state ----------------------//
		
		case freq5:
		if (button_released == 0x00)
		{
		if(button == 0x04)	//increment
			{
				next_state = freq6;
			}
		else if(button == 0x02) //decrement
			{
				next_state = freq4;
			}
		else if(button == 0x01) //reset
			{
				next_state = wait_start;
			}
		else //null transition
			{
				next_state = freq5;
			}
		}
		else
		{
			next_state = freq5;
		}
		break;
		
		//----------------- Frequency 6 state ----------------------//
		
		case freq6:
		if (button_released == 0x00)
		{
		if(button == 0x04) // increment
			{
				next_state = freq7;
			}
		else if(button == 0x02) // decrement
			{
				next_state = freq5;
			}
		else if(button == 0x01) // reset
			{
				next_state = wait_start;
			}
		else //null transition
			{
				next_state = freq6;
			}
		}
		else
		{
			next_state = freq6;
		}
		break;
		
		//----------------- Frequency 7 state ----------------------//
		
		case freq7:
		if (button_released == 0x00)
		{
		if(button == 0x04) // increment
		{
			next_state = freq8;
		}
		else if(button == 0x02) // decrement
		{
			next_state = freq6;
		}
		else if(button == 0x01) // reset
		{
			next_state = wait_start;
		}
		else // null transition
		{
			next_state = freq7;
		}
		}
		else
		{
		next_state = freq7;
		}
		break;
		
		//----------------- Frequency 8 state ----------------------//
		
		case freq8:
		if(button == 0x02) // decrement NOTE: no increment for frequency eight
			{
				next_state = freq7;
			}
		else if(button == 0x01) // reset
			{
				next_state = wait_start;
			}
		else //null transition
			{
				next_state = freq8;
			}
		break;
		
		default:
		next_state = init;
	}
	
	switch (next_state) { //action
		case init:
		input_frequency = 0;
		break;
		
		case wait_start:
		input_frequency = 0;
		break;
		
		case wait_input:
		input_frequency = 0;
		break;
		
		case freq1:
		input_frequency = C4;
		break;
		
		case freq2:
		if(button == 0x00)
		{
			button_released = 0x00;
		}
		else
		{
			button_released = 0x01;
		}
		input_frequency = D4;
		break;
		
		case freq3:
		if(button == 0x00)
		{
			button_released = 0x00;
		}
		else
		{
			button_released = 0x01;
		}
		input_frequency = E4;
		break;
		
		case freq4:
		if(button == 0x00)
		{
			button_released = 0x00;
		}
		else
		{
			button_released = 0x01;
		}
		input_frequency = F4;
		break;
		
		case freq5:
		if(button == 0x00)
		{
			button_released = 0x00;
		}
		else
		{
			button_released = 0x01;
		}
		input_frequency = G4;
		break;
		
		case freq6:
		if(button == 0x00)
		{
			button_released = 0x00;
		}
		else
		{
			button_released = 0x01;
		}
		input_frequency = A4;
		break;
		
		case freq7:
		if(button == 0x00)
		{
			button_released = 0x00;
		}
		else
		{
			button_released = 0x01;
		}
		input_frequency = B4;
		break;
		
		case freq8:
		input_frequency = C5;
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
	button_released = 0x00;
	PWM_on();
	while (1)
	{
		button = ~PINA & 0x07;
		
		tick();
		set_PWM(input_frequency);
	}
}

