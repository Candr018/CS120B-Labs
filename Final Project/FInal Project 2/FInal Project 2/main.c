
#include <avr/io.h>
#include "timer.h"
#include "scheduler.h"
#include "usart.h"

// 2nd Atmega64  
unsigned char Octave = 0x04;
enum Received_Octave {Init, Receive};

int TickFct_F(int state) {
	static unsigned char tempB = 0x00; 
	switch(state) { // Transitions
		case Init:
		state = Receive;
		break;
		
		case Receive:
		state = Receive;
		break;
		
		default:
		state = Init;
		break;
	}

	switch(state) { // Actions
		case Init:
		PORTB = 0x00;
		break;
		
		case Receive:
		if(USART_HasReceived(0)) {
			Octave = USART_Receive(0);
			}
				if(Octave == 0x01)
				{
					tempB = 0x01;
				}
				else if(Octave == 0x02)
				{
					tempB = 0x02;
				}
				else if(Octave == 0x03)
				{
					tempB = 0x04;
				}
				else if(Octave == 0x04)
				{
					tempB = 0x08;
				}
				else if(Octave == 0x05)
				{
					tempB = 0x10;
				}
				else if(Octave == 0x06)
				{
					tempB = 0x20;
				}
				else if(Octave == 0x07)
				{
					tempB = 0x30;
				}
				else
				{
					tempB = 0x00; // Should never occur
		}
		break;
		
		default:
		break;
	}
	PORTB = tempB;
	return state;
}


int main(void)
{
	// Set PORTB to output
	DDRB = 0xFF; PORTB = 0x00;

	const unsigned short numTasks = 1;
	task tasks[numTasks];
	unsigned char i = 0;

	// Initialize tasks
	tasks[i].state = 0;
	tasks[i].period = 10;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = &TickFct_F;

	TimerSet(100);
	TimerOn();
	initUSART(0);

	/* Replace with your application code */
	while (1)
	{
		// Scheduler code
		for ( i = 0; i < numTasks; i++ ) {
			// Task is ready to tick
			if ( tasks[i].elapsedTime >= tasks[i].period ) {
				// Setting next state for task
				tasks[i].state = tasks[i].TickFct(tasks[i].state);
				// Reset the elapsed time for next tick.
				tasks[i].elapsedTime = 0;
			}
			tasks[i].elapsedTime += 50;
		}
		while(!TimerFlag);
		TimerFlag = 0;
	}
}