#include <avr/io.h>

#include <avr/interrupt.h>

#include <stdlib.h>

#include <avr/eeprom.h>

#include "io.c"

#include "usart.h"



//
// Fourth (Middle) Octave
double C4 = 261.63; // middle C
double D4 = 293.66;
double E4 = 329.63;
double F4 = 349.23;
double G4 = 392.00;
double A4 = 440.00; // concert pitch
double B4 = 493.88;

double note = 0.0;

enum recording {unarmed, armed, in_progress, playback} rec_state;

unsigned short rec_tick = 0;

signed short x = 0;  // Value of ADC register now stored in variable x.

enum states {start, wait, play} state;

signed short prev_x = 0;

unsigned char viz_tick = 0; // used to track the vizualizer rise and fall

unsigned char button = 0; // used to track when the button changes

unsigned char rec_flag = 0;

//timer shit

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



//turn recording on and off

void tickRecord(unsigned char temp) {

	switch(rec_state) {

		case unarmed:

		if (temp == 0x01) {
			//User wants to record a set of notes
			rec_state = armed;
			rec_tick = 0;
		}

		if (temp == 0x02) { // User wants to playback
			rec_state = playback;
		}

		break;

		case in_progress:
			if (rec_tick < 200) 
				{
					eeprom_write_word((uint16_t *)(rec_tick*18), (uint16_t)(note));
					rec_tick++;
				}

			else 
				{
					rec_tick = 0;
					rec_state = unarmed;
				}	
		break;

		case playback:
			if (rec_tick < 200) 
				{
					note = eeprom_read_word((uint16_t *)(rec_tick*18));
					rec_tick++;
				}

			else 
				{
					rec_tick = 0;
					rec_state = unarmed;
				}
		break;
		
		default:
			rec_state = unarmed;
		break;

	}

}

//state shit

void tickSM(unsigned char display_counter) {
	unsigned char tempA;
	unsigned char tempB;
	unsigned char tempD = ~PIND & 0x3C;
	
	if(	tempD == 0x10 ) 
	{
		tempA = 1;
	}
	if( tempD == 0x20 )
	{
		tempB = 1; 
	}
	switch(state) { //transitions
		case start:
		state = wait;
		break;

		case wait:
		if (tempA == 0x00 && tempB == 0x00) {
			state = wait;
		}

		else {
			state = play;
		}

		break;

		case play:
		if (tempA == 0x00 && tempB == 0x00) {
			state = wait;
		}

		else {
			state = play;
		}

		break;
		
		default:
		state = wait;
		break;

	}//transitions

	

	switch(state) { //actions

		case start:
		break;

		case wait:
		note = 0;
		button = 0;
		break;

		case play:
		if (rec_state == armed) {
			rec_state = in_progress;
		}
		break;
		
		default:
		break;

	}

}


void update_LCD() {
	unsigned char temp_b;
	switch(rec_state) {

		case armed:
			temp_b = 0x01;
		break;

		case in_progress:

			temp_b = 0x02;

		rec_flag = 1;

		break;

		case playback:

			temp_b = 0x03;

		break;

		default:

		if (rec_flag == 1) {
			rec_flag = 0;
		}

		break;

	}

	PORTB = temp_b;

}
int main(void)

{

	DDRA = 0x00; PORTA = 0xFF; // potentiometer  inputs

	DDRB = 0xFF; PORTB = 0x00; // PB6 is speaker output, 0-3 are other inputs

	DDRC = 0xFF; PORTC = 0x00; // LCD data lines

	DDRD = 0xF0; PORTD = 0x0F; // LCD control lines on D6 and D7, USART receive on D0

	

	//init stuff

	PWM_on(); 

	TimerSet(20);

	TimerOn();

	

	//usart

	initUSART();

	USART_Flush();

	//init states

	state = start;

	rec_state = unarmed;

	unsigned char i = 0;

	unsigned char bong = 0;
	unsigned char tempB;
	while(1) 


	{

		
		unsigned char tempB = ~PINA & 0xFF;
		tempB = tempB >> 1;
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
			if (USART_HasReceived()) {

				USART_Flush();

			}

			else {

					bong = USART_Receive();


			}

			

		tickSM(i);

		tickRecord(bong);

			if (i == 4) {	//only update LCD every 80 ms, looks bad otherwise



				update_LCD();

				i = 0;

			}

			else {

				i++;

			}
		
		

		set_PWM(note);

		while (!TimerFlag);

		TimerFlag = 0;

	}

}