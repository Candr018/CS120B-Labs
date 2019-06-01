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
unsigned char cursor_loc = 1;
unsigned char x;
unsigned char unique_press = 0;
enum SM1_States{SM1_output};
enum SM2_States{SM2_logic};
	
int SMTick1(int state){
	x = GetKeypadKey();
	switch(state){
		case SM2_logic:
		switch (x) {
			case '\0': break; // All 5 LEDs on
			case '1': tmpB = 0x01;
			case '2': tmpB = 0x02;
			case '3': tmpB = 0x03;
			case '4': tmpB = 0x04;
			case '5': tmpB = 0x05;
			case '6': tmpB = 0x06;
			case '7': tmpB = 0x07;
			case '8': tmpB = 0x08;
			case '9': tmpB = 0x09;
			case 'A': tmpB = 0x0A;
			case 'B': tmpB = 0x0B;
			case 'C': tmpB = 0x0C;
			case 'D': tmpB = 0x0D;
			case '*': tmpB = 0x0E;
			case '0': tmpB = 0x00;
			case '#': tmpB = 0x0F;
			default: tmpB = 0x1B; break; // Should never occur. Middle LED off.
		}
		state = SM2_logic;
		break;
	}
	return state;
}

int SMTick2(int state){
	switch(state){
		case SM1_output:
			if(cursor_loc <= 16)
				{
					switch (x) 
				{
					case '\0': break; // All 5 LEDs on
			
					case '1': 
					LCD_Cursor(cursor_loc);
					LCD_WriteData(tmpB + '0');
					break;
			
					case '2': 
					LCD_Cursor(cursor_loc); 
					LCD_WriteData(tmpB + '0');
					break;
			
					case '3': 
					LCD_Cursor(cursor_loc); 
					LCD_WriteData(tmpB + '0');
					break;
			
					case '4': 
					LCD_Cursor(cursor_loc); 
					LCD_WriteData(tmpB + '0'); 
					break;
			
					case '5': 
					LCD_Cursor(cursor_loc); 
					LCD_WriteData(tmpB + '0'); 
					break;
			
					case '6': 
					LCD_Cursor(cursor_loc); 
					LCD_WriteData(tmpB + '0'); 
					break;
			
					case '7': 
					LCD_Cursor(cursor_loc); 
					LCD_WriteData(tmpB + '0');
					break;
			
					case '8': 
					LCD_Cursor(cursor_loc); 
					LCD_WriteData(tmpB + '0'); 
					break;
			
					case '9': 
					LCD_Cursor(cursor_loc); 
					LCD_WriteData(tmpB + '0'); 
					break;
			
					case 'A': 
					LCD_Cursor(cursor_loc); 
					LCD_WriteData(tmpB + 0x37); 
					break;
			
					case 'B':  
					LCD_Cursor(cursor_loc); 
					LCD_WriteData(tmpB + 0x37); 
					break;
			
					case 'C':  
					LCD_Cursor(cursor_loc); 
					LCD_WriteData(tmpB + 0x37); 
					break;
			
					case 'D':  
					LCD_Cursor(cursor_loc); 
					LCD_WriteData(tmpB + 0x37); 
					break;
			
					case '*':  
					LCD_Cursor(cursor_loc); 
					LCD_WriteData(tmpB + 0x1C); 
					break;
			
					case '0':  
					LCD_Cursor(cursor_loc); 
					LCD_WriteData(tmpB + '0'); 
					break;
			
					case '#':  
					LCD_Cursor(cursor_loc); 
					LCD_WriteData(tmpB + 0x14); 
					break;
			
					default:
					break;
				
					cursor_loc++;
				}
			}
			else
			{
				cursor_loc = 1;
				//if the project is running
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
	unsigned long int SMTick2_period = SMTick1_calc;

	//Declare an array of tasks
	static task task1;
	static task task2;
	task *tasks[] = {&task1,&task2};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	// Task 1
	task1.state = 0;//Task initial state.
	task1.period = SMTick1_period;//Task Period.
	task1.elapsedTime = SMTick1_period;//Task current elapsed time.
	task1.TickFct = &SMTick1;//Function pointer for the tick.

	// Task 2
	task2.state = 0;
	task2.period = SMTick2_period;
	task2.elapsedTime = SMTick2_period;
	task2.TickFct = &SMTick2;
	
	// Set the timer and turn it on
	TimerSet(GCD);
	TimerOn();
	// Initializes the LCD display
	LCD_init();
	// Starting at position 1 on the LCD screen, writes Hello World
	LCD_DisplayString(1,"Congratulations!");
	// ------------------1234567890123456

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
	return 0;
}
