#define F_CPU 8000000UL
#define CTC_MATCH_OVERFLOW ((F_CPU / 1000) / 8)

//Załączone biblioteki AVR
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/atomic.h>

//Załączone zewnętrzne biblioteki
#include "1Wire/ds18x20.h"
#include "sd/ff.h"
#include "adc/adc.h"
#include "adxl345/adxl345.h"
#include "I2C/i2cmaster.h"

volatile unsigned long long int timer1_millis;

/*
 * PD6 - 1Wire
 * ADC0, ADC2 - barometry
 * PB0 - beeper
 * PD7 - LED
 */

#define LED 7
#define BEEP 0

//Do zapisu na SD
FATFS FatFs;	//Ustawienie systemu plików
FIL Dump_cisnienie;
FIL Dump_akcelerometr;
FIL Dump_temperatura;

//Do termometru DS18x20
uint8_t czujniki_cnt;	//liczba czujników
uint8_t subzero;	//określenie + czy - temp
uint8_t cel;		//Temperatura
uint8_t cel_fract_bits;	//Wartość po przecinku temperatury

char bufor[64];
int akcelerometr1_odczyty[3];
int akcelerometr2_odczyty[3];

#define CISNIENIE1 ADC0
#define CISNIENIE2 ADC2
#define AKCELEROMETR1 (0x53<<1) // 0 na SD0
#define AKCELEROMETR2 (0x1D<<1) // 1 na SD0

#define AKCELEROMETR_PLIK "acc.txt"
#define CISNIENIE_PLIK "pres.txt"
#define TEMPERATURA_PLIK "temp.txt"

//Deklaracje funkcji
void dopiszDoLogu(FIL *Dump, char *filename, char *text2);
volatile unsigned long long int millis ();
void odczytAkcelerometru(unsigned long long t);

//Main
int main(void){
	wdt_enable(WDTO_2S);

	f_mount(&FatFs,"",0);
	f_open(&Dump_cisnienie, CISNIENIE_PLIK, FA_WRITE | FA_OPEN_ALWAYS);
	f_open(&Dump_akcelerometr, AKCELEROMETR_PLIK, FA_WRITE | FA_OPEN_ALWAYS);
	f_open(&Dump_temperatura, TEMPERATURA_PLIK, FA_WRITE | FA_OPEN_ALWAYS);

	int akcelerometr1_odczyty[3];
	int akcelerometr2_odczyty[3];
	int bar1 = 0;
	int bar2 = 0;

	DDRD |= (1 << LED); //dioda
	PORTD |= (1 << LED);
	DDRB |= (1 << BEEP); //beep
	PORTB &= ~(1 << BEEP);

	i2c_init();
	ADC_init(); //init barometrow
	czujniki_cnt = search_sensors(); //init termometry

	Acc_turn_on(AKCELEROMETR1);
	Acc_turn_on(AKCELEROMETR2);

	UINT bw;	//Bajty zapisane

	TCCR1B |= (1 << WGM12) | (1 << CS11);
	OCR1AH = (CTC_MATCH_OVERFLOW >> 8);
	OCR1AL = CTC_MATCH_OVERFLOW;
	TIMSK |= (1 << OCIE1A);
	sei(); //wlaczenie przerywan

	volatile unsigned long long int ostatni_odczyt_temperatura = 0;
	volatile unsigned long long int ostatni_odczyt_akcelerometr = 0;

	while(1)
	{
		unsigned long aktualny_czas = millis();

		if (aktualny_czas - ostatni_odczyt_temperatura >= 1000)
		{
			wdt_reset();
			ostatni_odczyt_temperatura = aktualny_czas;

			PORTB ^=(1 << BEEP);
			PORTD ^=(1 << LED);

			bar1 = ADC_read(CISNIENIE1);
			bar2 = ADC_read(CISNIENIE2);

			sprintf(bufor,"%d,%d\n",
					bar1,
					bar2);
			dopiszDoLogu(&Dump_cisnienie, CISNIENIE_PLIK , bufor);

			sprintf(bufor,"%d,", aktualny_czas / 1000);
			dopiszDoLogu(&Dump_temperatura, TEMPERATURA_PLIK, bufor);
			for(int sensor_numer = 0; sensor_numer < czujniki_cnt; sensor_numer++)
			{
				if(DS18X20_OK == DS18X20_read_meas(gSensorIDs[sensor_numer], &subzero, &cel, &cel_fract_bits))
				{
					if (subzero == 0){
						sprintf(bufor,"ID%d,+%d.%d,",gSensorIDs[sensor_numer],cel,cel_fract_bits);
						dopiszDoLogu(&Dump_temperatura, TEMPERATURA_PLIK, bufor);

					}
					if (subzero == 1){
						sprintf(bufor,"ID%d,-%d.%d,",gSensorIDs[sensor_numer],cel,cel_fract_bits);
						dopiszDoLogu(&Dump_temperatura, TEMPERATURA_PLIK, bufor);
					}
				}
			}
			dopiszDoLogu(&Dump_temperatura, TEMPERATURA_PLIK, "\n");

			DS18X20_start_meas( DS18X20_POWER_EXTERN, NULL );
		}

		if (aktualny_czas - ostatni_odczyt_akcelerometr >= 250)
		{
			wdt_reset();
			ostatni_odczyt_akcelerometr = aktualny_czas;
			odczytAkcelerometru(ostatni_odczyt_akcelerometr);
		}
	}

}

ISR (TIMER1_COMPA_vect)
{
	timer1_millis++;
}

//Definicje funkcji
void dopiszDoLogu(FIL *Dump, char *filename, char *text2)
{
	UINT bw;	//Bajty zapisane
	if (f_lseek(Dump, f_size(Dump)) == FR_OK)
	{
		f_write(Dump, text2, strlen(text2), &bw);
		if(bw == 0)
			while(1);
	}
	else
	{
		while(1);
	}
	f_sync(Dump);
}

volatile unsigned long long int millis ()
{
	volatile unsigned long long int millis_return;

	// Ensure this cannot be disrupted
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		millis_return = timer1_millis;
	}

	return millis_return;
}

void odczytAkcelerometru(unsigned long long t)
{
	Acc_get_Gxyz(AKCELEROMETR1, akcelerometr1_odczyty);
	Acc_get_Gxyz(AKCELEROMETR2, akcelerometr2_odczyty);

	sprintf(bufor,"%d,%d,%d,%d,%d,%d,%d\n",
			(int)(t / 1000),
			akcelerometr1_odczyty[0], akcelerometr1_odczyty[1], akcelerometr1_odczyty[2],
			akcelerometr2_odczyty[0], akcelerometr2_odczyty[1], akcelerometr2_odczyty[2]);
	dopiszDoLogu(&Dump_akcelerometr, AKCELEROMETR_PLIK, bufor);
}

//Bez tej funkcji zapis na SD nie zadziała. Nie ruszać (chyba ze chce wam się implementować ją do biblioteki).
DWORD get_fattime (void)
{
	/* Returns current time packed into a DWORD variable */
	return	  ((DWORD)(2013 - 1980) << 25)	/* Year 2013 */
			| ((DWORD)7 << 21)				/* Month 7 */
			| ((DWORD)28 << 16)				/* Mday 28 */
			| ((DWORD)0 << 11)				/* Hour 0 */
			| ((DWORD)0 << 5)				/* Min 0 */
			| ((DWORD)0 >> 1);				/* Sec 0 */
}
