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
enum SM1_States{SM1_input, SM1_output};
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
default: break; // Should never occur. Middle LED off.
}
state = SM2_logic;
break;
}
return state;
}
int SMTick2(int state){
switch(state){
case SM1_input:
if (cursor_loc == 1)
{
LCD_Cursor(1);
}
else if (cursor_loc == 2)
{
LCD_Cursor(2);
}
else if (cursor_loc == 3)
{
LCD_Cursor(3);
}
else if (cursor_loc == 4)
{
LCD_Cursor(4);
}
else if (cursor_loc == 5)
{
LCD_Cursor(5);
}
else if (cursor_loc == 6)
{
LCD_Cursor(6);
}
else if (cursor_loc == 7)
{
LCD_Cursor(7);
}
else if (cursor_loc == 8)
{
LCD_Cursor(8);
}
else if (cursor_loc == 9)
{
LCD_Cursor(9);
}
else if (cursor_loc == 10)
{
LCD_Cursor(10);
}
else if (cursor_loc == 11)
{
LCD_Cursor(11);
}
else if (cursor_loc == 12)
{
LCD_Cursor(12);
}
else if (cursor_loc == 13)
{
LCD_Cursor(13);
}
else if (cursor_loc == 14)
{
LCD_Cursor(14);
}
else if (cursor_loc == 15)
{
LCD_Cursor(15);
}
else if (cursor_loc == 16)
{
LCD_Cursor(16);
}
else if (cursor_loc == 17)
{
LCD_Cursor(17);
}
else
{
LCD_Cursor(1);
}
state = SM1_output;
break;

case SM1_output:
switch (x) 
{
case '\0': break; // All 5 LEDs on

case '1': 
LCD_WriteData(1 + '0');
cursor_loc = cursor_loc + 1;
break;

case '2':  
LCD_WriteData(2 + '0');
cursor_loc = cursor_loc + 1;
break;

case '3':  
LCD_WriteData(3 + '0');
cursor_loc = cursor_loc + 1;
break;

case '4': 
LCD_WriteData(4 + '0');
cursor_loc = cursor_loc + 1;	
break;

case '5': 
LCD_WriteData(5 + '0'); 
cursor_loc = cursor_loc + 1;
break;

case '6':  
LCD_WriteData(6 + '0');
cursor_loc = cursor_loc + 1;	
break;

case '7': 
LCD_WriteData(7 + '0');
cursor_loc = cursor_loc + 1;
break;

case '8': 
LCD_WriteData(8 + '0'); 
cursor_loc = cursor_loc + 1;
break;

case '9': 
LCD_WriteData(9 + '0');
cursor_loc = cursor_loc + 1;	
break;

case 'A': 
LCD_WriteData(0x0A + 0x37);
cursor_loc = cursor_loc + 1;
break;

case 'B':  
LCD_WriteData(0x0B + 0x37); 
cursor_loc = cursor_loc + 1;
break;

case 'C':  
LCD_WriteData(0x0C + 0x37);
cursor_loc = cursor_loc + 1;	
break;

case 'D':   
LCD_WriteData(0x0D + 0x37);
cursor_loc = cursor_loc + 1;
break;

case '*':  
LCD_WriteData(0x0E + 0x1C);
cursor_loc = cursor_loc + 1;
break;

case '0':  
LCD_WriteData(0x00 + '0');
cursor_loc = cursor_loc + 1;	
break;

case '#':  
LCD_WriteData(0x0F + 0x14);
cursor_loc = cursor_loc + 1;
break;

default:
break;
}
if(cursor_loc == 17)
{
cursor_loc = 1;
}
else
{
cursor_loc = cursor_loc;
}
state = SM1_input;
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
unsigned long int SMTick1_calc = 100;


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
LCD_DisplayString(1, "Congratulations!");
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
