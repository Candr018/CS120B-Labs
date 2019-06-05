/*
 * Idontevenknowanymore.c
 *
 * Created: 6/4/2019 2:54:45 PM
 * Author : Fuck My Life
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <avr/eeprom.h>
#include "io.c"


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

const double send_help = 261.63;										// DOUBLE TO TEST
const char i_need_sleep[8] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};	// Character Array to test
	
int main(void)
{
	DDRB = 0xFF; PORTB = 0x00; // output speaker and LEDS
	DDRC = 0xFF; PORTC = 0x00; // LCD data lines
	DDRD = 0xF0; PORTD = 0x0F; // LCD control lines on D6 and D7, USART receive on D0 sends on 

	// --- This contains all the data types if you want to send something --- 
	//   https://www.nongnu.org/avr-libc/user-manual/group__avr__eeprom.html
	
	uint16_t write_data_double = (uint16_t)(send_help);  // casting the double as a 16 bit uint variable 
	uint8_t write_data_char_array[6];					 // making an 8 bit (1 byte) uint array to hold the characters
	
	unsigned char i = 0;
	for(i = 0; i < 8; i++)
		{
			write_data_char_array[i] = (uint8_t)(i_need_sleep[i]);	// casting each character in the char array as a 8-bit uint variable
		}
		
	double read_send_help = 0;					// Double to read the saved value
	char read_i_need_sleep[8];					// Char array to read the send character values
	unsigned short address_location = 0;		// Variable to hold the location in memory where the saved values are stored
	
	// EEPROM write word (2 bytes) function call
	eeprom_write_word((uint16_t *)(address_location), write_data_double);
	
	address_location = address_location + 2; // since each location is 1 byte, increment the location by 2 
	
	for(i = 0; i < 8; i++)
	{
	    eeprom_write_byte((uint8_t *)(address_location), write_data_char_array[i]);	// EEPROM write byte function call
		address_location++;
	}
	
	address_location = 0; // Move the location in memory back to the first address
	
	read_send_help = eeprom_read_word((uint16_t *)(address_location)); // reads the next two bytes saved at the address and assigns them to a double
	
	address_location = address_location + 2; // since each location is 1 byte, increment the location by 2 
	
	for(i = 0; i < 8; i++)
	{
		read_i_need_sleep[i] = eeprom_read_byte((uint8_t *)(address_location));	// EEPROM write byte function call
		address_location++;
	}
	
	LCD_init();	
	LCD_ClearScreen(); 
	for(int j = 1; j <= 8; j++)
	{
		LCD_Cursor(j);
		LCD_WriteData(read_i_need_sleep[j-1]);
	}
	PWM_on();
	set_PWM(read_send_help);
    while (1);
	return 0; 
}