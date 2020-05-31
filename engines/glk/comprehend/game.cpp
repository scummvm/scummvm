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
#include "glk/comprehend/comprehend.h"
#include "glk/comprehend/debugger.h"
#include "glk/comprehend/dictionary.h"
#include "glk/comprehend/game_data.h"
#include "glk/comprehend/draw_surface.h"
#include "glk/comprehend/opcode_map.h"
#include "glk/comprehend/strings.h"
#include "glk/comprehend/util.h"

namespace Glk {
namespace Comprehend {

struct Sentence {
	Word words[4];
	size_t nr_words;
};

ComprehendGame::ComprehendGame() : _gameName(nullptr),
                                   _shortName(nullptr),
                                   _gameDataFile(nullptr),
                                   _savegameFileFormat(nullptr),
                                   _colorTable(0),
                                   _gameStrings(nullptr) {
}

ComprehendGame::~ComprehendGame() {
}

static void console_init(void) {
	//	ioctl(STDOUT_FILENO, TIOCGWINSZ, &console_winsize);
}

int console_get_key(void) {
	int c, dummy;

	dummy = c = getchar();

	/* Clear input buffer */
	while (dummy != '\n' && dummy != EOF)
		dummy = getchar();

	return c;
}

void console_println(ComprehendGame *game, const char *text) {
	const char *replace, *word = nullptr, *p = text;
	char bad_word[64];
	int word_len = 0;

	if (!text) {
		printf("\n");
		return;
	}

	while (*p) {
		switch (*p) {
		case '\n':
			word = NULL;
			word_len = 0;
			printf("\n");
			p++;
			break;

		case '@':
			/* Replace word */
			if (game->_currentReplaceWord >= game->_nr_replace_words) {
				snprintf(bad_word, sizeof(bad_word),
				         "[BAD_REPLACE_WORD(%.2x)]",
				         game->_currentReplaceWord);
				word = bad_word;
			} else {
				word = game->_replaceWords[game->_currentReplaceWord];
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
			printf("\n");
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

static Room *get_room(ComprehendGame *game, uint16 index) {
	/* Room zero is reserved for the players inventory */
	if (index == 0)
		fatal_error("Room index 0 (player inventory) is invalid");

	if (index - 1 >= (int)game->_nr_rooms)
		fatal_error("Room index %d is invalid", index);

	return &game->_rooms[index];
}

Item *get_item(ComprehendGame *game, uint16 index) {
	if (index >= game->_header.nr_items)
		fatal_error("Bad item %d\n", index);

	return &game->_items[index];
}

void game_save(ComprehendGame *game) {
	char filename[32];
	int c;

	console_println(game, game->_strings.strings[STRING_SAVE_GAME]);

	c = console_get_key();
	if (c < '1' || c > '3') {
		/*
		 * The original Comprehend games just silently ignore any
		 * invalid selection.
		 */
		console_println(game, "Invalid save game number");
		return;
	}

	snprintf(filename, sizeof(filename), game->_savegameFileFormat, c - '0');
	comprehend_save_game(game, filename);
}

void game_restore(ComprehendGame *game) {
	char filename[32];
	int c;

	console_println(game, game->_strings.strings[STRING_RESTORE_GAME]);

	c = console_get_key();
	if (c < '1' || c > '3') {
		/*
		 * The original Comprehend games just silently ignore any
		 * invalid selection.
		 */
		console_println(game, "Invalid save game number");
		return;
	}

	snprintf(filename, sizeof(filename), game->_savegameFileFormat, c - '0');
	comprehend_restore_game(game, filename);

	game->_updateFlags = UPDATE_ALL;
}

void game_restart(ComprehendGame *game) {
	console_println(game, string_lookup(game, game->_gameStrings->game_restart));
	console_get_key();

	comprehend_load_game(game);
	game->_updateFlags = UPDATE_ALL;
}

static WordIndex *is_word_pair(ComprehendGame *game,
                                       Word *word1, Word *word2) {
	WordMap *map;
	uint i;

	/* Check if this is a word pair */
	for (i = 0; i < game->_nr_word_maps; i++) {
		map = &game->_wordMaps[i];

		if (map->word[0].index == word1->_index &&
		    map->word[0].type == word1->_type &&
		    map->word[1].index == word2->_index &&
		    map->word[1].type == word2->_type)
			return &map->word[2];
	}

	return NULL;
}

static Item *get_item_by_noun(ComprehendGame *game,
                                     Word *noun) {
	uint i;

	if (!noun || !(noun->_type & WORD_TYPE_NOUN_MASK))
		return NULL;

	/*
	 * FIXME - in oo-topos the word 'box' matches more than one object
	 *         (the box and the snarl-in-a-box). The player is unable
	 *         to drop the latter because this will match the former.
	 */
	for (i = 0; i < game->_header.nr_items; i++)
		if (game->_items[i].word == noun->_index)
			return &game->_items[i];

	return NULL;
}

static void update_graphics(ComprehendGame *game) {
	Item *item;
	Room *room;
	int type;
	uint i;

	if (!g_comprehend->_graphicsEnabled)
		return;

	type = game->room_is_special(game->_currentRoom, NULL);

	switch (type) {
	case ROOM_IS_DARK:
		if (game->_updateFlags & UPDATE_GRAPHICS)
			draw_dark_room();
		break;

	case ROOM_IS_TOO_BRIGHT:
		if (game->_updateFlags & UPDATE_GRAPHICS)
			draw_bright_room();
		break;

	default:
		if (game->_updateFlags & UPDATE_GRAPHICS) {
			room = get_room(game, game->_currentRoom);
			draw_location_image(&game->_roomImages,
			                    room->graphic - 1);
		}

		if ((game->_updateFlags & UPDATE_GRAPHICS) ||
		    (game->_updateFlags & UPDATE_GRAPHICS_ITEMS)) {
			for (i = 0; i < game->_header.nr_items; i++) {
				item = &game->_items[i];

				if (item->room == game->_currentRoom &&
				    item->graphic != 0)
					draw_image(&game->_itemImages,
					           item->graphic - 1);
			}
		}
		break;
	}
}

static void describe_objects_in_current_room(ComprehendGame *game) {
	Item *item;
	size_t count = 0;
	uint i;

	for (i = 0; i < game->_header.nr_items; i++) {
		item = &game->_items[i];

		if (item->room == game->_currentRoom &&
		    item->string_desc != 0)
			count++;
	}

	if (count > 0) {
		console_println(game, string_lookup(game, STRING_YOU_SEE));

		for (i = 0; i < game->_header.nr_items; i++) {
			item = &game->_items[i];

			if (item->room == game->_currentRoom &&
			    item->string_desc != 0)
				console_println(game, string_lookup(game, item->string_desc));
		}
	}
}

static void update(ComprehendGame *game) {
	Room *room = get_room(game, game->_currentRoom);
	unsigned room_type, room_desc_string;

	update_graphics(game);

	/* Check if the room is special (dark, too bright, etc) */
	room_desc_string = room->string_desc;
	room_type = game->room_is_special(game->_currentRoom,
		&room_desc_string);

	if (game->_updateFlags & UPDATE_ROOM_DESC)
		console_println(game, string_lookup(game, room_desc_string));

	if ((game->_updateFlags & UPDATE_ITEM_LIST) &&
	    room_type == ROOM_IS_NORMAL)
		describe_objects_in_current_room(game);

	game->_updateFlags = 0;
}

static void move_to(ComprehendGame *game, uint8 room) {
	if (room - 1 >= (int)game->_nr_rooms)
		fatal_error("Attempted to move to invalid room %.2x\n", room);

	game->_currentRoom = room;
	game->_updateFlags = (UPDATE_GRAPHICS | UPDATE_ROOM_DESC |
	                            UPDATE_ITEM_LIST);
}

static void func_set_test_result(FunctionState *func_state, bool value) {
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

static size_t num_objects_in_room(ComprehendGame *game, int room) {
	size_t count = 0, i;

	for (i = 0; i < game->_header.nr_items; i++)
		if (game->_items[i].room == room)
			count++;

	return count;
}

void move_object(ComprehendGame *game, Item *item, int new_room) {
	unsigned obj_weight = item->flags & ITEMF_WEIGHT_MASK;

	if (item->room == new_room)
		return;

	if (item->room == ROOM_INVENTORY) {
		/* Removed from player's inventory */
		game->_variables[VAR_INVENTORY_WEIGHT] -= obj_weight;
	}
	if (new_room == ROOM_INVENTORY) {
		/* Moving to the player's inventory */
		game->_variables[VAR_INVENTORY_WEIGHT] += obj_weight;
	}

	if (item->room == game->_currentRoom) {
		/* Item moved away from the current room */
		game->_updateFlags |= UPDATE_GRAPHICS;

	} else if (new_room == game->_currentRoom) {
		/*
		 * Item moved into the current room. Only the item needs a
		 * redraw, not the whole room.
		 */
		game->_updateFlags |= (UPDATE_GRAPHICS_ITEMS |
		                             UPDATE_ITEM_LIST);
	}

	item->room = new_room;
}

static void eval_instruction(ComprehendGame *game,
                             FunctionState *func_state,
                             Instruction *instr,
                             Word *verb, Word *noun) {
	const byte *opcode_map = game->_opcodeMap;
	Room *room;
	Item *item;
	uint16 index;
	bool test;
	uint i, count;

	room = get_room(game, game->_currentRoom);

	if (gDebugLevel > 0) {
		Common::String line;
		if (!instr->is_command) {
			line += "? ";
		} else {
			if (func_state->test_result)
				line += "+ ";
			else
				line += "- ";
		}

		line += g_debugger->dumpInstruction(game, func_state, instr);
		debug("%s", line.c_str());
	}

	if (func_state->or_count)
		func_state->or_count--;

	if (instr->is_command) {
		bool do_command;

		func_state->in_command = true;
		do_command = func_state->test_result;

		if (func_state->or_count != 0)
			printf("Warning: or_count == %d\n",
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
		game->_variables[instr->operand[0]] +=
		    game->_variables[instr->operand[1]];
		break;

	case OPCODE_VAR_SUB:
		game->_variables[instr->operand[0]] -=
		    game->_variables[instr->operand[1]];
		break;

	case OPCODE_VAR_INC:
		game->_variables[instr->operand[0]]++;
		break;

	case OPCODE_VAR_DEC:
		game->_variables[instr->operand[0]]--;
		break;

	case OPCODE_VAR_EQ:
		func_set_test_result(func_state,
		                     game->_variables[instr->operand[0]] ==
		                         game->_variables[instr->operand[1]]);
		break;

	case OPCODE_TURN_TICK:
		game->_variables[VAR_TURN_COUNT]++;
		break;

	case OPCODE_PRINT:
		console_println(game, instr_lookup_string(game,
		                                          instr->operand[0],
		                                          instr->operand[1]));
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
		                     game->_currentRoom != instr->operand[0]);
		break;

	case OPCODE_IN_ROOM:
		func_set_test_result(func_state,
		                     game->_currentRoom == instr->operand[0]);
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

		move_to(game, instr->operand[0]);
		break;

	case OPCODE_MOVE:
		/* Move in the direction dictated by the current verb */
		if (verb->_index - 1 >= NR_DIRECTIONS)
			fatal_error("Bad verb %d:%d in move",
			            verb->_index, verb->_type);

		if (room->direction[verb->_index - 1])
			move_to(game, room->direction[verb->_index - 1]);
		else
			console_println(game, string_lookup(game, STRING_CANT_GO));
		break;

	case OPCODE_MOVE_DIRECTION:
		if (room->direction[instr->operand[0] - 1])
			move_to(game, room->direction[instr->operand[0] - 1]);
		else
			console_println(game, string_lookup(game, STRING_CANT_GO));
		break;

	case OPCODE_ELSE:
		func_state->test_result = func_state->else_result;
		break;

	case OPCODE_MOVE_OBJECT_TO_CURRENT_ROOM:
		item = get_item(game, instr->operand[0] - 1);
		move_object(game, item, game->_currentRoom);
		break;

	case OPCODE_OBJECT_IN_ROOM:
		item = get_item(game, instr->operand[0] - 1);
		func_set_test_result(func_state,
		                     item->room == instr->operand[1]);
		break;

	case OPCODE_OBJECT_NOT_IN_ROOM:
		item = get_item(game, instr->operand[0] - 1);
		func_set_test_result(func_state,
		                     item->room != instr->operand[1]);
		break;

	case OPCODE_MOVE_OBJECT_TO_ROOM:
		item = get_item(game, instr->operand[0] - 1);
		move_object(game, item, instr->operand[1]);
		break;

	case OPCODE_INVENTORY_FULL:
		item = get_item_by_noun(game, noun);
		func_set_test_result(func_state,
		                     game->_variables[VAR_INVENTORY_WEIGHT] +
		                             (item->flags & ITEMF_WEIGHT_MASK) >
		                         game->_variables[VAR_INVENTORY_LIMIT]);
		break;

	case OPCODE_DESCRIBE_CURRENT_OBJECT:
		/*
		 * This opcode is only used in version 2
		 * FIXME - unsure what the single operand is for.
		 */
		item = get_item_by_noun(game, noun);
		printf("%s\n", string_lookup(game, item->long_string));
		break;

	case OPCODE_CURRENT_OBJECT_IN_ROOM:
		/* FIXME - use common code for these two ops */
		test = false;

		if (noun) {
			for (i = 0; i < game->_header.nr_items; i++) {
				Item *itemP = &game->_items[i];

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
		item = get_item_by_noun(game, noun);
		if (item)
			func_set_test_result(func_state,
			                     item->room != game->_currentRoom);
		else
			func_set_test_result(func_state, true);
		break;

	case OPCODE_CURRENT_OBJECT_PRESENT:
		item = get_item_by_noun(game, noun);
		if (item)
			func_set_test_result(func_state,
			                     item->room == game->_currentRoom);
		else
			func_set_test_result(func_state, false);
		break;

	case OPCODE_HAVE_OBJECT:
		item = get_item(game, instr->operand[0] - 1);
		func_set_test_result(func_state,
		                     item->room == ROOM_INVENTORY);
		break;

	case OPCODE_NOT_HAVE_CURRENT_OBJECT:
		item = get_item_by_noun(game, noun);
		func_set_test_result(func_state,
		                     !item || item->room != ROOM_INVENTORY);
		break;

	case OPCODE_HAVE_CURRENT_OBJECT:
		item = get_item_by_noun(game, noun);
		func_set_test_result(func_state,
		                     item->room == ROOM_INVENTORY);
		break;

	case OPCODE_NOT_HAVE_OBJECT:
		item = get_item(game, instr->operand[0] - 1);
		func_set_test_result(func_state,
		                     item->room != ROOM_INVENTORY);
		break;

	case OPCODE_CURRENT_OBJECT_TAKEABLE:
		item = get_item_by_noun(game, noun);
		if (!item)
			func_set_test_result(func_state, false);
		else
			func_set_test_result(func_state,
			                     (item->flags & ITEMF_CAN_TAKE));
		break;

	case OPCODE_CURRENT_OBJECT_NOT_TAKEABLE:
		item = get_item_by_noun(game, noun);
		if (!item)
			func_set_test_result(func_state, true);
		else
			func_set_test_result(func_state,
			                     !(item->flags & ITEMF_CAN_TAKE));
		break;

	case OPCODE_CURRENT_OBJECT_IS_NOWHERE:
		item = get_item_by_noun(game, noun);
		if (!item)
			func_set_test_result(func_state, false);
		else
			func_set_test_result(func_state,
			                     item->room == ROOM_NOWHERE);
		break;

	case OPCODE_OBJECT_IS_NOWHERE:
		item = get_item(game, instr->operand[0] - 1);
		func_set_test_result(func_state,
		                     item->room == ROOM_NOWHERE);
		break;

	case OPCODE_OBJECT_IS_NOT_NOWHERE:
		item = get_item(game, instr->operand[0] - 1);
		func_set_test_result(func_state,
		                     item->room != ROOM_NOWHERE);
		break;

	case OPCODE_OBJECT_NOT_PRESENT:
		item = get_item(game, instr->operand[0] - 1);
		func_set_test_result(func_state,
		                     item->room != game->_currentRoom);
		break;

	case OPCODE_OBJECT_PRESENT:
		item = get_item(game, instr->operand[0] - 1);
		func_set_test_result(func_state,
		                     item->room == game->_currentRoom);
		break;

	case OPCODE_OBJECT_NOT_VALID:
		/* FIXME - should be called OPCODE_CURRENT_OBJECT_NOT_VALID */
		func_set_test_result(func_state, !noun ||
		                                     (noun->_type & WORD_TYPE_NOUN_MASK) == 0);
		break;

	case OPCODE_CURRENT_IS_OBJECT:
		func_set_test_result(func_state,
		                     get_item_by_noun(game, noun) != NULL);
		break;

	case OPCODE_CURRENT_NOT_OBJECT:
		func_set_test_result(func_state,
		                     get_item_by_noun(game, noun) == NULL);
		break;

	case OPCODE_REMOVE_OBJECT:
		item = get_item(game, instr->operand[0] - 1);
		move_object(game, item, ROOM_NOWHERE);
		break;

	case OPCODE_REMOVE_CURRENT_OBJECT:
		item = get_item_by_noun(game, noun);
		move_object(game, item, ROOM_NOWHERE);
		break;

	case OPCODE_INVENTORY:
		count = num_objects_in_room(game, ROOM_INVENTORY);
		if (count == 0) {
			console_println(game, string_lookup(game, STRING_INVENTORY_EMPTY));
			break;
		}

		console_println(game, string_lookup(game, STRING_INVENTORY));
		for (i = 0; i < game->_header.nr_items; i++) {
			item = &game->_items[i];
			if (item->room == ROOM_INVENTORY)
				printf("%s\n",
				       string_lookup(game, item->string_desc));
		}
		break;

	case OPCODE_INVENTORY_ROOM:
		count = num_objects_in_room(game, instr->operand[0]);
		if (count == 0) {
			console_println(game, string_lookup(game, instr->operand[1] + 1));
			break;
		}

		console_println(game, string_lookup(game, instr->operand[1]));
		for (i = 0; i < game->_header.nr_items; i++) {
			item = &game->_items[i];
			if (item->room == instr->operand[0])
				printf("%s\n",
				       string_lookup(game, item->string_desc));
		}
		break;

	case OPCODE_MOVE_CURRENT_OBJECT_TO_ROOM:
		item = get_item_by_noun(game, noun);
		if (!item)
			fatal_error("Bad current object\n");

		move_object(game, item, instr->operand[0]);
		break;

	case OPCODE_DROP_OBJECT:
		item = get_item(game, instr->operand[0] - 1);
		move_object(game, item, game->_currentRoom);
		break;

	case OPCODE_DROP_CURRENT_OBJECT:
		item = get_item_by_noun(game, noun);
		if (!item)
			fatal_error("Attempt to take object failed\n");

		move_object(game, item, game->_currentRoom);
		break;

	case OPCODE_TAKE_CURRENT_OBJECT:
		item = get_item_by_noun(game, noun);
		if (!item)
			fatal_error("Attempt to take object failed\n");

		move_object(game, item, ROOM_INVENTORY);
		break;

	case OPCODE_TAKE_OBJECT:
		item = get_item(game, instr->operand[0] - 1);
		move_object(game, item, ROOM_INVENTORY);
		break;

	case OPCODE_TEST_FLAG:
		func_set_test_result(func_state,
		                     game->_flags[instr->operand[0]]);
		break;

	case OPCODE_TEST_NOT_FLAG:
		func_set_test_result(func_state,
		                     !game->_flags[instr->operand[0]]);
		break;

	case OPCODE_CLEAR_FLAG:
		game->_flags[instr->operand[0]] = false;
		break;

	case OPCODE_SET_FLAG:
		game->_flags[instr->operand[0]] = true;
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
		item = get_item(game, instr->operand[0] - 1);
		item->string_desc = (instr->operand[2] << 8) | instr->operand[1];
		break;

	case OPCODE_SET_OBJECT_LONG_DESCRIPTION:
		item = get_item(game, instr->operand[0] - 1);
		item->long_string = (instr->operand[2] << 8) | instr->operand[1];
		break;

	case OPCODE_SET_ROOM_DESCRIPTION:
		room = get_room(game, instr->operand[0]);
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
			fatal_error("Bad string desc %.2x:%.2x\n",
			            instr->operand[1], instr->operand[2]);
			break;
		}
		break;

	case OPCODE_SET_OBJECT_GRAPHIC:
		item = get_item(game, instr->operand[0] - 1);
		item->graphic = instr->operand[1];
		if (item->room == game->_currentRoom)
			game->_updateFlags |= UPDATE_GRAPHICS;
		break;

	case OPCODE_SET_ROOM_GRAPHIC:
		room = get_room(game, instr->operand[0]);
		room->graphic = instr->operand[1];
		if (instr->operand[0] == game->_currentRoom)
			game->_updateFlags |= UPDATE_GRAPHICS;
		break;

	case OPCODE_CALL_FUNC:
		index = instr->operand[0];
		if (instr->operand[1] == 0x81)
			index += 256;
		if (index >= game->_nr_functions)
			fatal_error("Bad function %.4x >= %.4x\n",
			            index, (uint)game->_nr_functions);

		debug_printf(DEBUG_FUNCTIONS,
		             "Calling subfunction %.4x\n", index);
		eval_function(game, &game->_functions[index], verb, noun);
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
		game->_currentReplaceWord = instr->operand[0] - 1;
		break;

	case OPCODE_SET_CURRENT_NOUN_STRING_REPLACEMENT:
		/*
		 * FIXME - Not sure what the operand is for,
		 * maybe capitalisation?
		 */
		if (noun && (noun->_type & WORD_TYPE_NOUN_PLURAL))
			game->_currentReplaceWord = 3;
		else if (noun && (noun->_type & WORD_TYPE_FEMALE))
			game->_currentReplaceWord = 0;
		else if (noun && (noun->_type & WORD_TYPE_MALE))
			game->_currentReplaceWord = 1;
		else
			game->_currentReplaceWord = 2;
		break;

	case OPCODE_DRAW_ROOM:
		draw_location_image(&game->_roomImages,
		                    instr->operand[0] - 1);
		break;

	case OPCODE_DRAW_OBJECT:
		draw_image(&game->_itemImages, instr->operand[0] - 1);
		break;

	case OPCODE_WAIT_KEY:
		console_get_key();
		break;

	case OPCODE_SPECIAL:
		/* Game specific opcode */
		game->handle_special_opcode(instr->operand[0]);
		break;

	default:
		if (instr->opcode & 0x80) {
			debug_printf(DEBUG_FUNCTIONS,
			             "Unhandled command opcode %.2x\n",
			             instr->opcode);
		} else {
			debug_printf(DEBUG_FUNCTIONS,
			             "Unhandled test opcode %.2x - returning false\n",
			             instr->opcode);
			func_set_test_result(func_state, false);
		}
		break;
	}
}

/*
 * Comprehend functions consist of test and command instructions (if the MSB
 * of the opcode is set then it is a command). Functions are parsed by
 * evaluating each test until a command instruction is encountered. If the
 * overall result of the tests was true then the command instructions are
 * executed until either a test instruction is found or the end of the function
 * is reached. Otherwise the commands instructions are skipped over and the
 * next test sequence (if there is one) is tried.
 */
void eval_function(ComprehendGame *game, Function *func,
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

		eval_instruction(game, &func_state, &func->instructions[i],
		                 verb, noun);
	}
}

static void skip_whitespace(char **p) {
	while (**p && Common::isSpace(**p))
		(*p)++;
}

static void skip_non_whitespace(char **p) {
	while (**p && !Common::isSpace(**p) && **p != ',' && **p != '\n')
		(*p)++;
}

static bool handle_sentence(ComprehendGame *game,
                            Sentence *sentence) {
	Function *func;
	Action *action;
	uint i, j;

	if (sentence->nr_words == 0)
		return false;

	/* Find a matching action */
	for (i = 0; i < game->_nr_actions; i++) {
		action = &game->_actions[i];

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
			func = &game->_functions[action->function];
			eval_function(game, func,
			              &sentence->words[0], &sentence->words[1]);
			return true;
		}
	}

	/* No matching action */
	console_println(game, string_lookup(game, STRING_DONT_UNDERSTAND));
	return false;
}

static void read_sentence(ComprehendGame *game, char **line,
                          Sentence *sentence) {
	bool sentence_end = false;
	char *word_string, *p = *line;
	WordIndex *pair;
	Word *word;
	int index;

	memset(sentence, 0, sizeof(*sentence));
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
		word = dict_find_word_by_string(game, word_string);
		if (!word)
			memset(&sentence->words[sentence->nr_words], 0,
			       sizeof(sentence->words[sentence->nr_words]));
		else
			memcpy(&sentence->words[sentence->nr_words],
			       word, sizeof(*word));

		sentence->nr_words++;

		if (sentence->nr_words > 1) {
			index = sentence->nr_words;

			/* See if this word and the previous are a word pair */
			pair = is_word_pair(game,
			                    &sentence->words[index - 2],
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

static void before_turn(ComprehendGame *game) {
	// Run the game specific before turn bits
	game->before_turn();

	// Run the each turn functions
	eval_function(game, &game->_functions[0], NULL, NULL);

	update(game);
}

static void after_turn(ComprehendGame *game) {
	// Do post turn game specific bits
	game->after_turn();
}

static void read_input(ComprehendGame *game) {
	Sentence sentence;
	char *line = NULL, buffer[1024];
	bool handled;

	game->before_prompt();
	before_turn(game);

	do {
		g_comprehend->print("> ");
		g_comprehend->readLine(buffer, sizeof(buffer));
		if (g_comprehend->shouldQuit())
			return;
	} while (strlen(buffer) == 0);

	// Re-comprehend special commands start with '!'
	line = &buffer[0];

	while (1) {
		read_sentence(game, &line, &sentence);
		handled = handle_sentence(game, &sentence);
		if (handled)
			after_turn(game);

		/* FIXME - handle the 'before you can continue' case */
		if (*line == '\0')
			break;
		line++;

		if (handled)
			before_turn(game);
	}
}

void comprehend_play_game(ComprehendGame *game) {
	console_init();

	game->before_game();

	game->_updateFlags = (uint)UPDATE_ALL;
	while (!g_comprehend->shouldQuit())
		read_input(game);
}

} // namespace Comprehend
} // namespace Glk
