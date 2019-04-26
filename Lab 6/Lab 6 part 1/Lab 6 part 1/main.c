/*
 * Lab 6 part 1.c
 *
 * Created: 4/23/2019 3:11:12 PM
 * Author : User
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.

enum State_Machine {initalize, LED_1, LED_2, LED_3} LEDs;	//state machine to hold the LEDS
unsigned char temp_b;		// global variable to hold the values of the PORTB on ATMEL chip, NOTE: we do not read from the chip		

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
void Tick_LEDS()
{
	switch(LEDs)
	{
		// initalize case to ensure PORTB is zero
		case initalize:
		temp_b = 0x00;
		LEDs = LED_1;
		break;
		
		// case to set the first LED
		case LED_1:
		temp_b = 0x01;
		LEDs = LED_2;
		break;
		
		// case to set the second LED
		case LED_2:
		temp_b = 0x02;
		LEDs = LED_3;
		break;
		
		// case to set the thrid LED 
		case LED_3:
		temp_b = 0x04;
		LEDs = LED_1;
		break;
		
		default:
		LEDs = initalize;
		break; 
	}
}

int main(void)
{
	DDRB = 0xFF; // set port B to outputs
    PORTB = 0x00; // Init port B to 0s
	
	TimerSet(1000);
	TimerOn();
	while (1) 
		{
			Tick_LEDS();
			PORTB = temp_b;
			while(!TimerFlag);
				TimerFlag = 0;
			
		}
}

