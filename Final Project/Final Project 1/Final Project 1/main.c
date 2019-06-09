#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <avr/eeprom.h>
#include "io.c"
#include "timer.h"
#include "scheduler.h"
#include "usart.h"

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

// Bit-access function
unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b) {
	return (b ? x | (0x01 << k) : x & ~(0x01 << k));
}
unsigned char GetBit(unsigned char x, unsigned char k) {
	return ((x & (0x01 << k)) != 0);
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
const unsigned char menu_top[15] = {' ','O','C','T','A','V','E',' ','R','E','C','O','R','D',' '};
// ----------------------------- 1   2   3   4   5   6   7   8   9   10  11  12  13  14  15 -------------------------- //
const unsigned char menu_bot[15] = {' ','P','L','A','Y',' ','V','I','S','U','A','L','I','Z','E'};
// ----------------------------- 1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16  --------------------- //
const unsigned char menu_octave[15] = {' ','1',' ','2',' ','3',' ','4',' ','5',' ','6',' ','7',' '};
// -------------------------------- 1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16 ------------------- //

unsigned char button_left = 0x00;	// to record if the user wants to go left on the menu
unsigned char button_right = 0x00;	// to record if the user wants to go right on the menu ( only left and right are valid directions)
unsigned char button_enter = 0x00;	// If the user selects enter at a given location in the menu
unsigned char button_exit = 0x00;	// If the user wishes to go back in the nested menu
unsigned char force_exit = 0x00;	// Handshake variable for leaving menu
unsigned char force_pexit = 0x00;	// Handshake variable for leaving menu

unsigned char RXFlag = 0;	// record flag to prevent other actions
unsigned char OCTFlag = 0;  // octave flag to prevent other actions
unsigned char PYFlag = 0;  // playback flag to prevent other actions
unsigned char VISFlag = 0;  // vis flag to prevent other actions
unsigned char RXExit = 0; // variable to leave recording state automatically
unsigned char PYExit = 0; // variable to leave playback state automatically
unsigned char over_ride = 0; // Weather of not the LCD should be updated

unsigned char Octave = 0x04;	// The octave currently being played

double note;     // frequency to be output to the speaker
double override_note; // Playback note will override any note that is being played

enum PIANO {init, first_octave, second_octave, thrid_octave, fourth_octave, fifth_octave, sixth_octave, seventh_octave};
enum LCD_MENU{init_menu, change_octave, select_octave, visualization, select_vis, record, select_record, play, select_play};
enum OCT_MENU{init_oct, wait_oflag, oct_1, oct_2, oct_3, oct_4, oct_5, oct_6, oct_7};
enum REC_MENU{init_rec, wait_rxflag, recording, end_recording};
enum PLAY_MENU{init_play, wait_pyflag, playback, end_playback};
enum REC_BLINK{init_blink, wait_rec, ledon, ledoff};
enum LCD_VIS{init_vis, wait_flag, note_0,
	note_C_1, note_C_2, note_C_3, note_C_4, note_C_5, note_C_6, note_C_7,
	note_D_1, note_D_2, note_D_3, note_D_4, note_D_5, note_D_6, note_D_7,
	note_E_1, note_E_2, note_E_3, note_E_4, note_E_5, note_E_6, note_E_7,
	note_F_1, note_F_2, note_F_3, note_F_4, note_F_5, note_F_6, note_F_7,
	note_G_1, note_G_2, note_G_3, note_G_4, note_G_5, note_G_6, note_G_7,
	note_A_1, note_A_2, note_A_3, note_A_4, note_A_5, note_A_6, note_A_7,
note_B_1, note_B_2, note_B_3, note_B_4, note_B_5, note_B_6, note_B_7};

int piano_tick(int next_octave) {
	
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
			next_octave = seventh_octave;
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
			next_octave = seventh_octave;
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
			next_octave = seventh_octave;
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
			next_octave = seventh_octave;
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
			next_octave = seventh_octave;
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
			next_octave = seventh_octave;
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
			next_octave = seventh_octave;
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
		else if(button_enter == 1 && button_left == 0 && button_right == 0) // Enter
		{	// User wants to playback what is currently saved
			next_state = select_play;
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
		if(RXExit == 1)	// exit
		{	// The recording period has ended
			next_state = record;
		}
		else
		{
			next_state = select_record;
		}
		break;
		
		case select_play:
		if(PYExit == 1)	// exit
		{	// The playback period has ended
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
		if(button_exit == 1)
		{
			OCTFlag = 0;
		}
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
		if(RXExit == 1)
		{
			RXFlag = 0;
			force_exit = 0; // THIS MAY NOT GO HERE
			RXExit = 0;
		}
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
		if(PYExit == 1)
		{
			PYFlag = 0;
			force_pexit = 0; // AGAIN NOT SURE
			PYExit = 0;
			over_ride = 0;
		}
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
		LCD_Cursor(0);
		break;
		
		case visualization:
		if(button_exit == 1)
		{
			VISFlag = 0;
		}
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
		LCD_Cursor(0);
		break;
		
		case select_octave:
		LCD_Cursor(0);
		OCTFlag = 1;
		break;

		case select_record:
		LCD_Cursor(0);
		RXFlag = 1;
		break;
		
		case select_play:
		LCD_Cursor(0);
		over_ride = 1;
		PYFlag = 1;
		break;
		
		case select_vis:
		LCD_Cursor(0);
		VISFlag = 1;
		break;
	}
	return next_state;
}
int Vis_tick(int next_vis)
{
	switch(next_vis) // State Transitions
	{
		case init_vis:
		next_vis = wait_flag;
		break;
		
		case wait_flag:
		if(VISFlag == 1)
		{
			next_vis = note_0;
		}
		else  // otherwise wait for the flag to be set
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- No note being played ----------------- //
		case note_0:
		if(VISFlag == 1)
		{
			if(note == 0.0)
			{
				next_vis = note_0;
			}
			else if(note == C1 || note == C2 || note == C3 || note == C4 || note == C5 || note == C6 || note == C7)
			{
				next_vis = note_C_1;
			}
			else if(note == D1 || note == D2 || note == D3 || note == D4 || note == D5 || note == D6 || note == D7)
			{
				next_vis = note_D_1;
			}
			else if(note == E1 || note == E2 || note == E3 || note == E4 || note == E5 || note == E6 || note == E7)
			{
				next_vis = note_E_1;
			}
			else if(note == F1 || note == F2 || note == F3 || note == F4 || note == F5 || note == F6 || note == F7)
			{
				next_vis = note_F_1;
			}
			else if(note == G1 || note == G2 || note == G3 || note == G4 || note == G5 || note == G6 || note == G7)
			{
				next_vis = note_G_1;
			}
			else if(note == A1 || note == A2 || note == A3 || note == A4 || note == A5 || note == A6 || note == A7)
			{
				next_vis = note_A_1;
			}
			else if(note == B1 || note == B2 || note == B3 || note == B4 || note == B5 || note == B6 || note == B7)
			{
				next_vis = note_B_1;
			}
			else
			{
				next_vis = note_0;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		
		// ---------------- first C visualization ----------------- //
		case note_C_1:
		if(VISFlag == 1)
		{
			if(note == C2 || note == C3 || note == C4 || note == C5 || note == C6 || note == C7)
			{
				next_vis = note_C_2;
			}
			else if(note == C1)
			{
				next_vis = note_C_1;
			}
			else
			{
				next_vis = note_0;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- second C visualization ----------------- //
		case note_C_2:
		if(VISFlag == 1)
		{
			if(note == C3 || note == C4 || note == C5 || note == C6 || note == C7)
			{
				next_vis = note_C_3;
			}
			else if(note == C2)
			{
				next_vis = note_C_2;
			}
			else
			{
				next_vis = note_C_1;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- third C visualization ----------------- //
		case note_C_3:
		if(VISFlag == 1)
		{
			if(note == C4 || note == C5 || note == C6 || note == C7)
			{
				next_vis = note_C_4;
			}
			else if(note == C3)
			{
				next_vis = note_C_3;
			}
			else
			{
				next_vis = note_C_2;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- fourth C visualization ----------------- //
		case note_C_4:
		if(VISFlag == 1)
		{
			if(note == C5 || note == C6 || note == C7)
			{
				next_vis = note_C_5;
			}
			else if(note == C4)
			{
				next_vis = note_C_4;
			}
			else
			{
				next_vis = note_C_3;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- fifth C visualization ----------------- //
		case note_C_5:
		if(VISFlag == 1)
		{
			if(note == C6 || note == C7)
			{
				next_vis = note_C_6;
			}
			else if(note == C5)
			{
				next_vis = note_C_5;
			}
			else
			{
				next_vis = note_C_4;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- sixth C visualization ----------------- //
		case note_C_6:
		if(VISFlag == 1)
		{
			if(note == C7)
			{
				next_vis = note_C_7;
			}
			else if(note == C6)
			{
				next_vis = note_C_6;
			}
			else
			{
				next_vis = note_C_5;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- seventh C visualization ----------------- //
		case note_C_7:
		if(VISFlag == 1)
		{
			if(note == C7)
			{
				next_vis = note_C_7;
			}
			else
			{
				next_vis = note_C_6;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		
		// -----------------------------------------------------------------------------------------------------------//
		
		// ---------------- first D visualization ----------------- //
		case note_D_1:
		if(VISFlag == 1)
		{
			if(note == D2 || note == D3 || note == D4 || note == D5 || note == D6 || note == D7)
			{
				next_vis = note_D_2;
			}
			else if(note == D1)
			{
				next_vis = note_D_1;
			}
			else
			{
				next_vis = note_0;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- second D visualization ----------------- //
		case note_D_2:
		if(VISFlag == 1)
		{
			if(note == D3 || note == D4 || note == D5 || note == D6 || note == D7)
			{
				next_vis = note_D_3;
			}
			else if(note == D2)
			{
				next_vis = note_D_2;
			}
			else
			{
				next_vis = note_D_1;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- third D visualization ----------------- //
		case note_D_3:
		if(VISFlag == 1)
		{
			if(note == D4 || note == D5 || note == D6 || note == D7)
			{
				next_vis = note_D_4;
			}
			else if(note == D3)
			{
				next_vis = note_D_3;
			}
			else
			{
				next_vis = note_D_2;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- fourth D visualization ----------------- //
		case note_D_4:
		if(VISFlag == 1)
		{
			if(note == D5 || note == D6 || note == D7)
			{
				next_vis = note_D_5;
			}
			else if(note == D4)
			{
				next_vis = note_D_4;
			}
			else
			{
				next_vis = note_D_3;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- fifth D visualization ----------------- //
		case note_D_5:
		if(VISFlag == 1)
		{
			if(note == D6 || note == D7)
			{
				next_vis = note_D_6;
			}
			else if(note == D5)
			{
				next_vis = note_D_5;
			}
			else
			{
				next_vis = note_D_4;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- sixth D visualization ----------------- //
		case note_D_6:
		if(VISFlag == 1)
		{
			if(note == D7)
			{
				next_vis = note_D_7;
			}
			else if(note == D6)
			{
				next_vis = note_D_6;
			}
			else
			{
				next_vis = note_D_5;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- seventh D visualization ----------------- //
		case note_D_7:
		if(VISFlag == 1)
		{
			if(note == D7)
			{
				next_vis = note_D_7;
			}
			else
			{
				next_vis = note_D_6;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// -------------------------------------------------------------------------------------------------- //
		// ---------------- first E visualization ----------------- //
		case note_E_1:
		if(VISFlag == 1)
		{
			if(note == E2 || note == E3 || note == E4 || note == E5 || note == E6 || note == E7)
			{
				next_vis = note_E_2;
			}
			else if(note == E1)
			{
				next_vis = note_E_1;
			}
			else
			{
				next_vis = note_0;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- second E visualization ----------------- //
		case note_E_2:
		if(VISFlag == 1)
		{
			if(note == E3 || note == E4 || note == E5 || note == E6 || note == E7)
			{
				next_vis = note_E_3;
			}
			else if(note == E2)
			{
				next_vis = note_E_2;
			}
			else
			{
				next_vis = note_E_1;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- third E visualization ----------------- //
		case note_E_3:
		if(VISFlag == 1)
		{
			if(note == E4 || note == E5 || note == E6 || note == E7)
			{
				next_vis = note_E_4;
			}
			else if(note == E3)
			{
				next_vis = note_E_3;
			}
			else
			{
				next_vis = note_E_2;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- fourth E visualization ----------------- //
		case note_E_4:
		if(VISFlag == 1)
		{
			if(note == E5 || note == E6 || note == E7)
			{
				next_vis = note_E_5;
			}
			else if(note == E4)
			{
				next_vis = note_E_4;
			}
			else
			{
				next_vis = note_E_3;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- fifth E visualization ----------------- //
		case note_E_5:
		if(VISFlag == 1)
		{
			if(note == E6 || note == E7)
			{
				next_vis = note_E_6;
			}
			else if(note == E5)
			{
				next_vis = note_E_5;
			}
			else
			{
				next_vis = note_E_4;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- sixth E visualization ----------------- //
		case note_E_6:
		if(VISFlag == 1)
		{
			if(note == E7)
			{
				next_vis = note_E_7;
			}
			else if(note == E6)
			{
				next_vis = note_E_6;
			}
			else
			{
				next_vis = note_E_5;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- seventh E visualization ----------------- //
		case note_E_7:
		if(VISFlag == 1)
		{
			if(note == E7)
			{
				next_vis = note_E_7;
			}
			else
			{
				next_vis = note_E_6;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		
		//----------------------------------------------------------------------------------------------------------//
		// ---------------- first F visualization ----------------- //
		case note_F_1:
		if(VISFlag == 1)
		{
			if(note == F2 || note == F3 || note == F4 || note == F5 || note == F6 || note == F7)
			{
				next_vis = note_F_2;
			}
			else if(note == F1)
			{
				next_vis = note_F_1;
			}
			else
			{
				next_vis = note_0;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- second F visualization ----------------- //
		case note_F_2:
		if(VISFlag == 1)
		{
			if(note == F3 || note == F4 || note == F5 || note == F6 || note == F7)
			{
				next_vis = note_F_3;
			}
			else if(note == F2)
			{
				next_vis = note_F_2;
			}
			else
			{
				next_vis = note_F_1;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- third F visualization ----------------- //
		case note_F_3:
		if(VISFlag == 1)
		{
			if(note == F4 || note == F5 || note == F6 || note == F7)
			{
				next_vis = note_F_4;
			}
			else if(note == F3)
			{
				next_vis = note_F_3;
			}
			else
			{
				next_vis = note_F_2;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- fourth F visualization ----------------- //
		case note_F_4:
		if(VISFlag == 1)
		{
			if(note == F5 || note == F6 || note == F7)
			{
				next_vis = note_F_5;
			}
			else if(note == F4)
			{
				next_vis = note_F_4;
			}
			else
			{
				next_vis = note_F_3;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- fifth F visualization ----------------- //
		case note_F_5:
		if(VISFlag == 1)
		{
			if(note == F6 || note == F7)
			{
				next_vis = note_F_6;
			}
			else if(note == F5)
			{
				next_vis = note_F_5;
			}
			else
			{
				next_vis = note_F_4;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- sixth F visualization ----------------- //
		case note_F_6:
		if(VISFlag == 1)
		{
			if(note == F7)
			{
				next_vis = note_F_7;
			}
			else if(note == F6)
			{
				next_vis = note_F_6;
			}
			else
			{
				next_vis = note_F_5;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- seventh F visualization ----------------- //
		case note_F_7:
		if(VISFlag == 1)
		{
			if(note == F7)
			{
				next_vis = note_F_7;
			}
			else
			{
				next_vis = note_F_6;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// -------------------------------------------------------------------------------------
		// ---------------- first G visualization ----------------- //
		case note_G_1:
		if(VISFlag == 1)
		{
			if(note == G2 || note == G3 || note == G4 || note == G5 || note == G6 || note == G7)
			{
				next_vis = note_G_2;
			}
			else if(note == G1)
			{
				next_vis = note_G_1;
			}
			else
			{
				next_vis = note_0;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- second G visualization ----------------- //
		case note_G_2:
		if(VISFlag == 1)
		{
			if(note == G3 || note == G4 || note == G5 || note == G6 || note == G7)
			{
				next_vis = note_G_3;
			}
			else if(note == G2)
			{
				next_vis = note_G_2;
			}
			else
			{
				next_vis = note_G_1;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- third G visualization ----------------- //
		case note_G_3:
		if(VISFlag == 1)
		{
			if(note == G4 || note == G5 || note == G6 || note == G7)
			{
				next_vis = note_G_4;
			}
			else if(note == G3)
			{
				next_vis = note_G_3;
			}
			else
			{
				next_vis = note_G_2;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- fourth G visualization ----------------- //
		case note_G_4:
		if(VISFlag == 1)
		{
			if(note == G5 || note == G6 || note == G7)
			{
				next_vis = note_G_5;
			}
			else if(note == G4)
			{
				next_vis = note_G_4;
			}
			else
			{
				next_vis = note_G_3;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- fifth G visualization ----------------- //
		case note_G_5:
		if(VISFlag == 1)
		{
			if(note == G6 || note == G7)
			{
				next_vis = note_G_6;
			}
			else if(note == G5)
			{
				next_vis = note_G_5;
			}
			else
			{
				next_vis = note_G_4;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- sixth G visualization ----------------- //
		case note_G_6:
		if(VISFlag == 1)
		{
			if(note == G7)
			{
				next_vis = note_G_7;
			}
			else if(note == G6)
			{
				next_vis = note_G_6;
			}
			else
			{
				next_vis = note_G_5;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- seventh G visualization ----------------- //
		case note_G_7:
		if(VISFlag == 1)
		{
			if(note == G7)
			{
				next_vis = note_G_7;
			}
			else
			{
				next_vis = note_G_6;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ------------------------------------------------------------------------------------------------------------- //
		// ---------------- first A visualization ----------------- //
		case note_A_1:
		if(VISFlag == 1)
		{
			if(note == A2 || note == A3 || note == A4 || note == A5 || note == A6 || note == A7)
			{
				next_vis = note_A_2;
			}
			else if(note == A1)
			{
				next_vis = note_A_1;
			}
			else
			{
				next_vis = note_0;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- second A visualization ----------------- //
		case note_A_2:
		if(VISFlag == 1)
		{
			if(note == A3 || note == A4 || note == A5 || note == A6 || note == A7)
			{
				next_vis = note_A_3;
			}
			else if(note == A2)
			{
				next_vis = note_A_2;
			}
			else
			{
				next_vis = note_A_1;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- third A visualization ----------------- //
		case note_A_3:
		if(VISFlag == 1)
		{
			if(note == A4 || note == A5 || note == A6 || note == A7)
			{
				next_vis = note_A_4;
			}
			else if(note == A3)
			{
				next_vis = note_A_3;
			}
			else
			{
				next_vis = note_A_2;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- fourth A visualization ----------------- //
		case note_A_4:
		if(VISFlag == 1)
		{
			if(note == A5 || note == A6 || note == A7)
			{
				next_vis = note_A_5;
			}
			else if(note == A4)
			{
				next_vis = note_A_4;
			}
			else
			{
				next_vis = note_A_3;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- fifth A visualization ----------------- //
		case note_A_5:
		if(VISFlag == 1)
		{
			if(note == A6 || note == A7)
			{
				next_vis = note_A_6;
			}
			else if(note == A5)
			{
				next_vis = note_A_5;
			}
			else
			{
				next_vis = note_A_4;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- sixth A visualization ----------------- //
		case note_A_6:
		if(VISFlag == 1)
		{
			if(note == A7)
			{
				next_vis = note_A_7;
			}
			else if(note == A6)
			{
				next_vis = note_A_6;
			}
			else
			{
				next_vis = note_A_5;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- seventh A visualization ----------------- //
		case note_A_7:
		if(VISFlag == 1)
		{
			if(note == A7)
			{
				next_vis = note_A_7;
			}
			else
			{
				next_vis = note_A_6;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		
		// ---------------- first B visualization ----------------- //
		case note_B_1:
		if(VISFlag == 1)
		{
			if(note == B2 || note == B3 || note == B4 || note == B5 || note == B6 || note == B7)
			{
				next_vis = note_B_2;
			}
			else if(note == B1)
			{
				next_vis = note_B_1;
			}
			else
			{
				next_vis = note_0;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- second B visualization ----------------- //
		case note_B_2:
		if(VISFlag == 1)
		{
			if(note == B3 || note == B4 || note == B5 || note == B6 || note == B7)
			{
				next_vis = note_B_3;
			}
			else if(note == B2)
			{
				next_vis = note_B_2;
			}
			else
			{
				next_vis = note_B_1;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- third B visualization ----------------- //
		case note_B_3:
		if(VISFlag == 1)
		{
			if(note == B4 || note == B5 || note == B6 || note == B7)
			{
				next_vis = note_B_4;
			}
			else if(note == B3)
			{
				next_vis = note_B_3;
			}
			else
			{
				next_vis = note_B_2;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- fourth B visualization ----------------- //
		case note_B_4:
		if(VISFlag == 1)
		{
			if(note == B5 || note == B6 || note == B7)
			{
				next_vis = note_B_5;
			}
			else if(note == B4)
			{
				next_vis = note_B_4;
			}
			else
			{
				next_vis = note_B_3;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- fifth B visualization ----------------- //
		case note_B_5:
		if(VISFlag == 1)
		{
			if(note == B6 || note == B7)
			{
				next_vis = note_B_6;
			}
			else if(note == B5)
			{
				next_vis = note_B_5;
			}
			else
			{
				next_vis = note_B_4;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- sixth B visualization ----------------- //
		case note_B_6:
		if(VISFlag == 1)
		{
			if(note == B7)
			{
				next_vis = note_B_7;
			}
			else if(note == B6)
			{
				next_vis = note_B_6;
			}
			else
			{
				next_vis = note_B_5;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		// ---------------- seventh B visualization ----------------- //
		case note_B_7:
		if(VISFlag == 1)
		{
			if(note == B7)
			{
				next_vis = note_B_7;
			}
			else
			{
				next_vis = note_B_6;
			}
		}
		else
		{
			next_vis = wait_flag;
		}
		break;
		default:
		break;
	}
	
	switch(next_vis) // State Actions
	{
		case init_vis:
		break;
		
		case wait_flag:
		break;
		
		case note_0:
		LCD_ClearScreen();
		break;
		
		// -------------------------------------------------------------------------- //
		
		case note_C_1:
		LCD_Cursor(17);
		LCD_WriteData(1);
		LCD_WriteData(1);
		LCD_Cursor(0);
		break;
		
		case note_C_2:
		LCD_Cursor(17);
		LCD_WriteData(2);
		LCD_WriteData(1);
		LCD_WriteData(1);
		LCD_Cursor(0);
		break;
		
		case note_C_3:
		LCD_Cursor(17);
		LCD_WriteData(3);
		LCD_WriteData(2);
		LCD_WriteData(1);
		LCD_WriteData(1);
		LCD_Cursor(0);
		break;
		
		case note_C_4:
		LCD_Cursor(17);
		LCD_WriteData(4);
		LCD_WriteData(3);
		LCD_WriteData(2);
		LCD_WriteData(1);
		LCD_WriteData(1);
		LCD_Cursor(0);
		break;
		
		case note_C_5:
		LCD_Cursor(17);
		LCD_WriteData(5);
		LCD_WriteData(4);
		LCD_WriteData(3);
		LCD_WriteData(2);
		LCD_WriteData(1);
		LCD_WriteData(1);
		LCD_WriteData(1);
		LCD_Cursor(0);
		break;
		
		case note_C_6:
		LCD_Cursor(17);
		LCD_WriteData(6);
		LCD_WriteData(5);
		LCD_WriteData(4);
		LCD_WriteData(3);
		LCD_WriteData(3);
		LCD_WriteData(2);
		LCD_WriteData(2);
		LCD_WriteData(1);
		LCD_WriteData(1);
		LCD_WriteData(1);
		LCD_Cursor(0);
		break;
		
		case note_C_7:
		LCD_Cursor(17);
		LCD_WriteData(7);
		LCD_WriteData(6);
		LCD_WriteData(6);
		LCD_WriteData(5);
		LCD_WriteData(4);
		LCD_WriteData(3);
		LCD_WriteData(2);
		LCD_WriteData(2);
		LCD_WriteData(2);
		LCD_WriteData(1);
		LCD_WriteData(1);
		LCD_WriteData(1);
		LCD_WriteData(1);
		LCD_Cursor(0);
		break;
		
		// ---------------------------------------------------------------------------------- //
		
		case note_D_1:
		LCD_Cursor(18);
		LCD_WriteData(1);
		LCD_WriteData(1);
		LCD_WriteData(1);
		LCD_Cursor(0);
		break;
		
		case note_D_2:
		LCD_Cursor(18);
		LCD_WriteData(1);
		LCD_WriteData(2);
		LCD_WriteData(1);
		LCD_Cursor(0);
		break;
		
		case note_D_3:
		LCD_Cursor(17);
		LCD_WriteData(1);
		LCD_WriteData(2);
		LCD_WriteData(3);
		LCD_WriteData(2);
		LCD_WriteData(1);
		LCD_Cursor(0);
		break;
		
		case note_D_4:
		LCD_Cursor(17);
		LCD_WriteData(2);
		LCD_WriteData(3);
		LCD_WriteData(4);
		LCD_WriteData(3);
		LCD_WriteData(2);
		LCD_WriteData(1);
		LCD_Cursor(0);
		break;
		
		case note_D_5:
		LCD_Cursor(17);
		LCD_WriteData(3);
		LCD_WriteData(4);
		LCD_WriteData(5);
		LCD_WriteData(4);
		LCD_WriteData(3);
		LCD_WriteData(2);
		LCD_WriteData(1);
		LCD_WriteData(1);
		LCD_Cursor(0);
		break;
		
		case note_D_6:
		LCD_Cursor(17);
		LCD_WriteData(4);
		LCD_WriteData(5);
		LCD_WriteData(6);
		LCD_WriteData(5);
		LCD_WriteData(4);
		LCD_WriteData(3);
		LCD_WriteData(2);
		LCD_WriteData(2);
		LCD_WriteData(1);
		LCD_WriteData(1);
		LCD_Cursor(0);
		break;
		
		case note_D_7:
		LCD_Cursor(17);
		LCD_WriteData(5);
		LCD_WriteData(6);
		LCD_WriteData(7);
		LCD_WriteData(6);
		LCD_WriteData(5);
		LCD_WriteData(4);
		LCD_WriteData(3);
		LCD_WriteData(3);
		LCD_WriteData(2);
		LCD_WriteData(2);
		LCD_WriteData(1);
		LCD_WriteData(1);
		LCD_Cursor(0);
		break;
		
		// ---------------------------------------------------------------------------------- //
		
		case note_E_1:
		LCD_Cursor(21);
		LCD_WriteData(1);
		LCD_WriteData(1);
		LCD_WriteData(1);
		LCD_Cursor(0);
		break;
		
		case note_E_2:
		LCD_Cursor(21);
		LCD_WriteData(1);
		LCD_WriteData(2);
		LCD_WriteData(1);
		LCD_Cursor(0);
		break;
		
		case note_E_3:
		LCD_Cursor(20);
		LCD_WriteData(1);
		LCD_WriteData(2);
		LCD_WriteData(3);
		LCD_WriteData(2);
		LCD_WriteData(1);
		LCD_Cursor(0);
		break;
		
		case note_E_4:
		LCD_Cursor(19);
		LCD_WriteData(1);
		LCD_WriteData(2);
		LCD_WriteData(3);
		LCD_WriteData(4);
		LCD_WriteData(3);
		LCD_WriteData(2);
		LCD_WriteData(1);
		LCD_Cursor(0);
		break;
		
		case note_E_5:
		LCD_Cursor(17);
		LCD_WriteData(1);
		LCD_WriteData(1);
		LCD_WriteData(2);
		LCD_WriteData(3);
		LCD_WriteData(4);
		LCD_WriteData(5);
		LCD_WriteData(4);
		LCD_WriteData(3);
		LCD_WriteData(2);
		LCD_WriteData(1);
		LCD_WriteData(1);
		LCD_Cursor(0);
		break;
		
		case note_E_6:
		LCD_Cursor(17);
		LCD_WriteData(2);
		LCD_WriteData(2);
		LCD_WriteData(3);
		LCD_WriteData(4);
		LCD_WriteData(5);
		LCD_WriteData(6); // 22
		LCD_WriteData(5);
		LCD_WriteData(4);
		LCD_WriteData(3);
		LCD_WriteData(2);
		LCD_WriteData(2);
		LCD_WriteData(1);
		LCD_WriteData(1);
		LCD_Cursor(0);
		break;
		
		case note_E_7:
		LCD_Cursor(17);
		LCD_WriteData(3);
		LCD_WriteData(3);
		LCD_WriteData(4);
		LCD_WriteData(5);
		LCD_WriteData(6);
		LCD_WriteData(7);
		LCD_WriteData(6);
		LCD_WriteData(5);
		LCD_WriteData(4);
		LCD_WriteData(3);
		LCD_WriteData(3);
		LCD_WriteData(2);
		LCD_WriteData(2);
		LCD_WriteData(1);
		LCD_WriteData(1);
		LCD_Cursor(0);
		break;
		
		// ---------------------------------------------------------------------------------- //
		
		case note_F_1:
		LCD_Cursor(23);
		LCD_WriteData(1);
		LCD_WriteData(1);
		LCD_WriteData(1);
		LCD_Cursor(0);
		break;
		
		case note_F_2:
		LCD_Cursor(23);
		LCD_WriteData(1);
		LCD_WriteData(2);
		LCD_WriteData(1);
		LCD_Cursor(0);
		break;
		
		case note_F_3:
		LCD_Cursor(22);
		LCD_WriteData(1);
		LCD_WriteData(2);
		LCD_WriteData(3);
		LCD_WriteData(2);
		LCD_WriteData(1);
		LCD_Cursor(0);
		break;
		
		case note_F_4:
		LCD_Cursor(21);
		LCD_WriteData(1);
		LCD_WriteData(2);
		LCD_WriteData(3);
		LCD_WriteData(4);
		LCD_WriteData(3);
		LCD_WriteData(2);
		LCD_WriteData(1);
		LCD_Cursor(0);
		break;
		
		case note_F_5:
		LCD_Cursor(19);
		LCD_WriteData(1);
		LCD_WriteData(1);
		LCD_WriteData(2);
		LCD_WriteData(3);
		LCD_WriteData(4);
		LCD_WriteData(5);
		LCD_WriteData(4);
		LCD_WriteData(3);
		LCD_WriteData(2);
		LCD_WriteData(1);
		LCD_WriteData(1);
		LCD_Cursor(0);
		break;
		
		case note_F_6:
		LCD_Cursor(17);
		LCD_WriteData(1); // 17
		LCD_WriteData(1);
		LCD_WriteData(2); // 19
		LCD_WriteData(2);
		LCD_WriteData(3); // 21
		LCD_WriteData(4);
		LCD_WriteData(5); // 23
		LCD_WriteData(6);
		LCD_WriteData(5); // 25
		LCD_WriteData(4);
		LCD_WriteData(3); // 27
		LCD_WriteData(2);
		LCD_WriteData(2); // 29
		LCD_WriteData(1);
		LCD_WriteData(1); // 31
		LCD_Cursor(0);
		break;
		
		case note_F_7:
		LCD_Cursor(17);
		LCD_WriteData(1);
		LCD_WriteData(2);
		LCD_WriteData(2);
		LCD_WriteData(3);
		LCD_WriteData(3);
		LCD_WriteData(4);
		LCD_WriteData(5);
		LCD_WriteData(6);
		LCD_WriteData(7); // 24
		LCD_WriteData(6);
		LCD_WriteData(5);
		LCD_WriteData(4);
		LCD_WriteData(3);
		LCD_WriteData(3);
		LCD_WriteData(2);
		LCD_WriteData(2);
		LCD_WriteData(1);
		LCD_WriteData(1);
		LCD_Cursor(0);
		break;
		
		// ---------------------------------------------------------------------------------- //
		
		case note_G_1:
		LCD_Cursor(25);
		LCD_WriteData(1);
		LCD_WriteData(1);
		LCD_WriteData(1);
		LCD_Cursor(0);
		break;
		
		case note_G_2:
		LCD_Cursor(25);
		LCD_WriteData(1);
		LCD_WriteData(2);
		LCD_WriteData(1);
		LCD_Cursor(0);
		break;
		
		case note_G_3:
		LCD_Cursor(24);
		LCD_WriteData(1);
		LCD_WriteData(2);
		LCD_WriteData(3);
		LCD_WriteData(2);
		LCD_WriteData(1);
		LCD_Cursor(0);
		break;
		
		case note_G_4:
		LCD_Cursor(23);
		LCD_WriteData(1);
		LCD_WriteData(2);
		LCD_WriteData(3);
		LCD_WriteData(4);
		LCD_WriteData(3);
		LCD_WriteData(2);
		LCD_WriteData(1);
		LCD_Cursor(0);
		break;
		
		case note_G_5:
		LCD_Cursor(21);
		LCD_WriteData(1);
		LCD_WriteData(1);
		LCD_WriteData(2);
		LCD_WriteData(3);
		LCD_WriteData(4);
		LCD_WriteData(5);
		LCD_WriteData(4);
		LCD_WriteData(3);
		LCD_WriteData(2);
		LCD_WriteData(1);
		LCD_WriteData(1);
		LCD_Cursor(0);
		break;
		
		case note_G_6:
		LCD_Cursor(19);
		LCD_WriteData(1);
		LCD_WriteData(1);
		LCD_WriteData(2);
		LCD_WriteData(2);
		LCD_WriteData(3);
		LCD_WriteData(4);
		LCD_WriteData(5);
		LCD_WriteData(6); // 26
		LCD_WriteData(5);
		LCD_WriteData(4);
		LCD_WriteData(3);
		LCD_WriteData(2);
		LCD_WriteData(2);
		LCD_WriteData(1);
		LCD_Cursor(0);
		break;
		
		case note_G_7:
		LCD_Cursor(17);
		LCD_WriteData(1);
		LCD_WriteData(1);
		LCD_WriteData(2);
		LCD_WriteData(2);
		LCD_WriteData(3);
		LCD_WriteData(3);
		LCD_WriteData(4);
		LCD_WriteData(5);
		LCD_WriteData(6);
		LCD_WriteData(7);
		LCD_WriteData(6);
		LCD_WriteData(5);
		LCD_WriteData(4);
		LCD_WriteData(3);
		LCD_WriteData(3);
		LCD_WriteData(2);
		LCD_Cursor(0);
		break;
		
		// ---------------------------------------------------------------------------------- //
		
		case note_A_1:
		LCD_Cursor(28);
		LCD_WriteData(1);
		LCD_WriteData(1);
		LCD_WriteData(1);
		LCD_Cursor(0);
		break;
		
		case note_A_2:
		LCD_Cursor(28);
		LCD_WriteData(1);
		LCD_WriteData(2);
		LCD_WriteData(1);
		LCD_Cursor(0);
		break;
		
		case note_A_3:
		LCD_Cursor(27);
		LCD_WriteData(1);
		LCD_WriteData(2);
		LCD_WriteData(3);
		LCD_WriteData(2);
		LCD_WriteData(1);
		LCD_Cursor(0);
		break;
		
		case note_A_4:
		LCD_Cursor(26);
		LCD_WriteData(1);
		LCD_WriteData(2);
		LCD_WriteData(3);
		LCD_WriteData(4);
		LCD_WriteData(3);
		LCD_WriteData(2);
		LCD_WriteData(1);
		LCD_Cursor(0);
		break;
		
		case note_A_5:
		LCD_Cursor(24);
		LCD_WriteData(1);
		LCD_WriteData(1);
		LCD_WriteData(2);
		LCD_WriteData(3);
		LCD_WriteData(4);
		LCD_WriteData(5); // 29
		LCD_WriteData(4);
		LCD_WriteData(3);
		LCD_WriteData(2);
		LCD_Cursor(0);
		break;
		
		case note_A_6:
		LCD_Cursor(22);
		LCD_WriteData(1);
		LCD_WriteData(1);
		LCD_WriteData(2);
		LCD_WriteData(2);
		LCD_WriteData(3);
		LCD_WriteData(4);
		LCD_WriteData(5);
		LCD_WriteData(6); // 29
		LCD_WriteData(5);
		LCD_WriteData(4);
		LCD_WriteData(3);
		LCD_Cursor(0);
		break;
		
		case note_A_7:
		LCD_Cursor(20);
		LCD_WriteData(1);
		LCD_WriteData(1);
		LCD_WriteData(2);
		LCD_WriteData(2);
		LCD_WriteData(3);
		LCD_WriteData(3);
		LCD_WriteData(4);
		LCD_WriteData(5);
		LCD_WriteData(6);
		LCD_WriteData(7); // 29
		LCD_WriteData(6);
		LCD_WriteData(5);
		LCD_WriteData(4);
		LCD_Cursor(0);
		break;
		
		// ---------------------------------------------------------------------------------- //
		
		case note_B_1:
		LCD_Cursor(31);
		LCD_WriteData(1);
		LCD_WriteData(1);
		LCD_Cursor(0);
		break;
		
		case note_B_2:
		LCD_Cursor(31);
		LCD_WriteData(1);
		LCD_WriteData(2);
		LCD_Cursor(0);
		break;
		
		case note_B_3:
		LCD_Cursor(30);
		LCD_WriteData(1);
		LCD_WriteData(2);
		LCD_WriteData(3);
		LCD_Cursor(0);
		break;
		
		case note_B_4:
		LCD_Cursor(29);
		LCD_WriteData(1);
		LCD_WriteData(2);
		LCD_WriteData(3);
		LCD_WriteData(4);
		LCD_Cursor(0);
		break;
		
		case note_B_5:
		LCD_Cursor(27);
		LCD_WriteData(1);
		LCD_WriteData(1);
		LCD_WriteData(2);
		LCD_WriteData(3);
		LCD_WriteData(4);
		LCD_WriteData(5);
		LCD_Cursor(0);
		break;
		
		case note_B_6:
		LCD_Cursor(25);
		LCD_WriteData(1);
		LCD_WriteData(1);
		LCD_WriteData(2);
		LCD_WriteData(2);
		LCD_WriteData(3);
		LCD_WriteData(4);
		LCD_WriteData(5);
		LCD_WriteData(6);
		LCD_Cursor(0);
		break;
		
		case note_B_7:
		LCD_Cursor(23);
		LCD_WriteData(1);
		LCD_WriteData(1);
		LCD_WriteData(2);
		LCD_WriteData(2);
		LCD_WriteData(3);
		LCD_WriteData(3);
		LCD_WriteData(4);
		LCD_WriteData(5);
		LCD_WriteData(6);
		LCD_WriteData(7);
		LCD_Cursor(0);
		break;
	}
	return next_vis;
}
int Oct_tick(int next_oct)
{
	switch(next_oct)	// State Transtions
	{
		case init_oct:
		next_oct = wait_oflag;
		break;
		
		case wait_oflag:
		if(OCTFlag == 1)
		{
			next_oct = oct_1;
		}
		else
		{
			next_oct = wait_oflag;
		}
		break;
		
		case oct_1:
		if(OCTFlag == 1)
		{
			if(button_enter == 0 && button_right == 1 && button_left == 0)	// Right
			{	// User wants to go right on the menu
				next_oct = oct_2;
			}
			else
			{
				next_oct = oct_1;
			}
		}
		else
		{
			next_oct = wait_oflag;
		}
		break;
		
		case oct_2:
		if(OCTFlag == 1)
		{
			if( button_enter == 0 && button_right == 1 && button_left == 0)	// Right
			{	// User wants to go right on the menu
				next_oct = oct_3;
			}
			else if(button_enter == 0 && button_left == 1 && button_right == 0) // Left
			{	// User wants to go left on the menu
				next_oct = oct_1;
			}
			else
			{
				next_oct = oct_2;
			}
		}
		else
		{
			next_oct = wait_oflag;
		}
		break;
		
		case oct_3:
		if(OCTFlag == 1)
		{
			if( button_enter == 0 && button_right == 1 && button_left == 0)	// Right
			{	// User wants to go right on the menu
				next_oct = oct_4;
			}
			else if(button_enter == 0 && button_left == 1 && button_right == 0) // Left
			{	// User wants to go left on the menu
				next_oct = oct_2;
			}
			else
			{
				next_oct = oct_3;
			}
		}
		else
		{
			next_oct = wait_oflag;
		}
		break;
		
		case oct_4:
		if(OCTFlag == 1)
		{
			if( button_enter == 0 && button_right == 1 && button_left == 0)	// Right
			{	// User wants to go right on the menu
				next_oct = oct_5;
			}
			else if(button_enter == 0 && button_left == 1 && button_right == 0) // Left
			{	// User wants to go left on the menu
				next_oct = oct_3;
			}
			else
			{
				next_oct = oct_4;
			}
		}
		else
		{
			next_oct = wait_oflag;
		}
		break;
		
		case oct_5:
		if(OCTFlag == 1)
		{
			if( button_enter == 0 && button_right == 1 && button_left == 0)	// Right
			{	// User wants to go right on the menu
				next_oct = oct_6;
			}
			else if(button_enter == 0 && button_left == 1 && button_right == 0) // Left
			{	// User wants to go left on the menu
				next_oct = oct_4;
			}
			else
			{
				next_oct = oct_5;
			}
		}
		else
		{
			next_oct = wait_oflag;
		}
		break;
		
		case oct_6:
		if(OCTFlag == 1)
		{
			if( button_enter == 0 && button_right == 1 && button_left == 0)	// Right
			{	// User wants to go right on the menu
				next_oct = oct_7;
			}
			else if(button_enter == 0 && button_left == 1 && button_right == 0) // Left
			{	// User wants to go left on the menu
				next_oct = oct_5;
			}
			else
			{
				next_oct = oct_6;
			}
		}
		else
		{
			next_oct = wait_oflag;
		}
		break;
		
		case oct_7:
		if(OCTFlag == 1)
		{
			if(button_enter == 0 && button_left == 1 && button_right == 0) // Left
			{	// User wants to go left on the menu
				next_oct = oct_6;
			}
			else
			{
				next_oct = oct_7;
			}
		}
		else
		{
			next_oct = wait_oflag;
		}
		break;
		default:
		next_oct = init_oct;
		break;
	}
	
	switch(next_oct)	// State Actions
	{
		case init_oct:
		break;
		
		case wait_oflag:
		break;
		
		case oct_1:
		for(int j = 1; j <= 16; j++)
		{
			LCD_Cursor(j);
			LCD_WriteData(menu_octave[j-1]);
		}
		for(int j = 16; j <= 32; j++)
		{
			LCD_Cursor(j);
			LCD_WriteData(' ');
		}
		LCD_Cursor(16);
		LCD_WriteData(' ');		// Displays the octave menu
		
		LCD_Cursor(18);
		LCD_WriteData(0);
		
		LCD_Cursor(0);
		
		if(button_enter == 1)
		{
			Octave = 0x01;
		}
		break;
		
		case oct_2:
		for(int j = 16; j <= 32; j++)
		{
			LCD_Cursor(j);
			LCD_WriteData(' ');
		}
		LCD_Cursor(20);
		LCD_WriteData(0);
		
		LCD_Cursor(0);
		
		if(button_enter == 1)
		{
			Octave = 0x02;
		}
		break;
		
		case oct_3:
		for(int j = 16; j <= 32; j++)
		{
			LCD_Cursor(j);
			LCD_WriteData(' ');
		}
		LCD_Cursor(22);
		LCD_WriteData(0);
		
		LCD_Cursor(0);
		if(button_enter == 1)
		{
			Octave = 0x03;
		}
		break;
		
		case oct_4:
		for(int j = 16; j <= 32; j++)
		{
			LCD_Cursor(j);
			LCD_WriteData(' ');
		}
		LCD_Cursor(24);
		LCD_WriteData(0);
		
		LCD_Cursor(0);
		if(button_enter == 1)
		{
			Octave = 0x04;
		}
		break;
		
		case oct_5:
		for(int j = 16; j <= 32; j++)
		{
			LCD_Cursor(j);
			LCD_WriteData(' ');
		}
		LCD_Cursor(26);
		LCD_WriteData(0);
		
		LCD_Cursor(0);
		if(button_enter == 1)
		{
			Octave = 0x05;
		}
		break;
		
		case oct_6:
		for(int j = 16; j <= 32; j++)
		{
			LCD_Cursor(j);
			LCD_WriteData(' ');
		}
		LCD_Cursor(28);
		LCD_WriteData(0);
		
		LCD_Cursor(0);
		if(button_enter == 1)
		{
			Octave = 0x06;
		}
		break;
		
		case oct_7:
		for(int j = 16; j <= 32; j++)
		{
			LCD_Cursor(j);
			LCD_WriteData(' ');
		}
		LCD_Cursor(30);
		LCD_WriteData(0);
		
		LCD_Cursor(0);
		if(button_enter == 1)
		{
			Octave = 0x07;
		}
		break;
		
		default:
		break;
	}
	return next_oct;
}
int Rec_tick(int next_rec)
{
	static unsigned int address_val = 0;
	switch (next_rec) // State Actions
	{
		case init_rec:	// initialize the recording state
		next_rec = wait_rxflag;
		break;
		
		case wait_rxflag:	// wait for the flag to be set
		if(RXFlag == 1 && RXExit == 0)
		{
			next_rec = recording;
		}
		else
		{
			next_rec = wait_rxflag;
		}
		break;
		
		case recording:
		
		if (force_exit == 1) // This is the max amount an ATmega can store
		{
			next_rec = end_recording;
		}
		else
		{
			next_rec = recording;
		}
		break;
		
		case end_recording:
		next_rec = wait_rxflag;
		break;
		
		default:
		break;
	}
	switch(next_rec)
	{
		case init_rec:	// initialize the recording state
		address_val = 0;
		break;
		
		case wait_rxflag:	// wait for the flag to be set
		address_val = 0;
		break;
		
		case recording:
		if(address_val == 0)
		{
			LCD_DisplayString(1, "RECORDING");
		}

		if (address_val < 80) // This is the max amount an ATmega can store
		{
			eeprom_write_word((uint16_t *)(address_val*18), (uint16_t)(note));
			address_val++;
		}
		else
		{
			force_exit = 1;
		}
		
		break;
		
		case end_recording:
		LCD_DisplayString(1, "RECORDING OVER");
		RXExit = 1;
		break;
		
		default:
		break;
	}
	return next_rec;
}
int Play_tick(int next_play)
{
	static unsigned char address_valp = 0;
	switch (next_play) // State Actions
	{
		case init_play:	// initialize the recording state
		next_play = wait_pyflag;
		break;
		
		case wait_pyflag:	// wait for the flag to be set
		if(PYFlag == 1 && PYExit == 0)
		{
			next_play = playback;
		}
		else
		{
			next_play = wait_pyflag;
		}
		break;
		
		case playback:
		if (force_pexit == 1) // This is the max amount an ATmega can store
		{
			next_play = end_playback;
		}
		else
		{
			next_play = playback;
		}
		break;
		
		case end_playback:
		next_play = wait_pyflag;
		break;
		
		default:
		break;
	}
	switch(next_play)
	{
		case init_play:	// initialize the recording state
		address_valp = 0;
		break;
		
		case wait_pyflag:	// wait for the flag to be set
		address_valp = 0;
		break;
		
		case playback:
		if(address_valp == 0)
		{
			LCD_DisplayString(1, "PLAYBACK");
		}

		if (address_valp < 80) // This is the max amount an ATmega can store
		{
			override_note = eeprom_read_word((uint16_t *)(address_valp*18));
			address_valp++;
		}
		else
		{
			force_pexit = 1;
		}
		break;
		
		case end_playback:
		LCD_DisplayString(1, "PLAYBACK OVER");
		PYExit = 1;
		break;
		
		default:
		break;
	}
	return next_play;
}
int Blink_tick(int next_blink)
{
	static unsigned tempb = 0x00;
	switch(next_blink)
	{
		case init_blink:
		next_blink = wait_rec;
		break;
		
		case wait_rec:
		if(RXFlag == 1 && RXExit == 0)
		{
			next_blink = ledon;
		}
		else
		{
			next_blink = ledoff;
		}
		break;
		
		case ledon:
		if (force_exit == 1) // This is the max amount an ATmega can store
		{
			next_blink = wait_rec;
		}
		else
		{
			next_blink = ledoff;
		}
		break;
		
		case ledoff:
		if (force_exit == 1) // This is the max amount an ATmega can store
		{
			next_blink = wait_rec;
		}
		else
		{
			next_blink = ledon;
		}
		break;
		
		default:
		break;
	}
	
	switch(next_blink)
	{
		case init_blink:
		tempb = 0x00; 
		break;
		
		case wait_rec:
		tempb = 0x00;
		break;
		
		case ledon:
		tempb = 0x01;
		break;
		
		case ledoff:
		tempb = 0x00;
		break;
		
		default:
		tempb = 0x00;
		break;
}

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF; // Configure PORTA as input for the piano keys, initialize to 1s
	DDRB = 0xFF; PORTB = 0x00; // output speaker and LEDS
	DDRC = 0xFF; PORTC = 0x00; // LCD data lines
	DDRD = 0xC3; PORTD = 0x3C; // LCD control lines and menu control input

	LCD_init();
	initUSART(0);
	
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
	unsigned long int LCD_menu_period = 20;
	unsigned long int Piano_period = 10;
	unsigned long int Visualization_period = 5;
	unsigned long int Octave_menu_period = 10;
	unsigned long int Recording_period = 10;
	unsigned long int Playback_period = 10;

	//Calculating GCD
	unsigned long int tmpGCD = 5;

	//Greatest common divisor for all tasks or smallest time unit for tasks.
	unsigned long int GCD = tmpGCD;

	//Recalculate GCD periods for scheduler
	unsigned long int SMTick1_period = LCD_menu_period;
	unsigned long int SMTick2_period = Piano_period;
	unsigned long int SMTick3_period = Visualization_period;
	unsigned long int SMTick4_period = Octave_menu_period;
	unsigned long int SMTick5_period = Recording_period;
	unsigned long int SMTick6_period = Playback_period;

	//Declare an array of tasks
	static task task1;
	static task task2;
	static task task3;
	static task task4;
	static task task5;
	static task task6;
	task *tasks[] = {&task1, &task2, &task3, &task4, &task5, &task6};
	const unsigned short numTasks =  sizeof(tasks)/sizeof(task*);
	
	// Task 1
	task1.state = 0;//Task initial state.
	task1.period = SMTick1_period;//Task Period.
	task1.elapsedTime = SMTick1_period;//Task current elapsed time.
	task1.TickFct = &LCD_tick;//Function pointer for the tick.
	
	// Task 2
	task2.state = 0;//Task initial state.
	task2.period = SMTick2_period;//Task Period.
	task2.elapsedTime = SMTick2_period;//Task current elapsed time.
	task2.TickFct = &piano_tick;//Function pointer for the tick.
	
	// Task 3
	task3.state = 0;//Task initial state.
	task3.period = SMTick3_period;//Task Period.
	task3.elapsedTime = SMTick3_period;//Task current elapsed time.
	task3.TickFct = &Vis_tick;//Function pointer for the tick.
	
	// Task 4
	task4.state = 0;//Task initial state.
	task4.period = SMTick4_period;//Task Period.
	task4.elapsedTime = SMTick4_period;//Task current elapsed time.
	task4.TickFct = &Oct_tick;//Function pointer for the tick.
	
	// Task 5
	task5.state = 0;//Task initial state.
	task5.period = SMTick5_period;//Task Period.
	task5.elapsedTime = SMTick5_period;//Task current elapsed time.
	task5.TickFct = &Rec_tick;//Function pointer for the tick.
	
	// Task 6
	task6.state = 0;//Task initial state.
	task6.period = SMTick6_period;//Task Period.
	task6.elapsedTime = SMTick6_period;//Task current elapsed time.
	task6.TickFct = &Play_tick;//Function pointer for the tick.
	
	// Set the timer and turn it on
	TimerSet(GCD);
	TimerOn();
	
	// Initializes the LCD display
	// Starting at position 1 on the LCD screen, writes Hello World
	LCD_ClearScreen();
	PWM_on();

	unsigned short i; // Scheduler for-loop iterator
	unsigned char tempD; // local variable set from buttons to determine what buttons are being pressed
	
	while(1) {	// LETS GET IT
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
		
		if(USART_IsSendReady(0)) // Sends the Octave to another micro controller
		{
			USART_Send(Octave, 0);
		}

		// ----------------------------------------------------------------------//
		
		// Scheduler code
		for ( i = 0; i < numTasks; i++ ) {
			// Task is ready to tick
			if ( tasks[i]->elapsedTime >= tasks[i]->period ) {
				LCD_Cursor(0);
				// Setting next state for task
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				LCD_Cursor(0);
				// Reset the elapsed time for next tick.
				tasks[i]->elapsedTime = 0;
				LCD_Cursor(0);
			}
			tasks[i]->elapsedTime += 1;
		}
		if(over_ride == 1)
		{
			set_PWM(override_note);
		}
		else
		{
			set_PWM(note);
		}
		while(!TimerFlag);
		TimerFlag = 0;
	}
	return 0;
}