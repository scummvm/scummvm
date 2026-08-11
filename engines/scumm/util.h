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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef SCUMM_UTIL_H
#define SCUMM_UTIL_H


#include "common/scummsys.h"

namespace Scumm {

#define revBitMask(x)	(0x80 >> (x))

/* Direction conversion functions (between old dir and new dir format) */
int newDirToOldDir(int dir);
int oldDirToNewDir(int dir);

int normalizeAngle(int dirType, int angle);
int fromSimpleDir(int dirtype, int dir);
int toSimpleDir(int dirtype, int dir);

void assertRange(int min, int value, int max, const char *desc);

void *reallocateArray(void *buffer, size_t count, size_t size);

} // End of namespace Scumm

#endif
