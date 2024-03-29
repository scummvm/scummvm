
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

#ifndef M4_WSCRIPT_WS_SCRIPT_H
#define M4_WSCRIPT_WS_SCRIPT_H

#include "m4/wscript/ws_cruncher.h"
#include "m4/wscript/ws_hal.h"
#include "m4/wscript/ws_load.h"
#include "m4/wscript/ws_machine.h"
#include "m4/wscript/ws_timer.h"

namespace M4 {

struct WS_Globals : public WSCruncher_Globals, public WSHal_Globals,
		public WSLoad_Globals, public WSMachine_Globals, public WSTimer_Globals {
};

} // End of namespace M4

#endif
