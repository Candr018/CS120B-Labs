/*
 * Partner(s) Name & E-mail: Brain 
 * My Email: candr018@ucr.edu
 * Lab Section: 027
 * Assignment: Lab #4 Challenge #2
 * 
 * I acknowledge all content contained herein, excluding given examples and temples code, is my original work
 */ 

#include <avr/io.h>

enum State_Machine {start, start_input, button_release, button_press, input_pw, input_match, lock_start, lock_button_release, lock_button_press, input_lock} Next_state;
unsigned char temp_a;
unsigned char temp_b;
unsigned char temp_c;
unsigned char password[2];
unsigned char vaild_input;
unsigned char iteration;
	
void Tick_Keypad()
{
	switch(Next_state)
	{
		//case 1 -- initialize case on startup
		case start:
			temp_b = 0x00;
			temp_c = 0x01;
			iteration = 0x00;
			vaild_input = 0x01;
			Next_state = start_input;
		break; 
		
		/*case 2 -- only works if you are only pressing the # (A3) button
		  multiple buttons being pressed at the same time will not work*/
		case start_input:
			temp_c = 0x02;
			temp_b = 0x00;
			Next_state = temp_a == 0x04 ? button_release:start_input;
		break;
		
		/*case 3 -- only works if you stop pressing # (A3) button along with
		  all other buttons*/
		case button_release:
			temp_c = 0x04;
			temp_b = 0x00;
			Next_state = temp_a == 0x00 ? button_press:button_release;
		break;
		
		//case 5
		case button_press:
			temp_c = 0x08;
			temp_b = 0x00;
			Next_state = temp_a != 0x00 ? input_pw:button_press;
		break; 
		
		//case 6
		case input_pw:
			temp_c = 0x10;
			temp_b = 0x00;
			if(temp_a == 0x01 && iteration == 0x00 && vaild_input == 0x01)			// if the input matches what we expect at an iteration < lenght of the password 
			{																					//
				Next_state = button_release;													// then we increase the iteration to look for the next input of the password 
				iteration = iteration + 0x01;													// after the user releases the button
				vaild_input = 0x01;																//
			}																					//
			else if(temp_a == 0x02 && iteration == 0x01 && vaild_input == 0x01)			// if the input matches what we expect at an iteration < lenght of the password
			{																					//
				Next_state = button_release;													// then we increase the iteration to look for the next input of the password
				iteration = iteration + 0x01;													// after the user releases the button
				vaild_input = 0x01;																//
			}																					//
			else if(temp_a == 0x01 && iteration == 0x02 && vaild_input == 0x01)		// if the length of the password has been reached and all the inputs are valid
			{																					// then the password is valid
				vaild_input = 0x01;																//
				iteration = iteration + 0x01;													// after the user releases the button
				Next_state = button_release;													//
			}																					//
			else if(temp_a != 0x01 && iteration ==0x00 && vaild_input == 0x01)		//	if at any point we reach an invalid input then valid is assign to 0, however the code
			{																					//  will allow the user to enter all the inputs 
				Next_state = button_release;													//
				iteration = iteration + 0x01;													// after the user releases the button
				vaild_input = 0x00;																//
			}																					//
			else if(temp_a != 0x02 && iteration ==0x01 && vaild_input == 0x01)		//	if at any point we reach an invalid input then valid is assign to 0, however the code
			{																					//  will allow the user to enter all the inputs
				Next_state = button_release;													//
				iteration = iteration + 0x01;													// after the user releases the button
				vaild_input = 0x00;																//
			}
			else if(temp_a != 0x01 && iteration == 0x02 && vaild_input == 0x01)					//	check if last input is valid or not
			{																					//	if, at any point, there was a valid input then this check will never be reached
				Next_state = button_release;													//
				iteration = iteration + 0x01;													// after the user releases the button
				vaild_input = 0x00;																//
			}
			else if(vaild_input == 0x00 && iteration == 0x00)										//	code to allow the user to continue entering inputs even after an invalid entry
			{																					//
				Next_state = button_release;													//
				iteration = iteration + 0x01;													// after the user releases the button
			}																					//
			else if(vaild_input == 0x00 && iteration == 0x01)										//	code to allow the user to continue entering inputs even after an invalid entry
			{																					//
				Next_state = button_release;													//
				iteration = iteration + 0x01;													// after the user releases the button
			}																					//
			else if(vaild_input == 0x00 && iteration == 0x02)										//	code to allow the user to inter the last input even after an invalid entry
			{																					//
				Next_state = button_release;													//
				iteration = iteration + 0x01;													// after the user releases the button								
			}																					//
			else if(iteration == 0x03 && vaild_input == 0x01)										//
			{																					//
				Next_state = input_match;														//
			}																					//
			else if(iteration == 0x03 && vaild_input == 0x00)										//
			{																					//
				Next_state = start;																//
			}																					//
			else                                                                                //
			{																					//
				Next_state = start;															    //
			}
		break;	
		
		/*case 6 -- door unlocked
		  door only locks if the lock button (A7) if pressed without other inputs*/
		case input_match:
			temp_b = 0x01;
			iteration = 0x00;
			Next_state = lock_start;
		break;
		
		//case 6 -- locking door from outside requires same input
		//begins when # (A3) is pressed
		case lock_start:
			temp_c = 0x20;
			iteration = 0x00;
			temp_b = 0x01;
			if(temp_a == 0x70 || temp_a == 0x71 || temp_a == 0x72 || temp_a == 0x73 || temp_a == 0x75 || temp_a == 0x76 || temp_a == 0x77)
			{
				Next_state = start_input;
			}
			else if(temp_a == 0x04)
			{
				Next_state = lock_button_release;
			}
			else
			{
				Next_state = lock_start;
			}
		break;
		
		//case 7 -- button release for "#" key on Next_state
		case lock_button_release:
			temp_b = 0x01;
			if(temp_a == 0x70 || temp_a == 0x71 || temp_a == 0x72 || temp_a == 0x73 || temp_a == 0x75 || temp_a == 0x76 || temp_a == 0x77)
			{
				Next_state = start_input;
			}
			else if(temp_a == 0x00)
			{
				Next_state = lock_button_press;
			}
			else
			{
				Next_state = lock_button_release;
			}
		break;
		
		//case 8
		case lock_button_press:
		temp_c = 0x40;
		temp_b = 0x01;
		if(temp_a == 0x70 || temp_a == 0x71 || temp_a == 0x72 || temp_a == 0x73 || temp_a == 0x75 || temp_a == 0x76 || temp_a == 0x77)
		{
			Next_state = start_input;
		}
		else if(temp_a != 0x00)
		{
			Next_state = input_lock;
		}
		else
		{
			Next_state = lock_button_press;
		}
		break;
		
		//case 8 -- checking if lock input matches password
		case input_lock:
			temp_c = 0x80;
			temp_b = 0x01;
			if(temp_a == 0x01 && iteration == 0x00 && vaild_input == 0x01)			// if the input matches what we expect at an iteration < lenght of the password
			{																					//
				Next_state = lock_button_release;													// then we increase the iteration to look for the next input of the password
				iteration = iteration + 0x01;													// after the user releases the button
				vaild_input = 0x01;																//
			}																					//
			else if(temp_a == 0x02 && iteration == 0x01 && vaild_input == 0x01)			// if the input matches what we expect at an iteration < lenght of the password
			{																					//
				Next_state = lock_button_release;													// then we increase the iteration to look for the next input of the password
				iteration = iteration + 0x01;													// after the user releases the button
				vaild_input = 0x01;																//
			}																					//
			else if(temp_a == 0x01 && iteration == 0x02 && vaild_input == 0x01)		// if the length of the password has been reached and all the inputs are valid
			{																					// then the password is valid
				vaild_input = 0x01;																//
				iteration = iteration + 0x01;													// after the user releases the button
				Next_state = lock_button_release;													//
			}																					//
			else if(temp_a != 0x01 && iteration ==0x00 && vaild_input == 0x01)		//	if at any point we reach an invalid input then valid is assign to 0, however the code
			{																					//  will allow the user to enter all the inputs
				Next_state = lock_button_release;													//
				iteration = iteration + 0x01;													// after the user releases the button
				vaild_input = 0x00;																//
			}																					//
			else if(temp_a != 0x02 && iteration ==0x01 && vaild_input == 0x01)		//	if at any point we reach an invalid input then valid is assign to 0, however the code
			{																					//  will allow the user to enter all the inputs
				Next_state = lock_button_release;													//
				iteration = iteration + 0x01;													// after the user releases the button
				vaild_input = 0x00;																//
			}
			else if(temp_a != 0x01 && iteration == 0x02 && vaild_input == 0x01)					//	check if last input is valid or not
			{																					//	if, at any point, there was a valid input then this check will never be reached
				Next_state = lock_button_release;													//
				iteration = iteration + 0x01;													// after the user releases the button
				vaild_input = 0x00;																//
			}
			else if(vaild_input == 0x00 && iteration == 0x00)										//	code to allow the user to continue entering inputs even after an invalid entry
			{																					//
				Next_state = lock_button_release;													//
				iteration = iteration + 0x01;													// after the user releases the button
			}																					//
			else if(vaild_input == 0x00 && iteration == 0x01)										//	code to allow the user to continue entering inputs even after an invalid entry
			{																					//
				Next_state = lock_button_release;													//
				iteration = iteration + 0x01;													// after the user releases the button
			}																					//
			else if(vaild_input == 0x00 && iteration == 0x02)										//	code to allow the user to inter the last input even after an invalid entry
			{																					//
				Next_state = lock_button_release;													//
				iteration = iteration + 0x01;													// after the user releases the button
			}																					//
			else if(iteration == 0x03 && vaild_input == 0x01)										//
			{																					//
				Next_state = start;														//
			}																					//
			else if(iteration == 0x03 && vaild_input == 0x00)										//
			{																					//
				Next_state = lock_start;																//
			}																					//
			else                                                                                //
			{																					//
				Next_state = lock_start;															    //
			}
			break;

		default:
			Next_state = start;
			break;
	}
}

int main(void)
{
    DDRA = 0x00; PORTA = 0xFF; // Configure port A's 8 pins as inputs
    DDRB = 0xFF; PORTB = 0x00; // Configure port C's 8 pins as outputs
	DDRC = 0xFF; PORTC = 0x00; // Configure port C's 8 pins as outputs
	DDRD = 0xFF; PORTD = 0x00; // Configure port C's 8 pins as outputs
	Next_state = start;
	password[0] = 0x01;
	password[1] = 0x02;
	password[2] = 0x01;
	iteration = 0x00;
	vaild_input = 0x01;
	
    while (1) 
    {
		temp_a = PINA & 0x87; //allows for A7, A2, A1, and A0 to be inputs
		Tick_Keypad();
		PORTB = temp_b;
		PORTC = temp_c;
		PORTD = iteration;
    }
}



