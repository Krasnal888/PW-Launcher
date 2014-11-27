#include "../I2C/i2cmaster.h"
#define TO_READ (6)        //num of chars we are going to read each time (two chars for each axis)

char buff[TO_READ] ;    //6 chars buffer for saving data read from the device
#define ADXL345_RANGE2G 0x00
#define ADXL345_FULLRANGE 0 //1 to enable 0 to disable

void Acc_turn_on(int device)
{
	writeTo(device, 0x2D, 0);
	writeTo(device, 0x2D, 16);
	writeTo(device, 0x2D, 8);

	uint8_t range = ADXL345_RANGE2G | (ADXL345_FULLRANGE<<3);
	writeTo(device, 0x31, range);
}

void Acc_read(int device, int *x, int *y, int *z)
{
	int regAddress = 0x32;    //first axis-acceleration-data register on the ADXL345

	readFrom(device, regAddress, TO_READ, buff); //read the acceleration data from the ADXL345

	//each axis reading comes in 10 bit resolution, ie 2 chars.  Least Significat char first!!
	//thus we are converting both chars in to one int
	*x = (((int)buff[1]) << 8) | buff[0];
	*y = (((int)buff[3])<< 8) | buff[2];
	*z = (((int)buff[5]) << 8) | buff[4];
}

//Writes val to address register on device
void writeTo(int device, char address, char val)
{
	i2c_start_wait(device+I2C_WRITE);
	i2c_write(address);
	i2c_write(val);
	i2c_stop();
}

//reads num chars starting from address register on device in to buff array
void readFrom(int device, char address, int num, char buff[]) {
	i2c_start_wait(device+I2C_WRITE);
	i2c_write(address);
	i2c_stop();

	int i = 0;
	i2c_start_wait(device+I2C_READ);
	for(int i = 0; i < num - 1; i++)
	{
		buff[i] = i2c_read(1);
	}

	buff[num - 1] = i2c_read(0);

	i2c_stop();
}

void Acc_get_Gxyz(int device, int *xyz)
{
	Acc_read(device, xyz, xyz + 1, xyz + 2);
}



