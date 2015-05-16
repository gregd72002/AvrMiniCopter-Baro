/*
	SFE_BMP180.h
	Bosch BMP180 pressure sensor library for the Arduino microcontroller
	Mike Grusin, SparkFun Electronics

	Uses floating-point equations from the Weather Station Data Logger project
	http://wmrx00.sourceforge.net/
	http://wmrx00.sourceforge.net/Arduino/BMP085-Calcs.pdf

	Forked from BMP085 library by M.Grusin

	version 1.0 2013/09/20 initial version
	
	Our example code uses the "beerware" license. You can do anything
	you like with this code. No really, anything. If you find it useful,
	buy me a (root) beer someday.
*/

#ifndef BMP180_h
#define BMP180_h

#include "../avrbaro.h"
#include <inttypes.h>

int bs_open(uint8_t _addr);
int bs_update(struct s_baro *s);
int bs_close();

int getPressure(float &P);
int preparePressure(int oversampling);
int getTemperature(float &T);
int prepareTemperature(void);

int bsWriteBytes(unsigned char *values, char length);
int bsReadBytes(unsigned char *values, char length);
int bsReadU(unsigned char address, unsigned short &value);
int bsReadS(unsigned char address, short &value);
#endif
