#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <avr/eeprom.h>
#include "io.c"
#include "usart.h"

volatile unsigned char RXFlag = 0;
unsigned char rec_flag = 0;
unsigned char start_rec = 0;
unsigned short addr_val = 0;
double note;
volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.



// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.

unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.

unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks



void TimerOn() {

	// AVR timer/counter controller register TCCR1

	TCCR1B = 0x0B;// bit3 = 0: CTC mode (clear timer on compare)

	// bit2bit1bit0=011: pre-scaler /64

	// 00001011: 0x0B

	// SO, 8 MHz clock or 8,000,000 /64 = 125,000 ticks/s

	// Thus, TCNT1 register will count at 125,000 ticks/s



	// AVR output compare register OCR1A.

	OCR1A = 125;	// Timer interrupt will be generated when TCNT1==OCR1A

	// We want a 1 ms tick. 0.001 s * 125,000 ticks/s = 125

	// So when TCNT1 register equals 125,

	// 1 ms has passed. Thus, we compare to 125.

	// AVR timer interrupt mask register

	TIMSK1 = 0x02; // bit1: OCIE1A -- enables compare match interrupt



	//Initialize avr counter

	TCNT1=0;



	_avr_timer_cntcurr = _avr_timer_M;

	// TimerISR will be called every _avr_timer_cntcurr milliseconds



	//Enable global interrupts

	SREG |= 0x80; // 0x80: 1000000

}



void TimerOff() {

	TCCR1B = 0x00; // bit3bit1bit0=000: timer off

}



void TimerISR() {

	TimerFlag = 1;

}



// In our approach, the C programmer does not touch this ISR, but rather TimerISR()

ISR(TIMER1_COMPA_vect) {

	// CPU automatically calls when TCNT1 == OCR1 (every 1 ms per TimerOn settings)

	_avr_timer_cntcurr--; // Count down to 0 rather than up to TOP

	if (_avr_timer_cntcurr == 0) { // results in a more efficient compare

		TimerISR(); // Call the ISR that the user uses

		_avr_timer_cntcurr = _avr_timer_M;

	}

}



// Set TimerISR() to tick every M ms

void TimerSet(unsigned long M) {

	_avr_timer_M = M;

	_avr_timer_cntcurr = _avr_timer_M;

}



//PWM shit



// 0.954 hz is lowest frequency possible with this function,

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

enum recording{wait_input, recording, playback} rec_state;
	
void RxISR() {
	RXFlag = 1;
	}
	
void tick_recording(unsigned char var)
{
	switch(rec_state)
	{
		// WAITING TO SEE IF THE USER WANTS TO SAVE
		case wait_input:
			if(start_rec == 0x01)		// If the user wants to record
			{							//
				rec_state = recording;	// Transition to the set recording state
				addr_val = 0;			// How long the recording lasts
			}							//
			else if(start_rec == 0x02)	// If the user wants to playback
			{							// 
				rec_state = playback;	// Transition to the playback state
			}							//
			else                        // 
			{                           //
				rec_state = wait_input; //	
			}							//
		break;
		
		// RECORDING STATE
		case recording:
			if(addr_val < 250)
			{
				// This calls the EEPROM write word function that writes a string to memory
				eeprom_write_word((uint16_t *)(addr_val * 7), (uint16_t)(note));
								//	addr_val is the address in memory the value is going to be stored too
								//	
				addr_val++;		//	advance the address value to the next location in memory
			}
			else
			{
				addr_val = 0;
				rec_state = wait_input;
			}
		break;
		
		// PLAYBACK STATE
		case playback:
			if(addr_val < 250)
			{
				note = eeprom_read_word((uint16_t *)(addr_val * 7));
			}
			else
			{
				addr_val = 0;
				rec_state = wait_input;
			}
		break;
		
		default:
		rec_state = wait_input;
		break;
	}
}
int main(void)
{
    	DDRA = 0x00; PORTA = 0xFF; // Up down key inputs, NOTE: Potential to be Joystick to menu control
    	DDRB = 0xF0; PORTB = 0x0F; // PB7 is speaker output, 0-7 are piano keys
    	DDRC = 0xFF; PORTC = 0x00; // LCD data lines
    	DDRD = 0xF0; PORTD = 0x0F; // USART receive on D0
		
		//Initialize USART
		initUSART();
		USART_Flush(); // clears upon initialization of the program (NOTE in normal operation on same device would not execute
		
		unsigned char counter = 0; // counter to update USART
		
		note = 261.63; 
		rec_state = wait_input;
		
    while (1) 
    {
		if(USART_HasReceived()) // If the last transmission of data is good, clear the data register for the next transmission
		{
			USART_Flush();
		}
		else
		{
			counter = USART_Receive(); 
		}
		
		tick_recording(counter);
		set_PWM(note);
		while(!TimerFlag);
		
		TimerFlag = 0;
    }
}

