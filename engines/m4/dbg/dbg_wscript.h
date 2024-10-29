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

#ifndef M4_DBG_DBG_WSCRIPT_H
#define M4_DBG_DBG_WSCRIPT_H

#include "common/stream.h"
#include "m4/m4_types.h"
#include "m4/dbg/dbg_defs.h"
#include "m4/graphics/gr_font.h"
#include "m4/wscript/ws_machine.h"

namespace M4 {

extern bool dbg_ws_init(bool showTheScreen, Font *useThisFont, frac16 *theGlobals);
extern void dbg_ws_shutdown();
extern void dbg_SetCurrMachInstr(machine *m, int32 pcOffset, bool isPcode);
extern void dbg_EndCurrMachInstr();
extern void dbg_AddOpcodeToMachineInstr(int instruction);
extern void dbg_AddParamToCurrMachInstr(const char *param);
extern void dbg_AddGlobalParamToCurrMachInstr(int num, const char *prefix = nullptr);
extern void dbg_AddRegParamToCurrMachInstr(int num, const char *prefix = nullptr);

extern void dbg_ws_update();

extern void dbg_LaunchSequence(Anim8 *myAnim8);
extern void dbg_DebugWSMach(machine *m, bool debug);
extern void dbg_DebugNextCycle();
extern void dbg_RemoveWSMach(machine *m);
extern void dbg_SetCurrSequInstr(Anim8 *myAnim8, int32 compareCCR);
extern void dbg_WSError(Common::WriteStream *logFile, machine *m, int32 errorType, const char *errDesc, const char *errMsg, int32 pcOffset);

} // namespace M4

#endif
