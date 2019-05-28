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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "illusions/time.h"
#include "common/system.h"

namespace Illusions {

uint32 getCurrentTime() {
	return g_system->getMillis() / 16;
}

bool isTimerExpired(uint32 startTime, uint32 endTime) {
	uint32 currTime = getCurrentTime();
	return !(
		(startTime > endTime && (currTime <= endTime || currTime >= startTime)) ||
		(startTime < endTime && currTime <= endTime && currTime >= startTime));
}

uint32 getDurationElapsed(uint32 startTime, uint32 endTime) {
	uint32 currTime = getCurrentTime();
	uint32 elapsed = endTime - startTime;
	if (isTimerExpired(startTime, endTime))
		return elapsed;
	else if (startTime < endTime || currTime > startTime)
		return currTime - startTime;
	else
		return currTime + elapsed - endTime;
}

} // End of namespace Illusions
