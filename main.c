//Załączone biblioteki AVR
#include <avr/io.h>
#include <util/delay.h>

//Załączone zewnętrzne biblioteki
#include "1Wire/ds18x20.h"
#include "sd/ff.h"
#include "I2C/i2cmaster.h"
#include "adc/adc.h"
#include "adxl345/adxl345.h"

//Do zapisu na SD
FATFS FatFs;	//Ustawienie systemu plików
FIL Dump;		//Do obsługi otwartego pliku

//Do termometru DS18x20
uint8_t czujniki_cnt;	//liczba czujników
uint8_t subzero;	//określenie + czy - temp
uint8_t cel;		//Temperatura
uint8_t cel_fract_bits;	//Wartość po przecinku temperatury

#define CISNIENIE1 ADC0
#define CISNIENIE2 ADC1

#define AKCELEROMETR1 (0x1D<<1)
#define AKCELEROMETR2 (0x53<<1)

//Deklaracje funkcji


//Main
int main(void){
	ADC_init(); //init barometrow
	Acc_turn_on(AKCELEROMETR1);
	Acc_turn_on(AKCELEROMETR2);

	czujniki_cnt = search_sensors();
	UINT bw;	//Bajty zapisane
	char bufor[32];

	int akcelerometr1_odczyty[3];
	int akcelerometr2_odczyty[3];

	while(1)
	{
		f_mount(&FatFs,"",0);
		f_open(&Dump,"Test.txt",FA_WRITE | FA_OPEN_ALWAYS);

		int bar1 = ADC_read(CISNIENIE1);
		int bar2 = ADC_read(CISNIENIE2);
		Acc_get_Gxyz(AKCELEROMETR1, akcelerometr1_odczyty);
		Acc_get_Gxyz(AKCELEROMETR2, akcelerometr2_odczyty);

		DS18X20_start_meas( DS18X20_POWER_EXTERN, NULL ); // termometry

		_delay_ms(750);

		sprintf(bufor,"A1 %f %f %f A2 %f %f %f",
				akcelerometr1_odczyty[0], akcelerometr1_odczyty[1], akcelerometr1_odczyty[2],
				akcelerometr2_odczyty[0], akcelerometr2_odczyty[1], akcelerometr2_odczyty[2]);
		f_lseek(&Dump,f_size(&Dump));
		f_write(&Dump,bufor,sizeof(bufor),&bw);
		f_sync(&Dump);

		sprintf(bufor,"C1 %d C2 %d",
						bar1,
						bar2);
		f_lseek(&Dump,f_size(&Dump));
		f_write(&Dump,bufor,sizeof(bufor),&bw);
		f_sync(&Dump);

		for(int sensor_numer = 0; sensor_numer < czujniki_cnt; sensor_numer++)
		{
			if(DS18X20_OK == DS18X20_read_meas(gSensorIDs[sensor_numer], &subzero, &cel, &cel_fract_bits))
			{
				if (subzero == 0){
					sprintf(bufor," +%d,%d ",cel,cel_fract_bits);
					f_lseek(&Dump,f_size(&Dump));
					f_write(&Dump,bufor,7,&bw);
					f_sync(&Dump);
					_delay_ms(20);
				}
				if (subzero == 1){
					sprintf(bufor," -%d,%d ",cel,cel_fract_bits);
					f_lseek(&Dump,f_size(&Dump));
					f_write(&Dump,bufor,7,&bw);
					f_sync(&Dump);
					_delay_ms(20);
				}
			}
		}

		//nowa linia
		sprintf(bufor,"\n");
		f_lseek(&Dump,f_size(&Dump));
		f_write(&Dump,bufor,7,&bw);
		f_sync(&Dump);
		f_close(&Dump);

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
