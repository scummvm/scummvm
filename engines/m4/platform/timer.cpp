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

#include "common/system.h"
#include "m4/platform/timer.h"
#include "m4/vars.h"

namespace M4 {

// FIXME: I'm honestly not sure what the timer rate is meant to be.
// THe methods seem indicative of 60 time increments a second,
// but in ScummVM that makes things too slow. Doubling it to 120
// seems to resolve the slowness, so I'm leaving the change in place.
uint32 timer_read() {
	return g_system->getMillis() * 120 / 1000;
}

uint32 timer_read_dos() {
	return g_system->getMillis() * 120 / 1000;
}

uint32 timer_read_600() {
	return g_system->getMillis() * 1200 / 1000;
}

uint32 timer_read_60() {
	return g_system->getMillis() * 120 / 1000;
}

} // End of namespace M4
