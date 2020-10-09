/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef ICB_DEBUG_PC_H
#define ICB_DEBUG_PC_H

#include "common/system.h"

namespace ICB {

extern bool8 zdebug;

void Fatal_error(const char *format, ...);
void Message_box(const char *text, ...);
void ExitWithReport(char *format, ...);
void Zdebug(const char *, ...);
void Zdebug(uint32 stream, const char *format, ...);
void Tdebug(const char *file, const char *format, ...);

#define Real_Fatal_error Fatal_error
#define Real_Message_box Message_box

// XXX: FIXME:  This function stores the number of elapsed microseconds in an unsigned 32-bit int, which will overflow in just over an
// hour...
//             We should make sure all the client code deals with this properly!
inline unsigned int GetMicroTimer(void) {
	static int first = 1;

	static TimeDate startTime;
	if (first) {
		g_system->getTimeAndDate(startTime);
		first = 0;
	}

	TimeDate curTime;
	g_system->getTimeAndDate(curTime);
	return (uint32)(((curTime.tm_sec - startTime.tm_sec) * 1000000));// + (curTime.tv_usec - startTime.tv_usec)); // TODO: Fix micro-second-precision.
}

} // End of namespace ICB

#endif // #ifndef DEBUG_PC_H
