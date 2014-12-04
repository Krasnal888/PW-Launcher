/*
 * adc.c
 *
 *  Created on: 22 lis 2014
 *      Author: adrian
 */
#include <avr/io.h>
#include "adc.h"

char adc_on = 0;

void ADC_init()
{
	ADCSRA = (1<<ADEN) //wlaczenie przetwornika
	         |(1<<ADPS1)
			 |(1<<ADPS2);

	ADMUX  =    (1<<REFS1) | (1<<REFS0); // ref 2,56V/1,2V, kondensator na AREF

	adc_on = 1;

}

int ADC_read(enum kanal_adc kanal)
{
	if(adc_on == 0)
		ADC_init();

	ADMUX |= (kanal << MUX0);

	ADCSRA |= (1<<ADSC); //uruchomienie konwersji
	while(ADCSRA & (1<<ADSC)); //czeka na zakończenie konwersji

	return ADC;
}
