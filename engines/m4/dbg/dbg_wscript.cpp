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

#include "common/debug.h"
#include "m4/dbg/dbg_wscript.h"
#include "m4/detection.h"
#include "m4/vars.h"

namespace M4 {

static const char *IMM_OPCODES[16] = {
	"NONE",
	"op_DO_NOTHING",
	"op_GOTO",
	"op_JUMP",
	"op_TERMINATE",
	"op_START_SEQ",
	"op_PAUSE_SEQ",
	"op_RESUME_SEQ",
	"op_STORE_VAL",
	"op_SEND_MSG",
	"op_SEND_GMSG",
	"op_REPLY_MSG",
	"op_SYSTEM_MSG",
	"op_TRIG",
	"op_TRIG_W",
	"op_CLEAR_REGS"
};

static const char *COND_OPCODES[10] = {
	"op_AFTER",
	"op_ON_END_SEQ",
	"op_ON_MSG",
	"op_ON_P_MSG",
	"op_SWITCH_LT",
	"op_SWITCH_LE",
	"op_SWITCH_EQ",
	"op_SWITCH_NE",
	"op_SWITCH_GE",
	"op_SWITCH_GT"
};

static const char *PCODE_OPCODES[40] = {
	"op_END",
	"op_CLEAR",
	"op_SET",
	"op_COMPARE",
	"op_ADD",
	"op_SUB",
	"op_MULT",
	"op_DIV",
	"op_AND",
	"op_OR",
	"op_NOT",
	"op_SIN",
	"op_COS",
	"op_ABS",
	"op_MIN",
	"op_MAX",
	"op_MOD",
	"op_FLOOR",
	"op_ROUND",
	"op_CEIL",
	"op_POINT",
	"op_DIST2D",
	"op_CRUNCH",
	"op_BRANCH",
	"op_SETCEL",
	"op_SEQ_SEND_MSG",
	"op_PUSH",
	"op_POP",
	"op_JSR",
	"op_RETURN",
	"op_GET_CELS_COUNT",
	"op_GET_CELS_FRAME_RATE",
	"op_GET_CELS_PIX_SPEED",
	"op_SET_INDEX",
	"op_SET_LAYER",
	"op_SET_DEPTH",
	"op_SET_DATA",
	"op_OPEN_STREAM_SS",
	"op_NEXT_STREAM_SS",
	"op_CLOSE_STREAM_SS"
};

static char g_instructionText[256];
bool g_hasParams;
bool g_isPcode;

bool dbg_ws_init(bool showTheScreen, Font *useThisFont, frac16 *theGlobals) {
	return true;
}

void dbg_ws_shutdown() {
	// No implementation
}

void dbg_SetCurrMachInstr(machine *m, int32 pcOffset, bool isPcode) {
	if (debugChannelSet(1, kDebugScripts)) {
		Common::sprintf_s(g_instructionText, "%s (%.4x): ",
			m ? m->machName : "NONE", pcOffset);
		g_hasParams = false;
		g_isPcode = isPcode;
	}
}

void dbg_AddOpcodeToMachineInstr(int instruction) {
	if (debugChannelSet(1, kDebugScripts)) {
		if (g_isPcode) {
			Common::strcat_s(g_instructionText, PCODE_OPCODES[instruction]);
		} else if (instruction >= 64)
			Common::strcat_s(g_instructionText, COND_OPCODES[instruction - 64]);
		else
			Common::strcat_s(g_instructionText, IMM_OPCODES[instruction]);
		Common::strcat_s(g_instructionText, " ");
	}
}

void dbg_AddParamToCurrMachInstr(const char *param) {
	if (debugChannelSet(1, kDebugScripts)) {
		if (g_hasParams)
			Common::strcat_s(g_instructionText, ", ");

		Common::strcat_s(g_instructionText, param);
		g_hasParams = true;
	}
}

void dbg_EndCurrMachInstr() {
	debugC(1, kDebugScripts, "%s", g_instructionText);
}

void dbg_SetCurrSequInstr(Anim8 *myAnim8, int32 compareCCR) {
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

void dbg_WSError(Common::WriteStream *logFile, machine *m, int32 errorType,
		const char *errDesc, const char *errMsg, int32 pcOffset) {
	// No implementation
}

} // namespace M4
