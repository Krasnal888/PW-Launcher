//Załączone biblioteki AVR
#include <avr/io.h>
#include <util/delay.h>

//Załączone zewnętrzne biblioteki
#include "1Wire/ds18x20.h"
#include "sd/ff.h"
#include "I2C/i2cmaster.h"

//Deklaracje funkcji


//Main
int main(void){



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
