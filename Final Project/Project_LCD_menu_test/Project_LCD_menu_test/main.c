#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <avr/eeprom.h>
#include "io.c"
#include "usart.h"

// 0.954 Hz is lowest frequency possible with this function,

// based on settings in PWM_on()

// Passing in 0 as the frequency will stop the speaker from generating sound

volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------//

// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks

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

typedef struct _task {
	/*Tasks should have members that include: state, period,
		a measurement of elapsed time, and a function pointer.*/
	signed char state; //Task's current state
	unsigned long int period; //Task period
	unsigned long int elapsedTime; //Time elapsed since last task tick
	int (*TickFct)(int); //Task tick function
} task;

	unsigned char empty_bar[] = {
		0b00000,
		0b00000,
		0b00000,
		0b00000,
		0b00000,
		0b00000,
		0b00000,
		0b00000
	};

	unsigned char almost_low_bar[] = {
		0b00000,
		0b00000,
		0b00000,
		0b00000,
		0b00000,
		0b00000,
		0b00000,
		0b11111
	};

	unsigned char low_bar[] = {
		0b00000,
		0b00000,
		0b00000,
		0b00000,
		0b00000,
		0b00000,
		0b11111,
		0b11111
	};

	unsigned char almost_med_bar[] = {
		0b00000,
		0b00000,
		0b00000,
		0b00000,
		0b00000,
		0b11111,
		0b11111,
		0b11111
	};

	unsigned char med_bar[] = {
		0b00000,
		0b00000,
		0b00000,
		0b00000,
		0b11111,
		0b11111,
		0b11111,
		0b11111
	};

	unsigned char almost_high_bar[] = {
		0b00000,
		0b00000,
		0b00000,
		0b11111,
		0b11111,
		0b11111,
		0b11111,
		0b11111
	};

	unsigned char high_bar[] = {
		0b00000,
		0b00000,
		0b11111,
		0b11111,
		0b11111,
		0b11111,
		0b11111,
		0b11111
	};

	unsigned char almost_full_bar[] = {
		0b00000,
		0b11111,
		0b11111,
		0b11111,
		0b11111,
		0b11111,
		0b11111,
		0b11111
	};

	unsigned char full_bar[] = {
		0b11111,
		0b11111,
		0b11111,
		0b11111,
		0b11111,
		0b11111,
		0b11111,
		0b11111
	};

	unsigned char top_bar[] = {
		0b11111,
		0b00000,
		0b00000,
		0b00000,
		0b00000,
		0b00000,
		0b00000,
		0b00000
	};
// -------------------------------------------------------------------------------------------------------------- //

const unsigned char menu_top[15] = {' ','O','C','T','A','V','E',' ','R','E','C','O','R','D',' '};
	// ----------------------------- 1   2   3   4   5   6   7   8   9   10  11  12  14  15  16 ------------------- //
const unsigned char menu_bot[15] = {' ','P','L','A','Y','B','A','C','K',' ',' ',' ',' ',' ',' '};
	// ----------------------------- 1   2   3   4   5   6   7   8   9   10  11  12  14  15  16 ------------------- //
unsigned char button_left = 0x00;
unsigned char button_right = 0x00;
unsigned char button_enter = 0x00;
unsigned char button_exit = 0x00;

unsigned char RXFlag = 0;

unsigned char cursor_loc = 1;
unsigned char update_screen = 0;

enum LCD_MENU{init_menu, change_octave, select_octave, visualization, select_vis, record, select_record, playback, select_playback};
enum IO_CTL{init_buttons, user_io};
enum LCD_SEL{init_select, sel_octave, sel_rec, sel_play};

int LCD_tick(int next_state){
	
	switch(next_state){ // STATE MACHINE TRANSITIONS
		
		case init_menu:
		next_state = change_octave;
		break;
		
		case change_octave:
		
		break;

		case visualization:

		break;
		
		case record:
		
		break;
	}
	
	switch(next_state){ // STATE MACHINE ACTIONS
		
		case init_menu:
		LCD_DisplayString(1, menu_top);
		break;
		
		case change_octave:
		
		break;

		case visualization:

		break;
		
		case record:
		
		break;
	}
	return next_state;
}
	int main(void)
	{
		DDRA = 0xFF; PORTA = 0x00;
		DDRB = 0xFF; PORTB = 0x00;
		DDRC = 0xF0; PORTC = 0x0F; 
		DDRD = 0xFF; PORTD = 0x00;

		LCD_init();
		LCD_WriteCommand(0x0C);		
		// Period for the tasks
		unsigned long int LCD_menu_period = 10;

		//Calculating GCD
		unsigned long int tmpGCD = 10;

		//Greatest common divisor for all tasks or smallest time unit for tasks.
		unsigned long int GCD = tmpGCD;

		//Recalculate GCD periods for scheduler
		unsigned long int SMTick1_period = LCD_menu_period;

		//Declare an array of tasks
		static task task1;
		task *tasks[] = {&task1};
		const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

		// Task 1
		task1.state = 0;//Task initial state.
		task1.period = SMTick1_period;//Task Period.
		task1.elapsedTime = SMTick1_period;//Task current elapsed time.
		task1.TickFct = &LCD_MENU;//Function pointer for the tick.
		
		// Set the timer and turn it on
		TimerSet(GCD);
		TimerOn();
		
		// Initializes the LCD display
		// Starting at position 1 on the LCD screen, writes Hello World
		LCD_ClearScreen();
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

