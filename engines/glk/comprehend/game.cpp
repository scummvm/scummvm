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

#include "glk/comprehend/game.h"
#include "common/debug-channels.h"
#include "common/translation.h"
#include "glk/comprehend/comprehend.h"
#include "glk/comprehend/debugger.h"
#include "glk/comprehend/dictionary.h"
#include "glk/comprehend/draw_surface.h"
#include "glk/comprehend/game_data.h"
#include "glk/comprehend/opcode_map.h"

namespace Glk {
namespace Comprehend {

struct Sentence {
	Word words[4];
	size_t nr_words;

	Sentence() {
		clear();
	}

	void clear() {
		for (uint idx = 0; idx < 4; ++idx)
			words[idx].clear();
		nr_words = 0;
	}
};

ComprehendGame::ComprehendGame() : _colorTable(0), _gameStrings(nullptr) {
}

ComprehendGame::~ComprehendGame() {
}

void ComprehendGame::synchronizeSave(Common::Serializer &s) {
	uint dir, i;
	size_t nr_rooms, nr_items;

	s.syncAsUint16LE(_currentRoom);

	// Variables
	for (i = 0; i < ARRAY_SIZE(_variables); i++)
		s.syncAsUint16LE(_variables[i]);

	// Flags
	for (i = 0; i < ARRAY_SIZE(_flags); i++)
		s.syncAsByte(_flags[i]);

	// Rooms. Note that index 0 is the player's inventory
	nr_rooms = _rooms.size();
	s.syncAsByte(nr_rooms);
	assert(nr_rooms == _rooms.size());

	for (i = 1; i < _rooms.size(); ++i) {
		s.syncAsUint16LE(_rooms[i].string_desc);
		for (dir = 0; dir < NR_DIRECTIONS; dir++)
			s.syncAsByte(_rooms[i].direction[dir]);

		s.syncAsByte(_rooms[i].flags);
		s.syncAsByte(_rooms[i].graphic);
	}

	// Objects
	nr_items = _items.size();
	s.syncAsByte(nr_items);
	assert(nr_items == _items.size());

	for (i = 0; i < _items.size(); ++i)
		_items[i].synchronize(s);
}

Common::String ComprehendGame::stringLookup(uint16 index) {
	uint16 string;
	uint8 table;

	/*
	 * There are two tables of strings. The first is stored in the main
	 * game data file, and the second is stored in multiple string files.
	 *
	 * In instructions string indexes are split into a table and index
	 * value. In other places such as the save files strings from the
	 * main table are occasionally just a straight 16-bit index. We
	 * convert all string indexes to the former case so that we can handle
	 * them the same everywhere.
	 */
	table = (index >> 8) & 0xff;
	string = index & 0xff;

	switch (table) {
	case 0x81:
	case 0x01:
		string += 0x100;
	/* Fall-through */
	case 0x00:
	case 0x80:
		if (string < _strings.size())
			return _strings[string];
		break;

	case 0x83:
		string += 0x100;
	/* Fall-through */
	case 0x02:
	case 0x82:
		if (string < _strings2.size())
			return _strings2[string];
		break;
	}

	return Common::String::format("BAD_STRING(%.4x)", index);
}

Common::String ComprehendGame::instrStringLookup(uint8 index, uint8 table) {
	return stringLookup(table << 8 | index);
}

int ComprehendGame::console_get_key() {
	int c, dummy;

	dummy = c = g_comprehend->readChar();

	/* Clear input buffer */
	while (dummy != '\n' && dummy != EOF)
		dummy = g_comprehend->readChar();

	return c;
}

void ComprehendGame::console_println(const char *text) {
	const char *replace, *word = nullptr, *p = text;
	char bad_word[64];
	int word_len = 0;

	if (!text) {
		g_comprehend->print("\n");
		return;
	}

	while (*p) {
		switch (*p) {
		case '\n':
			word = NULL;
			word_len = 0;
			g_comprehend->print("\n");
			p++;
			break;

		case '@':
			/* Replace word */
			if (_currentReplaceWord >= _replaceWords.size()) {
				snprintf(bad_word, sizeof(bad_word),
				         "[BAD_REPLACE_WORD(%.2x)]",
				         _currentReplaceWord);
				word = bad_word;
			} else {
				word = _replaceWords[_currentReplaceWord].c_str();
			}
			word_len = strlen(word);
			p++;
			break;

		default:
			/* Find next space */
			word_len = strcspn(p, " \n");
			if (word_len == 0)
				break;

			/*
			 * If this word contains a replacement symbol, then
			 * print everything before the symbol.
			 */
			replace = strchr(p, '@');
			if (replace)
				word_len = replace - p;

			word = p;
			p += word_len;
			break;
		}

		if (!word || !word_len)
			continue;
#ifdef DEPRECATED
		/* Print this word */
		if (line_length + word_len > console_winsize.ws_col) {
			/* Too long - insert a line break */
			g_comprehend->print("\n");
			line_length = 0;
		}
#endif
		Common::String wordStr(word, word_len);
		g_comprehend->print("%s", wordStr.c_str());

		if (*p == ' ') {
			g_comprehend->print(" ");
			p++;

			/* Skip any double spaces */
			while (*p == ' ')
				p++;
		}
	}

	g_comprehend->print("\n");
}

Room *ComprehendGame::get_room(uint16 index) {
	/* Room zero is reserved for the players inventory */
	if (index == 0)
		error("Room index 0 (player inventory) is invalid");

	if (index >= (int)_rooms.size())
		error("Room index %d is invalid", index);

	return &_rooms[index];
}

Item *ComprehendGame::get_item(uint16 index) {
	if (index >= _items.size())
		error("Bad item %d\n", index);

	return &_items[index];
}

void ComprehendGame::game_save() {
	int c;

	console_println(_strings[STRING_SAVE_GAME].c_str());

	c = console_get_key();
	if (c < '1' || c > '3') {
		/*
		 * The original Comprehend games just silently ignore any
		 * invalid selection.
		 */
		console_println("Invalid save game number");
		return;
	}

	g_comprehend->saveGameState(c - '0', _("Savegame"));
}

void ComprehendGame::game_restore() {
	int c;

	console_println(_strings[STRING_RESTORE_GAME].c_str());

	c = console_get_key();
	if (c < '1' || c > '3') {
		/*
		 * The original Comprehend games just silently ignore any
		 * invalid selection.
		 */
		console_println("Invalid save game number");
		return;
	}

	(void)g_comprehend->loadGameState(c - '0');
}

void ComprehendGame::game_restart() {
	console_println(stringLookup(_gameStrings->game_restart).c_str());
	console_get_key();

	comprehend_load_game(this);
	_updateFlags = UPDATE_ALL;
}

WordIndex *ComprehendGame::is_word_pair(Word *word1, Word *word2) {
	WordMap *map;
	uint i;

	/* Check if this is a word pair */
	for (i = 0; i < _wordMaps.size(); i++) {
		map = &_wordMaps[i];

		if (map->word[0].index == word1->_index &&
		        map->word[0].type == word1->_type &&
		        map->word[1].index == word2->_index &&
		        map->word[1].type == word2->_type)
			return &map->word[2];
	}

	return nullptr;
}

Item *ComprehendGame::get_item_by_noun(Word *noun) {
	uint i;

	if (!noun || !(noun->_type & WORD_TYPE_NOUN_MASK))
		return NULL;

	/*
	 * FIXME - in oo-topos the word 'box' matches more than one object
	 *         (the box and the snarl-in-a-box). The player is unable
	 *         to drop the latter because this will match the former.
	 */
	for (i = 0; i < _items.size(); i++)
		if (_items[i].word == noun->_index)
			return &_items[i];

	return NULL;
}

void ComprehendGame::update_graphics() {
	Item *item;
	Room *room;
	int type;
	uint i;

	if (!g_comprehend->_graphicsEnabled)
		return;

	type = roomIsSpecial(_currentRoom, NULL);

	switch (type) {
	case ROOM_IS_DARK:
		if (_updateFlags & UPDATE_GRAPHICS)
			g_comprehend->clearScreen(false);
		break;

	case ROOM_IS_TOO_BRIGHT:
		if (_updateFlags & UPDATE_GRAPHICS)
			g_comprehend->clearScreen(false);
		break;

	default:
		if (_updateFlags & UPDATE_GRAPHICS) {
			room = get_room(_currentRoom);
			g_comprehend->drawLocationPicture(room->graphic - 1);
		}

		if ((_updateFlags & UPDATE_GRAPHICS) ||
		        (_updateFlags & UPDATE_GRAPHICS_ITEMS)) {
			for (i = 0; i < _items.size(); i++) {
				item = &_items[i];

				if (item->room == _currentRoom &&
				        item->graphic != 0)
					g_comprehend->drawItemPicture(item->graphic - 1);
			}
		}
		break;
	}
}

void ComprehendGame::describe_objects_in_current_room() {
	Item *item;
	size_t count = 0;
	uint i;

	for (i = 0; i < _items.size(); i++) {
		item = &_items[i];

		if (item->room == _currentRoom &&
		        item->string_desc != 0)
			count++;
	}

	if (count > 0) {
		console_println(stringLookup(STRING_YOU_SEE).c_str());

		for (i = 0; i < _items.size(); i++) {
			item = &_items[i];

			if (item->room == _currentRoom &&
			        item->string_desc != 0)
				console_println(stringLookup(item->string_desc).c_str());
		}
	}
}

void ComprehendGame::update() {
	Room *room = get_room(_currentRoom);
	unsigned room_type, room_desc_string;

	update_graphics();

	/* Check if the room is special (dark, too bright, etc) */
	room_desc_string = room->string_desc;
	room_type = roomIsSpecial(_currentRoom,
	                                &room_desc_string);

	if (_updateFlags & UPDATE_ROOM_DESC)
		console_println(stringLookup(room_desc_string).c_str());

	if ((_updateFlags & UPDATE_ITEM_LIST) &&
	        room_type == ROOM_IS_NORMAL)
		describe_objects_in_current_room();

	_updateFlags = 0;
}

void ComprehendGame::move_to(uint8 room) {
	if (room >= (int)_rooms.size())
		error("Attempted to move to invalid room %.2x\n", room);

	_currentRoom = room;
	_updateFlags = (UPDATE_GRAPHICS | UPDATE_ROOM_DESC |
	                      UPDATE_ITEM_LIST);
}

void ComprehendGame::func_set_test_result(FunctionState *func_state, bool value) {
	if (func_state->or_count == 0) {
		/* And */
		if (func_state->_and) {
			if (!value)
				func_state->test_result = false;
		} else {
			func_state->test_result = value;
			func_state->_and = true;
		}

	} else {
		/* Or */
		if (value)
			func_state->test_result = value;
	}
}

size_t ComprehendGame::num_objects_in_room(int room) {
	size_t count = 0, i;

	for (i = 0; i < _items.size(); i++)
		if (_items[i].room == room)
			count++;

	return count;
}

void ComprehendGame::move_object(Item *item, int new_room) {
	unsigned obj_weight = item->flags & ITEMF_WEIGHT_MASK;

	if (item->room == new_room)
		return;

	if (item->room == ROOM_INVENTORY) {
		/* Removed from player's inventory */
		_variables[VAR_INVENTORY_WEIGHT] -= obj_weight;
	}
	if (new_room == ROOM_INVENTORY) {
		/* Moving to the player's inventory */
		_variables[VAR_INVENTORY_WEIGHT] += obj_weight;
	}

	if (item->room == _currentRoom) {
		/* Item moved away from the current room */
		_updateFlags |= UPDATE_GRAPHICS;

	} else if (new_room == _currentRoom) {
		/*
		 * Item moved into the current room. Only the item needs a
		 * redraw, not the whole room.
		 */
		_updateFlags |= (UPDATE_GRAPHICS_ITEMS |
		                       UPDATE_ITEM_LIST);
	}

	item->room = new_room;
}

void ComprehendGame::eval_instruction(FunctionState *func_state,
                             Instruction *instr,
                             Word *verb, Word *noun) {
	const byte *opcode_map = _opcodeMap;
	Room *room;
	Item *item;
	uint16 index;
	bool test;
	uint i, count;

	room = get_room(_currentRoom);

	if (DebugMan.isDebugChannelEnabled(kDebugScripts)) {
		Common::String line;
		if (!instr->is_command) {
			line += "? ";
		} else {
			if (func_state->test_result)
				line += "+ ";
			else
				line += "- ";
		}

		line += g_debugger->dumpInstruction(this, func_state, instr);
		debugC(kDebugScripts, "%s", line.c_str());
	}

	if (func_state->or_count)
		func_state->or_count--;

	if (instr->is_command) {
		bool do_command;

		func_state->in_command = true;
		do_command = func_state->test_result;

		if (func_state->or_count != 0)
			g_comprehend->print("Warning: or_count == %d\n",
			                    func_state->or_count);
		func_state->or_count = 0;

		if (!do_command)
			return;

		func_state->else_result = false;
		func_state->executed = true;

	} else {
		if (func_state->in_command) {
			/* Finished command sequence - clear test result */
			func_state->in_command = false;
			func_state->test_result = false;
			func_state->_and = false;
		}
	}

	switch (opcode_map[instr->opcode]) {
	case OPCODE_VAR_ADD:
		_variables[instr->operand[0]] +=
		    _variables[instr->operand[1]];
		break;

	case OPCODE_VAR_SUB:
		_variables[instr->operand[0]] -=
		    _variables[instr->operand[1]];
		break;

	case OPCODE_VAR_INC:
		_variables[instr->operand[0]]++;
		break;

	case OPCODE_VAR_DEC:
		_variables[instr->operand[0]]--;
		break;

	case OPCODE_VAR_EQ:
		func_set_test_result(func_state,
		                     _variables[instr->operand[0]] ==
		                     _variables[instr->operand[1]]);
		break;

	case OPCODE_TURN_TICK:
		_variables[VAR_TURN_COUNT]++;
		break;

	case OPCODE_PRINT:
		console_println(instrStringLookup(
		                    instr->operand[0], instr->operand[1])
		                .c_str());
		break;

	case OPCODE_TEST_NOT_ROOM_FLAG:
		func_set_test_result(func_state,
		                     !(room->flags & instr->operand[0]));
		break;

	case OPCODE_TEST_ROOM_FLAG:
		func_set_test_result(func_state,
		                     room->flags & instr->operand[0]);
		break;

	case OPCODE_NOT_IN_ROOM:
		func_set_test_result(func_state,
		                     _currentRoom != instr->operand[0]);
		break;

	case OPCODE_IN_ROOM:
		func_set_test_result(func_state,
		                     _currentRoom == instr->operand[0]);
		break;

	case OPCODE_MOVE_TO_ROOM:
		if (instr->operand[0] == 0xff) {
			/*
			 * FIXME - Not sure what this is for. Transylvania
			 * uses it in the 'go north' case when in room
			 * 0x01 or 0x0c, and Oo-Topos uses it when you shoot
			 * the alien. Ignore it for now.
			 */
			break;
		}

		move_to(instr->operand[0]);
		break;

	case OPCODE_MOVE:
		/* Move in the direction dictated by the current verb */
		if (verb->_index - 1 >= NR_DIRECTIONS)
			error("Bad verb %d:%d in move",
			      verb->_index, verb->_type);

		if (room->direction[verb->_index - 1])
			move_to(room->direction[verb->_index - 1]);
		else
			console_println(stringLookup(STRING_CANT_GO).c_str());
		break;

	case OPCODE_MOVE_DIRECTION:
		if (room->direction[instr->operand[0] - 1])
			move_to(room->direction[instr->operand[0] - 1]);
		else
			console_println(stringLookup(STRING_CANT_GO).c_str());
		break;

	case OPCODE_ELSE:
		func_state->test_result = func_state->else_result;
		break;

	case OPCODE_MOVE_OBJECT_TO_CURRENT_ROOM:
		item = get_item(instr->operand[0] - 1);
		move_object(item, _currentRoom);
		break;

	case OPCODE_OBJECT_IN_ROOM:
		item = get_item(instr->operand[0] - 1);
		func_set_test_result(func_state,
		                     item->room == instr->operand[1]);
		break;

	case OPCODE_OBJECT_NOT_IN_ROOM:
		item = get_item(instr->operand[0] - 1);
		func_set_test_result(func_state,
		                     item->room != instr->operand[1]);
		break;

	case OPCODE_MOVE_OBJECT_TO_ROOM:
		item = get_item(instr->operand[0] - 1);
		move_object(item, instr->operand[1]);
		break;

	case OPCODE_INVENTORY_FULL:
		item = get_item_by_noun(noun);
		func_set_test_result(func_state,
		                     _variables[VAR_INVENTORY_WEIGHT] +
		                     (item->flags & ITEMF_WEIGHT_MASK) >
		                     _variables[VAR_INVENTORY_LIMIT]);
		break;

	case OPCODE_DESCRIBE_CURRENT_OBJECT:
		/*
		 * This opcode is only used in version 2
		 * FIXME - unsure what the single operand is for.
		 */
		item = get_item_by_noun(noun);
		g_comprehend->print("%s\n", stringLookup(item->long_string).c_str());
		break;

	case OPCODE_CURRENT_OBJECT_IN_ROOM:
		/* FIXME - use common code for these two ops */
		test = false;

		if (noun) {
			for (i = 0; i < _items.size(); i++) {
				Item *itemP = &_items[i];

				if (itemP->word == noun->_index &&
				        itemP->room == instr->operand[0]) {
					test = true;
					break;
				}
			}
		}

		func_set_test_result(func_state, test);
		break;

	case OPCODE_CURRENT_OBJECT_NOT_PRESENT:
		/* FIXME - use common code for these two ops */
		item = get_item_by_noun(noun);
		if (item)
			func_set_test_result(func_state,
			                     item->room != _currentRoom);
		else
			func_set_test_result(func_state, true);
		break;

	case OPCODE_CURRENT_OBJECT_PRESENT:
		item = get_item_by_noun(noun);
		if (item)
			func_set_test_result(func_state,
			                     item->room == _currentRoom);
		else
			func_set_test_result(func_state, false);
		break;

	case OPCODE_HAVE_OBJECT:
		item = get_item(instr->operand[0] - 1);
		func_set_test_result(func_state,
		                     item->room == ROOM_INVENTORY);
		break;

	case OPCODE_NOT_HAVE_CURRENT_OBJECT:
		item = get_item_by_noun(noun);
		func_set_test_result(func_state,
		                     !item || item->room != ROOM_INVENTORY);
		break;

	case OPCODE_HAVE_CURRENT_OBJECT:
		item = get_item_by_noun(noun);
		func_set_test_result(func_state,
		                     item->room == ROOM_INVENTORY);
		break;

	case OPCODE_NOT_HAVE_OBJECT:
		item = get_item(instr->operand[0] - 1);
		func_set_test_result(func_state,
		                     item->room != ROOM_INVENTORY);
		break;

	case OPCODE_CURRENT_OBJECT_TAKEABLE:
		item = get_item_by_noun(noun);
		if (!item)
			func_set_test_result(func_state, false);
		else
			func_set_test_result(func_state,
			                     (item->flags & ITEMF_CAN_TAKE));
		break;

	case OPCODE_CURRENT_OBJECT_NOT_TAKEABLE:
		item = get_item_by_noun(noun);
		if (!item)
			func_set_test_result(func_state, true);
		else
			func_set_test_result(func_state,
			                     !(item->flags & ITEMF_CAN_TAKE));
		break;

	case OPCODE_CURRENT_OBJECT_IS_NOWHERE:
		item = get_item_by_noun(noun);
		if (!item)
			func_set_test_result(func_state, false);
		else
			func_set_test_result(func_state,
			                     item->room == ROOM_NOWHERE);
		break;

	case OPCODE_OBJECT_IS_NOWHERE:
		item = get_item(instr->operand[0] - 1);
		func_set_test_result(func_state,
		                     item->room == ROOM_NOWHERE);
		break;

	case OPCODE_OBJECT_IS_NOT_NOWHERE:
		item = get_item(instr->operand[0] - 1);
		func_set_test_result(func_state,
		                     item->room != ROOM_NOWHERE);
		break;

	case OPCODE_OBJECT_NOT_PRESENT:
		item = get_item(instr->operand[0] - 1);
		func_set_test_result(func_state,
		                     item->room != _currentRoom);
		break;

	case OPCODE_OBJECT_PRESENT:
		item = get_item(instr->operand[0] - 1);
		func_set_test_result(func_state,
		                     item->room == _currentRoom);
		break;

	case OPCODE_OBJECT_NOT_VALID:
		/* FIXME - should be called OPCODE_CURRENT_OBJECT_NOT_VALID */
		func_set_test_result(func_state, !noun ||
		                     (noun->_type & WORD_TYPE_NOUN_MASK) == 0);
		break;

	case OPCODE_CURRENT_IS_OBJECT:
		func_set_test_result(func_state,
		                     get_item_by_noun(noun) != NULL);
		break;

	case OPCODE_CURRENT_NOT_OBJECT:
		func_set_test_result(func_state,
		                     get_item_by_noun(noun) == NULL);
		break;

	case OPCODE_REMOVE_OBJECT:
		item = get_item(instr->operand[0] - 1);
		move_object(item, ROOM_NOWHERE);
		break;

	case OPCODE_REMOVE_CURRENT_OBJECT:
		item = get_item_by_noun(noun);
		move_object(item, ROOM_NOWHERE);
		break;

	case OPCODE_INVENTORY:
		count = num_objects_in_room(ROOM_INVENTORY);
		if (count == 0) {
			console_println(stringLookup(STRING_INVENTORY_EMPTY).c_str());
			break;
		}

		console_println(stringLookup(STRING_INVENTORY).c_str());
		for (i = 0; i < _items.size(); i++) {
			item = &_items[i];
			if (item->room == ROOM_INVENTORY)
				g_comprehend->print("%s\n",
				                    stringLookup(item->string_desc).c_str());
		}
		break;

	case OPCODE_INVENTORY_ROOM:
		count = num_objects_in_room(instr->operand[0]);
		if (count == 0) {
			console_println(stringLookup(instr->operand[1] + 1).c_str());
			break;
		}

		console_println(stringLookup(instr->operand[1]).c_str());
		for (i = 0; i < _items.size(); i++) {
			item = &_items[i];
			if (item->room == instr->operand[0])
				g_comprehend->print("%s\n",
				                    stringLookup(item->string_desc).c_str());
		}
		break;

	case OPCODE_MOVE_CURRENT_OBJECT_TO_ROOM:
		item = get_item_by_noun(noun);
		if (!item)
			error("Bad current object\n");

		move_object(item, instr->operand[0]);
		break;

	case OPCODE_DROP_OBJECT:
		item = get_item(instr->operand[0] - 1);
		move_object(item, _currentRoom);
		break;

	case OPCODE_DROP_CURRENT_OBJECT:
		item = get_item_by_noun(noun);
		if (!item)
			error("Attempt to take object failed\n");

		move_object(item, _currentRoom);
		break;

	case OPCODE_TAKE_CURRENT_OBJECT:
		item = get_item_by_noun(noun);
		if (!item)
			error("Attempt to take object failed\n");

		move_object(item, ROOM_INVENTORY);
		break;

	case OPCODE_TAKE_OBJECT:
		item = get_item(instr->operand[0] - 1);
		move_object(item, ROOM_INVENTORY);
		break;

	case OPCODE_TEST_FLAG:
		func_set_test_result(func_state,
		                     _flags[instr->operand[0]]);
		break;

	case OPCODE_TEST_NOT_FLAG:
		func_set_test_result(func_state,
		                     !_flags[instr->operand[0]]);
		break;

	case OPCODE_CLEAR_FLAG:
		_flags[instr->operand[0]] = false;
		break;

	case OPCODE_SET_FLAG:
		_flags[instr->operand[0]] = true;
		break;

	case OPCODE_OR:
		if (func_state->or_count) {
			func_state->or_count += 2;
		} else {
			func_state->test_result = false;
			func_state->or_count += 3;
		}
		break;

	case OPCODE_SET_OBJECT_DESCRIPTION:
		item = get_item(instr->operand[0] - 1);
		item->string_desc = (instr->operand[2] << 8) | instr->operand[1];
		break;

	case OPCODE_SET_OBJECT_LONG_DESCRIPTION:
		item = get_item(instr->operand[0] - 1);
		item->long_string = (instr->operand[2] << 8) | instr->operand[1];
		break;

	case OPCODE_SET_ROOM_DESCRIPTION:
		room = get_room(instr->operand[0]);
		switch (instr->operand[2]) {
		case 0x80:
			room->string_desc = instr->operand[1];
			break;
		case 0x81:
			room->string_desc = instr->operand[1] + 0x100;
			break;
		case 0x82:
			room->string_desc = instr->operand[1] + 0x200;
			break;
		default:
			error("Bad string desc %.2x:%.2x\n",
			      instr->operand[1], instr->operand[2]);
			break;
		}
		break;

	case OPCODE_SET_OBJECT_GRAPHIC:
		item = get_item(instr->operand[0] - 1);
		item->graphic = instr->operand[1];
		if (item->room == _currentRoom)
			_updateFlags |= UPDATE_GRAPHICS;
		break;

	case OPCODE_SET_ROOM_GRAPHIC:
		room = get_room(instr->operand[0]);
		room->graphic = instr->operand[1];
		if (instr->operand[0] == _currentRoom)
			_updateFlags |= UPDATE_GRAPHICS;
		break;

	case OPCODE_CALL_FUNC:
		index = instr->operand[0];
		if (instr->operand[1] == 0x81)
			index += 256;
		if (index >= _functions.size())
			error("Bad function %.4x >= %.4x\n",
			      index, _functions.size());

		debugC(kDebugScripts, "Calling subfunction %.4x", index);
		eval_function(&_functions[index], verb, noun);
		break;

	case OPCODE_TEST_FALSE:
		/*
		 * FIXME - not sure what this is for. In Transylvania
		 * it is opcode 0x50 and is used when attempting to
		 * take the bar in the cellar. If it returns true then
		 * the response is "there's none here".
		 */
		func_set_test_result(func_state, false);
		break;

	case OPCODE_SAVE_ACTION:
		/*
		 * FIXME - This saves the current verb and allows the next
		 * command to use just the noun. This is used to allow
		 * responses to ask the player what they meant, e.g:
		 *
		 *   > drop
		 *   I don't understand what you want to drop.
		 *   > gun
		 *   Okay.
		 */
		break;

	case OPCODE_SET_STRING_REPLACEMENT:
		_currentReplaceWord = instr->operand[0] - 1;
		break;

	case OPCODE_SET_CURRENT_NOUN_STRING_REPLACEMENT:
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
		break;

	case OPCODE_DRAW_ROOM:
		g_comprehend->drawLocationPicture(instr->operand[0] - 1);
		break;

	case OPCODE_DRAW_OBJECT:
		g_comprehend->drawItemPicture(instr->operand[0] - 1);
		break;

	case OPCODE_WAIT_KEY:
		console_get_key();
		break;

	case OPCODE_SPECIAL:
		/* Game specific opcode */
		handleSpecialOpcode(instr->operand[0]);
		break;

	default:
		if (instr->opcode & 0x80) {
			debugC(kDebugScripts,
			       "Unhandled command opcode %.2x",
			       instr->opcode);
		} else {
			debugC(kDebugScripts,
			       "Unhandled test opcode %.2x - returning false",
			       instr->opcode);
			func_set_test_result(func_state, false);
		}
		break;
	}
}

void ComprehendGame::eval_function(Function *func,
                   Word *verb, Word *noun) {
	FunctionState func_state;
	uint i;

	func_state.else_result = true;
	func_state.executed = false;

	for (i = 0; i < func->nr_instructions; i++) {
		if (func_state.executed && !func->instructions[i].is_command) {
			/*
			 * At least one command has been executed and the
			 * current instruction is a test. Exit the function.
			 */
			break;
		}

		eval_instruction(&func_state, &func->instructions[i],
		                 verb, noun);
	}
}

void ComprehendGame::skip_whitespace(char **p) {
	while (**p && Common::isSpace(**p))
		(*p)++;
}

void ComprehendGame::skip_non_whitespace(char **p) {
	while (**p && !Common::isSpace(**p) && **p != ',' && **p != '\n')
		(*p)++;
}

bool ComprehendGame::handle_sentence(Sentence *sentence) {
	Function *func;
	Action *action;
	uint i, j;

	if (sentence->nr_words == 0)
		return false;

	/* Find a matching action */
	for (i = 0; i < _actions.size(); i++) {
		action = &_actions[i];

		if (action->type == ACTION_VERB_OPT_NOUN &&
		        sentence->nr_words > action->nr_words + 1)
			continue;
		if (action->type != ACTION_VERB_OPT_NOUN &&
		        sentence->nr_words != action->nr_words)
			continue;

		/*
		 * If all words in a sentence match those for an action then
		 * run that action's function.
		 */
		for (j = 0; j < action->nr_words; j++) {
			if (sentence->words[j]._index == action->word[j] &&
			        (sentence->words[j]._type & action->word_type[j]))
				continue;

			/* Word didn't match */
			break;
		}
		if (j == action->nr_words) {
			/* Match */
			func = &_functions[action->function];
			eval_function(func,
			              &sentence->words[0], &sentence->words[1]);
			return true;
		}
	}

	/* No matching action */
	console_println(stringLookup(STRING_DONT_UNDERSTAND).c_str());
	return false;
}

void ComprehendGame::read_sentence(char **line,
                          Sentence *sentence) {
	bool sentence_end = false;
	char *word_string, *p = *line;
	WordIndex *pair;
	Word *word;
	int index;

	sentence->clear();
	while (1) {
		skip_whitespace(&p);
		word_string = p;
		skip_non_whitespace(&p);

		if (*p == ',' || *p == '\n') {
			/* Sentence separator */
			*p++ = '\0';
			sentence_end = true;
		} else {
			if (*p == '\0')
				sentence_end = true;
			else
				*p++ = '\0';
		}

		/* Find the dictionary word for this */
		word = dict_find_word_by_string(this, word_string);
		if (!word)
			sentence->words[sentence->nr_words].clear();
		else
			sentence->words[sentence->nr_words] = *word;

		sentence->nr_words++;

		if (sentence->nr_words > 1) {
			index = sentence->nr_words;

			/* See if this word and the previous are a word pair */
			pair = is_word_pair(&sentence->words[index - 2],
			                    &sentence->words[index - 1]);
			if (pair) {
				sentence->words[index - 2]._index = pair->index;
				sentence->words[index - 2]._type = pair->type;
				strcpy(sentence->words[index - 2]._word,
				       "[PAIR]");
				sentence->nr_words--;
			}
		}

		if (sentence->nr_words >= ARRAY_SIZE(sentence->words) ||
		        sentence_end)
			break;
	}

	*line = p;
}

void ComprehendGame::doBeforeTurn() {
	// Run the game specific before turn bits
	beforeTurn();

	// Run the each turn functions
	eval_function(&_functions[0], NULL, NULL);

	update();
}

void ComprehendGame::doAfterTurn() {
	// Do post turn game specific bits
	afterTurn();
}

void ComprehendGame::read_input() {
	Sentence sentence;
	char *line = NULL, buffer[1024];
	bool handled;

	beforePrompt();
	doBeforeTurn();

	do {
		g_comprehend->print("> ");
		g_comprehend->readLine(buffer, sizeof(buffer));
		if (g_comprehend->shouldQuit())
			return;
	} while (strlen(buffer) == 0);

	// Re-comprehend special commands start with '!'
	line = &buffer[0];

	while (1) {
		read_sentence(&line, &sentence);
		handled = handle_sentence(&sentence);
		if (handled)
			doAfterTurn();

		/* FIXME - handle the 'before you can continue' case */
		if (*line == '\0')
			break;
		line++;

		if (handled)
			doBeforeTurn();
	}
}

void ComprehendGame::playGame() {
	beforeGame();

	_updateFlags = (uint)UPDATE_ALL;
	while (!g_comprehend->shouldQuit())
		read_input();
}

} // namespace Comprehend
} // namespace Glk
