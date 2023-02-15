/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * aint32 with this program; if not, write to the Free Software
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_CMISC_H
#define SAGA2_CMISC_H

namespace Saga2 {

//  Clamps the number "num" between two limits
int32 clamp(int32 lowerLimit, int32 num, int32 upperLimit);

//  Converts x/y point to angle + distance
int16 ptToAngle(int16 dx, int16 dy, int16 *dist = NULL);

#if     defined( USEWINDOWS )
inline void delay(int32 ms)        {
	Sleep(ms);
}
#endif

} // end of namespace Saga2

#endif
