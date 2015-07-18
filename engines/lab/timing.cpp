/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#include "lab/stddefines.h"
#include "lab/timing.h"
#include "lab/vga.h"

namespace Lab {

/*****************************************************************************/
/* Waits for for Secs seconds and Micros microseconds to pass.               */
/*****************************************************************************/
void microDelay(uint32 secs, uint32 micros) {
	uint32 waitSecs, waitMicros;

	addCurTime(secs, micros, &waitSecs, &waitMicros);

	while (1) {
		getTime(&secs, &micros);

		if ((secs > waitSecs) || ((secs == waitSecs) && (micros >= waitMicros)))
			return;

		g_system->delayMillis(10);
	}
}

/*****************************************************************************/
/* Gets the current system time.                                             */
/*****************************************************************************/
void getTime(uint32 *secs, uint32 *micros) {
	uint32 t = g_system->getMillis();

	*secs   = t / 1000;
	*micros = t % 1000;
}

/*****************************************************************************/
/* Adds seconds and microseconds to current time to get a new time.          */
/*****************************************************************************/
void addCurTime(uint32 sec, uint32 micros, uint32 *timeSec, uint32 *timeMicros) {
	getTime(timeSec, timeMicros);

	(*timeSec)    += sec;
	(*timeMicros) += micros;

	if (*timeMicros >= ONESECOND) {
		(*timeSec)++;
		(*timeMicros) -= ONESECOND;
	}
}

/*****************************************************************************/
/* Finds the difference between time1 and time2.  If time1 is later than     */
/* time2, returns 0.                                                         */
/*****************************************************************************/
void anyTimeDiff(uint32 sec1, uint32 micros1, uint32 sec2, uint32 micros2, uint32 *diffSecs, uint32 *diffMicros) {
	*diffSecs   = 0;
	*diffMicros = 0;

	if (sec1 > sec2)
		return;
	else if ((sec1 == sec2) && (micros1 >= micros2))
		return;

	if (micros1 > micros2) {
		*diffSecs   = sec2 - sec1 - 1;
		*diffMicros = (ONESECOND - micros1) + micros2;
	} else {
		*diffSecs   = sec2 - sec1;
		*diffMicros = micros2 - micros1;
	}
}

/*****************************************************************************/
/* Finds the difference between the current time, and a future time. Returns */
/* 0 if the future time is actually before the current time.                 */
/*****************************************************************************/
void timeDiff(uint32 sec, uint32 micros, uint32 *diffSec, uint32 *diffMicros) {
	uint32 curSec, curMicros;
	getTime(&curSec, &curMicros);
	anyTimeDiff(curSec, curMicros, sec, micros, diffSec, diffMicros);
}

/*****************************************************************************/
/* Waits for a specified time to occur.                                      */
/*****************************************************************************/
void waitForTime(uint32 sec, uint32 micros) {
	uint32 curSec, curMicros;

	getTime(&curSec, &curMicros);

	if (curSec > sec)
		return;
	else if ((curSec == sec) && (curMicros >= micros))
		return;

	if (curMicros > micros)
		microDelay(sec - curSec - 1, (ONESECOND - curMicros) + micros - 1);
	else
		microDelay(sec - curSec, micros - curMicros - 1);
}

} // End of namespace Lab
