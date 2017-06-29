//Steven Fan
//CS120B
//Space Invaders Project

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <util/delay.h>

#define F_CPU 1000000UL

//*Timer Variables*
unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;
//*Timer Variables*

typedef struct task
{
	int state;
	unsigned short period;
	unsigned short elapsedTime;
	int (*FctPtr)(int);
} task;

task tasks[9];
const unsigned char numTasks = 9;

typedef struct enemy
{
	unsigned char direction;
	unsigned char position;
} enemy;

unsigned char player = 0b00010000;
unsigned char playerProjectile[7] = {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000};
enemy enemies[3];
unsigned char enemyProjectile[8] = {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000};
unsigned char boss = 0b00000000;
unsigned char lives = 3;
unsigned char level = 0;
unsigned char override = 1;
unsigned char left = 0;
unsigned char right = 0;
unsigned char fire = 0;
unsigned char display[8] = {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000};
unsigned char score = 0;
unsigned char reset = 0;



//*Timer Functions* (Atmega1284 compatible timer functions) Source: (University of California Riverside CS120B Lab Manuals) https://docs.google.com/document/d/1IaeE6pic_WCvLSQVRAvTH9l3-2vuLtSh0cgozdyYCng/edit
void TimerOn()
{
	TCCR1B = 0x0B;
	OCR1A = 125;
	TIMSK1 = 0x02;
	TCNT1 = 0;

	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80;
}

void TimerOff()
{
	TCCR1B = 0x00;
}

void TimerISR()
{
	for(unsigned char i = 0; i < numTasks; i++)
	{
		if(tasks[i].elapsedTime >= tasks[i].period)
		{
			tasks[i].state = tasks[i].FctPtr(tasks[i].state);
			tasks[i].elapsedTime = 0;
		}
		tasks[i].elapsedTime += 10;
		rand(); //randomize numbers based on time
	}
}

ISR(TIMER1_COMPA_vect)
{
	_avr_timer_cntcurr--;
	if(_avr_timer_cntcurr == 0)
	{
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

void TimerSet(unsigned long M)
{
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}
//*Timer Functions*



//*Analog to Digital Convert Functions* (convert analog voltage inputs from PINA to digital) Source: www.embedds.com/interfacing-analog-joystick-with-avr/
void InitADC(void)
{
	ADMUX |= (1 << REFS0);
	ADCSRA |= (1 << ADEN) | (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2);
}

int readadc(int ch)
{
	ch &= 0b00000111;
	ADMUX = (ADMUX & 0xF8) | ch;
	ADCSRA |= (1 << ADSC);
	while( (ADCSRA) & (1 << ADSC) );
	return (ADC);
}
//*Analog to Digital Convert Functions*



//*EEPROM Functions* (save data even when device is switched off) Source: AVR Data Book Library
void EEPROM_Write(unsigned short address, unsigned char data)
{
	while(EECR & (1 << EEPE) );
	EEAR = address;
	EEDR = data;
	EECR |= (1 << EEMPE);
	EECR |= (1 << EEPE);
}

unsigned char EEPROM_Read(unsigned short address)
{
	while(EECR & (1 << EEPE) );
	EEAR = address;
	EECR |= (1 << EERE);
	EEAR = 0;
	
	return EEDR;
}
//*EEPROM Functions*



int Input(int state)
{
	unsigned char tempA = ~PINA & 0x0C;
	short xDirection = readadc(0) - 512;
	short yDirection = readadc(1) - 512;
	
	if(xDirection < -100)
	{
		left = 1;
		right = 0;
	}
	else if(xDirection > 100)
	{
		left = 0;
		right = 1;
	}
	else
	{
		left = 0;
		right = 0;
	}
	if(yDirection > 100)
	{
		fire = 1;
	}
	else
	{
		fire = 0;
	}
	if( (tempA & 0x04) == 0x04)
	{
		reset = 1;
	}
	else
	{
		reset = 0;
	}
	if( (tempA & 0x08) == 0x08)
	{
		EEPROM_Write(1, 0);
	}
	
	return 0;
}

int Player()
{
	static unsigned char i = 0;
	
	if(override)
	{
		return 0;
	}
	if(i >= 24)
	{
		if(left)
		{
			if( (player & 0b10000000) != 0b10000000)
			{
				player = player << 1;
			}
			i = 0;
		}
		else if(right)
		{
			if( (player & 0b00000001) != 0b00000001)
			{
				player = player >> 1;
			}
			i = 0;
		}
	}
	else
	{
		i++;
	}
	
	return 0;
}

int Enemy()
{
	if(override)
	{
		return 0;
	}
	for(unsigned char i = 0; i < 3; i++)
	{
		if(enemies[i].direction == 1)
		{
			if( (enemies[i].position & 0b10000000) == 0b10000000)
			{
				enemies[i].direction = 2;
				enemies[i].position = enemies[i].position >> 1;
			}
			else
			{
				enemies[i].position = enemies[i].position << 1;
			}
		}
		else if(enemies[i].direction == 2)
		{
			if( (enemies[i].position & 0b00000001) == 0b00000001)
			{
				enemies[i].direction = 1;
				enemies[i].position = enemies[i].position << 1;
			}
			else
			{
				enemies[i].position = enemies[i].position >> 1;
			}
		}
		else if(enemies[i].direction == 0)
		{
			enemies[i].position = 0b00000000;
		}
	}
	
	return 0;
}

int EnemyProjectile()
{
	static unsigned char charge = 0b00000000;
	static unsigned char firing = 0;
	static unsigned char i = 0;
	static unsigned char j = 0;
	static unsigned char k = 0;
	unsigned char enemyRow = rand() % 3;
	unsigned char enemyColumn = rand() % 8;
	unsigned char laser = rand() % 4 + 2;
	
	if(override)
	{
		return 0;
	}
	if(enemies[0].direction == 3 || enemies[1].direction == 3 || enemies[2].direction == 3)
	{
		if(i >= 2)
		{
			if(!firing)
			{
				charge = 0b10000000 >> (laser - 2);
				charge = charge | (0b10000000 >> (laser - 1) );
				charge = charge | (0b10000000 >> laser);
				charge = charge | (0b10000000 >> (laser + 1) );
				charge = charge | (0b10000000 >> (laser + 2) );
				j = 0;
				firing = 1;
			}
			if(j >= 7)
			{
				boss = 0b00000000;
				enemyProjectile[3] = enemyProjectile[3] | charge;
				charge = 0b00000000;
				firing = 0;
				i = 0;
			}
			else if(j <= 2)
			{
				boss = charge;
			}
			else if(j <= 4)
			{
				boss = 0b00000000;
			}
			else if(j <= 6)
			{
				boss = charge;
			}
			j++;
		}
		else
		{
			i++;
		}
	}
	else
	{
		charge = 0b00000000;
		boss = 0b00000000;
		firing = 0;
		i = 0;
	}
	for(unsigned char l = 7; l > 0; l--)
	{
		enemyProjectile[l] = enemyProjectile[l - 1];
	}
	enemyProjectile[0] = 0x00;
	if( (enemies[enemyRow].position & (0b10000000 >> enemyColumn) ) != 0b00000000)
	{
		if(!(enemies[0].direction == 3 || enemies[1].direction == 3 || enemies[2].direction == 3) || (k >= 2) )
		{
			enemyProjectile[enemyRow + 1] = enemyProjectile[enemyRow + 1] | (0b10000000 >> enemyColumn);
			k = 0;
		}
		k++;
	}
	
	return 0;
}

int PlayerProjectile()
{
	static unsigned char i = 0;
	static unsigned char j = 0;
	
	if(override)
	{
		return 0;
	}
	if(i >= 25)
	{
		for(unsigned char k = 0; k < 6; k++)
		{
			playerProjectile[k] = playerProjectile[k + 1];
		}
		playerProjectile[6] = 0x00;
		i = 0;
	}
	i++;
	if(j >= 99)
	{
		if(fire)
		{
			playerProjectile[6] = playerProjectile[6] | player;
			j = 0;
		}
	}
	else
	{
		j++;
	}
	
	return 0;
}

int EnemyHitbox()
{
	if(override)
	{
		return 0;
	}
	
	unsigned char projectile = 0b00000000;
	
	for(unsigned char i = 0; i < 3; i++)
	{
		if( (enemies[i].position & playerProjectile[i]) != 0b00000000)
		{
			score++;
			projectile = ~playerProjectile[i];
			playerProjectile[i] = playerProjectile[i] & ~enemies[i].position;
			enemies[i].position = enemies[i].position & projectile;
		}
		if(enemies[i].position == 0b00000000)
		{
			enemies[i].direction = 0;
		}
	}
	
	return 0;
}

int PlayerHitbox()
{
	if(override)
	{
		return 0;
	}
	if( (player & enemyProjectile[7]) != 0b00000000)
	{
		enemyProjectile[7] = enemyProjectile[7] & ~player;
		lives--;
	}
	
	return 0;
}

enum Status_States {TITLE, INITIALIZE, LEVEL, GAME, WIN, GAMEOVER};
int Status(int state)
{
	static unsigned short i = 0;
	static unsigned scoreTimer = 0;
	unsigned char j = 0;
	
	switch(state)
	{
		case TITLE:
			if(reset)
			{
				i = 0;
				state = TITLE;
			}
			else if(fire)
			{
				if(i >= 100)
				{
					i = 0;
					state = INITIALIZE;
				}
				else
				{
					i++;
					state = TITLE;
				}
			}
			else
			{
				i = 0;
				state = TITLE;
			}
			break;
		case INITIALIZE:
			if(reset)
			{
				state = TITLE;
			}
			else
			{
				state = LEVEL;
			}
			break;
		case LEVEL:
			if(reset)
			{
				i = 0;
				state = TITLE;
			}
			else if(i >= 300)
			{
				i = 0;
				state = GAME;
			}
			else
			{
				i++;
				state = LEVEL;
			}
			break;
		case GAME:
			if(reset)
			{
				state = TITLE;
			}
			else if(!lives)
			{
				state = GAMEOVER;
			}
			else if( !(enemies[0].direction || enemies[1].direction || enemies[2].direction) )
			{
				score += (lives * 5);
				state = WIN;
			}
			else
			{
				state = GAME;
			}
			break;
		case WIN:
			if(reset)
			{
				i = 0;
				state = TITLE;
			}
			else if(fire)
			{
				if(i >= 100)
				{
					i = 0;
					if(level >= 4)
					{
						if(score > EEPROM_Read(1) )
						{
							EEPROM_Write(1, score);
						}
						state = TITLE;
					}
					else
					{
						state = INITIALIZE;
					}
				}
				else
				{
					i++;
					state = WIN;
				}
			}
			else
			{
				i = 0;
				state = WIN;
			}
			break;
		case GAMEOVER:
			if(reset)
			{
				i = 0;
				state = TITLE;
			}
			else if(fire)
			{
				if(i >= 100)
				{
					if(score > EEPROM_Read(1) )
					{
						EEPROM_Write(1, score);
					}
					i = 0;
					state = TITLE;
				}
				else
				{
					i++;
					state = GAMEOVER;
				}
			}
			else
			{
				i = 0;
				state = GAMEOVER;
			}
			break;
		default:
			state = TITLE;
			break;
	}
	
	switch(state)
	{
		case TITLE:
			lives = 3;
			level = 0;
			score = 0;
			scoreTimer = 0;
			boss = 0b00000000;
			override = 1;
			break;
		case INITIALIZE:
			level++;
			for(unsigned char j = 0; j < 8; j++)
			{
				enemyProjectile[j] = 0b00000000;
				if(j < 7)
				{
					playerProjectile[j] = 0b00000000;
				}
			}
			player = 0b00010000;
			left = 0;
			right = 0;
			fire = 0;
			lives = 3;
			scoreTimer = 0;
			j = 0;
			if(level == 1)
			{
				enemies[j].direction = 2;
				enemies[j].position = 0b01000000;
				j++;
				enemies[j].direction = 2;
				enemies[j].position = 0b00010000;
				j++;
				enemies[j].direction = 2;
				enemies[j].position = 0b00000100;
			}
			else if(level == 2)
			{
				enemies[j].direction = 2;
				enemies[j].position = 0b10000000;
				j++;
				enemies[j].direction = 1;
				enemies[j].position = 0b00100100;
				j++;
				enemies[j].direction = 1;
				enemies[j].position = 0b00000001;
			}
			else if(level == 3)
			{
				enemies[j].direction = 2;
				enemies[j].position = 0b10010000;
				j++;
				enemies[j].direction = 2;
				enemies[j].position = 0b00100100;
				j++;
				enemies[j].direction = 1;
				enemies[j].position = 0b00001001;
			}
			else if(level == 4)
			{
				enemies[j].direction = 3;
				enemies[j].position = 0b10111101;
				j++;
				enemies[j].direction = 3;
				enemies[j].position = 0b10111101;
				j++;
				enemies[j].direction = 3;
				enemies[j].position = 0b10111101;
			}
			override = 2;
			break;
		case LEVEL:
			override = 2;
			break;
		case GAME:
			if(scoreTimer >= 500)
			{
				if(score > 0)
				{
					score--;
				}
				scoreTimer = 0;
			}
			scoreTimer++;
			override = 0;
			break;
		case WIN:
			override = 3;
			break;
		case GAMEOVER:
			override = 4;
			break;
		default:
			override = 0;
			break;
	}
	
	return state;
}

int Display()
{
	unsigned char tempD = 0x00;
	
	if(override == 1)
	{
		display[0] = 0b11111111;
		display[1] = 0b10000001;
		display[2] = 0b10111101;
		display[3] = 0b10100101;
		display[4] = 0b10100101;
		display[5] = 0b10111101;
		display[6] = 0b10000001;
		display[7] = 0b11111111;
	}
	else if(override == 2)
	{
		if(level == 1)
		{
			display[0] = 0b00000000;
			display[1] = 0b00010000;
			display[2] = 0b00110000;
			display[3] = 0b00010000;
			display[4] = 0b00010000;
			display[5] = 0b00010000;
			display[6] = 0b00111000;
			display[7] = 0b00000000;
		}
		else if(level == 2)
		{
			display[0] = 0b00000000;
			display[1] = 0b00111100;
			display[2] = 0b00000100;
			display[3] = 0b00001100;
			display[4] = 0b00110000;
			display[5] = 0b00100000;
			display[6] = 0b00111100;
			display[7] = 0b00000000;
		}
		else if(level == 3)
		{
			display[0] = 0b00000000;
			display[1] = 0b00111100;
			display[2] = 0b00000100;
			display[3] = 0b00111100;
			display[4] = 0b00000100;
			display[5] = 0b00000100;
			display[6] = 0b00111100;
			display[7] = 0b00000000;
		}
		else
		{
			display[0] = 0b01111100;
			display[1] = 0b11111110;
			display[2] = 0b10010010;
			display[3] = 0b11111110;
			display[4] = 0b11101110;
			display[5] = 0b01111100;
			display[6] = 0b01010100;
			display[7] = 0b00000000;
		}
	}
	else if(override == 3)
	{
		display[0] = 0b00000000;
		display[1] = 0b00111100;
		display[2] = 0b01000010;
		display[3] = 0b01000010;
		display[4] = 0b01000010;
		display[5] = 0b01000010;
		display[6] = 0b00111100;
		display[7] = 0b00000000;
	}
	else if(override == 4)
	{
		display[0] = 0b00000000;
		display[1] = 0b01000010;
		display[2] = 0b00100100;
		display[3] = 0b00011000;
		display[4] = 0b00011000;
		display[5] = 0b00100100;
		display[6] = 0b01000010;
		display[7] = 0b00000000;
	}
	else
	{
		for(unsigned char i = 0; i < 8; i++)
		{
			display[i] = 0b00000000;
			if(i < 3)
			{
				display[i] = enemies[i].position;
			}
			if(i < 7)
			{
				display[i] = display[i] | playerProjectile[i];
			}
			display[i] = display[i] | enemyProjectile[i];
		}
		display[3] = display[3] | boss;
		display[7] = display[7] | player;
	}
	for(unsigned char i = 0; i < 8; i++)
	{
		PORTC = 0b00000000;
		PORTB = ~(0b00000001 << i);
		PORTC = display[i];
		_delay_ms(10);
	}
	PORTB = 0xFF;
	if(lives == 3)
	{
		PORTA = 0x8F;
	}
	else if(lives == 2)
	{
		PORTA = 0xCF;
	}
	else if(lives == 1)
	{
		PORTA = 0xEF;
	}
	else
	{
		PORTA = 0xFF;
	}
	if(override == 1)
	{
		if(EEPROM_Read(1) == 0xFF)
		{
			EEPROM_Write(1, 0);
		}
		tempD = EEPROM_Read(1);
	}
	else
	{
		tempD = score;
	}
	PORTD = tempD;
	
	return 0;
}



int main(void)
{
	DDRA = 0xF0; PORTA = 0x0F;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	
	unsigned char i = 0;
	
	tasks[i].state = 0;
	tasks[i].period = 10;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].FctPtr = &Input;
	i++;
	tasks[i].state = 0;
	tasks[i].period = 10;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].FctPtr = &Player;
	i++;
	tasks[i].state = 0;
	tasks[i].period = 500;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].FctPtr = &Enemy;
	i++;
	tasks[i].state = 0;
	tasks[i].period = 250;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].FctPtr = &EnemyProjectile;
	i++;
	tasks[i].state = 0;
	tasks[i].period = 10;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].FctPtr = &PlayerProjectile;
	i++;
	tasks[i].state = 0;
	tasks[i].period = 10;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].FctPtr = &EnemyHitbox;
	i++;
	tasks[i].state = 0;
	tasks[i].period = 10;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].FctPtr = &PlayerHitbox;
	i++;
	tasks[i].state = TITLE;
	tasks[i].period = 10;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].FctPtr = &Status;
	i++;
	tasks[i].state = 0;
	tasks[i].period = 10;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].FctPtr = &Display;
	
	srand(3174);
	InitADC();
	TimerSet(10);
	TimerOn();
	
    while(1);
	
	return 0;
}