/*
 * Lab 9 part 2.c
 *
 * Created: 5/5/2019 3:04:06 PM
 * Author : Athena
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

// ---------------------------------------------------- Global Var definitions --------------------------------------------------------------------------------------------//
enum States {init, wait_start, space_state, wait_release, measure_1, measure_2, measure_3, measure_4, measure_5, measure_6, measure_7, measure_8} next_state;

// all 8 key notes
double C4 = 261.63;
double D4 = 293.66;
double E4 = 329.63;
double F4 = 349.23;
double G4 = 392.00;
double A4 = 440.00;
double B4 = 493.88;
double C5 = 523.25;

unsigned char button;		// variable to hold whether or not the button is currently being pressed

double input_frequency;     // frequency to be output to the speaker

volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.

unsigned char counter = 0; // counter to keep bpm

unsigned char counter_measure = 0;

unsigned char cur_state = 0x00; // keeps track of the current state to control next state

unsigned char button_pressed = 0x00; //boolean to keep track of the button press to make sure it resets

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------//

// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks

void TimerOn() {
	// AVR timer/counter controller register TCCR1
	TCCR1B = 0x0B;// bit3 = 0: CTC mode (clear timer on compare)
	// bit2bit1bit0=011: pre-scaler /64
	// 00001011: 0x0B
	// SO, 8 MHz clock or 8,000,000 /64 = 125,000 ticks/s
	// Thus, TCNT1 register will count at 125,000 ticks/s

	// AVR output compare register OCR1A.
	OCR1A = 125;	// Timer interrupt will be generated when TCNT1==OCR1A
	// We want a 1 ms tick. 0.001 s * 125,000 ticks/s = 125
	// So when TCNT1 register equals 125,
	// 1 ms has passed. Thus, we compare to 125.
	// AVR timer interrupt mask register
	TIMSK1 = 0x02; // bit1: OCIE1A -- enables compare match interrupt

	//Initialize avr counter
	TCNT1=0;

	_avr_timer_cntcurr = _avr_timer_M;
	// TimerISR will be called every _avr_timer_cntcurr milliseconds

	//Enable global interrupts
	SREG |= 0x80; // 0x80: 1000000
}

void TimerOff() {
	TCCR1B = 0x00; // bit3bit1bit0=000: timer off
}

void TimerISR() {
	TimerFlag = 1;
}

// In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect) {
	// CPU automatically calls when TCNT1 == OCR1 (every 1 ms per TimerOn settings)
	_avr_timer_cntcurr--; // Count down to 0 rather than up to TOP
	if (_avr_timer_cntcurr == 0) { // results in a more efficient compare
		TimerISR(); // Call the ISR that the user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}
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


// synchronous state machine to run the melody
void tick() {
	
	switch (next_state) //transitions states
	{ 
		
		case init:
		next_state = wait_start;
		break;
		
		case wait_start:
		if(button == 0x01 && button_pressed == 0x00)
		{
			next_state = measure_1;
		}
		else
		{
			next_state = wait_start;
		}
		break;
		
		//----------------- Measure 1 state ----------------------//
		
		case measure_1:
		if(counter < 10)	// first note for 50 ms
			{
				next_state = measure_1;
			}
		else if(!(counter < 10))  // space state to provide natural progression
			{
				next_state = space_state;
			}
		else
			{
				next_state = measure_1;
			}
		break;
		
		//----------------- Measure 2 state ----------------------//
		
		case measure_2:
		if(counter < 10)	// second note for 50 ms
		{
			next_state = measure_2;
		}
		else if(!(counter < 10)) // space state to provide a natural progression
		{
			next_state = space_state;
		}
		else // assign measure 2 encase of error
		{
			next_state = measure_2;
		}
		break;
		
		//----------------- Measure 3 state ----------------------//
		
		case measure_3: 
		if(counter < 10)	// second note for 50 ms
		{
			next_state = measure_3;
		}
		else if(!(counter < 10)) // space state to provide a natural progression
		{
			next_state = space_state;
		}
		else // assign measure 3 encase of error
		{
			next_state = measure_3;
		}
		break;
		
		//----------------- Measure 4 state ----------------------//
		
		case measure_4:
		if(counter < 80)	// second note for 50 ms
		{
			next_state = measure_4;
		}
		else if(!(counter < 80)) // space state to provide a natural progression
		{
			next_state = space_state;
		}
		else // assign measure 4 encase of error
		{
			next_state = measure_4;
		}
		break;
		
		//----------------- Measure 5 state ----------------------//
		
		case measure_5:
		if(counter < 10)	// second note for 50 ms
		{
			next_state = measure_5;
		}
		else if(!(counter < 10)) // space state to provide a natural progression
		{
			next_state = space_state;
		}
		else // assign measure 3 encase of error
		{
			next_state = measure_5;
		}
		break;
		
		//----------------- Measure 6 state ----------------------//
		
		case measure_6:
		if(counter < 10)	// second note for 50 ms
		{
			next_state = measure_6;
		}
		else if(!(counter < 10)) // space state to provide a natural progression
		{
			next_state = space_state;
		}
		else // assign measure 3 encase of error
		{
			next_state = measure_6;
		}
		break;
		
		//----------------- Measure 7 state ----------------------//
		
		case measure_7:
		if(counter < 10)	// second note for 50 ms
		{
			next_state = measure_7;
		}
		else if(!(counter < 10)) // space state to provide a natural progression
		{
			next_state = space_state;
		}
		else // assign measure 3 encase of error
		{
			next_state = measure_7;
		}
		break;
		
		//----------------- Measure 8 state ----------------------//
		
		case measure_8:
		if(counter < 80)	// second note for 50 ms
		{
			next_state = measure_8;
		}
		else if(!(counter < 80)) // space state to provide a natural progression
		{
			next_state = space_state;
		}
		else // assign measure 4 encase of error
		{
			next_state = measure_8;
		}
		break;
		
		// Space state to provide natural flow to music //
		
		case space_state:
		if(counter_measure > 5)
			{
				if(cur_state == 0x01) // Conditionals to manage current state and next state
					{
						next_state = measure_2;
					}
				else if(cur_state == 0x02)
					{
						next_state = measure_3;
					}
				else if(cur_state == 0x03)
					{
						next_state = measure_4;
					}
				else if(cur_state == 0x04)
					{
						next_state = measure_5;
					}
				else if(cur_state == 0x05)
					{
						next_state = measure_6;
					}
				else if(cur_state == 0x06)
					{
						next_state = measure_7;
					}
				else if(cur_state == 0x07)
					{
						next_state = measure_8;
					}
				else if(cur_state == 0x08 && button_pressed == 0x01)
					{
						next_state = wait_release;
					}
				else
					{
						next_state = init;
					}
			}
		else
		{
			next_state = space_state;
		}
		break; 
		
		// ------ wait release state to ensure button is released ---- //
		case wait_release:
		if(button == 0x01)
		{
			next_state = wait_release;
		}
		else
		{
			next_state = init;
		}
		break;
		
		default:
		next_state = init;
	}
	
	switch (next_state) //action states
	{
		case init:
		input_frequency = 0;
		cur_state = 0x00;
		break;
		
		case wait_start:
		counter_measure = 0x00;
		input_frequency = 0;
		cur_state = 0x00;
		break;
		
		case measure_1:
		counter_measure = 0x00;
		input_frequency = G4;
		if(button == 0x00)
		{
			button_pressed = 0x00;
		}
		else
		{
			button_pressed = 0x01;
		}
		counter++;
		cur_state = 0x01;
		break;
		
		case measure_2:
		counter_measure = 0x00;
		input_frequency = G4;
		if(button == 0x00)
		{
			button_pressed = 0x00;
		}
		else
		{
			button_pressed = 0x01;
		}
		counter++;
		cur_state = 0x02;
		break;
		
		case measure_3:
		counter_measure = 0x00;
		input_frequency = G4;
		if(button == 0x00)
		{
			button_pressed = 0x00;
		}
		else
		{
			button_pressed = 0x01;
		}
		counter++;
		cur_state = 0x03;
		break;
		
		case measure_4:
		counter_measure = 0x00;
		input_frequency = F4;
		if(button == 0x00)
		{
			button_pressed = 0x00;
		}
		else
		{
			button_pressed = 0x01;
		}
		counter++;
		cur_state = 0x04;
		break;
		
		case measure_5:
		counter_measure = 0x00;
		input_frequency = G4;
		if(button == 0x00)
		{
			button_pressed = 0x00;
		}
		else
		{
			button_pressed = 0x01;
		}
		counter++;
		cur_state = 0x05;
		break;
		
		case measure_6:
		counter_measure = 0x00;
		input_frequency = G4;
		if(button == 0x00)
		{
			button_pressed = 0x00;
		}
		else
		{
			button_pressed = 0x01;
		}
		counter++;
		cur_state = 0x06;
		break;
		
		case measure_7:
		counter_measure = 0x00;
		input_frequency = G4;
		if(button == 0x00)
		{
			button_pressed = 0x00;
		}
		else
		{
			button_pressed = 0x01;
		}
		counter++;
		cur_state = 0x07;
		break;
		
		case measure_8:
		counter_measure = 0x00;
		input_frequency = E4;
		if(button == 0x00)
		{
			button_pressed = 0x00;
		}
		else
		{
			button_pressed = 0x01;
		}
		counter++;
		cur_state = 0x08;
		break;
		
		case space_state:
			counter = 0x00;
			counter_measure++;
		if(button == 0x00)
		{
			button_pressed = 0x00;
		}
		else
		{
			button_pressed = 0x01;
		}
		break;
		
		case wait_release:
		if(button == 0x00)
		{
			button_pressed = 0x00;
		}
		else
		{
			button_pressed = 0x01;
		}
		break;
		
		default:
		input_frequency = 0;
		break;
	}
}

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF; // define port A as inputs
	DDRB = 0xFF; PORTB = 0x00; // using B3 (0x08) as output to speaker
	
	PWM_on();
	TimerSet(15);
	TimerOn(); 
	
	next_state = init;
	button_pressed = 0x00;

	while (1)
	{
		button = ~PINA & 0x01;
		
		tick();
		set_PWM(input_frequency);
		
		while (!TimerFlag);
		TimerFlag = 0;
	}
}


