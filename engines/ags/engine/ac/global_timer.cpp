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

#include "ags/engine/ac/global_timer.h"
#include "ags/engine/ac/runtime_defines.h"
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/game_state.h"
#include "ags/globals.h"

namespace AGS3 {

void script_SetTimer(int tnum, int timeout) {
	if ((tnum < 1) || (tnum >= MAX_TIMERS))
		quit("!StartTimer: invalid timer number");
	_GP(play).script_timers[tnum] = timeout;
}

int IsTimerExpired(int tnum) {
	if ((tnum < 1) || (tnum >= MAX_TIMERS))
		quit("!IsTimerExpired: invalid timer number");
	if (_GP(play).script_timers[tnum] == 1) {
		_GP(play).script_timers[tnum] = 0;
		return 1;
	}
	return 0;
}

} // namespace AGS3
