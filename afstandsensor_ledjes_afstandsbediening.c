/*
 * test_moto_inlezen.c
 *
 * Created: 18/04/2016 11:43:50
 *  Author: Thomas
 */ 


#include <avr/io.h>
#define F_CPU 8000000
#include <avr/interrupt.h>
#include <util/delay.h>
#define UITGANGV	PB6
#define UITGANGA	PB7
#define UITGANGL	PD5
#define UITGANGR	PD6
#define INGANGV		PC3
#define INGANGA		PC2
#define INGANGL		PC1
#define INGANGR		PC0
#define ECHO		PD2
#define TRIGGER_F	PD4
#define TRIGGER_L	PD7
#define TRIGGER_R	PB0
#define AFSTAND		20
volatile uint8_t	stop_F = 0;			//als stop == 0 dan geen object gedetecteerd
volatile uint8_t	stop_L = 0;
volatile uint8_t	stop_R = 0;
volatile uint8_t	poll_richting = 0;	//0=vooruit 1=links 2 = rechts


void timerStart();
void timerStop();

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
	DDRD &= ~(1<<ECHO);
	DDRD |=  (1<<TRIGGER_F);
	DDRD |=	 (1<<TRIGGER_L);
	DDRB |=  (1<<TRIGGER_R);
	
	//interrupt initialiseren
	
	sei();				//global interrupts enable (SREG interrupts 1)
	EICRA |= (1<<ISC01)| (1<<ISC00);   //rising edge interrupt controle (ISC00 op 0 en isc 01 op 1 voor falling)
	EIMSK |= (1<<INT0);					//enablen van externat interrupt 0
	
	
	OCR1A = 38000;
	TCCR1B |= (1<<WGM12);		//WGM12= ctc mode , compraen met ocr1A ...CS11 is 8 bit prescaler => counter om de 1us
	TCNT1 = 0;					//timer register op 0 zetten
	
	
	
    while(1)
    {
		poll_richting=0;
		//trigger signaal sturen
		PORTD &= ~(1<<TRIGGER_F);
		PORTD |= (1<<TRIGGER_F);
		_delay_us(10);
		PORTD &= ~(1<<TRIGGER_F);
		//eind trigger signaal
		
		if((PINC & (1<<INGANGV)) && (stop_F==0))
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
		if(stop_F==1)
		{
			_delay_ms(6);
			//trigger signaal LEFT sturen
			poll_richting = 1;
			PORTD &= ~(1<<TRIGGER_L);
			PORTD |= (1<<TRIGGER_L);
			_delay_us(10);
			PORTD &= ~(1<<TRIGGER_L);
			_delay_ms(6);
			//eind trigger LEFT signaal	
			//trigger signaal RIGHT sturen
			poll_richting = 2;
			/*PORTB &= ~(1<<TRIGGER_R);
			PORTB |= (1<<TRIGGER_R);
			_delay_us(10);
			PORTB &= ~(1<<TRIGGER_R);
			_delay_ms(40);
			//eind trigger RIGHT signaal*/
			
			if(PINC & (1<<INGANGL) && (stop_L==1))
			{
				PORTD |=(1<<UITGANGL);
			}
			//else if(PINC & (1<<INGANGR) && (stop_R ==0))
			//{
			//	PORTD |=(1<<UITGANGR);
			//}
			else
			{
				PORTD &=~(1<<UITGANGL);
				PORTD &=~(1<<UITGANGR);
			}
		}
    }
}
ISR(INT0_vect)
{
	static uint8_t status = 1; //als status==1 dan is de pin hoog. en omgekeerd
	static uint16_t timerwaarde  =0;
	if(status==1)
	{
		timerStart();
		status = 0;
	}
	else
	{
		timerStop();
		status = 1;
		timerwaarde = TCNT1 / 58;
		if(timerwaarde <= 20)
		{
			if(poll_richting==0)
			{
				stop_F = 1;
			}
			else if(poll_richting==1)
			{
				stop_L = 1;
			}
			else if(poll_richting==2)
			{
				stop_R = 1;
			}
			
		}
		else
		{
			if(poll_richting==0)
			{
				stop_F = 0;
			}
			else if(poll_richting==1)
			{
				stop_L = 0;
			}
			else if(poll_richting==2)
			{
				stop_R = 0;;
			}
			
		}
	}
}

ISR(TIMER1_COMPA_vect)
{
	//niets gemeten
}

void timerStart()
{
	TCNT1 = 0;
	EICRA &= ~(1<<ISC00);	//volgende keer op faling edge interrupten
	TCCR1B |= (1<<CS11);//prescaler(8) zetten = `klok starten	
}
void timerStop()
{
	EICRA |=(1<<ISC00);		//volgende keer op rising edge interrupten
	TCCR1B &= ~(1<<CS11);	
}

