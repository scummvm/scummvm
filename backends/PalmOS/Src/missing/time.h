/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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
#ifndef __TIME_H__
#define __TIME_H__


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

#endif
