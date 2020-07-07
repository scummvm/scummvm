/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers; whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License; or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful;
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not; write to the Free Software
 * Foundation; Inc.; 51 Franklin Street; Fifth Floor; Boston; MA 02110-1301; USA.
 *
 */

#include "glk/comprehend/opcode_map.h"
#include "glk/comprehend/game_data.h"
#include "common/algorithm.h"
#include "common/textconsole.h"

namespace Glk {
namespace Comprehend {

void OpcodeMap::loadOpcodes(int version) {
	Common::fill(&_opcodeMap[0], &_opcodeMap[0x100], 0);

	if (version == 1)
		loadVersion1();
	else if (version == 2)
		loadVersion2();
	else
		error("Invalid version");
}

void OpcodeMap::loadVersion1() {
	_opcodeMap[0x01] = OPCODE_HAVE_OBJECT;
	_opcodeMap[0x04] = OPCODE_OR;
	_opcodeMap[0x05] = OPCODE_IN_ROOM;
	_opcodeMap[0x06] = OPCODE_VAR_EQ;
	_opcodeMap[0x08] = OPCODE_CURRENT_OBJECT_TAKEABLE;
	_opcodeMap[0x09] = OPCODE_OBJECT_PRESENT;
	_opcodeMap[0x0c] = OPCODE_ELSE;
	_opcodeMap[0x0e] = OPCODE_OBJECT_IN_ROOM;
	_opcodeMap[0x14] = OPCODE_CURRENT_OBJECT_NOT_VALID;
	_opcodeMap[0x18] = OPCODE_INVENTORY_FULL;
	_opcodeMap[0x19] = OPCODE_TEST_FLAG;
	_opcodeMap[0x1d] = OPCODE_CURRENT_OBJECT_IN_ROOM;
	_opcodeMap[0x20] = OPCODE_HAVE_CURRENT_OBJECT;
	_opcodeMap[0x21] = OPCODE_OBJECT_IS_NOT_NOWHERE;
	_opcodeMap[0x24] = OPCODE_CURRENT_OBJECT_PRESENT;
	_opcodeMap[0x31] = OPCODE_TEST_ROOM_FLAG;
	_opcodeMap[0x41] = OPCODE_NOT_HAVE_OBJECT;
	_opcodeMap[0x45] = OPCODE_NOT_IN_ROOM;
	_opcodeMap[0x48] = OPCODE_CURRENT_OBJECT_NOT_PRESENT;
	_opcodeMap[0x49] = OPCODE_OBJECT_NOT_IN_ROOM;
	_opcodeMap[0x4E] = OPCODE_TEST_FALSE;
	_opcodeMap[0x50] = OPCODE_CURRENT_OBJECT_IS_NOWHERE;
	_opcodeMap[0x59] = OPCODE_TEST_NOT_FLAG;
	_opcodeMap[0x5D] = OPCODE_TEST_FALSE;
	_opcodeMap[0x60] = OPCODE_NOT_HAVE_CURRENT_OBJECT;
	_opcodeMap[0x61] = OPCODE_OBJECT_IS_NOWHERE;
	_opcodeMap[0x64] = OPCODE_CURRENT_OBJECT_NOT_IN_ROOM;
	_opcodeMap[0x68] = OPCODE_CURRENT_OBJECT_NOT_TAKEABLE;
	_opcodeMap[0x71] = OPCODE_TEST_NOT_ROOM_FLAG;
	_opcodeMap[0x80] = OPCODE_INVENTORY;
	_opcodeMap[0x81] = OPCODE_TAKE_OBJECT;
	_opcodeMap[0x82] = OPCODE_MOVE_OBJECT_TO_ROOM;
	_opcodeMap[0x84] = OPCODE_SAVE_ACTION;
	_opcodeMap[0x85] = OPCODE_MOVE_TO_ROOM;
	_opcodeMap[0x86] = OPCODE_VAR_ADD;
	_opcodeMap[0x87] = OPCODE_SET_ROOM_DESCRIPTION;
	_opcodeMap[0x89] = OPCODE_MOVE_OBJECT_TO_CURRENT_ROOM;
	_opcodeMap[0x8a] = OPCODE_VAR_SUB;
	_opcodeMap[0x8b] = OPCODE_SET_OBJECT_DESCRIPTION;
	_opcodeMap[0x8c] = OPCODE_MOVE_DEFAULT;
	_opcodeMap[0x8e] = OPCODE_PRINT;
	_opcodeMap[0x95] = OPCODE_REMOVE_OBJECT;
	_opcodeMap[0x99] = OPCODE_SET_FLAG;
	_opcodeMap[0x92] = OPCODE_CALL_FUNC;
	_opcodeMap[0x98] = OPCODE_TURN_TICK;
	_opcodeMap[0x9d] = OPCODE_CLEAR_FLAG;
	_opcodeMap[0x9e] = OPCODE_INVENTORY_ROOM;
	_opcodeMap[0xa0] = OPCODE_TAKE_CURRENT_OBJECT;
	_opcodeMap[0xa1] = OPCODE_SPECIAL;
	_opcodeMap[0xa4] = OPCODE_DROP_CURRENT_OBJECT;
	_opcodeMap[0xa2] = OPCODE_SET_ROOM_GRAPHIC;
	_opcodeMap[0xb0] = OPCODE_REMOVE_CURRENT_OBJECT;
	_opcodeMap[0xb1] = OPCODE_MOVE_DIR;
	_opcodeMap[0xb9] = OPCODE_SET_STRING_REPLACEMENT;
	_opcodeMap[0xbd] = OPCODE_VAR_INC;
	_opcodeMap[0xc1] = OPCODE_VAR_DEC;
	_opcodeMap[0xc9] = OPCODE_MOVE_CURRENT_OBJECT_TO_ROOM;
}

void OpcodeMap::loadVersion2() {
	_opcodeMap[0x01] = OPCODE_HAVE_OBJECT;
	_opcodeMap[0x04] = OPCODE_OR;
	_opcodeMap[0x05] = OPCODE_IN_ROOM;
	_opcodeMap[0x06] = OPCODE_VAR_EQ;
	_opcodeMap[0x08] = OPCODE_CURRENT_IS_OBJECT;
	_opcodeMap[0x09] = OPCODE_OBJECT_PRESENT;
	_opcodeMap[0x0c] = OPCODE_ELSE;
	_opcodeMap[0x11] = OPCODE_OBJECT_IS_NOWHERE;
	_opcodeMap[0x14] = OPCODE_CURRENT_OBJECT_NOT_VALID;
	_opcodeMap[0x19] = OPCODE_TEST_FLAG;
	_opcodeMap[0x1d] = OPCODE_TEST_ROOM_FLAG;
	_opcodeMap[0x20] = OPCODE_HAVE_CURRENT_OBJECT;
	_opcodeMap[0x21] = OPCODE_OBJECT_PRESENT;
	_opcodeMap[0x22] = OPCODE_OBJECT_IN_ROOM;
	_opcodeMap[0x30] = OPCODE_CURRENT_OBJECT_PRESENT;
	_opcodeMap[0x31] = OPCODE_TEST_ROOM_FLAG;
	_opcodeMap[0x38] = OPCODE_INVENTORY_FULL;
	_opcodeMap[0x41] = OPCODE_NOT_HAVE_OBJECT;
	_opcodeMap[0x45] = OPCODE_NOT_IN_ROOM;
	_opcodeMap[0x48] = OPCODE_CURRENT_OBJECT_NOT_PRESENT;
	_opcodeMap[0x43] = OPCODE_OBJECT_NOT_IN_ROOM;
	_opcodeMap[0x59] = OPCODE_TEST_NOT_FLAG;
	_opcodeMap[0x5d] = OPCODE_TEST_NOT_ROOM_FLAG;
	_opcodeMap[0x60] = OPCODE_NOT_HAVE_CURRENT_OBJECT;
	_opcodeMap[0x61] = OPCODE_OBJECT_NOT_PRESENT;
	_opcodeMap[0x70] = OPCODE_CURRENT_OBJECT_NOT_PRESENT;
	_opcodeMap[0x74] = OPCODE_CURRENT_NOT_OBJECT;
	_opcodeMap[0x80] = OPCODE_INVENTORY;
	_opcodeMap[0x81] = OPCODE_TAKE_OBJECT;
	_opcodeMap[0x82] = OPCODE_MOVE_OBJECT_TO_ROOM;
	_opcodeMap[0x84] = OPCODE_SAVE_ACTION;
	_opcodeMap[0x85] = OPCODE_MOVE_TO_ROOM;
	_opcodeMap[0x86] = OPCODE_VAR_ADD;
	_opcodeMap[0x87] = OPCODE_SET_ROOM_DESCRIPTION;
	_opcodeMap[0x89] = OPCODE_SPECIAL;
	_opcodeMap[0x8a] = OPCODE_VAR_SUB;
	_opcodeMap[0x8b] = OPCODE_SET_OBJECT_DESCRIPTION;
	_opcodeMap[0x8c] = OPCODE_MOVE_DEFAULT;
	_opcodeMap[0x8e] = OPCODE_PRINT;
	_opcodeMap[0x8f] = OPCODE_SET_OBJECT_LONG_DESCRIPTION;
	_opcodeMap[0x90] = OPCODE_WAIT_KEY;
	_opcodeMap[0x92] = OPCODE_CALL_FUNC;
	_opcodeMap[0x95] = OPCODE_REMOVE_OBJECT;
	_opcodeMap[0x98] = OPCODE_TURN_TICK;
	_opcodeMap[0x99] = OPCODE_SET_FLAG;
	_opcodeMap[0x9d] = OPCODE_CLEAR_FLAG;
	_opcodeMap[0x9e] = OPCODE_INVENTORY_ROOM;
	_opcodeMap[0xa0] = OPCODE_TAKE_CURRENT_OBJECT;
	_opcodeMap[0xa2] = OPCODE_SET_OBJECT_GRAPHIC;
	_opcodeMap[0xb1] = OPCODE_MOVE_DIR;
	_opcodeMap[0xb5] = OPCODE_DESCRIBE_CURRENT_OBJECT;
	_opcodeMap[0xc1] = OPCODE_VAR_DEC;
	_opcodeMap[0xc2] = OPCODE_SET_ROOM_GRAPHIC;
	_opcodeMap[0xc5] = OPCODE_SET_CURRENT_NOUN_STRING_REPLACEMENT;
	_opcodeMap[0xc6] = OPCODE_SET_OBJECT_GRAPHIC;
	_opcodeMap[0xc9] = OPCODE_MOVE_CURRENT_OBJECT_TO_ROOM;
	_opcodeMap[0xcd] = OPCODE_SET_STRING_REPLACEMENT;
	_opcodeMap[0xd1] = OPCODE_MOVE_DIRECTION;
	_opcodeMap[0xd5] = OPCODE_DRAW_ROOM;
	_opcodeMap[0xd9] = OPCODE_DRAW_OBJECT;
	_opcodeMap[0xdd] = OPCODE_VAR_INC;
	_opcodeMap[0xe1] = OPCODE_MOVE_OBJECT_TO_CURRENT_ROOM;
	_opcodeMap[0xed] = OPCODE_REMOVE_OBJECT;
	_opcodeMap[0xf0] = OPCODE_DROP_CURRENT_OBJECT;
	_opcodeMap[0xfc] = OPCODE_REMOVE_CURRENT_OBJECT;
}

} // namespace Comprehend
} // namespace Glk
