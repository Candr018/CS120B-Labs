#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <avr/eeprom.h>
#include "io.c"

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

int main(void)
{
	DDRC = 0xFF; PORTC = 0x00; // LCD data lines
	DDRD = 0xFF; PORTD = 0x00; // LCD control lines
	
    unsigned char i = 0;
    
    unsigned char one_bar[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F};	// 1
    unsigned char two_bars[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F};	// 2
    unsigned char three_bars[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F};	// 3
    unsigned char four_bars[8] = { 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1F};	// 4
    unsigned char five_bars[8] = { 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F};  // 5
    unsigned char six_bars[8] = { 0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F};	// 6
    unsigned char seven_bars[8] = { 0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F}; // 7
    unsigned char top_bar[8] = { 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};	// 8

    LCD_init();
	
	LCD_Custom_Char(0, top_bar);	/* Build Character1 at position 0 */
	LCD_Custom_Char(1, one_bar);	/* Build Character2 at position 1 */
	LCD_Custom_Char(2, two_bars);	/* Build Character3 at position 2 */
	LCD_Custom_Char(3, three_bars); /* Build Character4 at position 3 */
	LCD_Custom_Char(4, four_bars);  /* Build Character5 at position 4 */
	LCD_Custom_Char(5, five_bars);  /* Build Character6 at position 5 */
	LCD_Custom_Char(6, six_bars);	/* Build Character6 at position 6 */
	LCD_Custom_Char(7, seven_bars); /* Build Character6 at position 7 */
	
	LCD_WriteCommand(0x80);		/*cursor at home position */
	LCD_Cursor(1);
	LCD_DisplayString(1,"Custom char LCD");
	
	LCD_Cursor(17);
	
	
	for(i=0;i<6;i++)		/* function will send data 1 to 8 to lcd */
	{
		LCD_WriteData(0);		/* char at 'i'th position will display on lcd */
				/* space between each custom char. */
	}
    while (1) 
    {
    }
}

