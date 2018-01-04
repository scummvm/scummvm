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

#include "sludge/allfiles.h"
#include "sludge/timing.h"

namespace Sludge {

void Timer::init(void) {
	_desired_frame_time = 1000 / _desiredfps;
	_starttime = g_system->getMillis();
}

void Timer::initSpecial(int t) {
	_desired_frame_time = 1000 / t;
	_starttime = g_system->getMillis();
}

void Timer::waitFrame(void) {
	static uint32 addNextTime = 0;
	uint32 timetaken;

	for (;;) {
		_endtime = g_system->getMillis();
		timetaken = addNextTime + _endtime - _starttime;
		if (timetaken >= _desired_frame_time) break;
		g_system->delayMillis(1);
	}

	addNextTime = timetaken - _desired_frame_time;
	if (addNextTime > _desired_frame_time) addNextTime = _desired_frame_time;

	_starttime = _endtime;
}

} // End of namespace Sludge
