/*
 * Lab 7 part 1.c
 *
 * Created: 4/25/2019 4:32:32 PM
 * Author : User
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "io.c"

enum State_Machine {initalize, wait_Next_state, decrement, increment, wait_release_1, wait_release_2, double_press} Next_state;
unsigned char temp_a;
unsigned char temp_c;

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
void Tick_Next_state()
{
	switch(Next_state)
	{
		//case 1
		case initalize:
		temp_c = 0x07;
		Next_state = wait_Next_state;
		break;
		
		//case 2
		case wait_Next_state:
		temp_c = temp_c;
		if(temp_a == 0x01 && temp_c < 0x09)
		{
			Next_state = increment;
		}
		else if(temp_a == 0x02 && temp_c > 0x00)
		{
			Next_state = decrement;
		}
		else if(temp_a == 0x03)
		{
			Next_state = double_press;
		}
		else
		{
			Next_state = wait_Next_state;
		}
		break;
		
		//case 3
		case decrement:
		temp_c--;
		if(temp_a == 0x03)
		{
			Next_state = double_press;
		}
		else
		{
			Next_state = wait_release_2;
		}
		break;
		
		//case 4
		case increment:
		temp_c++;
		if(temp_a == 0x03)
		{
			Next_state = double_press;
		}
		else
		{
			Next_state = wait_release_1;
		}
		break;
		
		//case 5
		case wait_release_1:
		temp_c = temp_c;
		if(temp_a == 0x03)
		{
			Next_state = double_press;
		}
		else if(temp_a == 0x01)
		{
			Next_state = wait_release_1;
		}
		else
		{
			Next_state = wait_Next_state;
		}
		break;
		
		//case 6
		case wait_release_2:
		temp_c = temp_c;
		if(temp_a == 0x03)
		{
			Next_state = double_press;
		}
		else if(temp_a == 0x02)
		{
			Next_state = wait_release_2;
		}
		else
		{
			Next_state = wait_Next_state;
		}
		break;
		
		//case 7
		case double_press:
		temp_c = 0x00;
		Next_state = wait_Next_state;
		break;
		
		default:
		Next_state = initalize;
		break;
	}
}


int main(void)

{
	 DDRA = 0x00; PORTA = 0xFF; // Configure port A as inputs
	 DDRC = 0xFF; PORTC = 0x00; // LCD data lines
	 DDRD = 0xFF; PORTD = 0x00; // LCD control lines
    /* Replace with your application code */
	
	LCD_WriteData( 0 + '0' );
    while (1) 
    {
    }
}

