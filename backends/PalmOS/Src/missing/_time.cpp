/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2004 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "time.h"

// ignore GMT, only device time

time_t time(time_t *tloc) {
	UInt32 secs = TimGetSeconds();					// since 1/1/1904 12AM.
	DateTimeType Epoch = {1, 0, 0, 1, 1, 1970, 0};	// form 1/1/1904 12AM to 1/1/1970 12AM

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
	tmDate.tm_mon	= dt.month - 1;
	tmDate.tm_year	= dt.year - 1900;
	tmDate.tm_wday	= dt.weekDay;
	
	return &tmDate;
}