

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

		

		// prevents OCR0A from underflowing, using prescaler 64					// 31250 is largest frequency that will not result in underflow

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



//ADC shit

void ADC_init() {

	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);

	// ADEN: setting this bit enables analog-to-digital conversion.

	// ADSC: setting this bit starts the first conversion.

	// ADATE: setting this bit enables auto-triggering. Since we are

	//        in Free Running Mode, a new conversion will trigger whenever

	//        the previous conversion completes.

}

// -------------------------------------------------------------------------------------------------------------- //
unsigned char tmpB = 0x00;
unsigned char cursor_loc = 1;
unsigned char update_screen = 0;
enum SM1_States{SM1_input};
enum SM2_States{SM2_init, SM2_update};
	
int Tick1(int state){
	switch(state){
		case SM1_input:
		

	}
	return state;
}

int Tick2(int state){
	switch(state){
		case SM2_logic:
		switch (x) {
			switch(update_screen) {

				case 0:
				LCD_ClearScreen();
				break;

				case 1:
				LCD_Cursor(24);
				LCD_WriteData(0);
				LCD_WriteData(0);
				break;

				case 2:
				LCD_Cursor(22);
				LCD_WriteData(0);
				LCD_WriteData(0);
				LCD_WriteData(1);
				LCD_WriteData(1);
				LCD_WriteData(0);
				LCD_WriteData(0);
				break;

				case 3:
				LCD_Cursor(20);
				LCD_WriteData(0);
				LCD_WriteData(0);
				LCD_WriteData(1);
				LCD_WriteData(1);
				LCD_WriteData(2);
				LCD_WriteData(2);
				LCD_WriteData(1);
				LCD_WriteData(1);
				LCD_WriteData(0);
				LCD_WriteData(0);
				break;

				case 4:
				LCD_Cursor(18);
				LCD_WriteData(0);
				LCD_WriteData(0);
				LCD_WriteData(1);
				LCD_WriteData(1);
				LCD_WriteData(2);
				LCD_WriteData(2);
				LCD_WriteData(3);
				LCD_WriteData(3);
				LCD_WriteData(2);
				LCD_WriteData(2);
				LCD_WriteData(1);
				LCD_WriteData(1);
				LCD_WriteData(0);
				LCD_WriteData(0);
				break;

				default:
				break;
		}
		state = SM2_logic;
		break;
	}
	return state;
}

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

int main(void)
{
    // Set Data Direction Registers
    // Buttons PORTA[0-7], set AVR PORTA to pull down logic
    DDRA = 0xFF; PORTA = 0x00;
    DDRB = 0xFF; PORTB = 0x00;
    DDRC = 0xF0; PORTC = 0x0F; // PC7..4 outputs init 0s, PC3..0 inputs init 1s
    DDRD = 0xFF; PORTD = 0x00;
	
	PWM_on();
	LCD_init();
	LCD_WriteCommand(0x0C);
	ADC_init();
	
    // Period for the tasks
    unsigned long int SMTick1_calc = 10;
	unsigned long int SMTick2_calc = 5;

    //Calculating GCD
    unsigned long int tmpGCD = 1;

    //Greatest common divisor for all tasks or smallest time unit for tasks.
    unsigned long int GCD = tmpGCD;

    //Recalculate GCD periods for scheduler
    unsigned long int SMTick1_period = SMTick1_calc;

    //Declare an array of tasks
    static task task1;
	static task task2;
    task *tasks[] = {&task1,&task2};
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

    // Task 1
    task1.state = 0;//Task initial state.
    task1.period = SMTick1_period;//Task Period.
    task1.elapsedTime = SMTick1_period;//Task current elapsed time.
    task1.TickFct = &Tick1;//Function pointer for the tick.
    
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

