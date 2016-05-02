/*
 * test_moto_inlezen.c
 *
 * Created: 18/04/2016 11:43:50
 *  Author: Thomas
 */ 


#include <avr/io.h>
#define UITGANGV	PB6
#define UITGANGA	PB7
#define UITGANGL	PD5
#define UITGANGR	PD6
#define INGANGV		PC3
#define INGANGA		PC2
#define INGANGL		PC1
#define INGANGR		PC0


int main(void)
{
	DDRB |=  (1<<UITGANGV);
	DDRB |=  (1<<UITGANGA);
	DDRD |=  (1<<UITGANGL);
	DDRD |=  (1<<UITGANGR);
	DDRC &= ~(1<<INGANGV);
	DDRC &= ~(1<<INGANGA);
	DDRC &= ~(1<<INGANGL);
	DDRC &= ~(1<<INGANGR);
    while(1)
    {
		if(PINC & (1<<INGANGV))
		{
			PORTB |=(1<<UITGANGV);
			if(PINC & (1<<INGANGL))
			{
				PORTD |=(1<<UITGANGL);
			}
			else if(PINC & (1<<INGANGR))
			{
				PORTD |= (1<<UITGANGR);
			}
			else
			{
				PORTD &=~(1<<UITGANGL);
				PORTD &=~(1<<UITGANGR);
			}
		}
		else if(PINC & (1<<INGANGA))
		{
			PORTB |=(1<<UITGANGA);
			if(PINC & (1<<INGANGL))
			{
				PORTD |=(1<<UITGANGL);
			}
			else if(PINC & (1<<INGANGR))
			{
				PORTD |= (1<<UITGANGR);
			}
			else
			{
				PORTD &=~(1<<UITGANGL);
				PORTD &=~(1<<UITGANGR);
			}
		}
		else if(PINC & (1<<INGANGL))
		{
			PORTD |=(1<<UITGANGL);
		}
		else if(PINC & (1<<INGANGR))
		{
			PORTD |=(1<<UITGANGR);
		}
		else
		{
			PORTB &=~(1<<UITGANGV);
			PORTB &=~(1<<UITGANGA);
			PORTD &=~(1<<UITGANGL);
			PORTD &=~(1<<UITGANGR);
				
		}
        //TODO:: Please write your application code 
    }
}