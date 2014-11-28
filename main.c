//Załączone biblioteki AVR
#include <avr/io.h>
#include <util/delay.h>

//Załączone zewnętrzne biblioteki
#include "1Wire/ds18x20.h"
#include "sd/ff.h"
#include "adc/adc.h"
#include "adxl345/adxl345.h"

/*
 * PD6 - 1Wire
 * ADC0, ADC2 - barometry
 */

//Do zapisu na SD
FATFS FatFs;	//Ustawienie systemu plików
FIL Dump;		//Do obsługi otwartego pliku

//Do termometru DS18x20
uint8_t czujniki_cnt;	//liczba czujników
uint8_t subzero;	//określenie + czy - temp
uint8_t cel;		//Temperatura
uint8_t cel_fract_bits;	//Wartość po przecinku temperatury

#define CISNIENIE1 ADC0
#define CISNIENIE2 ADC2
#define AKCELEROMETR1 (0x1D<<1) // 1 na SD0
#define AKCELEROMETR2 (0x53<<1) // 0 na SD0
#define NAZWAPLIKU "TEST.TXT"

//PD7 dioda

//Deklaracje funkcji
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

//Main
int main(void){
	int akcelerometr1_odczyty[3];
	int akcelerometr2_odczyty[3];
	int bar1;
	int bar2;

	//Acc_turn_on(AKCELEROMETR1);
	//Acc_turn_on(AKCELEROMETR2);
	ADC_init(); //init barometrow
	czujniki_cnt = search_sensors(); //init termometry

	DDRD |= (1 << 7); //dioda


	UINT bw;	//Bajty zapisane
	char bufor[64];

	while(1)
	{
		if(PORTD & (1 << 7))
			PORTD &= ~(1 << 7);
		else
			PORTD |= (1 << 7);

		DS18X20_start_meas( DS18X20_POWER_EXTERN, NULL ); // termometry
		bar1 = ADC_read(CISNIENIE1);
		bar2 = ADC_read(CISNIENIE2);
		//Acc_get_Gxyz(AKCELEROMETR1, akcelerometr1_odczyty);
		//Acc_get_Gxyz(AKCELEROMETR2, akcelerometr2_odczyty);

		sprintf(bufor,"C1,%d,C2,%d",
						bar1,
						bar2);
		dopiszDoLogu(NAZWAPLIKU, bufor);

		/*sprintf(bufor,",A1,%d,%d,%d,A2,%d,%d,%d",
						akcelerometr1_odczyty[0], akcelerometr1_odczyty[1], akcelerometr1_odczyty[2],
						akcelerometr2_odczyty[0], akcelerometr2_odczyty[1], akcelerometr2_odczyty[2]);
		dopiszDoLogu(NAZWAPLIKU, bufor);*/

		_delay_ms(750);

		for(int sensor_numer = 0; sensor_numer < czujniki_cnt; sensor_numer++)
		{
			if(DS18X20_OK == DS18X20_read_meas(gSensorIDs[sensor_numer], &subzero, &cel, &cel_fract_bits))
			{
				if (subzero == 0){
					sprintf(bufor,",ID%d,+%d.%d",gSensorIDs[sensor_numer],cel,cel_fract_bits);
					dopiszDoLogu(NAZWAPLIKU, bufor);

				}
				if (subzero == 1){
					sprintf(bufor,",ID%d,-%d.%d",gSensorIDs[sensor_numer],cel,cel_fract_bits);
					dopiszDoLogu(NAZWAPLIKU, bufor);
				}
			}
		}
		dopiszDoLogu(NAZWAPLIKU, "\n");

	}

}

//Definicje funkcji


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
