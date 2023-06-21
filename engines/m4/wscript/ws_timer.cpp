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

#include "m4/wscript/ws_timer.h"
#include "m4/globals.h"

namespace M4 {

static void dispose_timeRequest(onTimeReq *timeReq);

bool ws_InitWSTimer(void) {
	_G(firstTimeReq) = nullptr;
	return true;
}

void ws_KillTime() {
	onTimeReq *tempTime;

	tempTime = _G(firstTimeReq);
	while (tempTime) {
		_G(firstTimeReq) = _G(firstTimeReq)->next;
		dispose_timeRequest(tempTime);
		tempTime = _G(firstTimeReq);
	}
}

static void dispose_timeRequest(onTimeReq *timeReq) {
	if (timeReq) {
		mem_free(timeReq);
	}
}

} // End of namespace M4
