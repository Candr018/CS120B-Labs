/*
 * bnguy097_lab6_part2.cpp
 *
 * Created: 4/23/2019 4:40:39 PM
 * Author : Bryan
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.

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

enum States {init, b0, b1, b2, stop, reset} state;
	
	
unsigned char tempB;

void tick() {
	
	unsigned char button = ~PINA & 0x01;
	
	switch (state) { //transitions
		case init:
			tempB = 0x00;
			state = b0;
			break;
		case b0:
			if (button)
				state = stop;
			else
				state = b1;
			break;
		case b1:
			if (button)
				state = stop;
			else
				state = b2;
			break;
		case b2: 
			if (button)
				state = stop;
			else		
				state = b0;
			break;
		case stop:
			if (button)
				state = stop;
			else
				state = reset;
			break;
		case reset:
			if (button)
				state = init;
			else 
				state = reset;
			break;
		default:
			state = init;
	}
	
	switch (state) { //action
		case init:
			break;
		case b0:
			tempB = 0x01;
			break;
		case b1:
			tempB = 0x02;
			break;
		case b2:
			tempB = 0x04;
			break;
		case stop:
			break;
		case reset:
			break;
		default:
			break;
			
	}
	
	PORTB = tempB;
	
}

int main(void)
{
	DDRB = 0xFF; PORTB = 0x00;
	DDRA = 0x00; PORTA = 0xFF;
	TimerSet(300);
	TimerOn();
 
	
	while(1) {
		tick();
		while(!TimerFlag);
		TimerFlag = 0;
	}
}

