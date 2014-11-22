/*
 * adc.h
 *
 *  Created on: 22 lis 2014
 *      Author: adrian
 */

#ifndef ADC_ADC_H_
#define ADC_ADC_H_

enum kanal_adc
{
    ADC0 = 0,
	ADC1, ADC2, ADC3, ADC4, ADC5, ADC6, ADC7
};

void ADC_init();
int ADC_read(enum kanal_adc);

#endif /* ADC_ADC_H_ */
