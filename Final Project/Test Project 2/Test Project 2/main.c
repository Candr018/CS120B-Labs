#include <avr/io.h>
#include <avr/interrupt.h>

// ---------------------------------------------------- Global Var definitions --------------------------------------------------------------------------------------------//
enum Piano {init, first_octave, second_octave, thrid_octave, fourth_octave, fifth_octave, sixth_octave, seventh_octave} next_octave;

// First Octave
double C1 = 32.70; 
double D1 = 36.70;
double E1 = 41.20;
double F1 = 43.65;
double G1 = 48.99;
double A1 = 55.00;
double B1 = 61.74;

// Second Octave
double C2 = 65.41; // low C
double D2 = 73.42;
double E2 = 82.41;
double F2 = 87.30;
double G2 = 97.99;
double A2 = 110.00;
double B2 = 123.47;

// Third Octave
double C3 = 130.81;
double D3 = 146.83;
double E3 = 164.81;
double F3 = 174.61; 
double G3 = 195.99;
double A3 = 220.00;
double B3 = 246.92;

// Fourth (Middle) Octave
double C4 = 261.63; // middle C
double D4 = 293.66;
double E4 = 329.63;
double F4 = 349.23;
double G4 = 392.00;
double A4 = 440.00; // concert pitch
double B4 = 493.88;

//Fifth Octave
double C5 = 523.25;
double D5 = 587.33;
double E5 = 659.26;
double F5 = 698.46;
double G5 = 783.99;
double A5 = 880.00;
double B5 = 987.77;

//Sixth Octave
double C6 = 1046.50; // High C
double D6 = 1174.66;
double E6 = 1318.51;
double F6 = 1396.91; 
double G6 = 1567.98;
double A6 = 1760.00;
double B6 = 1975.53;

//Seventh (Highest) Octave
double C7 = 2093.00;
double D7 = 2349.32;
double E7 = 2637.02;
double F7 = 2793.83;
double G7 = 3135.96;
double A7 = 3520.00;
double B7 = 3951.07;

unsigned char button_up;		// button to move up an octave
unsigned char button_down;		// button to move down an octave
unsigned char time_passed;  // check to see if the buttons are not longer being pressed

double note;     // frequency to be output to the speaker

volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.
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


// synchronous state machine to run the piano
void piano_tick() {
	unsigned char tempB = ~PINB & 0xFF;
	
	switch (next_octave) // State Transitions
	{ 
		case init:
		next_octave = fourth_octave;
		break;
		
		//----------------- Octave 1 Transitions ----------------------//
		
		case first_octave:
		
		if(button_up == 0x01 && button_down == 0x00) // check in the user wants to go up in octave
		{
			if(time_passed == 0x01) // ensure that the buttons have been released to avoid quickly changing octaves
			{
				next_octave = second_octave;				
			}
			else
			{
				next_octave = first_octave;
			}
		}
		
		else
		{
			next_octave = first_octave;
		}
		break;
		
		//----------------- Octave 2 Transitions ----------------------//
		
		case second_octave:
		if(button_up == 0x01 && button_down == 0x00)
		{
			if(time_passed == 0x01) // ensure that the buttons have been released to avoid quickly changing octaves
			{
				next_octave = thrid_octave;		
			}
			else
			{
				next_octave = second_octave;
			}
		}
		
		else if(button_down == 0x01 && button_up == 0x00)
		{
			if(time_passed == 0x01) // ensure that the buttons have been released to avoid quickly changing octaves
			{
				next_octave = first_octave;
			}
			else
			{
				next_octave = second_octave;
			}
		}
		
		else
		{
			next_octave = second_octave;
		}
		break;
			
		//----------------- Octave 3 Transitions ----------------------//
		
		case thrid_octave:
		if(button_up == 0x01 && button_down == 0x00)
		{
			if(time_passed == 0x01) // ensure that the buttons have been released to avoid quickly changing octaves
			{
				next_octave = fourth_octave;
			}
			else
			{
				next_octave = thrid_octave;
			}
		}
		
		else if(button_down == 0x01 && button_up == 0x00)
		{
			if(time_passed == 0x01) // ensure that the buttons have been released to avoid quickly changing octaves
			{
				next_octave = second_octave;
			}
			else
			{
				next_octave = thrid_octave;
			}
		}
		
		else
		{
			next_octave = thrid_octave;
		}
		break;
		
		//----------------- Octave 4 Transitions ----------------------//
		
		case fourth_octave:
		if(button_up == 0x01 && button_down == 0x00)
		{
			if(time_passed == 0x01) // ensure that the buttons have been released to avoid quickly changing octaves
			{
				next_octave = fifth_octave;
			}
			else
			{
				next_octave = fourth_octave;
			}
		}
		
		else if(button_down == 0x01 && button_up == 0x00)
		{
			if(time_passed == 0x01) // ensure that the buttons have been released to avoid quickly changing octaves
			{
				next_octave = thrid_octave;
			}
			else
			{
				next_octave = fourth_octave;
			}
		}
		
		else
		{
			next_octave = fourth_octave;
		}
		break;
		
		//----------------- Octave 5 Transitions ----------------------//
		
		case fifth_octave:
		if(button_up == 0x01 && button_down == 0x00)
		{
			if(time_passed == 0x01) // ensure that the buttons have been released to avoid quickly changing octaves
			{
				next_octave = sixth_octave;
			}
			else
			{
				next_octave = fifth_octave;
			}
		}
		
		else if(button_down == 0x01 && button_up == 0x00)
		{
			if(time_passed == 0x01) // ensure that the buttons have been released to avoid quickly changing octaves
			{
				next_octave = fourth_octave;
			}
			else
			{
				next_octave = fifth_octave;
			}
		}
		
		else
		{
			next_octave = fifth_octave;
		}
		break;
		
		//----------------- Octave 6 Transitions ----------------------//
		
		case sixth_octave:
		if(button_up == 0x01 && button_down == 0x00)
		{
			if(time_passed == 0x01) // ensure that the buttons have been released to avoid quickly changing octaves
			{
				next_octave = seventh_octave;
			}
			else
			{
				next_octave = sixth_octave;
			}
		}
		
		else if(button_down == 0x01 && button_up == 0x00)
		{
			if(time_passed == 0x01) // ensure that the buttons have been released to avoid quickly changing octaves
			{
				next_octave = fifth_octave;
			}
			else
			{
				next_octave = sixth_octave;
			}
		}
		
		else
		{
			next_octave = sixth_octave;
		}
		break;
		
		//----------------- Octave 7 Transitions ----------------------//
		
		case seventh_octave:
		
		if(button_down == 0x01 && button_up == 0x00)
		{
			if(time_passed == 0x01) // ensure that the buttons have been released to avoid quickly changing octaves
			{
				next_octave = sixth_octave;
			}
			else
			{
				next_octave = seventh_octave;
			}
		}
		
		else
		{
			next_octave = second_octave;
		}
		break;
	}
			
	switch (next_octave) // State Actions
	{
		case init:
		note = 0;
		break;
		
		// -------- First Octave Actions ----------- //
		case first_octave:
		if(tempB == 0x01)
			{
				note = C1; // note 1
			}
		else if(tempB == 0x02)
			{
				note = D1; // note 2
			}
		else if(tempB == 0x04)
			{
				note = E1; // note 3
			}
		else if(tempB == 0x08)
			{
				note = F1;  // note 4
			}
		else if(tempB == 0x10)
			{
				note = G1;  // note 5
			}
		else if(tempB == 0x20)
			{
				note = A1;  // note 6
			}
		else if(tempB == 0x40)
			{
				note = B1;  // note 7
			}
		else
			{
				note = 0.0; 
			}
		break;
		
		case second_octave:
		if(tempB == 0x01)
		{
			note = C2; // note 1
		}
		else if(tempB == 0x02)
		{
			note = D2; // note 2
		}
		else if(tempB == 0x04)
		{
			note = E2; // note 3
		}
		else if(tempB == 0x08)
		{
			note = F2;  // note 4
		}
		else if(tempB == 0x10)
		{
			note = G2;  // note 5
		}
		else if(tempB == 0x20)
		{
			note = A2;  // note 6
		}
		else if(tempB == 0x40)
		{
			note = B2;  // note 7
		}
		else
		{
			note = 0.0;
		}
		break;
		
		case thrid_octave:
		if(tempB == 0x01)
		{
			note = C3; // note 1
		}
		else if(tempB == 0x02)
		{
			note = D3; // note 2
		}
		else if(tempB == 0x04)
		{
			note = E3; // note 3
		}
		else if(tempB == 0x08)
		{
			note = F3;  // note 4
		}
		else if(tempB == 0x10)
		{
			note = G3;  // note 5
		}
		else if(tempB == 0x20)
		{
			note = A3;  // note 6
		}
		else if(tempB == 0x40)
		{
			note = B3;  // note 7
		}
		else
		{
			note = 0.0;
		}
		break;
		
		case fourth_octave:
		if(tempB == 0x01)
		{
			note = C4; // note 1
		}
		else if(tempB == 0x02)
		{
			note = D4; // note 2
		}
		else if(tempB == 0x04)
		{
			note = E4; // note 3
		}
		else if(tempB == 0x08)
		{
			note = F4;  // note 4
		}
		else if(tempB == 0x10)
		{
			note = G4;  // note 5
		}
		else if(tempB == 0x20)
		{
			note = A4;  // note 6
		}
		else if(tempB == 0x40)
		{
			note = B4;  // note 7
		}
		else
		{
			note = 0.0;
		}
		break;
		
		case fifth_octave:
		if(tempB == 0x01)
		{
			note = C5; // note 1
		}
		else if(tempB == 0x02)
		{
			note = D5; // note 2
		}
		else if(tempB == 0x04)
		{
			note = E5; // note 3
		}
		else if(tempB == 0x08)
		{
			note = F5;  // note 4
		}
		else if(tempB == 0x10)
		{
			note = G5;  // note 5
		}
		else if(tempB == 0x20)
		{
			note = A5;  // note 6
		}
		else if(tempB == 0x40)
		{
			note = B5;  // note 7
		}
		else
		{
			note = 0.0;
		}
		break;
		
		case sixth_octave:
		if(tempB == 0x01)
		{
			note = C6; // note 1
		}
		else if(tempB == 0x02)
		{
			note = D6; // note 2
		}
		else if(tempB == 0x04)
		{
			note = E6; // note 3
		}
		else if(tempB == 0x08)
		{
			note = F6;  // note 4
		}
		else if(tempB == 0x10)
		{
			note = G6;  // note 5
		}
		else if(tempB == 0x20)
		{
			note = A6;  // note 6
		}
		else if(tempB == 0x40)
		{
			note = B6;  // note 7
		}
		else
		{
			note = 0.0;
		}
		break;
		
		case seventh_octave:
		if(tempB == 0x01)
		{
			note = C7; // note 1
		}
		else if(tempB == 0x02)
		{
			note = D7; // note 2
		}
		else if(tempB == 0x04)
		{
			note = E7; // note 3
		}
		else if(tempB == 0x08)
		{
			note = F7;  // note 4
		}
		else if(tempB == 0x10)
		{
			note = G7;  // note 5
		}
		else if(tempB == 0x20)
		{
			note = A7;  // note 6
		}
		else if(tempB == 0x40)
		{
			note = B7;  // note 7
		}
		else
		{
			note = 0.0;
		}
		break;
	}
}
int main(void)
{
	DDRA = 0x00; PORTA = 0xFF; // user i/o (either joystick or left-right buttons)
	DDRB = 0xFF; PORTB = 0x00; // using B3 (0x08) as output to speaker
	DDRC = 0x00; PORTC = 0xFF; // piano keys
	
	PWM_on();
	TimerSet(2);
	TimerOn(); 
	
	next_octave = init;
	unsigned char cooldown = 0;

	while(1)
	{
		unsigned char tempA = ~PINA & 0x03;
		
		if(cooldown == 0)
		{
			if(tempA == 0x01)
			{
				button_up = 0x01;
				time_passed = 0x01;
				cooldown = 50;
			}
			else if(tempA == 0x02)
			{
				button_down = 0x01;
				time_passed = 0x01;
				cooldown = 50;
			}
			else
			{
				button_up = 0;
				button_down = 0;
			}
		}
		else
		{
			cooldown = cooldown - 1;
			time_passed = 0x00;
		}
		piano_tick();
		set_PWM(note);
		TimerFlag = 0;
	}
}