/*
 * ADXL345.c
 *
 *  Created on: 22 lis 2014
 *      Author: adrian
 */

#include "ADXL345.h"
#include "../I2C/i2cmaster.h"

#define TO_READ 6


char Acc_writeTo(char device_address, char address, char val)
{
	if(i2c_start(device_address | I2C_WRITE) == 0 && i2c_write(address) == 0 && i2c_write(val) == 0)
	{
		i2c_stop();
		return 1; //sukces
	}
	else
	{
		i2c_stop();
		return 0; //porazka
	}


}

int Acc_powerOn(char device_address)
{
  gains[0] = 0.00376390;
  gains[1] = 0.00376009;
  gains[2] = 0.00349265;

  Acc_writeTo(device_address, ADXL345_POWER_CTL, 0);
  Acc_writeTo(device_address, ADXL345_POWER_CTL, 16);
  Acc_writeTo(device_address, ADXL345_POWER_CTL, 8);
}


void Acc_readFrom(char device_address, char address, int num, char buff[])
{
	for(int i = 0; i < num; i++)
	{
		i2c_start_wait(device_address | I2C_WRITE);
		i2c_write(address + i);
		i2c_rep_start(device_address | I2C_READ);
		buff[i] = i2c_readNak();
	}

	i2c_stop();
}

void Acc_readAccel(char device_address, int *x, int *y, int *z)
{
  Acc_readFrom(device_address, ADXL345_DATAX0, TO_READ, _buff);

  *x = (((int)_buff[1]) << 8) | _buff[0];
  *y = (((int)_buff[3]) << 8) | _buff[2];
  *z = (((int)_buff[5]) << 8) | _buff[4];
}

void Acc_get_Gxyz(char device_address, double *xyz)
{
  Acc_readAccel(device_address, (int*)xyz, (int*)(xyz + 1), (int*)(xyz + 2));
  for(int i = 0; i < 3; i++)
  {
	  *(xyz + i) *= gains[i];
  }
}
