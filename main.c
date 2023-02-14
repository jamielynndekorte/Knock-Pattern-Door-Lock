/*
 * timertesting.c
 *
 * Created: 4/12/2019 5:57:58 PM
 * Author : Jamie
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

int getDigit(volatile int);
int knockListen();
void wait(volatile int, volatile char);
void delay_T_msec_timer0(volatile char);
void step_CW(void);
void step_CCW(void);
void unlockdoor(void);
void lockdoor(void);

volatile int loop;
char counter;
char password = 0x00;
char guess = 0x00;
int nextDig;
int phase_step = 1;
int doorStatus = 0; // 0 for locked, 1 for unlocked

int main(void)
{
	DDRD = 0xF0;
	//PD0 - Knock Listen Switch (input) 
	//PD1 - Unlock Switch (input & LED) 
	//PD2 - Program Password Switch (input)
	//PD3 - Remote Interrupt (input) 
	//PD4-7 - Stepper Motor (output) 
	DDRC = 0b00000110;
	//PC0 - Pickup ADC (input)
	//PC1 - Knock Listen LED (output)
	//PC2 - Program Password LED (Output)
	//PC3 - Limit Switch Lock (input)
	
	// setting up interrupt
	EICRA = 0b00001000;
	EIMSK = 0b00000010;
	sei();
	
	// Setting up ADC
	PRR = 0x00; // clear Power Reduction ADC bit (0) in PRR register
	ADCSRA = 1<<ADEN | 1<<ADPS2 | 1<<ADPS1 | 1<<ADPS0; // Set ADC enable bit (7), set ADC prescaler to 128 (2-0)
	ADMUX = 0<<REFS1 | 1<<REFS0 | 1<<ADLAR; // Select analog reference voltage to be AVcc (bits 7-6), left justification (5), select channel C0 (bits 3-0)
	char sensorvalue;
	
    while (1) 
    {
		// turn portC lights off
		PORTC |= 0b00000110;
		PORTD |= 0b00000010;
		
// knock listen mode ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~		
		if ((PIND & 0x01) == 0) {
			// reset guess, counter, turn light on
			guess = 0x00;
			counter = 1;
			PORTC &= 0b11111101;
			
			for (int i = 8; i--; i>=0) {
				// turn light on and get the next digit
				PORTC &= 0b11111101;
				nextDig = getDigit(1000);
				// if the digit is 1, add it to password, otherwise leave as 0
				if (nextDig == 1) {
					switch (counter) {
						case 1:
							guess |= 0b10000000;
							break;
						case 2:
							guess |= 0b01000000;
							break;
						case 3:
							guess |= 0b00100000;
							break;
						case 4:
							guess |= 0b00010000;
							break;
						case 5:
							guess |= 0b00001000;
							break;
						case 6:
							guess |= 0b00000100;
							break;
						case 7:
							guess |= 0b00000010;
							break;
						case 8:
							guess |= 0b00000001;
							break;
					}
				}
				// increment counter, blink light
				counter++;
				PORTC |= 0b00000010;
				wait(500, 2);
			}
			if (password == guess) {
				unlockdoor();
				wait(4000, 2);
			}
			
		}
		
// Lock switch ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~	
		if ((PIND & 0x02) == 0) {		
		
			unlockdoor();			
			wait(4000, 2);
		
		}		
		
// password programming ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		if ((PIND & 0x04) == 0) {
			// reset password, counter, turn light on
			password = 0x00;
			counter = 1;
			PORTC &= 0b11111011;
			
			for (int i = 8; i--; i>=0) {
				// turn light on and get the next digit
				PORTC &= 0b11111011;
				nextDig = getDigit(1000);
				// if the digit is 1, add it to password, otherwise leave as 0
				if (nextDig == 1) {
					switch (counter) {
						case 1:
							password |= 0b10000000;
							break;
						case 2:
							password |= 0b01000000;
							break;
						case 3:
							password |= 0b00100000;
							break;
						case 4:
							password |= 0b00010000;
							break;
						case 5:
							password |= 0b00001000;
							break;
						case 6:
							password |= 0b00000100;
							break;
						case 7:
							password |= 0b00000010;
							break;
						case 8:
							password |= 0b00000001;
							break;
					}
				} 
				// increment counter, blink light
				counter++;
				PORTC |= 0b00000100;
				wait(500, 2);
			}
			// blink light to signal end of password
			for (int i = 8; i--; i>=0) {
				PORTC &= 0b11111011;
				wait(250, 2);
				PORTC |= 0b00000100;
				wait(250, 2);
			}
			
			// blink password to confirm to user the correct password was recorded
			counter = 1;
			for (int i = 8; i--; i>=0) {
				
				switch (counter) {
					case 1:
						if (password & 0b10000000) {
							PORTC &= 0b11111011;
						} else {
							PORTC |= 0b00000100;
						}
						wait(1000,2);
						break;
					case 2:
						if (password & 0b01000000) {
							PORTC &= 0b11111011;
						} else {
							PORTC |= 0b00000100;
						}
						wait(1000,2);
						break;
					case 3:
						if (password & 0b00100000) {
							PORTC &= 0b11111011;
						} else {
							PORTC |= 0b00000100;
						}
						wait(1000,2);
						break;
					case 4:
						if (password & 0b00010000) {
							PORTC &= 0b11111011;
						} else {
							PORTC |= 0b00000100;
						}
						wait(1000,2);
						break;
					case 5:
						if (password & 0b00001000) {
							PORTC &= 0b11111011;
						} else {
							PORTC |= 0b00000100;
						}
						wait(1000,2);
						break;
					break;
					case 6:
						if (password & 0b00000100) {
							PORTC &= 0b11111011;
						} else {
							PORTC |= 0b00000100;
						}
						wait(1000,2);
						break;
					case 7:
						if (password & 0b00000010) {
							PORTC &= 0b11111011;
						} else {
							PORTC |= 0b00000100;
						}
						wait(1000,2);
						break;
					break;
					case 8:
						if (password & 0b00000001) {
							PORTC &= 0b11111011;
						} else {
							PORTC |= 0b00000100;
						}
						wait(1000,2);
						break;
				}
				counter++;
			}
		} 		
		
// limit switch automatic lock ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~	
		if (PINC & 0b00010000) {
			lockdoor();
		}
				
    }
}

// does setup to get password ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int getDigit(volatile int multiple) {
	int temp;
	int digit = 0;
	while (multiple >0) {
		temp = knockListen();
		if (temp == 1) {
			digit = 1;
		}
		multiple--;
	}
	return digit;
}

// knock listening and timing ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int knockListen() {
	int sensorvalue;
	int digit = 0;
	
	TCCR0A = 0x00;
	TCNT0 = 0x00;
	TCCR0B = 1<<CS01 | 1<<CS00;
	
	while(TCNT0 < 250) {
		// Read analog input
		ADCSRA |= (1<<ADSC);	// Start conversion
		while((ADCSRA & (1<<ADIF)) == 0);	// wait for conversion to finish
		sensorvalue = ADCH; // keep high byte of 10 bit result
		PORTD = sensorvalue ^ 0xFF;
		if(sensorvalue > 32) {
			digit = 1;
		}
	}
	TCCR0A = 0x00;
	TCNT0 = 0x00;
	loop = 1;
	return digit;
}

// wait function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void wait(volatile int multiple, volatile char time_choice) {
	while (multiple >0) {
		delay_T_msec_timer0(time_choice);
		multiple--;
	}
}

// delay function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void delay_T_msec_timer0(volatile char choice) {
	TCCR0A = 0x00;
	TCNT0 = 0x00;
	switch (choice) {
		case 1:
		TCCR0B = 1<<CS01;
		break;
		case 2:
		TCCR0B = 1<<CS01 | 1<<CS00;
		break;
		case 3:
		TCCR0B = 1<<CS02;
		break;
		case 4:
		TCCR0B = 1<<CS02 | 1<<CS00;
		break;
		default:
		TCCR0B = 1<<CS00;
		break;
		
	}
	while(TCNT0 < 250);
	TCCR0A = 0x00;
	TCNT0 = 0x00;
	loop = 1;
}

// motor step CW ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void step_CW() {
	// moves the motor clockwise one step
	switch(phase_step) {
		case 1:
		// step to 4
		PORTD = 0b00100000;
		phase_step = 4;
		break;
		case 2:
		// step to 1
		PORTD = 0b10000000;
		phase_step = 1;
		break;
		case 3:
		// step to 2
		PORTD = 0b00010000;
		phase_step = 2;
		break;
		case 4:
		// step to 3
		PORTD = 0b01000000;
		phase_step = 3;
		break;
	}
}

// motor step CCW ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void step_CCW() {
	// moves the motor counterclockwise 1 step
	switch(phase_step) {
		case 1:
		// step to 2
		PORTD = 0b00010000;
		phase_step = 2;
		break;
		case 2:
		// step to 3
		PORTD = 0b01000000;
		phase_step = 3;
		break;
		case 3:
		// step to 4
		PORTD = 0b00100000;
		phase_step = 4;
		break;
		case 4:
		// step to 1
		PORTD = 0b10000000;
		phase_step = 1;
		break;
	}
}

// remote interrupt ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ISR(INT1_vect) {
	
	unlockdoor();
		
	//resetting timer
	TCCR0A = 0x00;
	TCNT0 = 5;
	TCCR0B = 1<<CS00;
}

// lock door ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void lockdoor() {
	if (doorStatus == 1) {
		int i = 0;
		while (i<90) {
			step_CW();
			wait(25, 2);
			i++;
		}
		doorStatus = 0;
	}
}

// unlock door ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void unlockdoor() {
	if (doorStatus == 0){
		int i = 0;
		while (i<90) {
			step_CCW();
			wait(25, 2);
			i++;
		}
		doorStatus = 1;
	}
}