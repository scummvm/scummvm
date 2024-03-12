
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

#ifndef M4_WSCRIPT_WS_TIMER_H
#define M4_WSCRIPT_WS_TIMER_H

#include "m4/m4_types.h"
#include "m4/wscript/ws_machine.h"

namespace M4 {

struct WSTimer_Globals {
	onTimeReq *_firstTimeReq = nullptr;
};

bool ws_InitWSTimer(void);
void ws_KillTime(void);
void ws_MakeOnTimeReq(int32 wakeUpTime, machine *myXM, int32 pcOffset, int32 pcCount);
void ws_CancelOnTimeReqs(machine *m);
void ws_CheckTimeReqs(int32 curTime);

} // End of namespace M4

#endif
