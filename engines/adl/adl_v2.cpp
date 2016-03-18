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

#include "adl/adl_v2.h"

namespace Adl {

AdlEngine_v2::~AdlEngine_v2() {
	delete _random;
}

AdlEngine_v2::AdlEngine_v2(OSystem *syst, const AdlGameDescription *gd) :
		AdlEngine(syst, gd) {
	_random = new Common::RandomSource("adl");
}

typedef Common::Functor1Mem<ScriptEnv &, int, AdlEngine_v2> OpcodeV2;
#define SetOpcodeTable(x) table = &x;
#define Opcode(x) table->push_back(new OpcodeV2(this, &AdlEngine_v2::x))
#define OpcodeUnImpl() table->push_back(new OpcodeV2(this, 0))

void AdlEngine_v2::setupOpcodeTables() {
	Common::Array<const Opcode *> *table = 0;

	SetOpcodeTable(_condOpcodes);
	// 0x00
	OpcodeUnImpl();
	Opcode(o2_isFirstTime);
	Opcode(o2_isRandomGT);
	Opcode(o2_isItemInRoom);
	// 0x04
	Opcode(o2_isNounNotInRoom);
	Opcode(o1_isMovesGT);
	Opcode(o1_isVarEQ);
	Opcode(o2_isCarryingSomething);
	// 0x08
	OpcodeUnImpl();
	Opcode(o1_isCurPicEQ);
	Opcode(o1_isItemPicEQ);

	SetOpcodeTable(_actOpcodes);
	// 0x00
	OpcodeUnImpl();
	Opcode(o1_varAdd);
	Opcode(o1_varSub);
	Opcode(o1_varSet);
	// 0x04
	Opcode(o1_listInv);
	Opcode(o2_moveItem);
	Opcode(o1_setRoom);
	Opcode(o1_setCurPic);
	// 0x08
	Opcode(o1_setPic);
	Opcode(o1_printMsg);
	Opcode(o1_setLight);
	Opcode(o1_setDark);
	// 0x0c
	OpcodeUnImpl();
	Opcode(o1_quit);
	OpcodeUnImpl();
	Opcode(o1_save);
	// 0x10
	Opcode(o1_restore);
	Opcode(o1_restart);
	Opcode(o1_placeItem);
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
}

int AdlEngine_v2::o2_isFirstTime(ScriptEnv &e) {
	bool oldFlag = getCurRoom().isFirstTime;

	getCurRoom().isFirstTime = false;

	if (!oldFlag)
		return -1;

	return 0;
}

int AdlEngine_v2::o2_isRandomGT(ScriptEnv &e) {
	byte rnd = _random->getRandomNumber(255);

	if (rnd > e.arg(1))
		return 1;

	return -1;
}

int AdlEngine_v2::o2_isItemInRoom(ScriptEnv &e) {
	byte room = e.arg(2);

	if (room == IDI_CUR_ROOM)
		room = _state.room;

	if (getItem(e.arg(1)).room == room)
		return 2;

	return -1;
}

int AdlEngine_v2::o2_isNounNotInRoom(ScriptEnv &e) {
	Common::Array<Item>::const_iterator item;

	byte room = e.arg(1);

	if (room == IDI_CUR_ROOM)
		room = _state.room;

	for (item = _state.items.begin(); item != _state.items.end(); ++item)
		if (item->noun == e.getNoun() && (item->room == room))
			return -1;

	return 1;
}

int AdlEngine_v2::o2_isCarryingSomething(ScriptEnv &e) {
	Common::Array<Item>::const_iterator item;

	for (item = _state.items.begin(); item != _state.items.end(); ++item)
		if (item->room == IDI_NONE)
			return 0;
	return -1;
}

int AdlEngine_v2::o2_moveItem(ScriptEnv &e) {
	byte room = e.arg(2);

	if (room == IDI_CUR_ROOM)
		room = _state.room;

	Item &item = getItem(e.arg(1));

	// Not implemented: set redraw flag if item room == displayed room

	// Set items that move from inventory to a room to state "dropped"
	if (item.room == IDI_NONE && room != IDI_VOID_ROOM)
		item.state = IDI_ITEM_DROPPED;

	item.room = room;
	return 2;
}

} // End of namespace Adl
