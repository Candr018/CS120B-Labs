#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <avr/eeprom.h>
#include "io.c"
#include "timer.h"
#include "scheduler.h"

// 0.954 Hz is lowest frequency possible with this function,
// based on settings in PWM_on()
// Passing in 0 as the frequency will stop the speaker from generating sound
void set_PWM(double frequency) {
	static double current_frequency; // Keeps track of the currently set frequency
	// Will only update the registers when the frequency changes, otherwise allows
	// music to play uninterrupted.
	if (frequency != current_frequency) {
		if (!frequency) { TCCR3B &= 0x08; } //stops timer/counter
		else { TCCR3B |= 0x03; } // resumes/continues timer/counter
		// prevents OCR3A from overflowing, using prescaler 64
		// 0.954 is smallest frequency that will not result in overflow
		if (frequency < 0.954) { OCR3A = 0xFFFF; }
		// prevents OCR0A from underflowing, using prescaler 64					
		// 31250 is largest frequency that will not result in underflow
		else if (frequency > 31250) { OCR3A = 0x0000; }
		// set OCR3A based on desired frequency
		else { OCR3A = (short)(8000000 / (128 * frequency)) - 1; }
		TCNT3 = 0; // resets counter
		current_frequency = frequency; // Updates the current frequency
	}
}

void PWM_on() {
	TCCR3A = (1 << COM3A0);
	// COM3A0: Toggle PB6 on compare match between counter and OCR0A
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
	// WGM02: When counter (TCNT0) matches OCR0A, reset counter
	// CS01 & CS30: Set a prescaler of 64
	set_PWM(0);
}

void PWM_off() {
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}

// ------- FUNCTION TO BUILD CUSTOM CHARACTERS ON THE LCD -------- // 
void LCD_Custom_Char (unsigned char loc, unsigned char *msg)
{
	unsigned char i;
	if(loc<8)
	{
		LCD_WriteCommand(0x40 + (loc*8));	/* Command 0x40 and onwards forces the device to point CGRAM address */
		for(i=0;i<8;i++)	/* Write 8 byte for generation of 1 character */
		LCD_WriteData(msg[i]);
	}
}


// --------------------------- Piano Octave Definitions -------------------------- //
// First (Lowest) Octave
double C1 = 32.70;
double D1 = 36.70;
double E1 = 41.20;
double F1 = 43.65;
double G1 = 48.99;
double A1 = 55.00;
double B1 = 61.74;

// Second Octave
double C2 = 65.41; // low C
double D2 = 73.42;
double E2 = 82.41;
double F2 = 87.30;
double G2 = 97.99;
double A2 = 110.00;
double B2 = 123.47;

// Third Octave
double C3 = 130.81;
double D3 = 146.83;
double E3 = 164.81;
double F3 = 174.61;
double G3 = 195.99;
double A3 = 220.00;
double B3 = 246.92;

// Fourth (Middle) Octave
double C4 = 261.63; // middle C
double D4 = 293.66;
double E4 = 329.63;
double F4 = 349.23;
double G4 = 392.00;
double A4 = 440.00; // concert pitch
double B4 = 493.88;

//Fifth Octave
double C5 = 523.25;
double D5 = 587.33;
double E5 = 659.26;
double F5 = 698.46;
double G5 = 783.99;
double A5 = 880.00;
double B5 = 987.77;

//Sixth Octave
double C6 = 1046.50; // High C
double D6 = 1174.66;
double E6 = 1318.51;
double F6 = 1396.91;
double G6 = 1567.98;
double A6 = 1760.00;
double B6 = 1975.53;

//Seventh (Highest) Octave
double C7 = 2093.00;
double D7 = 2349.32;
double E7 = 2637.02;
double F7 = 2793.83;
double G7 = 3135.96;
double A7 = 3520.00;
double B7 = 3951.07;

// CUSTOM CHARACTER DEFINTIONS TO BE USED ON THE LCD --- (MAX 8) --- //
unsigned char one_bar[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F};	// 1
unsigned char two_bars[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F};	// 2
unsigned char three_bars[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F};	// 3
unsigned char four_bars[] = { 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1F};	// 4
unsigned char five_bars[] = { 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F};  // 5
unsigned char six_bars[] = { 0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F};	// 6
unsigned char seven_bars[] = { 0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F}; // 7
unsigned char top_bar[] = { 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};	// 8
	


// ----------------------------------------------------------------------------------------------------------------------- //
// ----------------------------------------------- MENU DEFINITIONS ------------------------------------------------------ //
const unsigned char menu_top[15] = {' ','O','C','T','A','V','E',' ','R','E','C','O','R','D',' ',' '};
	// ----------------------------- 1   2   3   4   5   6   7   8   9   10  11  12  13  14  15 -------------------------- //
const unsigned char menu_bot[15] = {' ','P','L','A','Y',' ','V','I','S','U','A','L','I','Z','E',' '};
	// ----------------------------- 1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16  --------------------- //
const unsigned char menu_octave[15] = {' ','1',' ','2',' ','3',' ','4',' ','5',' ','6',' ','7',' ',' '};
	// -------------------------------- 1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16 ------------------- //
	
unsigned char button_left = 0x00;	// to record if the user wants to go left on the menu
unsigned char button_right = 0x00;	// to record if the user wants to go right on the menu ( only left and right are valid directions)
unsigned char button_enter = 0x00;	// If the user selects enter at a given location in the menu
unsigned char button_exit = 0x00;	// If the user wishes to go back in the nested menu
unsigned char blink = 0x00;			// Blink the selection indicator

unsigned char RXFlag = 0;	// record flag to prevent other actions
unsigned char OCTFlag = 0;  // octave flag to prevent other actions
unsigned char cursor_loc = 1; // position of the cursor on the LCD
unsigned char update_screen = 1; // Weather of not the LCD should be updated

unsigned char button_up;		// button to move up an octave
unsigned char button_down;		// button to move down an octave
unsigned char Octave = 0x04;	// The octave currently being played
unsigned char time_passed;  // check to see if the buttons are not longer being pressed

double note;     // frequency to be output to the speaker

enum Piano {init, first_octave, second_octave, thrid_octave, fourth_octave, fifth_octave, sixth_octave, seventh_octave};
enum LCD_MENU{init_menu, change_octave, select_octave, visualization, select_vis, record, select_record, play, select_play};
	

int piano_tick(next_octave) {
	
	unsigned char tempB = ~PINA & 0xFF;
	tempB = tempB >> 1;
	switch (next_octave) // State Transitions
	{
		case init:
		next_octave = first_octave;
		break;
		
		//----------------- Octave 1 Transitions ----------------------//
		
		case first_octave:
		
		if(Octave == 0x01) // check in the user wants to the first
		{
			next_octave = first_octave;
		}
		else if(Octave == 0x02) // check in the user wants to go the second octave
		{
			next_octave = second_octave;
		}
		else if(Octave == 0x03) // check in the user wants to go to the third octave
		{
			next_octave = thrid_octave;
		}
		else if(Octave == 0x04) // check in the user wants to go to the fourth octave
		{
			next_octave = fourth_octave;
		}
		else if(Octave == 0x05) // check in the user wants to go to the fifth octave
		{
			next_octave = fifth_octave;
		}
		else if(Octave == 0x06) // check in the user wants to go to the sixth octave
		{
			next_octave = sixth_octave;
		}
		else if(Octave == 0x07) // check in the user wants to go to the seventh octave
		{
			next_octave = second_octave;
		}
		else                    // Null transition, should never occur
		{
			next_octave = first_octave;
		}
		break;
		
		//----------------- Octave 2 Transitions ----------------------//
		
		case second_octave:
		if(Octave == 0x01) // check in the user wants to the first
		{
			next_octave = first_octave;
		}
		else if(Octave == 0x02) // check in the user wants to go the second octave
		{
			next_octave = second_octave;
		}
		else if(Octave == 0x03) // check in the user wants to go to the third octave
		{
			next_octave = thrid_octave;
		}
		else if(Octave == 0x04) // check in the user wants to go to the fourth octave
		{
			next_octave = fourth_octave;
		}
		else if(Octave == 0x05) // check in the user wants to go to the fifth octave
		{
			next_octave = fifth_octave;
		}
		else if(Octave == 0x06) // check in the user wants to go to the sixth octave
		{
			next_octave = sixth_octave;
		}
		else if(Octave == 0x07) // check in the user wants to go to the seventh octave
		{
			next_octave = second_octave;
		}
		else                    // Null transition, should never occur
		{
			next_octave = second_octave;
		}
		break;
		
		//----------------- Octave 3 Transitions ----------------------//
		
		case thrid_octave:
		if(Octave == 0x01) // check in the user wants to the first
		{
			next_octave = first_octave;
		}
		else if(Octave == 0x02) // check in the user wants to go the second octave
		{
			next_octave = second_octave;
		}
		else if(Octave == 0x03) // check in the user wants to go to the third octave
		{
			next_octave = thrid_octave;
		}
		else if(Octave == 0x04) // check in the user wants to go to the fourth octave
		{
			next_octave = fourth_octave;
		}
		else if(Octave == 0x05) // check in the user wants to go to the fifth octave
		{
			next_octave = fifth_octave;
		}
		else if(Octave == 0x06) // check in the user wants to go to the sixth octave
		{
			next_octave = sixth_octave;
		}
		else if(Octave == 0x07) // check in the user wants to go to the seventh octave
		{
			next_octave = second_octave;
		}
		else                    // Null transition, should never occur
		{
			next_octave = thrid_octave;
		}
		break;
		
		//----------------- Octave 4 Transitions ----------------------//
		
		case fourth_octave:
		if(Octave == 0x01) // check in the user wants to the first
		{
			next_octave = first_octave;
		}
		else if(Octave == 0x02) // check in the user wants to go the second octave
		{
			next_octave = second_octave;
		}
		else if(Octave == 0x03) // check in the user wants to go to the third octave
		{
			next_octave = thrid_octave;
		}
		else if(Octave == 0x04) // check in the user wants to go to the fourth octave
		{
			next_octave = fourth_octave;
		}
		else if(Octave == 0x05) // check in the user wants to go to the fifth octave
		{
			next_octave = fifth_octave;
		}
		else if(Octave == 0x06) // check in the user wants to go to the sixth octave
		{
			next_octave = sixth_octave;
		}
		else if(Octave == 0x07) // check in the user wants to go to the seventh octave
		{
			next_octave = second_octave;
		}
		else                    // Null transition, should never occur
		{
			next_octave = fourth_octave;
		}
		break;
		
		//----------------- Octave 5 Transitions ----------------------//
		
		case fifth_octave:
		if(Octave == 0x01) // check in the user wants to the first
		{
			next_octave = first_octave;
		}
		else if(Octave == 0x02) // check in the user wants to go the second octave
		{
			next_octave = second_octave;
		}
		else if(Octave == 0x03) // check in the user wants to go to the third octave
		{
			next_octave = thrid_octave;
		}
		else if(Octave == 0x04) // check in the user wants to go to the fourth octave
		{
			next_octave = fourth_octave;
		}
		else if(Octave == 0x05) // check in the user wants to go to the fifth octave
		{
			next_octave = fifth_octave;
		}
		else if(Octave == 0x06) // check in the user wants to go to the sixth octave
		{
			next_octave = sixth_octave;
		}
		else if(Octave == 0x07) // check in the user wants to go to the seventh octave
		{
			next_octave = second_octave;
		}
		else                    // Null transition, should never occur
		{
			next_octave = fifth_octave;
		}
		break;
		
		//----------------- Octave 6 Transitions ----------------------//
		
		case sixth_octave:
		if(Octave == 0x01) // check in the user wants to the first
		{
			next_octave = first_octave;
		}
		else if(Octave == 0x02) // check in the user wants to go the second octave
		{
			next_octave = second_octave;
		}
		else if(Octave == 0x03) // check in the user wants to go to the third octave
		{
			next_octave = thrid_octave;
		}
		else if(Octave == 0x04) // check in the user wants to go to the fourth octave
		{
			next_octave = fourth_octave;
		}
		else if(Octave == 0x05) // check in the user wants to go to the fifth octave
		{
			next_octave = fifth_octave;
		}
		else if(Octave == 0x06) // check in the user wants to go to the sixth octave
		{
			next_octave = sixth_octave;
		}
		else if(Octave == 0x07) // check in the user wants to go to the seventh octave
		{
			next_octave = second_octave;
		}
		else                    // Null transition, should never occur
		{
			next_octave = sixth_octave;
		}
		break;
		
		//----------------- Octave 7 Transitions ----------------------//
		
		case seventh_octave:
		
		if(Octave == 0x01) // check in the user wants to the first
		{
			next_octave = first_octave;
		}
		else if(Octave == 0x02) // check in the user wants to go the second octave
		{
			next_octave = second_octave;
		}
		else if(Octave == 0x03) // check in the user wants to go to the third octave
		{
			next_octave = thrid_octave;
		}
		else if(Octave == 0x04) // check in the user wants to go to the fourth octave
		{
			next_octave = fourth_octave;
		}
		else if(Octave == 0x05) // check in the user wants to go to the fifth octave
		{
			next_octave = fifth_octave;
		}
		else if(Octave == 0x06) // check in the user wants to go to the sixth octave
		{
			next_octave = sixth_octave;
		}
		else if(Octave == 0x07) // check in the user wants to go to the seventh octave
		{
			next_octave = second_octave;
		}
		else                    // Null transition, should never occur
		{
			next_octave = seventh_octave;
		}
		break;
	}
	
	switch (next_octave) // State Actions
	{
		case init:
		note = 0;
		break;
		
		// -------- First Octave Actions ----------- //
		case first_octave:
		if(tempB == 0x01)
		{
			note = C1; // note 1
		}
		else if(tempB == 0x02)
		{
			note = D1; // note 2
		}
		else if(tempB == 0x04)
		{
			note = E1; // note 3
		}
		else if(tempB == 0x08)
		{
			note = F1;  // note 4
		}
		else if(tempB == 0x10)
		{
			note = G1;  // note 5
		}
		else if(tempB == 0x20)
		{
			note = A1;  // note 6
		}
		else if(tempB == 0x40)
		{
			note = B1;  // note 7
		}
		else
		{
			note = 0.0;
		}
		break;
		
		case second_octave:
		if(tempB == 0x01)
		{
			note = C2; // note 1
		}
		else if(tempB == 0x02)
		{
			note = D2; // note 2
		}
		else if(tempB == 0x04)
		{
			note = E2; // note 3
		}
		else if(tempB == 0x08)
		{
			note = F2;  // note 4
		}
		else if(tempB == 0x10)
		{
			note = G2;  // note 5
		}
		else if(tempB == 0x20)
		{
			note = A2;  // note 6
		}
		else if(tempB == 0x40)
		{
			note = B2;  // note 7
		}
		else
		{
			note = 0.0;
		}
		break;
		
		case thrid_octave:
		if(tempB == 0x01)
		{
			note = C3; // note 1
		}
		else if(tempB == 0x02)
		{
			note = D3; // note 2
		}
		else if(tempB == 0x04)
		{
			note = E3; // note 3
		}
		else if(tempB == 0x08)
		{
			note = F3;  // note 4
		}
		else if(tempB == 0x10)
		{
			note = G3;  // note 5
		}
		else if(tempB == 0x20)
		{
			note = A3;  // note 6
		}
		else if(tempB == 0x40)
		{
			note = B3;  // note 7
		}
		else
		{
			note = 0.0;
		}
		break;
		
		case fourth_octave:
		if(tempB == 0x01)
		{
			note = C4; // note 1
		}
		else if(tempB == 0x02)
		{
			note = D4; // note 2
		}
		else if(tempB == 0x04)
		{
			note = E4; // note 3
		}
		else if(tempB == 0x08)
		{
			note = F4;  // note 4
		}
		else if(tempB == 0x10)
		{
			note = G4;  // note 5
		}
		else if(tempB == 0x20)
		{
			note = A4;  // note 6
		}
		else if(tempB == 0x40)
		{
			note = B4;  // note 7
		}
		else
		{
			note = 0.0;
		}
		break;
		
		case fifth_octave:
		if(tempB == 0x01)
		{
			note = C5; // note 1
		}
		else if(tempB == 0x02)
		{
			note = D5; // note 2
		}
		else if(tempB == 0x04)
		{
			note = E5; // note 3
		}
		else if(tempB == 0x08)
		{
			note = F5;  // note 4
		}
		else if(tempB == 0x10)
		{
			note = G5;  // note 5
		}
		else if(tempB == 0x20)
		{
			note = A5;  // note 6
		}
		else if(tempB == 0x40)
		{
			note = B5;  // note 7
		}
		else
		{
			note = 0.0;
		}
		break;
		
		case sixth_octave:
		if(tempB == 0x01)
		{
			note = C6; // note 1
		}
		else if(tempB == 0x02)
		{
			note = D6; // note 2
		}
		else if(tempB == 0x04)
		{
			note = E6; // note 3
		}
		else if(tempB == 0x08)
		{
			note = F6;  // note 4
		}
		else if(tempB == 0x10)
		{
			note = G6;  // note 5
		}
		else if(tempB == 0x20)
		{
			note = A6;  // note 6
		}
		else if(tempB == 0x40)
		{
			note = B6;  // note 7
		}
		else
		{
			note = 0.0;
		}
		break;
		
		case seventh_octave:
		if(tempB == 0x01)
		{
			note = C7; // note 1
		}
		else if(tempB == 0x02)
		{
			note = D7; // note 2
		}
		else if(tempB == 0x04)
		{
			note = E7; // note 3
		}
		else if(tempB == 0x08)
		{
			note = F7;  // note 4
		}
		else if(tempB == 0x10)
		{
			note = G7;  // note 5
		}
		else if(tempB == 0x20)
		{
			note = A7;  // note 6
		}
		else if(tempB == 0x40)
		{
			note = B7;  // note 7
		}
		else
		{
			note = 0.0;
		}
		break;
	}
	return next_octave; 
}
		
int LCD_tick(int next_state){
	unsigned char i = 0;
	switch(next_state){ // STATE MACHINE TRANSITIONS

		case init_menu:
		next_state = change_octave;
		break;
		
		case change_octave:
			if(button_enter == 0 && button_right == 1 && button_left == 0)	// Right
				{	// User wants to go right on the menu
					next_state = record;
				}
			else if(button_enter == 1 && button_left == 0 && button_right == 0) // Enter
				{	// User wants to change the octave
					next_state = select_octave;
				}
			else
				{ 
					next_state = change_octave;
				}
		break;

		case record:
			if( button_enter == 0 && button_right == 1 && button_left == 0)	// Right
				{	// User wants to go right on the menu
					next_state = play;
				}
			else if(button_enter == 0 && button_left == 1 && button_right == 0) // Left
				{	// User wants to go left on the menu
					next_state = change_octave;
				}
			else if(button_enter == 1 && button_left == 0 && button_right == 0) // Enter
				{	// User wants to record the present notes
					next_state = select_record;
				}
			else
				{
					next_state = record;
				}
		break;
		
		case play:
			if( button_enter == 0 && button_right == 1 && button_left == 0)	// Right
				{	// User wants to go right on the menu
					next_state = visualization;
				}
			else if(button_enter == 0 && button_left == 1 && button_right == 0) // Left
				{	// User wants to go left on the menu
					next_state = record;
				}
			else if(button_enter == 1 && button_left == 0 && button_right == 0 && RXFlag == 0) // Enter
				{	// User wants to playback what is currently saved on the other ATMEGA
					next_state = select_record;
				}
			else
				{
					next_state = play;
				}
		break;
		
		case visualization:
			if(button_enter == 0 && button_left == 1 && button_right == 0) // Left
				{	// User wants to go left on the menu
					next_state = play;
				}
			else if(button_enter == 1 && button_left == 0 && button_right == 0 && RXFlag == 0) // Enter
				{	// User wants to display the visualizer
					next_state = select_vis;
				}
			else
				{ 
					next_state = visualization;
				}
		break;
		
		case select_octave:
			if(button_exit == 1)	// exit
			{	// User wants to exit the octave state
				next_state = change_octave;
			}
			else
			{
				next_state = select_octave;
			}
			break;

		case select_record:
			if(button_exit == 1)	// exit
			{	// User wants to exit the octave state
				next_state = record;
			}
			else
			{
				next_state = select_record;
			}
			break;
			
		case select_play:
			if(button_exit == 1)	// exit
			{	// User wants to exit the octave state
				next_state = play;
			}
			else
			{
				next_state = select_play;
			}
			break;
			
		case select_vis:
			if(button_exit == 1)	// exit
			{	// User wants to exit the octave state
				next_state = visualization;
			}
			else
			{
				next_state = select_vis;
			}
			break;
	}
	
	switch(next_state){ // STATE MACHINE ACTIONS
		
		case init_menu:
			for(int j = 1; j <= 16; j++)
			{
				LCD_Cursor(j);
				LCD_WriteData(menu_top[j-1]);
			}
			
			LCD_Cursor(16);
			LCD_WriteData(' ');		// Displays the top menu
		break;
		
		case change_octave:
			for(int j = 16; j <= 32; j++)
				{
					LCD_Cursor(j);
					LCD_WriteData(' ');
				}
				
			for(int j = 1; j <= 16; j++)
				{
					LCD_Cursor(j);
					LCD_WriteData(menu_top[j-1]);
				}
				
				LCD_Cursor(16);
				LCD_WriteData(' ');
					
				LCD_Cursor(18);
				for(i=0;i<6;i++)		// This shows the user that he or she is selected change octave
					{
						LCD_WriteData(0);		// Using custom char 0
					}
				LCD_Cursor(0);		
		break;

		case record:
			for(int j = 16; j <= 32; j++)
				{
					LCD_Cursor(j);
					LCD_WriteData(' ');
				}
				
			for(int j = 1; j <= 16; j++)
			{
				LCD_Cursor(j);
				LCD_WriteData(menu_top[j-1]);
			}
			
			LCD_Cursor(16);
			LCD_WriteData(' ');

				
			LCD_Cursor(25);
			for(i=0;i<6;i++)		// This shows the user that he or she is selected change octave
			{
				LCD_WriteData(0);		// Using custom char 0
			}
				LCD_Cursor(0);
			break;
		
		case play:
			for(int j = 16; j <= 32; j++)
				{
					LCD_Cursor(j);
					LCD_WriteData(' ');
				}
				
			for(int j = 1; j <= 16; j++)
			{
				LCD_Cursor(j);
				LCD_WriteData(menu_bot[j-1]);
			}
			LCD_Cursor(16);
			LCD_WriteData(' ');
			
			LCD_Cursor(18);
	
			for(i=0;i<4;i++)		// This shows the user that he or she is selected change octave
			{
				LCD_WriteData(0);		// Using custom char 0
			}
		break;
		
		case visualization:
			for(int j = 16; j <= 32; j++)
				{
					LCD_Cursor(j);
					LCD_WriteData(' ');
				}
				
			for(int j = 1; j <= 16; j++)
			{
				LCD_Cursor(j);
				LCD_WriteData(menu_bot[j-1]);
			}
			LCD_Cursor(16);
			LCD_WriteData(' ');
			
			LCD_Cursor(23);
			
			for(i=0;i<9;i++)		// This shows the user that he or she is selected change octave
			{
				LCD_WriteData(0);		// Using custom char 0
			}
		break;
		
		case select_octave:
			LCD_DisplayString(1, menu_octave);	// TODO
			break;

		case select_record:
			LCD_DisplayString(1, "in record");	// TODO
			break;
			
		case select_play:
			LCD_DisplayString(1, "in play");	// TODO
			break;
			
		case select_vis:
			LCD_DisplayString(1, "in vis");		// TODO
			break;
	}
	return next_state;
}
	int main(void)
	{
		DDRA = 0x00; PORTA = 0xFF; // Configure PORTA as input for the piano keys, initialize to 1s
		DDRB = 0xFF; PORTB = 0x00; // output speaker and LEDS
		DDRC = 0xFF; PORTC = 0x00; // LCD data lines (NOT USED OUTSIDE OF LCD)
		DDRD = 0xC3; PORTD = 0x3C; // LCD control lines and menu control input

		LCD_init();	
		
		LCD_Custom_Char(0, top_bar);	/*  Menu select custom character  */
		LCD_Custom_Char(1, one_bar);	/* Visualization custom character */
		LCD_Custom_Char(2, two_bars);	/* Visualization custom character */
		LCD_Custom_Char(3, three_bars); /* Visualization custom character */
		LCD_Custom_Char(4, four_bars);  /* Visualization custom character */
		LCD_Custom_Char(5, five_bars);  /* Visualization custom character */
		LCD_Custom_Char(6, six_bars);	/* Visualization custom character */
		LCD_Custom_Char(7, seven_bars); /* Visualization custom character */
			
		LCD_WriteCommand(0x80);
		LCD_Cursor(1);
		// Period for the tasks
		unsigned long int LCD_menu_period = 100;
		unsigned long int Piano_period = 10;

		//Calculating GCD
		unsigned long int tmpGCD = 10;

		//Greatest common divisor for all tasks or smallest time unit for tasks.
		unsigned long int GCD = tmpGCD;

		//Recalculate GCD periods for scheduler
		unsigned long int SMTick1_period = LCD_menu_period;
		unsigned long int SMTick2_period = Piano_period;

		//Declare an array of tasks
		static task task1;
		static task task2;
		task *tasks[] = {&task1, &task2}; // FIX FOR NUMBER OF SM
		const unsigned short numTasks =  sizeof(tasks)/sizeof(task*);
		
		// Task 1
		task1.state = 0;//Task initial state.
		task1.period = SMTick1_period;//Task Period.
		task1.elapsedTime = SMTick1_period;//Task current elapsed time.
		task1.TickFct = &LCD_tick;//Function pointer for the tick.
		
		task2.state = 0;//Task initial state.
		task2.period = SMTick2_period;//Task Period.
		task2.elapsedTime = SMTick2_period;//Task current elapsed time.
		task2.TickFct = &piano_tick;//Function pointer for the tick.
		
		// Set the timer and turn it on
		TimerSet(GCD);
		TimerOn();
		
		// Initializes the LCD display
		// Starting at position 1 on the LCD screen, writes Hello World
		LCD_ClearScreen();
		PWM_on();

		unsigned short i; // Scheduler for-loop iterator
		unsigned char tempD; // Global variable set from buttons
		while(1) {
			unsigned char tempD = ~PIND & 0x3C;
			
			// Global User input definitions
			if(tempD == 0x04)
				{
					button_exit = 0;
					button_enter = 0; 
					button_left = 0; 
					button_right = 1;
				}
			else if(tempD == 0x08)
				{
					button_exit = 0;
					button_enter = 0;
					button_left = 1;
					button_right = 0;
				}
			else if(tempD == 0x10)
				{
					button_exit = 0;
					button_enter = 1;
					button_left = 0;
					button_right = 0;
				}
			else if(tempD == 0x20)
				{
					button_exit = 1;
					button_enter = 0;
					button_left = 0;
					button_right = 0;
				}
			else 
				{
					button_exit = 0;
					button_enter = 0;
					button_left = 0;
					button_right = 0;
				}
				
			// ----------------------------------------------------------------------//
			
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
			set_PWM(note);
			while(!TimerFlag);
			TimerFlag = 0;
		}
		return 0;
	}

