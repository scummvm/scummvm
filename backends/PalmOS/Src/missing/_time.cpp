#include "time.h"

// ignore GMT, only device time

time_t time(time_t *tloc) {
	UInt32 secs = TimGetSeconds();					// since 1/1/1904 12AM.
	DateTimeType Epoch = {0, 0, 0, 1, 1, 1970, 0};	// form 1/1/1904 12AM to 1/1/1970 12AM

	secs -= TimDateTimeToSeconds (&Epoch);
	
	if (tloc)
		*tloc = secs;
	
	return (secs);
}


struct tm *localtime(const time_t *timer) {
	static struct tm tmDate;

	DateTimeType dt;
	UInt32 secs = *timer;
	DateTimeType Epoch = {0, 0, 0, 1, 1, 1970, 0};	// form 1/1/1904 12AM to 1/1/1970 12AM
	// timer supposed to be based on Epoch
	secs += TimDateTimeToSeconds(&Epoch);

	TimSecondsToDateTime (secs, &dt);
	
	tmDate.tm_sec	= dt.second;
	tmDate.tm_min	= dt.minute;
	tmDate.tm_hour	= dt.hour;
	tmDate.tm_mday	= dt.day;
	tmDate.tm_mon	= dt.month;
	tmDate.tm_year	= dt.year;
	tmDate.tm_wday	= dt.weekDay;
	
	return &tmDate;
}