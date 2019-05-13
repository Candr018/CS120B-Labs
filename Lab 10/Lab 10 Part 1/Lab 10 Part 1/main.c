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

enum TLED_States {init, b0, b1, b2} TLED_state;
	
unsigned char TtempB = 0x00;	
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


int main(void)
{
    DDRB = 0xFF; PORTB = 0x00;
	unsigned long TLED_time = 0;
	unsigned long BLED_time = 0;
	TimerSet(100);
	TimerOn();
	BLED_state = binit;
	TLED_state = init;
    while (1) 
    {
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
		while(!TimerFlag);
		TimerFlag = 0;
		TLED_time += 100;
		BLED_time += 100;
		
    }
}

