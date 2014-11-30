//Załączone biblioteki AVR
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

//Załączone zewnętrzne biblioteki
#include "1Wire/ds18x20.h"
#include "sd/ff.h"
#include "adc/adc.h"
#include "adxl345/adxl345.h"
#include "I2C/i2cmaster.h"

/*
 * PD6 - 1Wire
 * ADC0, ADC2 - barometry
 * PD0 - beeper
 * PD7 - LED
 */

#define LED 7
#define BEEP 0

//Do zapisu na SD
FATFS FatFs;	//Ustawienie systemu plików
FIL Dump;		//Do obsługi otwartego pliku

//Do termometru DS18x20
uint8_t czujniki_cnt;	//liczba czujników
uint8_t subzero;	//określenie + czy - temp
uint8_t cel;		//Temperatura
uint8_t cel_fract_bits;	//Wartość po przecinku temperatury

#define TIMER_START 6 //wartosc poczatkowa timera
volatile uint8_t cnt=0;
volatile unsigned int time = 0; //w sekundach

#define CISNIENIE1 ADC0
#define CISNIENIE2 ADC2
#define AKCELEROMETR1 (0x53<<1) // 0 na SD0
#define AKCELEROMETR2 (0x1D<<1) // 1 na SD0

//Deklaracje funkcji
void dopiszDoLogu(char *, char *);

//Main
int main(void){
	wdt_enable(WDTO_2S);

	int akcelerometr1_odczyty[3];
	int akcelerometr2_odczyty[3];
	int bar1 = 0;
	int bar2 = 0;

	DDRD |= (1 << LED); //dioda
	PORTD |= (1 << LED);
	DDRD |= (1 << BEEP); //beep
	PORTD |= (1 << BEEP);

	i2c_init();
	ADC_init(); //init barometrow
	czujniki_cnt = search_sensors(); //init termometry

	Acc_turn_on(AKCELEROMETR1);
	Acc_turn_on(AKCELEROMETR2);

	UINT bw;	//Bajty zapisane
	char bufor[64];

	TIMSK |= (1<<TOIE0);           //przerwanie overflow
	TCCR0 |= (1<<CS02) | (1<<CS00); //preskaler 1024
	TCNT0 = TIMER_START;//          //poczatkowa wartosc licznika

	sei(); //wlaczenie przerywan

	while(1)
	{
		wdt_reset();

		PORTD ^=(1 << BEEP);
		PORTD ^=(1 << LED);


		DS18X20_start_meas( DS18X20_POWER_EXTERN, NULL ); // termometry
		bar1 = ADC_read(CISNIENIE1);
		bar2 = ADC_read(CISNIENIE2);
		Acc_get_Gxyz(AKCELEROMETR1, akcelerometr1_odczyty);
		Acc_get_Gxyz(AKCELEROMETR2, akcelerometr2_odczyty);

		sprintf(bufor,"%d,%d,%d\n",
						time,
						bar1,
						bar2);
		dopiszDoLogu("press.txt", bufor);

		sprintf(bufor,"%d,%d,%d,%d,%d,%d\n",
						akcelerometr1_odczyty[0], akcelerometr1_odczyty[1], akcelerometr1_odczyty[2],
						akcelerometr2_odczyty[0], akcelerometr2_odczyty[1], akcelerometr2_odczyty[2]);
		dopiszDoLogu("akcel.txt", bufor);

		int t = time;
		while(time != (t + 1)); //czeka sekunde (delay blokuje przerywania)

		for(int sensor_numer = 0; sensor_numer < czujniki_cnt; sensor_numer++)
		{
			if(DS18X20_OK == DS18X20_read_meas(gSensorIDs[sensor_numer], &subzero, &cel, &cel_fract_bits))
			{
				if (subzero == 0){
					sprintf(bufor,"ID%d,+%d.%d,",gSensorIDs[sensor_numer],cel,cel_fract_bits);
					dopiszDoLogu("temp.txt", bufor);

				}
				if (subzero == 1){
					sprintf(bufor,"ID%d,-%d.%d,",gSensorIDs[sensor_numer],cel,cel_fract_bits);
					dopiszDoLogu("temp.txt", bufor);
				}
			}
		}

	}

}

//przerywanie
ISR(TIMER0_OVF_vect)
{
	TCNT0 = TIMER_START;          //Początkowa wartość licznika

	cnt++;     //zwiększa zmienną licznik
	if(cnt>3)  //jeśli 4 przerwania (czyli ok 1 s)
	{
		cnt=0;     //zeruje zmienną licznik
		time++;
	}

	if(time >= 10 && time < 15) //od 5 sekundy do 15
	{
		//wystaw jedynke
	}
	else
	{
		//i ja wylacz
	}

}

//Definicje funkcji
void dopiszDoLogu(char *filename, char *text2)
{
	UINT bw;	//Bajty zapisane
	f_mount(&FatFs,"",0);
	f_open(&Dump,filename,FA_WRITE | FA_OPEN_ALWAYS);

	if (f_lseek(&Dump, f_size(&Dump)) == FR_OK)
	{
		f_write(&Dump, text2, strlen(text2), &bw);
	}
	f_close(&Dump);
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
