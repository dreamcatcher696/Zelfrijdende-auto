/*
 * ultrasone_interrupt_timers.c
 *
 * Created: 17/03/2016 10:24:32.1'
 *  Author: Thomas
 */ 


#include <avr/io.h>
#define F_CPU 8000000
#include <util/delay.h>
#include <avr/interrupt.h>
#define LED PB0
#define ECHO PD2 //pcint18//int0
#define TRIGGER PD7
#define INTLED PB1
#define TIMERLED PD5
volatile uint16_t teller = 0;			//sneller geheugen Danku olivier
volatile uint32_t timerteller = 0;
volatile uint16_t bewerking = 0;


void timerStart();
void timerStop();
int main(void)
{
	DDRB |= (1<<LED);
	DDRB |= (1<<INTLED);
	DDRD |= (1<<TIMERLED);
	DDRD |= (1<<TRIGGER);
	DDRD &= ~(1<<ECHO);
	PORTB &= ~(1<<LED);
	uint16_t input = 0;
	
	//uint8_t teller = 0;
	//uint8_t pwmval = 0;
	
	
	//interrupt initialiseren
	
	sei();				//global interrupts enable (SREG interrupts 1)
	EICRA |= (1<<ISC01)| (1<<ISC00);   //rising edge interrupt controle (ISC00 op 0 en isc 01 op 1 voor falling)
	EIMSK |= (1<<INT0);					//enablen van externat interrupt 0
	
	
	OCR1A = 38000;
	TCCR1B |= (1<<WGM12);		//WGM12= ctc mode , compraen met ocr1A ...CS11 is 8 bit prescaler => counter om de 1us
	TCNT1 = 0;					//timer register op 0 zetten
	
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
ISR(INT0_vect)
{
	static uint8_t status = 1; //als status==1 dan is de pin hoog. en omgekeerd
	static uint16_t teller = 0;
	if(status==1)
	{
		timerStart();
		status = 0;
		teller++;
	}
	else
	{
		timerStop();
		status = 1;
		bewerking = TCNT1 / 58;
		if(bewerking <= 20)
		{
			PORTD &= ~(1<<TIMERLED);
		}
		else
		{
			PORTD |= (1<<TIMERLED);
		}
	}
		if(teller > 1000)
		{
			PORTB ^= (1<<INTLED);
			teller = 0;
			
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