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

#include "common/random.h"
#include "common/error.h"

#include "adl/adl_v5.h"
#include "adl/display.h"
#include "adl/graphics.h"

namespace Adl {

AdlEngine_v5::AdlEngine_v5(OSystem *syst, const AdlGameDescription *gd) :
		AdlEngine_v4(syst, gd) {
}

AdlEngine_v5::RegionChunkType AdlEngine_v5::getRegionChunkType(const uint16 addr) const {
	switch (addr) {
	case 0x7b00:
		return kRegionChunkGlobalCmds;
	case 0x9500:
		return kRegionChunkRoomCmds;
	default:
		return AdlEngine_v4::getRegionChunkType(addr);
	}
}

typedef Common::Functor1Mem<ScriptEnv &, int, AdlEngine_v5> OpcodeV5;
#define SetOpcodeTable(x) table = &x;
#define Opcode(x) table->push_back(new OpcodeV5(this, &AdlEngine_v5::x))
#define OpcodeUnImpl() table->push_back(new OpcodeV5(this, 0))

void AdlEngine_v5::setupOpcodeTables() {
	Common::Array<const Opcode *> *table = 0;

	SetOpcodeTable(_condOpcodes);
	// 0x00
	OpcodeUnImpl();
	Opcode(o2_isFirstTime);
	Opcode(o2_isRandomGT);
	Opcode(o4_isItemInRoom);
	// 0x04
	Opcode(o5_isNounNotInRoom);
	Opcode(o1_isMovesGT);
	Opcode(o1_isVarEQ);
	Opcode(o2_isCarryingSomething);
	// 0x08
	Opcode(o4_isVarGT);
	Opcode(o1_isCurPicEQ);
	Opcode(o5_abortScript);

	SetOpcodeTable(_actOpcodes);
	// 0x00
	OpcodeUnImpl();
	Opcode(o1_varAdd);
	Opcode(o1_varSub);
	Opcode(o1_varSet);
	// 0x04
	Opcode(o1_listInv);
	Opcode(o4_moveItem);
	Opcode(o1_setRoom);
	Opcode(o2_setCurPic);
	// 0x08
	Opcode(o2_setPic);
	Opcode(o1_printMsg);
	Opcode(o5_dummy);
	Opcode(o5_setTextMode);
	// 0x0c
	Opcode(o2_moveAllItems);
	Opcode(o1_quit);
	Opcode(o5_dummy);
	Opcode(o2_save);
	// 0x10
	Opcode(o2_restore);
	Opcode(o1_restart);
	Opcode(o5_setRegionRoom);
	Opcode(o5_dummy);
	// 0x14
	Opcode(o1_resetPic);
	Opcode(o1_goDirection<IDI_DIR_NORTH>);
	Opcode(o1_goDirection<IDI_DIR_SOUTH>);
	Opcode(o1_goDirection<IDI_DIR_EAST>);
	// 0x18
	Opcode(o1_goDirection<IDI_DIR_WEST>);
	Opcode(o1_goDirection<IDI_DIR_UP>);
	Opcode(o1_goDirection<IDI_DIR_DOWN>);
	Opcode(o1_takeItem);
	// 0x1c
	Opcode(o1_dropItem);
	Opcode(o1_setRoomPic);
	Opcode(o_winGame);
	OpcodeUnImpl();
	// 0x20
	Opcode(o2_initDisk);
}

int AdlEngine_v5::o5_isNounNotInRoom(ScriptEnv &e) {
	OP_DEBUG_1("\t&& NO_SUCH_ITEMS_IN_ROOM(%s)", itemRoomStr(e.arg(1)).c_str());

	Common::List<Item>::const_iterator item;

	setVar(24, 0);

	for (item = _state.items.begin(); item != _state.items.end(); ++item)
		if (item->noun == e.getNoun()) {
			setVar(24, 1);

			if (item->room == roomArg(e.arg(1)))
				return -1;
		}

	return 1;
}

int AdlEngine_v5::o5_abortScript(ScriptEnv &e) {
	OP_DEBUG_0("\t&& ABORT_SCRIPT()");

	_abortScript = true;
	setVar(2, 0);

	return -1;
}

int AdlEngine_v5::o5_dummy(ScriptEnv &e) {
	OP_DEBUG_0("\tDUMMY()");

	return 0;
}

int AdlEngine_v5::o5_setTextMode(ScriptEnv &e) {
	OP_DEBUG_1("\tSET_TEXT_MODE(%d)", e.arg(1));

	// TODO
	// 1: 4-line mode
	// 2: 24-line mode

	switch (e.arg(1)) {
	case 3:
		// We re-use the restarting flag here, to simulate a long jump
		_isRestarting = true;
		return -1;
	}

	return 1;
}

int AdlEngine_v5::o5_setRegionRoom(ScriptEnv &e) {
	OP_DEBUG_2("\tSET_REGION_ROOM(%d, %d)", e.arg(1), e.arg(2));

	// TODO
	return 2;
}

int AdlEngine_v5::o_winGame(ScriptEnv &e) {
	OP_DEBUG_0("\tWIN_GAME()");

	// TODO

	return 0;
}

} // End of namespace Adl
