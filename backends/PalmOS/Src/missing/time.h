#include <PalmOS.h>

typedef UInt32 time_t;

struct tm {
	Int16 tm_sec;		//	seconds [0,61]
	Int16 tm_min;		//	minutes [0,59]
	Int16 tm_hour;		//	hour [0,23]
	Int16 tm_mday;		//	day of month [1,31]
	Int16 tm_mon;		//	month of year [0,11]
	Int16 tm_year;		//	years since 1900
	Int16 tm_wday;		//	day of week [0,6] (Sunday = 0)
	Int16 tm_yday;		//	day of year [0,365]
	Int16 tm_isdst;		//	daylight savings flag
};

time_t time(time_t *tloc);
struct tm *localtime(const time_t *timer);
