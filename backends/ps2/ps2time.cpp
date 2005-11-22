/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005 The ScummVM project
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */

#include "backends/ps2/systemps2.h"
#include "eecodyvdfs.h"
#include <osd_config.h>
#define FROM_BCD(a) ((a >> 4) * 10 + (a & 0xF))

static int	  g_timeSecs;
static int	  g_day, g_month, g_year;
static int	  g_lastTimeCheck;
extern volatile uint32 msecCount;

void buildNewDate(int dayDiff) {
	static int daysPerMonth[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	if (((g_year % 4) == 0) && (((g_year % 100) != 0) || ((g_year % 1000) == 0)))
		daysPerMonth[1] = 29;
	else
		daysPerMonth[1] = 28;

	if (dayDiff == -1) {
		g_day--;
		if (g_day == 0) {
			g_month--;
			if (g_month == 0) {
				g_year--;
				g_month = 12;
			}
			g_day = daysPerMonth[g_month - 1];
		}
	} else if (dayDiff == 1) {
		g_day++;
		if (g_day > daysPerMonth[g_month - 1]) {
			g_day = 1;
			g_month++;
			if (g_month > 12) {
				g_month = 1;
				g_year++;
			}
		}
	}
}

#define SECONDS_PER_DAY (24 * 60 * 60)

void OSystem_PS2::readRtcTime(void) {
	struct CdClock cdClock;
	readRTC(&cdClock);

	g_lastTimeCheck = getMillis();

	if (cdClock.stat) {
		msgPrintf(5000, "Unable to read RTC time, EC: %d\n", cdClock.stat);
		g_day = g_month = 1;
		g_year = 0;
		g_timeSecs = 0;
	} else {
		int gmtOfs = configGetTimezone();
		if (configIsDaylightSavingEnabled())
			gmtOfs += 60;

		int timeSecs = (FROM_BCD(cdClock.hour) * 60 + FROM_BCD(cdClock.minute)) * 60 + FROM_BCD(cdClock.second);
		timeSecs -= 9 * 60 * 60; // minus 9 hours, JST -> GMT conversion
		timeSecs += gmtOfs * 60; // GMT -> timezone the user selected

		g_day = FROM_BCD(cdClock.day);
		g_month = FROM_BCD(cdClock.month);
		g_year = FROM_BCD(cdClock.year);

		if (timeSecs < 0) {
			buildNewDate(-1);
			timeSecs += SECONDS_PER_DAY;
		} else if (timeSecs >= SECONDS_PER_DAY) {
			buildNewDate(+1);
			timeSecs -= SECONDS_PER_DAY;
		}
		g_timeSecs = (uint32)timeSecs;
	}

	sioprintf("Time: %d:%02d:%02d - %d.%d.%4d", g_timeSecs / (60 * 60), (g_timeSecs / 60) % 60, g_timeSecs % 60,
		g_day, g_month, g_year + 2000);
}

extern "C" time_t time(time_t *p) {
	return (time_t)g_timeSecs;
}

extern "C" struct tm *localtime(const time_t *p) {
	uint32 currentSecs = g_timeSecs + (msecCount - g_lastTimeCheck) / 1000;
	if (currentSecs >= SECONDS_PER_DAY) {
		buildNewDate(+1);
		g_lastTimeCheck += SECONDS_PER_DAY * 1000;
		currentSecs = g_timeSecs + (msecCount - g_lastTimeCheck) / 1000;
	}

	static struct tm retStruct;
	memset(&retStruct, 0, sizeof(retStruct));

	retStruct.tm_hour = currentSecs / (60 * 60);
	retStruct.tm_min  = (currentSecs / 60) % 60;
	retStruct.tm_sec  = currentSecs % 60;
	retStruct.tm_year = g_year + 100;
	retStruct.tm_mday = g_day;
	retStruct.tm_mon  = g_month - 1;
	// tm_wday, tm_yday and tm_isdst are zero for now
    return &retStruct;
}
