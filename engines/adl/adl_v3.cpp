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

#include "adl/adl_v3.h"
#include "adl/display.h"
#include "adl/graphics.h"

namespace Adl {

AdlEngine_v3::AdlEngine_v3(OSystem *syst, const AdlGameDescription *gd) :
		AdlEngine_v2(syst, gd),
		_curDisk(1) {
}

Common::String AdlEngine_v3::loadMessage(uint idx) const {
	Common::String str = AdlEngine_v2::loadMessage(idx);

	for (uint i = 0; i < str.size(); ++i) {
		const char *xorStr = "AVISDURGAN";
		str.setChar(str[i] ^ xorStr[i % strlen(xorStr)], i);
	}

	return str;
}

Common::String AdlEngine_v3::getItemDescription(const Item &item) const {
	return _itemDesc[item.id - 1];
}

typedef Common::Functor1Mem<ScriptEnv &, int, AdlEngine_v3> OpcodeV3;
#define SetOpcodeTable(x) table = &x;
#define Opcode(x) table->push_back(new OpcodeV3(this, &AdlEngine_v3::x))
#define OpcodeUnImpl() table->push_back(new OpcodeV3(this, 0))

void AdlEngine_v3::setupOpcodeTables() {
	Common::Array<const Opcode *> *table = 0;

	SetOpcodeTable(_condOpcodes);
	// 0x00
	OpcodeUnImpl();
	Opcode(o2_isFirstTime);
	Opcode(o2_isRandomGT);
	Opcode(o3_isItemInRoom);
	// 0x04
	Opcode(o3_isNounNotInRoom);
	Opcode(o1_isMovesGT);
	Opcode(o1_isVarEQ);
	Opcode(o2_isCarryingSomething);
	// 0x08
	Opcode(o3_isVarGT);
	Opcode(o1_isCurPicEQ);
	Opcode(o3_skipOneCommand);

	SetOpcodeTable(_actOpcodes);
	// 0x00
	OpcodeUnImpl();
	Opcode(o1_varAdd);
	Opcode(o1_varSub);
	Opcode(o1_varSet);
	// 0x04
	Opcode(o1_listInv);
	Opcode(o3_moveItem);
	Opcode(o1_setRoom);
	Opcode(o1_setCurPic);
	// 0x08
	Opcode(o1_setPic);
	Opcode(o1_printMsg);
	Opcode(o1_setLight);
	Opcode(o1_setDark);
	// 0x0c
	Opcode(o2_moveAllItems);
	Opcode(o1_quit);
	OpcodeUnImpl();
	Opcode(o2_save);
	// 0x10
	Opcode(o2_restore);
	Opcode(o1_restart);
	Opcode(o2_placeItem);
	Opcode(o1_setItemPic);
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
	Opcode(o2_tellTime);
	Opcode(o2_setRoomFromVar);
	// 0x20
	Opcode(o2_initDisk);
}

int AdlEngine_v3::o3_isVarGT(ScriptEnv &e) {
	OP_DEBUG_2("\t&& VARS[%d] > %d", e.arg(1), e.arg(2));

	if (getVar(e.arg(1)) > e.arg(2))
		return 2;

	return -1;
}

int AdlEngine_v3::o3_skipOneCommand(ScriptEnv &e) {
	OP_DEBUG_0("\t&& SKIP_ONE_COMMAND()");

	_skipOneCommand = true;
	setVar(2, 0);

	return -1;
}

// FIXME: Rename "isLineArt" and look at code duplication
int AdlEngine_v3::o3_isItemInRoom(ScriptEnv &e) {
	OP_DEBUG_2("\t&& GET_ITEM_ROOM(%s) == %s", itemStr(e.arg(1)).c_str(), itemRoomStr(e.arg(2)).c_str());

	const Item &item = getItem(e.arg(1));

	if (e.arg(2) != IDI_ANY && item.isLineArt != _curDisk)
		return -1;

	if (item.room == roomArg(e.arg(2)))
		return 2;

	return -1;
}

int AdlEngine_v3::o3_isNounNotInRoom(ScriptEnv &e) {
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

int AdlEngine_v3::o3_moveItem(ScriptEnv &e) {
	OP_DEBUG_2("\tSET_ITEM_ROOM(%s, %s)", itemStr(e.arg(1)).c_str(), itemRoomStr(e.arg(2)).c_str());

	byte room = roomArg(e.arg(2));

	Item &item = getItem(e.arg(1));

	if (item.room == _roomOnScreen)
		_picOnScreen = 0;

	// Set items that move from inventory to a room to state "dropped"
	if (item.room == IDI_ANY && room != IDI_VOID_ROOM)
		item.state = IDI_ITEM_DROPPED;

	item.room = room;
	item.isLineArt = _curDisk;
	return 2;
}

} // End of namespace Adl
