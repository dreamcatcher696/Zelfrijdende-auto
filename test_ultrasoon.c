/*
 * test_altrasone.c
 *
 * Created: 14/03/2016 10:24:32.1'
 *  Author: Thomas
 */ 


#include <avr/io.h>
#define F_CPU 8000000
#include <util/delay.h>
#define LED PB0
#define ECHO PD6
#define TRIGGER PD7

int main(void)
{
	DDRB |= (1<<LED);
	DDRD |= (1<<TRIGGER);
	DDRD &= ~(1<<ECHO);
	PORTB &= ~(1<<LED);
	uint16_t input = 0;
	//uint8_t teller = 0;
	//uint8_t pwmval = 0;
    while(1)
    {
        PORTD &= ~(1<<TRIGGER);
		PORTD |= (1<<TRIGGER);
		_delay_us(10);
		PORTD &= ~(1<<TRIGGER);
		while(!(PIND & (1<<ECHO))); //wachten op hoog
		while(PIND & (1<<ECHO)) //zolang hoog is 
		{
			input++;
			_delay_us(100);
		}
		// input = input * 15;
		 //input = input / 58;
		 if(input < 20)
		 {
			 PORTB |= (1<<LED);
			 
		 }
		 else
		 {
			 PORTB &= ~(1<<LED);
			 
		 }
		 input = 0;
		 
		 
    }
}