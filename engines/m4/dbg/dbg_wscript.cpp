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

#include "m4/dbg/dbg_wscript.h"

namespace M4 {

bool dbg_ws_init(bool showTheScreen, Font *useThisFont, frac16 *theGlobals) {
	return true;
}

void dbg_ws_shutdown() {
	// No implementation
}

void dbg_ws_update() {
	// No implementation
}

void dbg_LaunchSequence(Anim8 *myAnim8) {
	// No implementation
}

void dbg_DebugWSMach(machine *m, bool debug) {
	// No implementation
}

void dbg_DebugNextCycle() {
	// No implementation
}

void dbg_RemoveWSMach(machine *m) {
	// No implementation
}

void dbg_SetCurrMachInstr(machine *m, int32 pcOffset) {
	// No implementation
}

void dbg_SetCurrSequInstr(Anim8 *myAnim8, int32 compareCCR) {
	// No implementation
}

void dbg_WSError(Common::WriteStream *logFile, machine *m, int32 errorType,
		const char *errDesc, const char *errMsg, int32 pcOffset) {
	// No implementation
}

} // namespace M4
