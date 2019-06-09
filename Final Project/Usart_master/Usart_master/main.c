#include <avr/io.h>
#include "timer.h"
#include "scheduler.h"
#include "usart.h"

// Global Variables
unsigned char input = 0;

enum B_states {B_Init, B_Blink} B_state;

int TickFct_B(int state) {
	// Local variables
	static unsigned char count = 0;

	switch(state) { // Transitions
		case B_Init:
		state = B_Blink;
		break;
		case B_Blink:
		state = B_Blink;
		break;
		default:
		state = B_Init;
		break;
	}

	switch(state) { // Actions
		case B_Init:
		input = 0;
		count = 0;
		break;
		case B_Blink:
		if (count < 10) {
			count++;
			} else {
			count = 0;
			input = ~input & 0x01;
		}
		break;
		default:
		break;
	}

	return state;
}

enum L_states {L_Init, L_Send} F_state;

int TickFct_L(int state) {
	switch(state) { // Transitions
		case L_Init:
		state = L_Send;
		break;
		case L_Send:
		state = L_Send;
		break;
		default:
		state = L_Init;
		break;
	}

	switch(state) { // Actions
		case L_Init:
		PORTB = 0;
		break;
		case L_Send:
		PORTB = input;
		if(USART_IsSendReady(0))
		USART_Send(0x06, 0);
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

	const unsigned short numTasks = 2;
	task tasks[numTasks];
	unsigned char i = 0;

	// Initialize tasks
	tasks[i].state = 0;
	tasks[i].period = 10;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = &TickFct_B;

	i++;
	tasks[i].state = 0;
	tasks[i].period = 10;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = &TickFct_L;


	TimerSet(10);
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
			tasks[i].elapsedTime += 100;
		}
		while(!TimerFlag);
		TimerFlag = 0;
	}
}