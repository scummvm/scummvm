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

#include "glk/comprehend/game_opcodes.h"
#include "glk/comprehend/game_data.h"
#include "glk/comprehend/comprehend.h"
#include "glk/comprehend/debugger.h"
#include "common/algorithm.h"
#include "common/textconsole.h"

namespace Glk {
namespace Comprehend {

ComprehendGameOpcodes::ComprehendGameOpcodes() {
	Common::fill(&_opcodeMap[0], &_opcodeMap[0x100], OPCODE_UNKNOWN);
}

void ComprehendGameOpcodes::execute_opcode(const Instruction *instr, const Sentence *sentence,
		FunctionState *func_state) {
	byte verb = sentence ? sentence->_formattedWords[0] : 0;
	byte noun = sentence ? sentence->_formattedWords[2] : 0;
	Room *room = get_room(_currentRoom);
	Item *item;
	uint index;

	byte opcode = getOpcode(instr);
	switch (_opcodeMap[opcode]) {
	case OPCODE_CALL_FUNC:
	case OPCODE_CALL_FUNC2:
		// Note: CALL_FUNC2 in the original did some extra backing of data which is
		// redundant in the ScummVM version, so it can be handled the same as CALL_FUNC.
		index = instr->_operand[0];
		if (instr->_operand[1] == 0x81)
			index += 256;
		if (index >= _functions.size())
			error("Bad function %.4x >= %.4x\n", index, _functions.size());

		eval_function(index, sentence);
		break;

	case OPCODE_CLEAR_CAN_TAKE:
		item = get_item_by_noun(noun);
		item->_flags &= ~ITEMF_CAN_TAKE;
		break;

	case OPCODE_CLEAR_FLAG:
		_flags[instr->_operand[0]] = false;
		break;

	case OPCODE_CLEAR_FLAG40:
		item = getItem(instr);
		item->_flags &= ~ITEMF_UNKNOWN;
		break;

	case OPCODE_CLEAR_INVISIBLE:
		item = get_item_by_noun(noun);
		item->_flags &= ~ITEMF_INVISIBLE;
		break;

	case OPCODE_CLEAR_WORD:
		item = getItem(instr);
		item->_word = 0;
		break;

	case OPCODE_CURRENT_OBJECT_NOT_VALID:
		func_set_test_result(func_state, !noun);
		break;

	case OPCODE_CURRENT_IS_OBJECT:
		item = get_item_by_noun(noun);
		func_set_test_result(func_state, item != nullptr);
		break;

	case OPCODE_ELSE:
		func_state->_testResult = func_state->_elseResult;
		break;

	case OPCODE_HAVE_CURRENT_OBJECT:
		item = get_item_by_noun(noun);
		func_set_test_result(func_state, item->_room == ROOM_INVENTORY);
		break;

	case OPCODE_HAVE_OBJECT:
		item = getItem(instr);
		func_set_test_result(func_state, item->_room == ROOM_INVENTORY);
		break;

	case OPCODE_IN_ROOM:
		func_set_test_result(func_state, _currentRoom == instr->_operand[0]);
		break;

	case OPCODE_INVENTORY: {
		uint count = num_objects_in_room(ROOM_INVENTORY);
		if (count == 0) {
			console_println(stringLookup(STRING_INVENTORY_EMPTY).c_str());
			break;
		}

		console_println(stringLookup(STRING_INVENTORY).c_str());
		for (uint i = 0; i < _items.size(); i++) {
			item = &_items[i];
			if (item->_room == ROOM_INVENTORY)
				g_comprehend->print("%s\n",
					stringLookup(item->_stringDesc).c_str());
		}
		break;
	}

	case OPCODE_MOVE_DEFAULT:
		// Move in the direction dictated by the current verb
		if (verb - 1 >= NR_DIRECTIONS)
			error("Bad verb %d in move", verb);

		if (room->_direction[verb - 1])
			move_to(room->_direction[verb - 1]);
		else
			console_println(stringLookup(STRING_CANT_GO).c_str());
		break;

	case OPCODE_MOVE_OBJECT_TO_CURRENT_ROOM:
		item = getItem(instr);
		move_object(item, _currentRoom);
		break;

	case OPCODE_MOVE_OBJECT_TO_ROOM:
		item = getItem(instr);
		move_object(item, instr->_operand[1]);
		break;

	case OPCODE_MOVE_TO_ROOM:
		if (instr->_operand[0] != 0xff)
			move_to(instr->_operand[0]);
		break;

	case OPCODE_OBJECT_IN_ROOM:
		item = getItem(instr);
		func_set_test_result(func_state, item->_room == instr->_operand[1]);
		break;

	case OPCODE_OBJECT_IS_NOWHERE:
		item = getItem(instr);
		func_set_test_result(func_state, item->_room == ROOM_NOWHERE);
		break;

	case OPCODE_OBJECT_PRESENT:
		item = getItem(instr);
		func_set_test_result(func_state, item->_room == _currentRoom);
		break;

	case OPCODE_OR:
		if (func_state->_orCount) {
			func_state->_orCount += 2;
		} else {
			func_state->_testResult = false;
			func_state->_orCount += 3;
		}
		break;

	case OPCODE_PRINT:
		console_println(instrStringLookup(instr->_operand[0], instr->_operand[1]).c_str());
		break;

	case OPCODE_RANDOM_MSG: {
		int msgId = (instr->_operand[2] << 8 | instr->_operand[1]) +
			getRandomNumber(instr->_operand[0] - 1);
		console_println(stringLookup(msgId).c_str());
		break;
	}

	case OPCODE_REMOVE_OBJECT:
		item = getItem(instr);
		move_object(item, ROOM_NOWHERE);
		break;

	case OPCODE_SAVE_ACTION:
		// Causes the next sentence inputed to re-use the first word of the current one.
		// As far as I'm aware, this is only used for handling responses to questions
		_nounState = NOUNSTATE_QUERY;
		// fall-through

	case OPCODE_CLEAR_LINE:
		// Resets the input line, removing any pending further actions that were specified
		Common::fill(&_inputLine[0], &_inputLine[INPUT_LINE_SIZE], 0);
		_inputLineIndex = 0;
		break;

	case OPCODE_SET_CAN_TAKE:
		item = get_item_by_noun(noun);
		item->_flags |= ITEMF_CAN_TAKE;
		break;

	case OPCODE_SET_FLAG:
		_flags[instr->_operand[0]] = true;
		break;

	case OPCODE_SET_FLAG40:
		item = getItem(instr);
		item->_flags |= ITEMF_UNKNOWN;
		break;

	case OPCODE_SET_INVISIBLE:
		item = get_item_by_noun(noun);
		item->_flags |= ITEMF_INVISIBLE;
		break;

	case OPCODE_SET_OBJECT_DESCRIPTION:
		item = getItem(instr);
		item->_stringDesc = (instr->_operand[2] << 8) | instr->_operand[1];
		break;

	case OPCODE_SET_ROOM_DESCRIPTION:
		room = get_room(instr->_operand[0]);
		switch (instr->_operand[2]) {
		case 0x80:
			room->_stringDesc = instr->_operand[1];
			break;
		case 0x81:
			room->_stringDesc = instr->_operand[1] + 0x100;
			break;
		case 0x82:
			room->_stringDesc = instr->_operand[1] + 0x200;
			break;
		default:
			error("Bad string desc %.2x:%.2x\n", instr->_operand[1], instr->_operand[2]);
			break;
		}
		break;

	case OPCODE_SET_ROOM_GRAPHIC:
		room = get_room(instr->_operand[0]);
		room->_graphic = instr->_operand[1];
		if (instr->_operand[0] == _currentRoom)
			_updateFlags |= UPDATE_GRAPHICS;
		break;

	case OPCODE_SET_STRING_REPLACEMENT1:
		_currentReplaceWord = (instr->_operand[0] & 0x80) - 1;
		break;

	case OPCODE_SET_STRING_REPLACEMENT2:
		_currentReplaceWord = instr->_operand[0] - 1;
		break;

	case OPCODE_SET_WORD:
		item = getItem(instr);
		item->_word = instr->_operand[1];
		break;

	case OPCODE_SPECIAL:
		// Game specific opcode
		_specialOpcode = instr->_operand[0];
		break;

	case OPCODE_TAKE_CURRENT_OBJECT:
		item = get_item_by_noun(noun);
		if (!item)
			error("Attempt to take object failed\n");

		move_object(item, ROOM_INVENTORY);
		break;

	case OPCODE_TAKE_OBJECT:
		item = getItem(instr);
		move_object(item, ROOM_INVENTORY);
		break;

	case OPCODE_TEST_FLAG:
		func_set_test_result(func_state, _flags[instr->_operand[0]]);
		break;

	case OPCODE_TEST_ROOM_FLAG:
		func_set_test_result(func_state, room->_flags & instr->_operand[0]);
		break;

	case OPCODE_TURN_TICK:
		_variables[VAR_TURN_COUNT]++;
		break;

	case OPCODE_VAR_ADD:
		_variables[instr->_operand[0]] += _variables[instr->_operand[1]];
		break;

	case OPCODE_VAR_DEC:
		_variables[instr->_operand[0]]--;
		break;

	case OPCODE_VAR_EQ2:
		func_set_test_result(func_state,
			_variables[instr->_operand[0]] == _variables[instr->_operand[1]]);
		break;

	case OPCODE_VAR_GT1:
		func_set_test_result(func_state,
			_variables[0] >
			_variables[instr->_operand[0]]);
		break;

	case OPCODE_VAR_GT2:
		func_set_test_result(func_state, _variables[instr->_operand[0]] >
			_variables[instr->_operand[1]]);
		break;

	case OPCODE_VAR_GTE1:
		func_set_test_result(func_state,
			_variables[0] >=
			_variables[instr->_operand[0]]);
		break;

	case OPCODE_VAR_GTE2:
		func_set_test_result(func_state,
			_variables[instr->_operand[0]] >=
			_variables[instr->_operand[1]]);
		break;

	case OPCODE_VAR_EQ1:
		func_set_test_result(func_state,
			_variables[0] ==
			_variables[instr->_operand[0]]);
		break;

	case OPCODE_VAR_INC:
		_variables[instr->_operand[0]]++;
		break;

	case OPCODE_VAR_SUB:
		_variables[instr->_operand[0]] -= _variables[instr->_operand[1]];
		break;

	default:
		if (instr->_opcode & 0x80) {
			warning("Unhandled command opcode %.2x", opcode);
		} else {
			warning("Unhandled test opcode %.2x - returning false", opcode);
			func_set_test_result(func_state, false);
		}
		break;
	}
}

void ComprehendGameOpcodes::func_set_test_result(FunctionState *func_state, bool value) {
	if (func_state->_orCount == 0) {
		/* And */
		if (func_state->_and) {
			if (!value)
				func_state->_testResult = false;
		} else {
			func_state->_testResult = value;
			func_state->_and = true;
		}

	} else {
		/* Or */
		if (value)
			func_state->_testResult = value;
	}
}

bool ComprehendGameOpcodes::isItemPresent(Item *item) const {
	return item && (
		item->_room == _currentRoom || item->_room == ROOM_INVENTORY
		|| item->_room == ROOM_CONTAINER
		);
}

Item *ComprehendGameOpcodes::getItem(const Instruction *instr) {
	return get_item(instr->_operand[0] - 1);
}

/*-------------------------------------------------------*/

ComprehendGameV1::ComprehendGameV1() {
	_opcodeMap[0x01] = OPCODE_HAVE_OBJECT;
	_opcodeMap[0x02] = OPCODE_VAR_GT2;
	_opcodeMap[0x04] = OPCODE_OR;
	_opcodeMap[0x05] = OPCODE_IN_ROOM;
	_opcodeMap[0x06] = OPCODE_VAR_EQ2;
	_opcodeMap[0x08] = OPCODE_CURRENT_IS_OBJECT;
	_opcodeMap[0x09] = OPCODE_OBJECT_PRESENT;
	_opcodeMap[0x0a] = OPCODE_VAR_GTE2;
	_opcodeMap[0x0c] = OPCODE_ELSE;
	_opcodeMap[0x0e] = OPCODE_OBJECT_IN_ROOM;
	_opcodeMap[0x14] = OPCODE_CURRENT_OBJECT_NOT_VALID;
	_opcodeMap[0x18] = OPCODE_INVENTORY_FULL;
	_opcodeMap[0x19] = OPCODE_TEST_FLAG;
	_opcodeMap[0x1d] = OPCODE_CURRENT_OBJECT_IN_ROOM;
	_opcodeMap[0x20] = OPCODE_HAVE_CURRENT_OBJECT;
	_opcodeMap[0x21] = OPCODE_OBJECT_IS_NOT_NOWHERE;
	_opcodeMap[0x24] = OPCODE_CURRENT_OBJECT_PRESENT;
	_opcodeMap[0x25] = OPCODE_VAR_GT1;
	_opcodeMap[0x29] = OPCODE_VAR_EQ1;
	_opcodeMap[0x2d] = OPCODE_VAR_GTE1;
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
	_opcodeMap[0x83] = OPCODE_RANDOM_MSG;
	_opcodeMap[0x84] = OPCODE_SAVE_ACTION;
	_opcodeMap[0x85] = OPCODE_MOVE_TO_ROOM;
	_opcodeMap[0x86] = OPCODE_VAR_ADD;
	_opcodeMap[0x87] = OPCODE_SET_ROOM_DESCRIPTION;
	_opcodeMap[0x88] = OPCODE_CLEAR_LINE;
	_opcodeMap[0x89] = OPCODE_MOVE_OBJECT_TO_CURRENT_ROOM;
	_opcodeMap[0x8a] = OPCODE_VAR_SUB;
	_opcodeMap[0x8b] = OPCODE_SET_OBJECT_DESCRIPTION;
	_opcodeMap[0x8c] = OPCODE_MOVE_DEFAULT;
	_opcodeMap[0x8d] = OPCODE_SET_CAN_TAKE;
	_opcodeMap[0x8e] = OPCODE_PRINT;
	_opcodeMap[0x91] = OPCODE_CLEAR_CAN_TAKE;
	_opcodeMap[0x95] = OPCODE_REMOVE_OBJECT;
	_opcodeMap[0x99] = OPCODE_SET_FLAG;
	_opcodeMap[0x92] = OPCODE_CALL_FUNC;
	_opcodeMap[0x98] = OPCODE_TURN_TICK;
	_opcodeMap[0x9a] = OPCODE_SET_WORD;
	_opcodeMap[0x9d] = OPCODE_CLEAR_FLAG;
	_opcodeMap[0x9e] = OPCODE_INVENTORY_ROOM;
	_opcodeMap[0xa0] = OPCODE_TAKE_CURRENT_OBJECT;
	_opcodeMap[0xa1] = OPCODE_SPECIAL;
	_opcodeMap[0xa4] = OPCODE_DROP_CURRENT_OBJECT;
	_opcodeMap[0xa2] = OPCODE_SET_ROOM_GRAPHIC;
	_opcodeMap[0xad] = OPCODE_CLEAR_WORD;
	_opcodeMap[0xb0] = OPCODE_REMOVE_CURRENT_OBJECT;
	_opcodeMap[0xb1] = OPCODE_MOVE_DIR;
	_opcodeMap[0xb5] = OPCODE_SET_STRING_REPLACEMENT1;
	_opcodeMap[0xb9] = OPCODE_SET_STRING_REPLACEMENT2;
	_opcodeMap[0xbd] = OPCODE_VAR_INC;
	_opcodeMap[0xc1] = OPCODE_VAR_DEC;
	_opcodeMap[0xc5] = OPCODE_SET_STRING_REPLACEMENT3;
	_opcodeMap[0xc9] = OPCODE_MOVE_CURRENT_OBJECT_TO_ROOM;
	_opcodeMap[0xcd] = OPCODE_CLEAR_INVISIBLE;
	_opcodeMap[0xd1] = OPCODE_SET_INVISIBLE;
	_opcodeMap[0xd5] = OPCODE_CLEAR_FLAG40;
	_opcodeMap[0xd9] = OPCODE_SET_FLAG40;
}

void ComprehendGameV1::execute_opcode(const Instruction *instr, const Sentence *sentence,
		FunctionState *func_state) {
	byte noun = sentence ? sentence->_formattedWords[2] : 0;
	Room *room = get_room(_currentRoom);
	Item *item;
	uint count;

	switch (_opcodeMap[getOpcode(instr)]) {
	case OPCODE_INVENTORY_FULL:
		item = get_item_by_noun(noun);

		if (g_debugger->_invLimit)
			func_set_test_result(func_state, _variables[VAR_INVENTORY_WEIGHT] +
			(item->_flags & ITEMF_WEIGHT_MASK) > _variables[VAR_INVENTORY_LIMIT]);
		else
			// Allow for an unlimited number of items in inventory
			func_set_test_result(func_state, false);
		break;

	case OPCODE_OBJECT_NOT_PRESENT:
		item = getItem(instr);
		func_set_test_result(func_state, !isItemPresent(item));
		break;

	case OPCODE_SET_STRING_REPLACEMENT3:
		_currentReplaceWord = instr->_operand[0] - 1;
		break;

	/*--------------------------------------*/

	case OPCODE_TEST_NOT_ROOM_FLAG:
		func_set_test_result(func_state,
			!(room->_flags & instr->_operand[0]));
		break;

	case OPCODE_NOT_IN_ROOM:
		func_set_test_result(func_state,
			_currentRoom != instr->_operand[0]);
		break;

	case OPCODE_OBJECT_NOT_IN_ROOM:
		item = getItem(instr);
		func_set_test_result(func_state, !item || item->_room != _currentRoom);
		break;

	case OPCODE_CURRENT_OBJECT_NOT_IN_ROOM:
		item = get_item_by_noun(noun);
		func_set_test_result(func_state, !item || item->_room != _currentRoom);
		break;

	case OPCODE_DESCRIBE_CURRENT_OBJECT:
		/*
		 * This opcode is only used in version 2
		 * FIXME - unsure what the single operand is for.
		 */
		item = get_item_by_noun(noun);
		g_comprehend->print("%s\n", stringLookup(item->_longString).c_str());
		break;

	case OPCODE_CURRENT_OBJECT_IN_ROOM: {
		/* FIXME - use common code for these two ops */
		bool test = false;

		if (noun) {
			for (uint i = 0; i < _items.size(); i++) {
				Item *itemP = &_items[i];

				if (itemP->_word == noun && itemP->_room == instr->_operand[0]) {
					test = true;
					break;
				}
			}
		}

		func_set_test_result(func_state, test);
		break;
	}

	case OPCODE_CURRENT_OBJECT_PRESENT:
		item = get_item_by_noun(noun);
		if (item)
			func_set_test_result(func_state,
				item->_room == _currentRoom);
		else
			func_set_test_result(func_state, false);
		break;

	case OPCODE_NOT_HAVE_CURRENT_OBJECT:
		item = get_item_by_noun(noun);
		func_set_test_result(func_state,
			!item || item->_room != ROOM_INVENTORY);
		break;

	case OPCODE_NOT_HAVE_OBJECT:
		item = getItem(instr);
		func_set_test_result(func_state,
			item->_room != ROOM_INVENTORY);
		break;

	case OPCODE_CURRENT_OBJECT_NOT_TAKEABLE:
		item = get_item_by_noun(noun);
		if (!item)
			func_set_test_result(func_state, true);
		else
			func_set_test_result(func_state,
				!(item->_flags & ITEMF_CAN_TAKE));
		break;

	case OPCODE_CURRENT_OBJECT_IS_NOWHERE:
		item = get_item_by_noun(noun);
		func_set_test_result(func_state, item && item->_room == ROOM_NOWHERE);
		break;

	case OPCODE_OBJECT_IS_NOT_NOWHERE:
		item = getItem(instr);
		func_set_test_result(func_state, item->_room != ROOM_NOWHERE);
		break;

	case OPCODE_CURRENT_OBJECT_NOT_PRESENT:
		item = get_item_by_noun(noun);
		func_set_test_result(func_state, !isItemPresent(item));
		break;

	case OPCODE_REMOVE_CURRENT_OBJECT:
		item = get_item_by_noun(noun);
		move_object(item, ROOM_NOWHERE);
		break;

	case OPCODE_INVENTORY_ROOM:
		count = num_objects_in_room(instr->_operand[0]);
		if (count == 0) {
			console_println(stringLookup(instr->_operand[1] + 1).c_str());
			break;
		}

		console_println(stringLookup(instr->_operand[1]).c_str());
		for (uint i = 0; i < _items.size(); i++) {
			item = &_items[i];
			if (item->_room == instr->_operand[0])
				g_comprehend->print("%s\n",
					stringLookup(item->_stringDesc).c_str());
		}
		break;

	case OPCODE_MOVE_CURRENT_OBJECT_TO_ROOM:
		item = get_item_by_noun(noun);
		if (!item)
			error("Bad current object\n");

		move_object(item, instr->_operand[0]);
		break;

	case OPCODE_DROP_OBJECT:
		item = getItem(instr);
		move_object(item, _currentRoom);
		break;

	case OPCODE_DROP_CURRENT_OBJECT:
		item = get_item_by_noun(noun);
		if (!item)
			error("Attempt to take object failed\n");

		move_object(item, _currentRoom);
		break;

	case OPCODE_TEST_NOT_FLAG:
		func_set_test_result(func_state,
			!_flags[instr->_operand[0]]);
		break;

	case OPCODE_TEST_FALSE:
		// The original had two opcodes mapped to the same code that does
		// a test, but ignores the result, and is always false
		func_set_test_result(func_state, false);
		break;

	case OPCODE_SET_CURRENT_NOUN_STRING_REPLACEMENT:
#if 1
		error("TODO: OPCODE_SET_CURRENT_NOUN_STRING_REPLACEMENT");
#else
		/*
		 * FIXME - Not sure what the operand is for,
		 * maybe capitalisation?
		 */
		if (noun && (noun->_type & WORD_TYPE_NOUN_PLURAL))
			_currentReplaceWord = 3;
		else if (noun && (noun->_type & WORD_TYPE_FEMALE))
			_currentReplaceWord = 0;
		else if (noun && (noun->_type & WORD_TYPE_MALE))
			_currentReplaceWord = 1;
		else
			_currentReplaceWord = 2;
#endif
		break;

	case OPCODE_MOVE_DIR:
		doMovementVerb(instr->_operand[0]);
		break;

	default:
		ComprehendGameOpcodes::execute_opcode(instr, sentence, func_state);
		break;
	}
}

/*-------------------------------------------------------*/

ComprehendGameV2::ComprehendGameV2() {
	_opcodeMap[0x01] = OPCODE_HAVE_OBJECT;
	_opcodeMap[0x02] = OPCODE_VAR_GT2;
	_opcodeMap[0x04] = OPCODE_OR;
	_opcodeMap[0x05] = OPCODE_IN_ROOM;
	_opcodeMap[0x06] = OPCODE_VAR_EQ2;
	_opcodeMap[0x08] = OPCODE_CURRENT_IS_OBJECT;
	_opcodeMap[0x09] = OPCODE_VAR_GT1;
	_opcodeMap[0x0a] = OPCODE_VAR_GTE2;
	_opcodeMap[0x0c] = OPCODE_ELSE;
	_opcodeMap[0x0d] = OPCODE_VAR_EQ1;
	_opcodeMap[0x11] = OPCODE_OBJECT_IS_NOWHERE;
	_opcodeMap[0x14] = OPCODE_CURRENT_OBJECT_NOT_VALID;
	_opcodeMap[0x15] = OPCODE_INVENTORY_FULL_X;
	_opcodeMap[0x19] = OPCODE_TEST_FLAG;
	_opcodeMap[0x1d] = OPCODE_TEST_ROOM_FLAG;
	_opcodeMap[0x20] = OPCODE_HAVE_CURRENT_OBJECT;
	_opcodeMap[0x21] = OPCODE_OBJECT_PRESENT;
	_opcodeMap[0x22] = OPCODE_OBJECT_IN_ROOM;
	_opcodeMap[0x25] = OPCODE_OBJECT_TAKEABLE;
	_opcodeMap[0x29] = OPCODE_INVENTORY_FULL;
	_opcodeMap[0x2d] = OPCODE_OBJECT_CAN_TAKE;
	_opcodeMap[0x80] = OPCODE_INVENTORY;
	_opcodeMap[0x81] = OPCODE_TAKE_OBJECT;
	_opcodeMap[0x83] = OPCODE_RANDOM_MSG;
	_opcodeMap[0x84] = OPCODE_SAVE_ACTION;
	_opcodeMap[0x85] = OPCODE_MOVE_TO_ROOM;
	_opcodeMap[0x86] = OPCODE_VAR_ADD;
	_opcodeMap[0x87] = OPCODE_SET_ROOM_DESCRIPTION;
	_opcodeMap[0x88] = OPCODE_CLEAR_LINE;
	_opcodeMap[0x89] = OPCODE_SPECIAL;
	_opcodeMap[0x8a] = OPCODE_VAR_SUB;
	_opcodeMap[0x8b] = OPCODE_SET_OBJECT_DESCRIPTION;
	_opcodeMap[0x8c] = OPCODE_MOVE_DEFAULT;
	_opcodeMap[0x8e] = OPCODE_PRINT;
	_opcodeMap[0x8f] = OPCODE_SET_OBJECT_LONG_DESCRIPTION;
	_opcodeMap[0x90] = OPCODE_WAIT_KEY;
	_opcodeMap[0x92] = OPCODE_CALL_FUNC;
	_opcodeMap[0x95] = OPCODE_CLEAR_WORD;
	_opcodeMap[0x96] = OPCODE_CALL_FUNC2;
	_opcodeMap[0x98] = OPCODE_TURN_TICK;
	_opcodeMap[0x99] = OPCODE_SET_FLAG;
	_opcodeMap[0x9a] = OPCODE_SET_WORD;
	_opcodeMap[0x9d] = OPCODE_CLEAR_FLAG;
	_opcodeMap[0xa0] = OPCODE_TAKE_CURRENT_OBJECT;
	_opcodeMap[0xa1] = OPCODE_CLEAR_FLAG40;
	_opcodeMap[0xa2] = OPCODE_MOVE_OBJECT_TO_ROOM;
	_opcodeMap[0xa5] = OPCODE_SET_FLAG40;
	_opcodeMap[0xa9] = OPCODE_CLEAR_INVISIBLE;
	_opcodeMap[0xad] = OPCODE_SET_INVISIBLE;
	_opcodeMap[0xc1] = OPCODE_VAR_DEC;
	_opcodeMap[0xc2] = OPCODE_SET_ROOM_GRAPHIC;
	_opcodeMap[0xc5] = OPCODE_SET_STRING_REPLACEMENT3;
	_opcodeMap[0xc9] = OPCODE_SET_STRING_REPLACEMENT1;
	_opcodeMap[0xcd] = OPCODE_SET_STRING_REPLACEMENT2;
	_opcodeMap[0xd1] = OPCODE_MOVE_DIR;
	_opcodeMap[0xd5] = OPCODE_DRAW_ROOM;
	_opcodeMap[0xd9] = OPCODE_DRAW_OBJECT;
	_opcodeMap[0xdd] = OPCODE_VAR_INC;
	_opcodeMap[0xe1] = OPCODE_MOVE_OBJECT_TO_CURRENT_ROOM;
	_opcodeMap[0xe5] = OPCODE_SET_CAN_TAKE;
	_opcodeMap[0xe9] = OPCODE_CLEAR_CAN_TAKE;
	_opcodeMap[0xed] = OPCODE_REMOVE_OBJECT;

#if 0
	_opcodeMap[0x9e] = OPCODE_INVENTORY_ROOM;
	_opcodeMap[0xc6] = OPCODE_SET_OBJECT_GRAPHIC;
	_opcodeMap[0xf0] = OPCODE_DROP_CURRENT_OBJECT;
	_opcodeMap[0xfc] = OPCODE_REMOVE_CURRENT_OBJECT;
#endif
}

void ComprehendGameV2::execute_opcode(const Instruction *instr, const Sentence *sentence,
		FunctionState *func_state) {
	Instruction instrCopy;
	byte noun = sentence ? sentence->_formattedWords[2] : 0;
	Room *room = get_room(_currentRoom);
	Item *item;

	// In case a single opcode is being executed outside of a function, use a dummy function state
	FunctionState dummyState;
	if (!func_state)
		func_state = &dummyState;

	if ((instr->_opcode & 0x30) == 0x30) {
		// First operand comes from entered sentence noun, shifting out existing operands
		instrCopy = *instr;
		instrCopy._operand[2] = instrCopy._operand[1];
		instrCopy._operand[1] = instrCopy._operand[0];
		instrCopy._operand[0] = get_item_id(noun) + 1;
		instr = &instrCopy;
	}

	func_state->_notComparison = (instr->_opcode & 0x40) != 0;

	switch (_opcodeMap[getOpcode(instr)]) {
	case OPCODE_CLEAR_INVISIBLE:
		item = get_item_by_noun(noun);
		item->_flags &= ~ITEMF_INVISIBLE;
		break;

	case OPCODE_DRAW_OBJECT:
		g_comprehend->drawItemPicture(instr->_operand[0] - 1);
		break;

	case OPCODE_DRAW_ROOM:
		g_comprehend->drawLocationPicture(instr->_operand[0] - 1);
		g_comprehend->readChar();
		break;

	case OPCODE_INVENTORY_FULL:
		item = get_item_by_noun(noun);

		weighInventory();
		func_set_test_result(func_state, _totalInventoryWeight + (item->_flags & ITEMF_WEIGHT_MASK) >
			_variables[VAR_INVENTORY_LIMIT]);
		break;

	case OPCODE_INVENTORY_FULL_X:
		item = get_item_by_noun(noun);

		weighInventory();
		func_set_test_result(func_state, _totalInventoryWeight + (item->_flags & ITEMF_WEIGHT_MASK) >
			_variables[instr->_operand[1]]);
		break;

	case OPCODE_MOVE_DIR:
		if (room->_direction[instr->_operand[0] - 1])
			move_to(room->_direction[instr->_operand[0] - 1]);
		else
			console_println(stringLookup(STRING_CANT_GO).c_str());
		break;

	case OPCODE_OBJECT_TAKEABLE:
		// WORKAROUND: Trying to get non-items in OO-Topos
		func_set_test_result(func_state, instr->_operand[0]
			&& (getItem(instr)->_flags & ITEMF_WEIGHT_MASK) != ITEMF_WEIGHT_MASK);
		break;

	case OPCODE_OBJECT_CAN_TAKE:
		item = getItem(instr);
		func_set_test_result(func_state, item->_flags & ITEMF_CAN_TAKE);
		break;

	case OPCODE_SET_OBJECT_GRAPHIC:
		item = getItem(instr);
		item->_graphic = instr->_operand[1];
		if (item->_room == _currentRoom)
			_updateFlags |= UPDATE_GRAPHICS;
		break;

	case OPCODE_SET_OBJECT_LONG_DESCRIPTION:
		item = getItem(instr);
		item->_longString = (instr->_operand[2] << 8) | instr->_operand[1];
		break;

	case OPCODE_SET_STRING_REPLACEMENT3: {
		int articleNum, bits = _wordFlags;
		for (articleNum = 3; articleNum >= 0; --articleNum, bits <<= 1) {
			if (bits >= 0x100)
				break;
		}
		if (articleNum == -1)
			articleNum = 2;

		_currentReplaceWord = instr->_operand[0] + articleNum - 1;
		break;
	}

	case OPCODE_WAIT_KEY:
		console_get_key();
		break;

	default:
		ComprehendGameOpcodes::execute_opcode(instr, sentence, func_state);
		break;
	}
}

byte ComprehendGameV2::getOpcode(const Instruction *instr) {
	// Special pre-processing for opcodes
	byte opcode = instr->_opcode;
	if (!(opcode & 0x80))
		opcode &= 0x3f;
	if ((opcode & 0x30) == 0x30) {
		opcode = (opcode & ~0x10) + 1;
	}

	return opcode;
}

void ComprehendGameV2::func_set_test_result(FunctionState *func_state, bool value) {
	ComprehendGameOpcodes::func_set_test_result(func_state, value ^ func_state->_notComparison);
}


} // namespace Comprehend
} // namespace Glk
