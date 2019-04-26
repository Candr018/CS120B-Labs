/*
 * Lab 6 part 1.c
 *
 * Created: 4/23/2019 3:11:12 PM
 * Author : User
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.

enum State_Machine {initalize, LED_1, LED_2, LED_3, wait_release, button_press, stop, wait_release_2} LEDs;	//state machine to hold the LEDS
unsigned char temp_b;		// global variable to hold the values of the pins on the ATMEL chip
unsigned char temp_a;	

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
	temp_a = ~PINA & 0x01;
	switch(LEDs)
	{
		// initalize case to ensure PORTB is zero
		case initalize:
		temp_b = 0x00;
		temp_a = 0x00;
		LEDs = LED_1;
		break;
		
		// case to set the first LED
		case LED_1:
		temp_b = 0x01;
		if(temp_a == 0x01)
		{
			LEDs = button_press; 
		}
		else
		{
			LEDs = LED_2;
		}

		break;
		
		// case to set the second LED
		case LED_2:
		temp_b = 0x02;
			if(temp_a == 0x01)
			{
				LEDs = button_press;
			}
			else
			{
				LEDs = LED_3;
			}

		break;
		
		// case to set the third LED 
		case LED_3:
		temp_b = 0x04;
			if(temp_a == 0x01)
			{
				LEDs = button_press;
			}
			else
			{
				LEDs = LED_1;
			}
		break;
		
		case button_press:
			if(temp_a == 0x01)
			{
				LEDs = button_press;
			}
			else
			{
				LEDs = stop;
			}
		break;
		
		case stop:
			if(temp_a == 0x00)
			{
				LEDs = stop;
			}
			else
			{
				LEDs = wait_release_2;
			}
			
		case wait_release_2:
		{
			temp_b = temp_b;
			if(temp_a == 0x01)
			{
				LEDs = wait_release_2;
			}
			else
			{
				LEDs = initalize;
			}
		}
		default:
		LEDs = initalize;
		break; 
	}
}

int main(void)
{
	DDRB = 0xFF; // set port B to outputs
	DDRA = 0x00; // set port A to outputs
    PORTB = 0x00; // Init port B to 0s
	PORTA = 0xFF; // Init port A to 1s
	
	TimerSet(300);
	TimerOn();
	LEDs = initalize;
	while (1) 
		{
			Tick_LEDS();
			PORTB = temp_b;
			while(!TimerFlag);
				TimerFlag = 0;
			
		}
}


