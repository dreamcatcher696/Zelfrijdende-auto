//		Inlcludes/Defines		//
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
#define ECHO_F		PD2
#define ECHO_L		PD1
#define ECHO_R		PC5
#define TRIGGER_F	PD4
#define TRIGGER_L	PD7
#define TRIGGER_R	PB0
#define MODE_SEL1	PD0
#define MODE_SEL2	PD3
#define AFSTAND		20
//		Aanmaken snel geheugen om stop te schrijven		//
volatile uint8_t	stop_F = 0;			//als stop == 0 dan geen object gedetecteerd
volatile uint8_t	stop_L = 0;
volatile uint8_t	stop_R = 0;
volatile uint8_t	afstand_L = 0;
volatile uint8_t	afstand_R = 0;
volatile uint8_t	richting = 0;		//0 - vooruit	1 - links	2 - rechts	3 - achteruit
volatile uint8_t	stoppen = 0;
volatile uint8_t	stopdebounce = 0;
//		Functie declaraties		//
void autonoom();
void manueel();
void timer1Start();
void timer1Stop();
void timer0Start();
void timer0Stop();
void timer2Start();
void timer2Stop();

void vooruit(uint8_t);
void achteruit(uint8_t, uint8_t);
void links();
void rechts();
void centreer();

// ###################### MAIN ######################

int main(void)
{
	//		poort declaratie(IO)		//
	DDRB |=  (1<<UITGANGV);
	DDRB |=  (1<<UITGANGA);
	DDRD |=  (1<<UITGANGL);
	DDRD |=  (1<<UITGANGR);
	DDRC &= ~(1<<INGANGV);
	DDRC &= ~(1<<INGANGA);
	DDRC &= ~(1<<INGANGL);
	DDRC &= ~(1<<INGANGR);
	DDRD &= ~(1<<ECHO_F);
	DDRD &= ~(1<<ECHO_L);
	DDRC &= ~(1<<ECHO_R);
	DDRD &= ~(1<<MODE_SEL1);
	DDRD &= ~(1<<MODE_SEL2);
	DDRD |=  (1<<TRIGGER_F);
	DDRD |=	 (1<<TRIGGER_L);
	DDRB |=  (1<<TRIGGER_R);
	
	//interrupt initialiseren
	
	sei();				//global interrupts enable (SREG interrupts 1)
	EICRA |= (1<<ISC01)| (1<<ISC00);   //rising edge interrupt controle (ISC00 op 0 en isc 01 op 1 voor falling)
	EIMSK |= (1<<INT0);					//enablen van external interrupt 0
	
	//PCICR voor enabken van pin change interrupt[23-16] enz.
	//PCMSK voor enablen van apparte interrupts?
	//PCIFR bevat de interrupt flags
	
	PCICR |= (1<<PCIE2);
	PCICR |= (1<<PCIE1);	//enablen van pin change interrupt 23tot 6 en 14 tot 8
	PCMSK2 |=(1<<PCINT17);				//enablen van pin change interrupt 17
	PCMSK1 |=(1<<PCINT13);				//enablen van pin change interrupt 13
	
	TCNT0 = 0;					//timer register op 0 zetten
	TCNT1 = 0;							//TCCR0B voor prescaler settings(CS02 op 1 voor 256 prescaler=>max range = 1.4m)
	TCNT2 = 0;							//TCCR2B ook op 256 prescaler => timer puls om de (8000000/256)==>1/31250==> om de 32 microseconden==> 32*255 = max lengte van object ==>/58==>1.4m
	//TCCR2B CS22=> 1 CS21 => 1 CS20==> 0
	OCR1A = 38000;
	TCCR1B |= (1<<WGM12);		//WGM12= ctc mode , compraen met ocr1A ...CS11 is 8 bit prescaler => counter om de 1us
	//TCNT1 = 0;					//timer register op 0 zetten
	while(1)
	{
		if(PIND & (1<<MODE_SEL1))
		{
			autonoom();
		}
		if(PIND & (1<<MODE_SEL2))
		{
			manueel();
		}
		_delay_ms(1000);
	}
}

// ###################### AUTONOME FUNCTIE ######################

void autonoom()
{
	volatile uint8_t stoppen_auto = 0;
	while(PIND & (1<<MODE_SEL1))
	{
		//trigger signaal sturen
		PORTD &= ~(1<<TRIGGER_F);
		PORTD |= (1<<TRIGGER_F);
		_delay_us(10);
		PORTD &= ~(1<<TRIGGER_F);
		_delay_ms(30);
		//eind trigger signaal
		
		if(stop_F == 0)
		{
			//vooruit rijden
			richting = 0;
			stoppen_auto = 0;
			centreer();
			vooruit(1);
		}
		else
		{
			if(stoppen_auto == 0)
			{
				
				achteruit(1, 1);					//remmen, functie 1
				stoppen_auto = 1;
			}
			//trigger signaal LEFT sturen
			PORTD &= ~(1<<TRIGGER_L);
			PORTD |= (1<<TRIGGER_L);
			_delay_us(10);
			PORTD &= ~(1<<TRIGGER_L);
			//eind trigger signaal LEFT
			
			//trigger signaal RIGHT sturen
			PORTB &= ~(1<<TRIGGER_R);
			PORTB |= (1<<TRIGGER_R);
			_delay_us(10);
			PORTB &= ~(1<<TRIGGER_R);
			//eind trigger RIGHT signaal
			
			_delay_ms(30);							//wachten op echo signalen
			
			if(richting == 0)
			{
				if((afstand_R >= afstand_L) && (stop_R == 0))
				{
					//rechts rijden
					richting = 2;
				}
				else if(stop_L == 0)
				{
					//links rijden
					richting = 1;
				}
				else
				{
					//achteruit rijden
					richting = 3;
				}
			}
			else
			{
				if(richting == 1)		//links
				{
					links();
					for(int i=0;i<10;i++)
					{
						vooruit(1);
					}
					
				}
				else if(richting == 2)		//rechts
				{
					rechts();
					for(int i=0;i<10;i++)
					{
						vooruit(1);
					}
					
				}
				else if(richting == 3)		//achteruit
				{
					centreer();
					for(int i = 0; i < 10; i++)
					{
						achteruit(0, 1);	//achteruit 1ste functie
					}
				}
				richting = 0;
			}
		}
	}
}

// ###################### MANUELE FUNCTIE ######################

void manueel()
{
	while(PIND & (1<<MODE_SEL2))
	{
		//trigger signaal sturen
		PORTD &= ~(1<<TRIGGER_F);
		PORTD |= (1<<TRIGGER_F);
		_delay_us(10);
		PORTD &= ~(1<<TRIGGER_F);
		_delay_ms(30);
		//eind trigger signaal
		if((PINC & (1<<INGANGV)) && (stop_F==0))
		{
			vooruit(2);
			
			stoppen = 0;
			if(PINC & (1<<INGANGL))
			{
				links();
			}
			else if(PINC & (1<<INGANGR))
			{
				rechts();
			}
			else
			{
				centreer();
			}
		}
		else if(PINC & (1<<INGANGA))
		{
			achteruit(0, 2);				//achteruit, 2de functie
			if(PINC & (1<<INGANGL))
			{
				links();
			}
			else if(PINC & (1<<INGANGR))
			{
				rechts();
			}
			else
			{
				centreer();
			}
		}
		else if(PINC & (1<<INGANGL))
		{
			links();
		}
		else if(PINC & (1<<INGANGR))
		{
			rechts();
		}
		else
		{
			centreer();
		}
		if(stop_F==1)
		{
			if(stoppen == 0)
			{
				if(stopdebounce < 2)
				{
					stopdebounce++;
				}
				else
				{
					stoppen = 1;
					centreer();
					achteruit(1, 2);		//remmen, 2de functie
					stopdebounce = 0;
				}
			}
		}
		else stopdebounce = 0;
	}
	stoppen = 0;
	stop_F = 0;
}

// ###################### INTERRUPTS ######################

ISR(INT0_vect)
{
	static uint8_t status1 = 1; //als status==1 dan is de pin hoog. en omgekeerd
	static uint16_t timerwaarde1  =0;
	if(status1==1)
	{
		timer1Start();
		status1 = 0;
	}
	else
	{
		timer1Stop();
		status1 = 1;
		timerwaarde1 = TCNT1 / 58;
		if(timerwaarde1 <= 50)
		{
			stop_F = 1;
		}
		else
		{
			stop_F = 0;
		}
	}
}
ISR(PCINT1_vect)
{
	static uint8_t status0 = 1; //als status==1 dan is de pin hoog. en omgekeerd
	static uint16_t timerwaarde0  =0;
	if(status0==1)
	{
		timer0Start();
		status0 = 0;
	}
	else
	{
		timer0Stop();
		status0 = 1;
		//timerwaarde1 = TCNT0 / 1856;		//1 timer waarde om de 32 us. dus delen door 32 om 1 us te hebben, dan delen door 58 om naar cm te gaan
		timerwaarde0 = TCNT0 ;
		afstand_R = timerwaarde0;
		if(timerwaarde0 <= 20)
		{
			stop_R = 1;
		}
		else
		{
			stop_R = 0;
		}
	}
}
ISR(PCINT2_vect)
{
	static uint8_t status2 = 1; //als status==1 dan is de pin hoog. en omgekeerd
	static uint16_t timerwaarde2  =0;
	if(status2==1)
	{
		timer2Start();
		status2 = 0;
	}
	else
	{
		timer2Stop();
		status2 = 1;
		timerwaarde2 = TCNT2;
		afstand_L = timerwaarde2;
		if(timerwaarde2 <= 20)
		{
			stop_L = 1;
		}
		else
		{
			stop_L = 0;
		}
	}
}

ISR(TIMER1_COMPA_vect)
{
	//niets gemeten
}

void timer1Start()
{
	TCNT1 = 0;
	EICRA &= ~(1<<ISC00);	//volgende keer op faling edge interrupten
	TCCR1B |= (1<<CS11);//prescaler(8) zetten = `klok starten
}
void timer1Stop()
{
	EICRA |=(1<<ISC00);		//volgende keer op rising edge interrupten
	TCCR1B &= ~(1<<CS11);
}

void timer0Start()
{
	TCNT0 = 0;
	TCCR0B |= (1<<CS02);
}

void timer0Stop()
{
	TCCR0B &= ~(1<<CS02);
}

void timer2Start()
{
	TCNT2 = 0;
	TCCR2B |= (1<<CS22);
	TCCR2B |= (1<<CS21);
}
void timer2Stop()
{
	TCCR2B &= ~(1<<CS22);
	TCCR2B &= ~(1<<CS21);
}

// ###################### MOTORSTURING ######################

void vooruit(uint8_t modus)
{
	PORTB |=(1<<UITGANGV);					//gas geven
	if(modus == 1) _delay_ms(10);							//niet te snel rijden (auto niet kapot rijden tijdens tests!)
	else _delay_ms(20);
	PORTB &=~(1<<UITGANGV);
	
}
void achteruit(uint8_t remmen, uint8_t modus)		//remmen, modus
{
	if (remmen == 1)
	{
		PORTB |=(1<<UITGANGA);					//achteruit rijden
		if(modus == 1) _delay_ms(100);
		else _delay_ms(250);
		PORTB &=~(1<<UITGANGA);
	}
	else
	{
		PORTB |=(1<<UITGANGA);
		if(modus == 1) _delay_ms(10);			//achteruit rijden
		else _delay_ms(20);
		PORTB &=~(1<<UITGANGA);
	}
}
void links()
{
	PORTD &=~(1<<UITGANGR);
	PORTD |=(1<<UITGANGL);					//wielen naar links draaien
}
void rechts()
{
	PORTD &=~(1<<UITGANGL);
	PORTD |=(1<<UITGANGR);					//wielen naar rechts draaien -> blijven in deze positie tot de weg voor de auto vrij is
}
void centreer()
{
	PORTD &=~(1<<UITGANGL);					//wielen centreren
	PORTD &=~(1<<UITGANGR);
}