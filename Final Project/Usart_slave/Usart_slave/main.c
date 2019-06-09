
#include <avr/io.h>
#include "timer.h"
#include "scheduler.h"
#include "usart.h"

// Follower Code

enum F_states {F_Init, F_Receive} F_state;

int TickFct_F(int state) {
	switch(state) { // Transitions
		case F_Init:
		state = F_Receive;
		break;
		case F_Receive:
		state = F_Receive;
		break;
		default:
		state = F_Init;
		break;
	}

	switch(state) { // Actions
		case F_Init:
		PORTB = 0;
		break;
		case F_Receive:
		if(USART_HasReceived(0)) {
			PORTB = USART_Receive(0);
		}
		break;
		default:
		break;
	}

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
	tasks[i].period = 50;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = &TickFct_F;

	TimerSet(50);
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