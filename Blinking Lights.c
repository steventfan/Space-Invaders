/* Partner(s) Name & E-mail: Henry Hua hhua003@ucr.edu, Steven Fan sfan008@ucr.edu
 * Lab Section: 021
 * Assignment: Lab 4 Exercise 3
 * 
 * I acknowledge all content contained herein, excluding template or example code, is my own original work.
 */ 

#define F_CPU 125000UL
#include <avr/io.h>
#include <util/delay.h>

enum States {OFFHOLD, OFF, ONEHOLD, ONE1HOLD, ONE, ONE1, TWOHOLD, TWO1HOLD, TWO, TWO1, THREEHOLD, THREE1HOLD, THREE, THREE1} state;

void Sequencing();
void Outputs();

int main(void)
{	
	DDRA = 0x00;
	DDRC = 0xFF;
	PORTA = 0xFF;
	PORTC = 0x00;
	state = OFF;
	
    while(1)
    {
		Sequencing();
		Outputs();
	}
	
	return 0;
}

void Sequencing()
{
	unsigned char tempA = ~PINA & 1;
	
	_delay_ms(5250);
	
	switch(state)
	{
		case OFFHOLD:
			if(tempA)
			{
				state = OFFHOLD;
			}
			else
			{
				state = OFF;
			}
			break;
		case OFF:
			if(tempA)
			{
				state = ONEHOLD;
			}
			else
			{
				state = OFF;
			}
			break;
		case ONEHOLD:
			if(tempA)
			{
				state = ONE1HOLD;
			}
			else
			{
				state = ONE1;
			}
			break;
		case ONE1HOLD:
			if(tempA)
			{
				state = ONEHOLD;
			}
			else
			{
				state = ONE;
			}
			break;
		case ONE:
			if(tempA)
			{
				state = TWOHOLD;
			}
			else
			{
				state = ONE1;
			}
			break;
		case ONE1:
			if(tempA)
			{
				state = TWOHOLD;
			}
			else
			{
				state = ONE;
			}
			break;
		case TWOHOLD:
			if(tempA)
			{
				state = TWO1HOLD;
			}
			else
			{
				state = TWO1;
			}
			break;
		case TWO1HOLD:
			if(tempA)
			{
				state = TWOHOLD;
			}
			else
			{
				state = TWO;
			}
			break;
		case TWO:
			if(tempA)
			{
				state = THREEHOLD;
			}
			else
			{
				state = TWO1;
			}
			break;
		case TWO1:
			if(tempA)
			{
				state = THREEHOLD;
			}
			else
			{
				state = TWO;
			}
			break;
		case THREEHOLD:
			if(tempA)
			{
				state = THREE1HOLD;
			}
			else
			{
				state = THREE1;
			}
			break;
		case THREE1HOLD:
			if(tempA)
			{
				state = THREEHOLD;
			}
			else
			{
				state = THREE;
			}
			break;
		case THREE:
			if(tempA)
			{
				state = OFFHOLD;
			}
			else
			{
				state = THREE1;
			}
			break;
		case THREE1:
			if(tempA)
			{
				state = OFFHOLD;
			}
			else
			{
				state = THREE;
			}
			break;
		default:
			state = OFF;
			break;
	}
}

void Outputs()
{
	unsigned char tempC = 0;
	
	switch(state)
	{
		case OFFHOLD:
			tempC = 0;
			break;
		case OFF:
			tempC = 0;
			break;
		case ONEHOLD:
			tempC = 0xFF;
			break;
		case ONE1HOLD:
			tempC = 0;
			break;
		case ONE:
			tempC = 0xFF;
			break;
		case ONE1:
			tempC = 0;
			break;
		case TWOHOLD:
			tempC = 0xF0;
			break;
		case TWO1HOLD:
			tempC = 0x0F;
			break;
		case TWO:
			tempC = 0xF0;
			break;
		case TWO1:
			tempC = 0x0F;
			break;
		case THREEHOLD:
			tempC = 0xAA;
			break;
		case THREE1HOLD:
			tempC = 0x55;
			break;
		case THREE:
			tempC = 0xAA;
			break;
		case THREE1:
			tempC = 0x55;
			break;
		default:
			tempC = 0;
			break;
	}
	
	PORTC = tempC;
}