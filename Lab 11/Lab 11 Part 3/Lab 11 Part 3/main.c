/*
 * bnguy097_lab11_part3.c
 *
 * Created: 5/16/2019 5:00:10 PM
 * Author : Bryan
 */ 

#include <avr/io.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include "timer.h"
#include "io.c"
#include "io.h"
#include "keypad.h"
#include "scheduler.h"

unsigned char tmpB = 0x00;

enum SM1_States{SM1_output};

int SMTick1(int state){
	unsigned char x;
	x = GetKeypadKey();
	switch(state){
		case SM1_output:
		switch (x) {
			case '\0': break; // All 5 LEDs on
			case '1': tmpB = 0x01;
			LCD_Cursor(1); LCD_WriteData(tmpB + '0'); break; // hex equivalent
			case '2': tmpB = 0x02;
			LCD_Cursor(1); LCD_WriteData(tmpB + '0'); break;
			case '3': tmpB = 0x03;
			LCD_Cursor(1); LCD_WriteData(tmpB + '0'); break;
			case '4': tmpB = 0x04;
			LCD_Cursor(1); LCD_WriteData(tmpB + '0'); break;
			case '5': tmpB = 0x05;
			LCD_Cursor(1); LCD_WriteData(tmpB + '0'); break;
			case '6': tmpB = 0x06;
			LCD_Cursor(1); LCD_WriteData(tmpB + '0'); break;
			case '7': tmpB = 0x07;
			LCD_Cursor(1); LCD_WriteData(tmpB + '0'); break;
			case '8': tmpB = 0x08;
			LCD_Cursor(1); LCD_WriteData(tmpB + '0'); break;
			case '9': tmpB = 0x09;
			LCD_Cursor(1); LCD_WriteData(tmpB + '0'); break;
			case 'A': tmpB = 0x0A;
			LCD_Cursor(1); LCD_WriteData(tmpB + 0x37); break;
			case 'B': tmpB = 0x0B;
			LCD_Cursor(1); LCD_WriteData(tmpB + 0x37); break;
			case 'C': tmpB = 0x0C;
			LCD_Cursor(1); LCD_WriteData(tmpB + 0x37); break;
			case 'D': tmpB = 0x0D;
			LCD_Cursor(1); LCD_WriteData(tmpB + 0x37); break;
			case '*': tmpB = 0x0E;
			LCD_Cursor(1); LCD_WriteData(tmpB + 0x1C); break;
			case '0': tmpB = 0x00;
			LCD_Cursor(1); LCD_WriteData(tmpB + '0'); break;
			case '#': tmpB = 0x0F;
			LCD_Cursor(1); LCD_WriteData(tmpB + 0x14); break;
			default: tmpB = 0x1B; break; // Should never occur. Middle LED off.
		}
		state = SM1_output;
		PORTB=tmpB;
		break;
	}
	return state;
}

int main(void)
{
	// Set Data Direction Registers
	// Buttons PORTA[0-7], set AVR PORTA to pull down logic
	DDRA = 0xFF; PORTA = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xF0; PORTC = 0x0F; // PC7..4 outputs init 0s, PC3..0 inputs init 1s
	DDRD = 0xFF; PORTD = 0x00;
	// Period for the tasks
	unsigned long int SMTick1_calc = 50;


	//Calculating GCD
	unsigned long int tmpGCD = 1;

	//Greatest common divisor for all tasks or smallest time unit for tasks.
	unsigned long int GCD = tmpGCD;

	//Recalculate GCD periods for scheduler
	unsigned long int SMTick1_period = SMTick1_calc;

	//Declare an array of tasks
	static task task1;
	task *tasks[] = { &task1};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	// Task 1
	task1.state = 0;//Task initial state.
	task1.period = SMTick1_period;//Task Period.
	task1.elapsedTime = SMTick1_period;//Task current elapsed time.
	task1.TickFct = &SMTick1;//Function pointer for the tick.


	// Set the timer and turn it on
	TimerSet(GCD);
	TimerOn();
	LCD_init();

	unsigned short i; // Scheduler for-loop iterator
	while(1) {
		// Scheduler code
		for ( i = 0; i < numTasks; i++ ) {
			// Task is ready to tick
			if ( tasks[i]->elapsedTime >= tasks[i]->period ) {
				// Setting next state for task
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				// Reset the elapsed time for next tick.
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 1;
		}
		while(!TimerFlag);
		TimerFlag = 0;
	}

	// Error: Program should not exit!
	return 0;
}


