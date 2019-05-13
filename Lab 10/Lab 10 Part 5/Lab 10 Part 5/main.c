/*
 * bnguy097_lab10_part1.c
 *
 * Created: 5/7/2019 4:09:59 PM
 * Author : Bryan
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.

// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks

unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b) {
	return (b ? x | (0x01 << k) : x & ~(0x01 << k));
}

unsigned char GetBit(unsigned char x, unsigned char k) {
	return ((x & (0x01 << k)) != 0);
}


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

unsigned char TtempB = 0x00;
unsigned char TtempA = 0x00;
unsigned char Oscillator = 0x00;
double input_frequency;
double frequency;
unsigned char button;
unsigned char increment;
unsigned char decrement;
const double C4 = 261.63;
const double D4 = 293.66;
const double E4 = 329.63;
const double F4 = 349.23;
const double G4 = 392.00;
const double A4 = 440.00;
const double B4 = 493.88;
const double C5 = 523.25;

enum TLED_States {init, b0, b1, b2} TLED_state;

void TLED_tick(){
		
		switch (TLED_state) { //transitions
			case init:
				TtempB = 0x00;
				TLED_state = b0;
				break;
			case b0:
				TLED_state = b1;
				break;
			case b1:
				TLED_state = b2;
				break;
			case b2:
				TLED_state = b0;
				break;
			default:
				TLED_state = init;
				break;
		}
		switch (TLED_state) { //action
			case init:
				TtempB = 0x00;
				break;
			case b0:
				TtempB = 0x01;
				break;
			case b1:
				TtempB = 0x02;
				break;
			case b2:
				TtempB = 0x04;
				break;
		}
		PORTB = TtempB | (PORTB & 0x08);
}

enum BLED_States {binit, bon, boff} BLED_state;
	
void BLED_tick(){
	
	switch (BLED_state){
		case binit:
			BLED_state = bon;
			break;
		case bon:
			BLED_state = boff;
			break;
		case boff:
			BLED_state = bon;
			break;
		default:
			BLED_state = binit;
			break;
	}
	
	switch (BLED_state) {
		case binit:
			break;
		case bon:
			TtempB = SetBit(TtempB,3,1);
			break;
		case boff:
			TtempB = SetBit(TtempB,3,0);
			break;
	}
	
	PORTB = TtempB;
}

 enum SPEK_States {sinit, Wait_toggle_2, A_toggled_2} SPEK_state;
	 
 void SPEK_tick()
 
 {		 
		 switch (SPEK_state) // Speaker State Machines Transitions 
		 {
			 case sinit:
				SPEK_state = Wait_toggle_2;
			 break;
			 
			 case Wait_toggle_2:
				if(TtempA == 0x04)
					{
						SPEK_state = A_toggled_2;
					}
				else
					{
						SPEK_state = Wait_toggle_2;
					}
			 break;
			 
			 case A_toggled_2:
				if(TtempA == 0x04)
					{
						SPEK_state = A_toggled_2;
					}
				else
					{
						SPEK_state = Wait_toggle_2;
					}
			 break;
			 
			 default:
				SPEK_state = sinit;
			 break;
		 }
		 
		 switch (SPEK_state)  // Speaker State Machine Actions 
		 {
			 case sinit:
				Oscillator = 1;
				input_frequency = 0;
				frequency = C4;
			 break;
			 
			 case Wait_toggle_2:
				input_frequency = 0;
			 break;
			 
			 case A_toggled_2:
				if(Oscillator)
				{
					input_frequency = frequency;
				}
				else if(!Oscillator)
				{
					input_frequency = 0;
				}
				else
				{
					input_frequency = 0;
				}
				
				if(button == 0)
					{
						if(increment == 1)
							{
								if(input_frequency == C4)
									{
										frequency = D4;
									}
								else if(input_frequency == D4)
									{
										frequency = E4;
									}
								else if(input_frequency == E4)
									{
										frequency = F4;
									}
								else if(input_frequency == E4)
									{
										frequency = G4;
									}
								else if(input_frequency == G4)
									{
										frequency = A4;
									}
								else if(input_frequency == B4)
									{
										 frequency = C5;
									}
								else
									{
										frequency = frequency;
									}
								increment = 0;					 
							}
						else if(decrement == 1)
							{
								if(input_frequency == C5)
									{
										frequency = B4;
									}
								else if(input_frequency == B4)
									{
										frequency = A4;
									}
								else if(input_frequency == A4)
									{
										frequency = G4;
									}
								else if(input_frequency == G4)
									{
										frequency = F4;
									}
								else if(input_frequency == E4)
									{
										frequency = D4;
									}
								else if(input_frequency == D4)
									{
										frequency = C4;
									}
								else
									{
										frequency = frequency;
									}
							}
						else
							{
								frequency = frequency;
							}
						decrement = 0;
									
					}
				Oscillator = !Oscillator;
			 break;
			 
			 default:
				input_frequency = 0;
			 break;
		 }
		 
		 PORTB = TtempB;
}

enum INPUT_States {iinit, Wait_toggle, A_toggled} input_state;

void INPUT_tick()
{
	switch (input_state) // Speaker State Machines Transitions
	{
		case iinit:
			input_state = Wait_toggle;
		break;
	 
		case Wait_toggle:
			if(GetBit(TtempA,4))
			{
				input_state = A_toggled;
			}
			else 
			{
				input_state = Wait_toggle;
			}
		break;
		
		case A_toggled:
			if(GetBit(TtempA,4))
			{
				input_state = A_toggled;
			}
			else
			{
				input_state = Wait_toggle;
			}
			break;
			
			 default:
			 input_state = sinit;
			 break;
		 }
		 
		 switch (input_state)  // Speaker State Machine Actions
		 {
			 case iinit:
			 button = ~PINA & 0x03;
			 break;
			 
			 case Wait_toggle:
			 button = ~PINA & 0x03;
			 break;
			 
			 case A_toggled:
			 button = ~PINA & 0x03;
			 if(button == 0x01)
			 {
					 increment = 1;

			 }
			 else if(button == 0x03)
			 {
					 decrement = 1;
			 }
			 else
			 {
				 increment = increment;
				 decrement = decrement;
			 }
			 break;
			 
			 default:
			 increment = 0;
			 decrement = 0;
			 break;
		 }
	 }
	
int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
    DDRB = 0xFF; PORTB = 0x00;
	unsigned long TLED_time = 0;
	unsigned long BLED_time = 0;
	TimerSet(1);
	TimerOn();
	BLED_state = binit;
	SPEK_state = sinit;
	TLED_state = init;
	input_state = iinit;
    while (1) 
    {
		button = ~PINA & 0x03;
		if (TLED_time >= 300)
		{
			TLED_tick();
			TLED_time = 0;
		}
		if(BLED_time >= 1000)
		{
		BLED_tick();
		BLED_time = 0;
		}
		SPEK_tick();
		INPUT_tick();
		
		TimerFlag = 0;
		TLED_time += 1;
		BLED_time += 1;
		set_PWM(input_frequency);
		while(!TimerFlag);
		
    }
} 


