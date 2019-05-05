/*
 * Lab 7 Challenge.c
 *
 * Created: 4/29/2019 6:29:19 PM
 * Author : User
 */ 

/*	Partner 1 Name & E-mail: Chris
 *	Partner 2 Name & E-mail: X
 *	Lab Section: 027
 *	Assignment: Lab #7  Exercise #1
 *	Exercise Description: [optional - include for your own benefit]
 *	
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */

#include <avr/io.h>
#include "io.c"
#include <avr/interrupt.h>

volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.

// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks
unsigned char tmpA = 0x00;
unsigned char tmpC = 0x00;

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
unsigned char if_press;
unsigned char score;

void tick() {
	
	unsigned char button = ~PINA & 0x01;
	
	switch (state) { //transitions
		case (init):
		tempB = 0x00;
		if_press = 0x00;
		state = b0;
		break;
		
		case b0:
		state = button ? stop:b1;
		break;
		
		case b1:
		state = button ? stop:b2;
		break;
		
		case b2:
		state = button ? stop:b0;
		break;
		
		case stop:
		state = button ? stop:reset;
		
		if(if_press == 0x00)
		{	
			if(tempB == 0x02)
			{
				score++;
			}
			else if(tempB == 0x04 || tempB == 0x01)
			{
				score--;
			}
			else
			{
				score = score;
			}
		}
		else
		{
			score = score;
		}
		if_press = 0x01;
		break;
		
		case reset:
		state = button ? init:reset;
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
}

int main(void)
{

	DDRA = 0xFE; PORTA = 0x01;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00; 
	DDRD = 0xFF; PORTD = 0x00;
	
	score = 0x05;
	if_press = 0x00;
	TimerSet(300);
	TimerOn();
	
	LCD_init();	
	LCD_ClearScreen();
	
	state = init;
	while(1){
		LCD_Cursor(1);
		PORTB = tempB;
		if(score > 0 && score < 0x09)
		{
			tick();
			if (score == 0x01)
			{
				LCD_WriteData(1 + '0');
			}
			else if (score == 0x02)
			{
				LCD_WriteData(2 + '0');
			}
			else if (score == 0x03)
			{
				LCD_WriteData(3 + '0');
			}
			else if (score == 0x04)
			{
				LCD_WriteData(4 + '0');
			}
			else if (score == 0x05)
			{
				LCD_WriteData(5 + '0');
			}
			else if (score == 0x06)
			{
				LCD_WriteData(6 + '0');
			}
			else if (score == 0x07)
			{
				LCD_WriteData(7 + '0');
			}
			else
			{
				LCD_WriteData(8 + '0');
			}
		}
		else if(score == 0)
		{
			LCD_DisplayString(1, "GAME OVER!");
		}
		else
		{
			LCD_DisplayString(1, "VICTORY!");
		}
		while(!TimerFlag){}
		TimerFlag = 0;
	}	
}
