/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios
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
 */

#include "common/random.h"
#include "common/util.h"

#include "pegasus/MMShell/Utilities/MMUtilities.h"

namespace Pegasus {

inline int32 Round(const int32 a, const int32 b) {
	if (b < 0)
		if (a < 0)
			return -((a - (-b >> 1)) / -b);
		else
			return -((a + (-b >> 1)) / -b);
	else
		if (a < 0)
			return (a - (b >> 1)) / b;
		else
			return (a + (b >> 1)) / b;
}

int32 LinearInterp(const int32 start1, const int32 stop1, const int32 current1, const int32 start2, const int32 stop2) {
	if (start2 == stop2)
		return start2;
	else
		return start2 + Round((current1 - start1) * (stop2 - start2), (stop1 - start1));
}

void ShuffleArray(int32 *arr, int32 count, Common::RandomSource &random) {	
	if (count > 1) {
		for (int32 i = 1; i < count; ++i) {
			int32 j = random.getRandomNumber(i);
			if (j != i)
				SWAP(arr[i], arr[j]);
		}
	}
}

} // End of namespace Pegasus
