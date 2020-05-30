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

#include "glk/comprehend/comprehend.h"
#include "glk/comprehend/game_data.h"
#include "glk/comprehend/dump_game_data.h"
#include "glk/comprehend/dictionary.h"
#include "glk/comprehend/file_buf.h"
#include "glk/comprehend/strings.h"
#include "glk/comprehend/game.h"
#include "glk/comprehend/util.h"
#include "glk/comprehend/opcode_map.h"

namespace Glk {
namespace Comprehend {

static const char *opcode_names[] = {
#ifdef TODO
	[OPCODE_UNKNOWN]			= "unknown",

	[OPCODE_HAVE_OBJECT]			= "have_object",
	[OPCODE_NOT_HAVE_OBJECT]		= "not_have_object",
	[OPCODE_HAVE_CURRENT_OBJECT]		= "have_current_object",
	[OPCODE_NOT_HAVE_CURRENT_OBJECT]	= "not_have_current_object",

	[OPCODE_OBJECT_IS_NOT_NOWHERE]		= "object_is_not_nowhere",

	[OPCODE_CURRENT_OBJECT_TAKEABLE]	= "current_object_takeable",
	[OPCODE_CURRENT_OBJECT_NOT_TAKEABLE]	= "current_object_not_takeable",

	[OPCODE_CURRENT_OBJECT_IS_NOWHERE]	= "current_object_is_nowhere",

	[OPCODE_CURRENT_OBJECT_NOT_PRESENT]	= "current_object_not_present",

	[OPCODE_TAKE_OBJECT]			= "take_object",
	[OPCODE_TAKE_CURRENT_OBJECT]		= "take_current_object",
	[OPCODE_DROP_OBJECT]			= "drop_object",
	[OPCODE_DROP_CURRENT_OBJECT]		= "drop_current_object",

	[OPCODE_OR]				= "or",
	[OPCODE_IN_ROOM]			= "in_room",
	[OPCODE_VAR_EQ]				= "var_eq",
	[OPCODE_OBJECT_NOT_VALID]	        = "object_not_valid",
	[OPCODE_INVENTORY_FULL]			= "inventory_full",
	[OPCODE_OBJECT_PRESENT]			= "object_present",
	[OPCODE_ELSE]				= "else",
	[OPCODE_OBJECT_IN_ROOM]			= "object_in_room",
	[OPCODE_TEST_FLAG]			= "test_flag",
	[OPCODE_CURRENT_OBJECT_IN_ROOM]		= "current_object_in_room",
	[OPCODE_CURRENT_OBJECT_PRESENT]		= "current_object_present",
	[OPCODE_TEST_ROOM_FLAG]			= "test_room_flag",
	[OPCODE_NOT_IN_ROOM]			= "not_in_room",
	[OPCODE_OBJECT_NOT_PRESENT]		= "object_not_present",
	[OPCODE_OBJECT_NOT_IN_ROOM]		= "object_not_in_room",
	[OPCODE_TEST_NOT_FLAG]			= "test_not_flag",
	[OPCODE_OBJECT_IS_NOWHERE]		= "object_is_nowhere",
	[OPCODE_TEST_NOT_ROOM_FLAG]		= "test_not_room_flag",
	[OPCODE_INVENTORY]			= "inventory",
	[OPCODE_MOVE_OBJECT_TO_ROOM]		= "move_object_to_room",
	[OPCODE_SAVE_ACTION]			= "save_action",
	[OPCODE_MOVE_TO_ROOM]			= "move_to_room",
	[OPCODE_VAR_ADD]			= "var_add",
	[OPCODE_SET_ROOM_DESCRIPTION]		= "set_room_description",
	[OPCODE_MOVE_OBJECT_TO_CURRENT_ROOM]	= "move_object_to_current_room",
	[OPCODE_VAR_SUB]			= "var_sub",
	[OPCODE_SET_OBJECT_DESCRIPTION]		= "set_object_description",
	[OPCODE_SET_OBJECT_LONG_DESCRIPTION]	= "set_object_long_description",
	[OPCODE_MOVE]				= "move",
	[OPCODE_PRINT]				= "print",
	[OPCODE_REMOVE_OBJECT]			= "remove_object",
	[OPCODE_SET_FLAG]			= "set_flag",
	[OPCODE_CALL_FUNC]			= "call_func",
	[OPCODE_TURN_TICK]			= "turn_tick",
	[OPCODE_CLEAR_FLAG]			= "clear_flag",
	[OPCODE_INVENTORY_ROOM]			= "inventory_room",
	[OPCODE_SPECIAL]			= "special",
	[OPCODE_SET_ROOM_GRAPHIC]		= "set_room_graphic",
	[OPCODE_SET_OBJECT_GRAPHIC]		= "set_object_graphic",
	[OPCODE_REMOVE_CURRENT_OBJECT]		= "remove_current_object",
	[OPCODE_DO_VERB]			= "do_verb",
	[OPCODE_VAR_INC]			= "var_inc",
	[OPCODE_VAR_DEC]			= "var_dec",
	[OPCODE_MOVE_CURRENT_OBJECT_TO_ROOM]	= "move_current_object_to_room",
	[OPCODE_DESCRIBE_CURRENT_OBJECT]	= "describe_current_object",
	[OPCODE_SET_STRING_REPLACEMENT]		= "set_string_replacement",
	[OPCODE_SET_CURRENT_NOUN_STRING_REPLACEMENT] = "set_current_noun_string_replacement",
	[OPCODE_CURRENT_NOT_OBJECT]		= "current_not_object",
	[OPCODE_CURRENT_IS_OBJECT]		= "current_is_object",
	[OPCODE_DRAW_ROOM]			= "draw_room",
	[OPCODE_DRAW_OBJECT]			= "draw_object",
	[OPCODE_WAIT_KEY]			= "wait_key",
#else
	"TODO"
#endif
};

void dump_instruction(ComprehendGame *game,
		      function_state *func_state,
		      instruction *instr)
{
	uint i;
	int str_index, str_table;
	uint8 *opcode_map, opcode;

	if (func_state)
		debugN("[or=%d,and=%d,test=%d,else=%d]",
		       func_state->or_count, func_state->_and,
		       func_state->test_result, func_state->else_result);

	opcode_map = get_opcode_map(game);
	opcode = opcode_map[instr->opcode];

	debugN("  [%.2x] ", instr->opcode);
	if (opcode < ARRAY_SIZE(opcode_names) && opcode_names[opcode])
		debugN("%s", opcode_names[opcode]);
	else
		debugN("unknown");

	if (instr->nr_operands) {
		debugN("(");
		for (i = 0; i < instr->nr_operands; i++)
			debugN("%.2x%s", instr->operand[i],
			       i == instr->nr_operands - 1 ? ")" : ", ");
	}

	switch (opcode) {
	case OPCODE_PRINT:
	case OPCODE_SET_ROOM_DESCRIPTION:
	case OPCODE_SET_OBJECT_DESCRIPTION:
	case OPCODE_SET_OBJECT_LONG_DESCRIPTION:

		if (opcode == OPCODE_PRINT) {
			str_index = instr->operand[0];
			str_table = instr->operand[1];
		} else {
			str_index = instr->operand[1];
			str_table = instr->operand[2];
		}

		debugN(" %s", instr_lookup_string(game, str_index, str_table));
		break;

	case OPCODE_SET_STRING_REPLACEMENT:
		debugN(" %s", game->_replaceWords[instr->operand[0] - 1]);
		break;
	}

	debugN("\n");
}

static void dump_functions(ComprehendGame *game)
{
	function *func;
	uint i, j;

	debugN("Functions (%zd entries)\n", game->_nr_functions);
	for (i = 0; i < game->_nr_functions; i++) {
		func = &game->_functions[i];

		debugN("[%.4x] (%zd instructions)\n", i, func->nr_instructions);
		for (j = 0; j < func->nr_instructions; j++)
			dump_instruction(game, NULL, &func->instructions[j]);
		debugN("\n");
	}
}

static void dump_action_table(ComprehendGame *game)
{
	action *action;
	word *word;
	uint i, j;

	debugN("Action table (%zd entries)\n", game->_nr_actions);
	for (i = 0; i < game->_nr_actions; i++) {
		action = &game->_actions[i];

		debugN("(");
		for (j = 0; j < 4; j++) {
			if (j < action->nr_words) {
				switch (action->word_type[j]) {
				case WORD_TYPE_VERB: debugN("v"); break;
				case WORD_TYPE_JOIN: debugN("j"); break;
				case WORD_TYPE_NOUN_MASK: debugN("n"); break;
				default: debugN("?"); break;
				}
			} else {
				debugN(" ");
			}
		}

		debugN(") [%.4x] ", i );

		for (j = 0; j < action->nr_words; j++)
			debugN("%.2x:%.2x ",
			       action->word[j], action->word_type[j]);

		debugN("| ");

		for (j = 0; j < action->nr_words; j++) {
			word = find_dict_word_by_index(game, action->word[j],
						       action->word_type[j]);
			if (word)
				debugN("%-6s ", word->_word);
			else
				debugN("%.2x:%.2x  ", action->word[j],
				       action->word_type[j]);
		}

		debugN("-> %.4x\n", action->function);
	}
}

static int word_index_compare(const void *a, const void *b)
{
	const word *word_a = (const word *)a, *word_b = (const word *)b;

	if (word_a->_index > word_b->_index)
		return 1;
	if (word_a->_index < word_b->_index)
		return -1;
	return 0;
}

static void dump_dictionary(ComprehendGame *game)
{
	word *dictionary;
	word *words;
	uint i;

	/* Sort the dictionary by index */
	dictionary = (word *)xmalloc(sizeof(*words) * game->_nr_words);
	memcpy(dictionary, game->_words,
	       sizeof(*words) * game->_nr_words);
	qsort(dictionary, game->_nr_words, sizeof(*words),
	      word_index_compare);

	debugN("Dictionary (%zd words)\n", game->_nr_words);
	for (i = 0; i < game->_nr_words; i++) {
		words = &dictionary[i];
		debugN("  [%.2x] %.2x %s\n", words->_index, words->_type,
		       words->_word);
	}

	free(dictionary);
}

static void dump_word_map(ComprehendGame *game)
{
	word *word[3];
	char str[3][6];
	word_map *map;
	uint i, j;

	debugN("Word pairs (%zd entries)\n", game->_nr_word_maps);
	for (i = 0; i < game->_nr_word_maps; i++) {
		map = &game->_wordMaps[i];

		for (j = 0; j < 3; j++) {
			word[j] = dict_find_word_by_index_type(
				game, map->word[j].index, map->word[j].type);
			if (word[j])
				snprintf(str[j], sizeof(str[j]),
				         "%s", word[j]->_word);
			else
				snprintf(str[j], sizeof(str[j]), "%.2x:%.2x ",
					 map->word[j].index, map->word[j].type);
		}

		debugN("  [%.2x] %-6s %-6s -> %-6s\n",
		       i, str[0], str[1], str[2]);
	}
}

static void dump_rooms(ComprehendGame *game)
{
	room *room;
	uint i;

	/* Room zero acts as the players inventory */
	debugN("Rooms (%zd entries)\n", game->_nr_rooms);
	for (i = 1; i <= game->_nr_rooms; i++) {
		room = &game->_rooms[i];

		debugN("  [%.2x] flags=%.2x, graphic=%.2x\n",
		       i, room->flags, room->graphic);
		debugN("    %s\n", string_lookup(game, room->string_desc));
		debugN("    n: %.2x  s: %.2x  e: %.2x  w: %.2x\n",
		       room->direction[DIRECTION_NORTH],
		       room->direction[DIRECTION_SOUTH],
		       room->direction[DIRECTION_EAST],
		       room->direction[DIRECTION_WEST]);
		debugN("    u: %.2x  d: %.2x  i: %.2x  o: %.2x\n",
		       room->direction[DIRECTION_UP],
		       room->direction[DIRECTION_DOWN],
		       room->direction[DIRECTION_IN],
		       room->direction[DIRECTION_OUT]);
		debugN("\n");
	}
}

static void dump_items(ComprehendGame *game)
{
	item *item;
	uint i, j;

	debugN("Items (%zd entries)\n", game->_header.nr_items);
	for (i = 0; i < game->_header.nr_items; i++) {
		item = &game->_items[i];

		debugN("  [%.2x] %s\n", i + 1,
		       item->string_desc ?
		       string_lookup(game, item->string_desc) : "");
		if (game->_comprehendVersion == 2)
			debugN("    long desc: %s\n",
			       string_lookup(game, item->long_string));

		debugN("    words: ");
		for (j = 0; j < game->_nr_words; j++)
			if (game->_words[j]._index == item->word &&
			    (game->_words[j]._type & WORD_TYPE_NOUN_MASK))
				debugN("%s ", game->_words[j]._word);
		debugN("\n");
		debugN("    flags=%.2x (takeable=%d, weight=%d)\n",
		       item->flags, !!(item->flags & ITEMF_CAN_TAKE),
		       (item->flags & ITEMF_WEIGHT_MASK));
		debugN("    room=%.2x, graphic=%.2x\n",
		       item->room, item->graphic);
		debugN("\n");
	}
}

static void dump_string_table(string_table *table)
{
	uint i;

	for (i = 0; i < table->nr_strings; i++)
		debugN("[%.4x] %s\n", i, table->strings[i]);
}

static void dump_game_data_strings(ComprehendGame *game)
{
	debugN("Main string table (%zd entries)\n",
	       game->_strings.nr_strings);
	dump_string_table(&game->_strings);
}

static void dump_extra_strings(ComprehendGame *game)
{
	debugN("Extra strings (%zd entries)\n",
	       game->_strings2.nr_strings);
	dump_string_table(&game->_strings2);
}

static void dump_replace_words(ComprehendGame *game)
{
	uint i;

	debugN("Replacement words (%zd entries)\n",
	       game->_nr_replace_words);
	for (i = 0; i < game->_nr_replace_words; i++)
		debugN("  [%.2x] %s\n", i + 1, game->_replaceWords[i]);
}

static void dump_header(ComprehendGame *game)
{
	game_header *header = &game->_header;
	uint16 *dir_table = header->room_direction_table;

	debugN("Game header:\n");
	debugN("  magic:                %.4x\n", header->magic);
	debugN("  action(vvnn):         %.4x\n", header->addr_actions_vvnn);
	debugN("  actions(?):\n");
	debugN("  actions(vnjn):        %.4x\n", header->addr_actions_vnjn);
	debugN("  actions(vjn):         %.4x\n", header->addr_actions_vjn);
	debugN("  actions(vdn):         %.4x\n", header->addr_actions_vdn);
	debugN("  actions(vnn):         %.4x\n", header->addr_actions_vnn);
	debugN("  actions(vn):          %.4x\n", header->addr_actions_vn);
	debugN("  actions(v):           %.4x\n", header->addr_actions_v);
	debugN("  functions:            %.4x\n", header->addr_vm);
	debugN("  dictionary:           %.4x\n", header->addr_dictionary);
	debugN("  word map pairs:       %.4x\n", header->addr_word_map);
	debugN("  room desc strings:    %.4x\n", header->room_desc_table);
	debugN("  room north:           %.4x\n", dir_table[DIRECTION_NORTH]);
	debugN("  room south:           %.4x\n", dir_table[DIRECTION_SOUTH]);
	debugN("  room east:            %.4x\n", dir_table[DIRECTION_EAST]);
	debugN("  room west:            %.4x\n", dir_table[DIRECTION_WEST]);
	debugN("  room up:              %.4x\n", dir_table[DIRECTION_UP]);
	debugN("  room down:            %.4x\n", dir_table[DIRECTION_DOWN]);
	debugN("  room in:              %.4x\n", dir_table[DIRECTION_IN]);
	debugN("  room out:             %.4x\n", dir_table[DIRECTION_OUT]);
	debugN("  room flags:           %.4x\n", header->room_flags_table);
	debugN("  room images:          %.4x\n", header->room_graphics_table);
	debugN("  item locations:       %.4x\n", header->addr_item_locations);
	debugN("  item flags:           %.4x\n", header->addr_item_flags);
	debugN("  item words:           %.4x\n", header->addr_item_word);
	debugN("  item desc strings:    %.4x\n", header->addr_item_strings);
	debugN("  item images:          %.4x\n", header->addr_item_graphics);
	debugN("  string table:         %.4x\n", header->addr_strings);
	debugN("  string table end:     %.4x\n", header->addr_strings_end);
}

typedef void (*dump_func_t)(ComprehendGame *game);

struct dumper {
	dump_func_t	dump_func;
	unsigned	flag;
};

static dumper dumpers[] = {
	{dump_header,			DUMP_HEADER},
	{dump_game_data_strings,	DUMP_STRINGS},
	{dump_extra_strings,		DUMP_EXTRA_STRINGS},
	{dump_rooms,			DUMP_ROOMS},
	{dump_items,			DUMP_ITEMS},
	{dump_dictionary,		DUMP_DICTIONARY},
	{dump_word_map,			DUMP_WORD_PAIRS},
	{dump_action_table,		DUMP_ACTIONS},
	{dump_functions,		DUMP_FUNCTIONS},
	{dump_replace_words,		DUMP_REPLACE_WORDS},
};

void dump_game_data(ComprehendGame *game, unsigned flags)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(dumpers); i++)
		if (flags & dumpers[i].flag) {
			dumpers[i].dump_func(game);
			debugN("\n\n");
		}
}

} // namespace Comprehend
} // namespace Glk
