/* Header is not present in Windows CE SDK */

#ifndef A800_TIME_H
#define A800_TIME_H

#include <stdlib.h>

struct tm
{
	short tm_year;
	short tm_mon;
	short tm_mday;
	short tm_wday;
	short tm_hour;
	short tm_min;
	short tm_sec;
};

time_t time(time_t* dummy);
struct tm* localtime(time_t* dummy);

unsigned int clock();

#endif
