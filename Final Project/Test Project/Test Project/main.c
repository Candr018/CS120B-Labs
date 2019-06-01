/*
 * Test Project.c
 *
 * Created: 5/23/2019 5:22:40 PM
 * Author : User
 */ 

#include <avr/io.h>


int main(void)
{
    //write custom chars

    LCD_WriteCommand(0x40);

    for(unsigned char i=0; i<8; i++){

	    LCD_WriteData(low_bar[i]);

    }

    LCD_WriteCommand(0x80);

    

    LCD_WriteCommand(0x48);

    for(unsigned char i=0; i<8; i++){

	    LCD_WriteData(med_bar[i]);

    }

    LCD_WriteCommand(0x80);

    

    LCD_WriteCommand(0x50);

    for(unsigned char i=0; i<8; i++){

	    LCD_WriteData(high_bar[i]);

    }

    LCD_WriteCommand(0x80);

    

    LCD_WriteCommand(0x58);

    for(unsigned char i=0; i<8; i++){

	    LCD_WriteData(top_bar[i]);

    }

    LCD_WriteCommand(0x80);
	
	
    while (1) 
    {
    }
}

