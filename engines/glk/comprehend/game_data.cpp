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

#include "glk/comprehend/game_data.h"
#include "glk/comprehend/comprehend.h"
#include "glk/comprehend/dictionary.h"
#include "glk/comprehend/dump_game_data.h"
#include "glk/comprehend/file_buf.h"
#include "glk/comprehend/game.h"
#include "glk/comprehend/graphics.h"
#include "glk/comprehend/strings.h"
#include "glk/comprehend/util.h"

namespace Glk {
namespace Comprehend {

static char charset[] = "..abcdefghijklmnopqrstuvwxyz .";
static char special_charset[] = "[]\n!\"#$%&'(),-/0123456789:;?<>";

static uint16 magic_offset;

function_state::function_state() : test_result(true),
                                   else_result(false),
                                   or_count(0),
                                   _and(false),
                                   in_command(false),
                                   executed(false) {
}

/*-------------------------------------------------------*/

room::room() : flags(0),
               graphic(0),
               string_desc(0) {
	Common::fill(&direction[0], &direction[NR_DIRECTIONS], 0);
}

/*-------------------------------------------------------*/

item::item() : string_desc(0),
               long_string(0),
               room(0),
               flags(0),
               word(0),
               graphic(0) {
}

/*-------------------------------------------------------*/

word::word() : _index(0), _type(0) {
	Common::fill(&_word[0], &_word[7], '\0');
}

/*-------------------------------------------------------*/

action::action() : type(0), nr_words(0), function(0) {
	Common::fill(&word[0], &word[4], 0);
	Common::fill(&word_type[0], &word_type[4], 0);
}

instruction::instruction() : opcode(0), nr_operands(0),
		is_command(false) {
	Common::fill(&operand[0], &operand[3], 0);
}

/*-------------------------------------------------------*/

string_table::string_table() : nr_strings(0) {
	Common::fill(&strings[0], &strings[0xffff], nullptr);
}

/*-------------------------------------------------------*/

game_header::game_header() : magic(0),
	                             room_desc_table(0),
	                             room_flags_table(0),
	                             room_graphics_table(0),
	                             nr_items(0),
	                             addr_item_locations(0),
	                             addr_item_flags(0),
	                             addr_item_word(0),
	                             addr_item_strings(0),
	                             addr_item_graphics(0),
	                             addr_dictionary(0),
	                             addr_word_map(0),
	                             addr_strings(0),
	                             addr_strings_end(0),
	                             addr_actions_vvnn(0),
	                             addr_actions_unknown(0),
	                             addr_actions_vnjn(0),
	                             addr_actions_vjn(0),
	                             addr_actions_vdn(0),
	                             addr_actions_vnn(0),
	                             addr_actions_vn(0),
	                             addr_actions_v(0),
	                             addr_vm(0) // FIXME - functions
{
	Common::fill(&room_direction_table[0], &room_direction_table[NR_DIRECTIONS], 0);
}

/*-------------------------------------------------------*/

game_info::game_info() : comprehend_version(0),
	                        start_room(0),
	                        nr_rooms(0),
	                        current_room(0),
	                        words(nullptr),
	                        nr_words(0),
	                        nr_word_maps(0),
	                        nr_actions(0),
	                        nr_functions(0),
	                        nr_replace_words(0),
	                        current_replace_word(0),
	                        update_flags(0) {
}

static void parse_header_le16(struct file_buf * fb, uint16 * val) {
	file_buf_get_le16(fb, val);
	*val += (uint16)magic_offset;
}

static size_t opcode_nr_operands(uint8 opcode) {
	/* Number of operands is encoded in the low 2 bits */
	return opcode & 0x3;
}

static bool opcode_is_command(uint8 opcode) {
	/* If the MSB is set the instruction is a command */
	return opcode & 0x80;
}

static uint8 parse_vm_instruction(struct file_buf * fb,
	                                struct instruction * instr) {
	uint i;

	/* Get the opcode */
	file_buf_get_u8(fb, &instr->opcode);
	instr->nr_operands = opcode_nr_operands(instr->opcode);

	/* Get the operands */
	for (i = 0; i < instr->nr_operands; i++)
		file_buf_get_u8(fb, &instr->operand[i]);

	instr->is_command = opcode_is_command(instr->opcode);

	return instr->opcode;
}

static void parse_function(struct file_buf * fb, struct function * func) {
	struct instruction *instruction;
	uint8 *p, opcode;

	p = (uint8 *)memchr(file_buf_data_pointer(fb), 0x00,
		                fb->size - file_buf_get_pos(fb));
	if (!p)
		fatal_error("bad function @ %.4x", file_buf_get_pos(fb));

	while (1) {
		instruction = &func->instructions[func->nr_instructions];

		opcode = parse_vm_instruction(fb, instruction);
		if (opcode == 0)
			break;

		func->nr_instructions++;
		if (func->nr_instructions >= ARRAY_SIZE(func->instructions))
			fatal_error("Function has too many instructions");
	}
}

static void parse_vm(struct comprehend_game * game, struct file_buf * fb) {
	struct function *func;

	file_buf_set_pos(fb, game->info->header.addr_vm);
	while (1) {
		func = &game->info->functions[game->info->nr_functions];

		parse_function(fb, func);
		if (func->nr_instructions == 0)
			break;

		game->info->nr_functions++;
	}
}

static void parse_action_table_vvnn(struct comprehend_game * game,
	                                struct file_buf * fb, size_t * index) {
	struct action *action;
	uint8 verb, count;
	int i, j;

	/*
	* <verb> <verb> <noun> <noun>
	*
	* u8: verb1
	* u8: count
	*     u8:   verb2
	*     u8:   noun1
	*     u8:   noun2
	*     le16: action
	*/
	file_buf_set_pos(fb, game->info->header.addr_actions_vvnn);
	while (1) {
		file_buf_get_u8(fb, &verb);
		if (verb == 0)
			break;
		file_buf_get_u8(fb, &count);

		for (i = 0; i < count; i++) {
			action = &game->info->action[*index];
			action->type = ACTION_VERB_VERB_NOUN_NOUN;

			action->nr_words = 4;
			action->word_type[0] = WORD_TYPE_VERB;
			action->word_type[1] = WORD_TYPE_VERB;
			action->word_type[2] = WORD_TYPE_NOUN_MASK;
			action->word_type[3] = WORD_TYPE_NOUN_MASK;

			action->word[0] = verb;

			for (j = 0; j < 3; j++)
				file_buf_get_u8(fb, &action->word[j + 1]);
			file_buf_get_le16(fb, &action->function);

			(*index)++;
		}
	}
}

static void parse_action_table_vnjn(struct comprehend_game * game,
	                                struct file_buf * fb, size_t * index) {
	struct action *action;
	uint8 join, count;
	int i;

	/*
	* <verb> <noun> <join> <noun>
	*
	* u8: join
	* u8: count
	*     u8:   verb
	*     u8:   noun1
	*     u8:   noun2
	*     le16: action
	*/
	file_buf_set_pos(fb, game->info->header.addr_actions_vnjn);
	while (1) {
		file_buf_get_u8(fb, &join);
		if (join == 0)
			break;
		file_buf_get_u8(fb, &count);

		for (i = 0; i < count; i++) {
			action = &game->info->action[*index];
			action->type = ACTION_VERB_NOUN_JOIN_NOUN;

			action->nr_words = 4;
			action->word_type[0] = WORD_TYPE_VERB;
			action->word_type[1] = WORD_TYPE_NOUN_MASK;
			action->word_type[2] = WORD_TYPE_JOIN;
			action->word_type[3] = WORD_TYPE_NOUN_MASK;

			action->word[2] = join;

			file_buf_get_u8(fb, &action->word[0]);
			file_buf_get_u8(fb, &action->word[1]);
			file_buf_get_u8(fb, &action->word[3]);
			file_buf_get_le16(fb, &action->function);

			(*index)++;
		}
	}
}

static void parse_action_table_vjn(struct comprehend_game * game,
	                                struct file_buf * fb, size_t * index) {
	struct action *action;
	uint8 join, count;
	int i;

	/*
	* <verb> <join> <noun>
	*
	* u8: join
	* u8: count
	*     u8:   verb
	*     u8:   noun
	*     le16: action
	*/
	file_buf_set_pos(fb, game->info->header.addr_actions_vjn);
	while (1) {
		file_buf_get_u8(fb, &join);
		if (join == 0)
			break;
		file_buf_get_u8(fb, &count);

		for (i = 0; i < count; i++) {
			action = &game->info->action[*index];
			action->type = ACTION_VERB_JOIN_NOUN;
			action->word[1] = join;

			action->nr_words = 3;
			action->word_type[0] = WORD_TYPE_VERB;
			action->word_type[1] = WORD_TYPE_JOIN;
			action->word_type[2] = WORD_TYPE_NOUN_MASK;

			file_buf_get_u8(fb, &action->word[0]);
			file_buf_get_u8(fb, &action->word[2]);
			file_buf_get_le16(fb, &action->function);

			(*index)++;
		}
	}
}

static void parse_action_table_vdn(struct comprehend_game * game,
	                                struct file_buf * fb, size_t * index) {
	struct action *action;
	uint8 verb, count;
	int i;

	/*
	* <verb> <dir> <noun>
	*
	* u8: verb
	* u8: count
	*     u8:   dir
	*     u8:   noun
	*     le16: action
	*/
	file_buf_set_pos(fb, game->info->header.addr_actions_vdn);
	while (1) {
		file_buf_get_u8(fb, &verb);
		if (verb == 0)
			break;
		file_buf_get_u8(fb, &count);

		for (i = 0; i < count; i++) {
			action = &game->info->action[*index];
			action->type = ACTION_VERB_JOIN_NOUN;
			action->word[0] = verb;

			action->nr_words = 3;
			action->word_type[0] = WORD_TYPE_VERB;
			action->word_type[1] = WORD_TYPE_VERB;
			action->word_type[2] = WORD_TYPE_NOUN_MASK;

			file_buf_get_u8(fb, &action->word[1]);
			file_buf_get_u8(fb, &action->word[2]);
			file_buf_get_le16(fb, &action->function);

			(*index)++;
		}
	}
}

static void parse_action_table_vnn(struct comprehend_game * game,
	                                struct file_buf * fb, size_t * index) {
	struct action *action;
	uint8 verb, count;
	int i;

	/*
	* <verb> <noun> <noun>
	*
	* u8: verb
	* u8: count
	*     u8:   noun1
	*     u8:   noun2
	*     le16: action
	*/
	file_buf_set_pos(fb, game->info->header.addr_actions_vnn);
	while (1) {
		/* 2-byte header */
		file_buf_get_u8(fb, &verb);
		if (verb == 0)
			break;
		file_buf_get_u8(fb, &count);

		for (i = 0; i < count; i++) {
			action = &game->info->action[*index];
			action->type = ACTION_VERB_NOUN_NOUN;
			action->word[0] = verb;

			action->nr_words = 3;
			action->word_type[0] = WORD_TYPE_VERB;
			action->word_type[1] = WORD_TYPE_NOUN_MASK;
			action->word_type[2] = WORD_TYPE_NOUN_MASK;

			file_buf_get_u8(fb, &action->word[1]);
			file_buf_get_u8(fb, &action->word[2]);
			file_buf_get_le16(fb, &action->function);

			(*index)++;
		}
	}
}

static void parse_action_table_vn(struct comprehend_game * game,
	                                struct file_buf * fb, size_t * index) {
	struct action *action;
	uint8 verb, count;
	int i;

	/*
	* <verb> <noun>
	*
	* u8: verb
	* u8: count
	*     u8:   noun
	*     le16: action
	*/
	file_buf_set_pos(fb, game->info->header.addr_actions_vn);
	while (1) {
		/* 2-byte header */
		file_buf_get_u8(fb, &verb);
		if (verb == 0)
			break;
		file_buf_get_u8(fb, &count);

		for (i = 0; i < count; i++) {
			action = &game->info->action[*index];
			action->type = ACTION_VERB_NOUN;
			action->word[0] = verb;

			action->nr_words = 2;
			action->word_type[0] = WORD_TYPE_VERB;
			action->word_type[1] = WORD_TYPE_NOUN_MASK;

			file_buf_get_u8(fb, &action->word[1]);
			file_buf_get_le16(fb, &action->function);

			(*index)++;
		}
	}
}

static void parse_action_table_v(struct comprehend_game * game,
	                                struct file_buf * fb, size_t * index) {
	struct action *action;
	uint8 verb, nr_funcs;
	uint16 func;
	int i;

	/*
	* <verb> [<noun>]
	*
	* u8: verb
	* u8: count (num actions)
	*     le16: action
	*/
	file_buf_set_pos(fb, game->info->header.addr_actions_v);
	while (1) {
		file_buf_get_u8(fb, &verb);
		if (verb == 0)
			break;

		action = &game->info->action[*index];
		action->type = ACTION_VERB_OPT_NOUN;
		action->word[0] = verb;

		/* Can take an optional noun (nr_words here is maximum) */
		action->nr_words = 1;
		action->word_type[0] = WORD_TYPE_VERB;

		/*
		* Default actions can have more than one function, but only
		* the first one actually seems to be used?
		*/
		file_buf_get_u8(fb, &nr_funcs);
		for (i = 0; i < nr_funcs; i++) {
			file_buf_get_le16(fb, &func);
			if (i == 0)
				action->function = func;
		}

		(*index)++;
	}
}

static void parse_action_table(struct comprehend_game * game,
	                            struct file_buf * fb) {
	game->info->nr_actions = 0;

	if (game->info->comprehend_version == 1) {
		parse_action_table_vvnn(game, fb, &game->info->nr_actions);
		parse_action_table_vdn(game, fb, &game->info->nr_actions);
	}
	if (game->info->comprehend_version >= 2) {
		parse_action_table_vnn(game, fb, &game->info->nr_actions);
	}

	parse_action_table_vnjn(game, fb, &game->info->nr_actions);
	parse_action_table_vjn(game, fb, &game->info->nr_actions);
	parse_action_table_vn(game, fb, &game->info->nr_actions);
	parse_action_table_v(game, fb, &game->info->nr_actions);
}

static void parse_dictionary(struct comprehend_game * game, struct file_buf * fb) {
	word *words;
	uint i, j;

	// FIXME - fixed size 0xff array?
	game->info->words = (word *)xmalloc(game->info->nr_words * sizeof(words));

	file_buf_set_pos(fb, game->info->header.addr_dictionary);
	for (i = 0; i < game->info->nr_words; i++) {
		words = &game->info->words[i];

		file_buf_get_data(fb, words->_word, 6);

		/* Decode */
		for (j = 0; j < 6; j++)
			words->_word[j] ^= 0x8a;
		words->_word[6] = '\0';

		file_buf_get_u8(fb, &words->_index);
		file_buf_get_u8(fb, &words->_type);
	}
}

static void parse_word_map(struct comprehend_game * game, struct file_buf * fb) {
	struct word_map *map;
	uint8 index, type, dummy;
	uint i;

	game->info->nr_word_maps = 0;
	file_buf_set_pos(fb, game->info->header.addr_word_map);

	/*
	* Parse the word pair table. Each entry has a pair of dictionary
	* index/type values for a first and second word.
	*/
	while (1) {
		map = &game->info->word_map[game->info->nr_word_maps];

		file_buf_get_u8(fb, &index);
		file_buf_get_u8(fb, &type);
		if (type == 0 && index == 0) {
			/* End of pairs */
			break;
		}

		map->word[0].index = index;
		map->word[0].type = type;
		file_buf_get_u8(fb, &map->flags);
		file_buf_get_u8(fb, &map->word[1].index);
		file_buf_get_u8(fb, &map->word[1].type);

		game->info->nr_word_maps++;
	}

	/* Consume two more null bytes (type and index were also null) */
	file_buf_get_u8(fb, &dummy);
	file_buf_get_u8(fb, &dummy);

	/*
	* Parse the target word table. Each entry has a dictionary
	* index/type. The first and second words from above map to the
	* target word here. E.g. 'go north' -> 'north'.
	*/
	for (i = 0; i < game->info->nr_word_maps; i++) {
		map = &game->info->word_map[i];

		file_buf_get_u8(fb, &map->word[2].index);
		file_buf_get_u8(fb, &map->word[2].type);
	}
}

static void parse_items(struct comprehend_game * game, struct file_buf * fb) {
	size_t nr_items = game->info->header.nr_items;

	/* Item descriptions */
	file_buf_set_pos(fb, game->info->header.addr_item_strings);
	file_buf_get_array_le16(fb, 0, game->info->item, string_desc, nr_items);

	if (game->info->comprehend_version == 2) {
		/* Comprehend version 2 adds long string descriptions */
		file_buf_set_pos(fb, game->info->header.addr_item_strings +
			                        (game->info->header.nr_items * sizeof(uint16)));
		file_buf_get_array_le16(fb, 0, game->info->item, long_string, nr_items);
	}

	/* Item flags */
	file_buf_set_pos(fb, game->info->header.addr_item_flags);
	file_buf_get_array_u8(fb, 0, game->info->item, flags, nr_items);

	/* Item word */
	file_buf_set_pos(fb, game->info->header.addr_item_word);
	file_buf_get_array_u8(fb, 0, game->info->item, word, nr_items);

	/* Item locations */
	file_buf_set_pos(fb, game->info->header.addr_item_locations);
	file_buf_get_array_u8(fb, 0, game->info->item, room, nr_items);

	/* Item graphic */
	file_buf_set_pos(fb, game->info->header.addr_item_graphics);
	file_buf_get_array_u8(fb, 0, game->info->item, graphic, nr_items);
}

static void parse_rooms(struct comprehend_game * game, struct file_buf * fb) {
	size_t nr_rooms = game->info->nr_rooms;
	int i;

	/* Room exit directions */
	for (i = 0; i < NR_DIRECTIONS; i++) {
		file_buf_set_pos(fb, game->info->header.room_direction_table[i]);
		file_buf_get_array_u8(fb, 1, game->info->rooms,
			                    direction[i], nr_rooms);
	}

	/* Room string descriptions */
	file_buf_set_pos(fb, game->info->header.room_desc_table);
	file_buf_get_array_le16(fb, 1, game->info->rooms, string_desc, nr_rooms);

	/* Room flags */
	file_buf_set_pos(fb, game->info->header.room_flags_table);
	file_buf_get_array_u8(fb, 1, game->info->rooms, flags, nr_rooms);

	/* Room graphic */
	file_buf_set_pos(fb, game->info->header.room_graphics_table);
	file_buf_get_array_u8(fb, 1, game->info->rooms, graphic, nr_rooms);
}

static uint64 string_get_chunk(uint8 * string) {
	uint64 c, val = 0;
	int i;

	for (i = 0; i < 5; i++) {
		c = string[i] & 0xff;
		val |= (c << ((4 - i) * 8));
	}

	return val;
}

static char decode_string_elem(uint8 c, bool capital, bool special) {
	if (special) {
		if (c < sizeof(special_charset) - 1)
			return special_charset[c];
	} else {
		if (c < sizeof(charset) - 1) {
			c = charset[c];
			if (capital) {
				/*
				* A capital space means that the character
				* is dynamically replaced by at runtime.
				* We use the character '@' since it cannot
				* otherwise appear in strings.
				*/
				if (c == ' ')
					return '@';
				return c - 0x20;
			} else {
				return c;
			}
		}
	}

	/* Unknown character */
	printf("Unknown char %d, caps=%d, special=%d\n", c, capital, special);
	return '*';
}

/*
* Game strings are stored using 5-bit characters. By default a character
* value maps to the lower-case letter table. If a character has the value 0x1e
* then the next character is upper-case. An upper-case space is used to
* specify that the character should be replaced at runtime (like a '%s'
* specifier). If a character has the value 0x1f then the next character is
* taken from the symbols table.
*/
static char *parse_string(struct file_buf * fb) {
	bool capital_next = false, special_next = false;
	unsigned i, j, k = 0;
	uint64 chunk;
	uint8 elem, *encoded;
	char *string, c;
	size_t encoded_len;

	encoded_len = file_buf_strlen(fb, NULL);
	string = (char *)xmalloc(encoded_len * 2);

	/* Get the encoded string */
	encoded = (uint8 *)xmalloc(encoded_len + 5);
	memset(encoded, 0, encoded_len);
	file_buf_get_data(fb, encoded, encoded_len);

	/* Skip over the zero byte */
	if (file_buf_get_pos(fb) < fb->size)
		file_buf_get_u8(fb, NULL);

	for (i = 0; i < encoded_len; i += 5) {
		chunk = string_get_chunk(&encoded[i]);

		for (j = 0; j < 8; j++) {
			elem = (chunk >> (35 - (5 * j))) & 0x1f;

			if (elem == 0)
				goto done;
			if (elem == 0x1e) {
				capital_next = true;
			} else if (elem == 0x1f) {
				special_next = true;
			} else {
				c = decode_string_elem(elem, capital_next,
					                    special_next);
				special_next = false;
				capital_next = false;
				string[k++] = c;
			}
		}
	}

done:
	string[k] = '\0';
	free(encoded);

	return string;
}

static void parse_string_table(struct file_buf * fb, unsigned start_addr,
	                            uint32 end_addr, struct string_table *table) {
	file_buf_set_pos(fb, start_addr);
	while (1) {
		table->strings[table->nr_strings++] = parse_string(fb);
		if (file_buf_get_pos(fb) >= end_addr)
			break;
	}
}

static void parse_variables(struct comprehend_game * game, struct file_buf * fb) {
	int i;

	for (i = 0; i < ARRAY_SIZE(game->info->variable); i++)
		file_buf_get_le16(fb, &game->info->variable[i]);
}

static void parse_flags(struct comprehend_game * game, struct file_buf * fb) {
	int i, bit, flag_index = 0;
	uint8 bitmask;

	for (i = 0; i < ARRAY_SIZE(game->info->flags) / 8; i++) {
		file_buf_get_u8(fb, &bitmask);
		for (bit = 7; bit >= 0; bit--) {
			game->info->flags[flag_index] = !!(bitmask & (1 << bit));
			flag_index++;
		}
	}
}

static void parse_replace_words(struct comprehend_game * game,
	                            struct file_buf * fb) {
	uint16 dummy;
	size_t len;
	bool eof;
	int i;

	/* FIXME - Rename addr_strings_end */
	file_buf_set_pos(fb, game->info->header.addr_strings_end);

	/* FIXME - what is this for */
	file_buf_get_le16(fb, &dummy);

	for (i = 0;; i++) {
		len = file_buf_strlen(fb, &eof);
		if (len == 0)
			break;

		game->info->replace_words[i] = xstrndup((char *)fb->p, len);
		file_buf_get_data(fb, NULL, len + (eof ? 0 : 1));
		if (eof)
			break;
	}
	game->info->nr_replace_words = i;
}

/*
* The main game data file header has the offsets for where each bit of
* game data is. The offsets have a magic constant value added to them.
*/
static void parse_header(struct comprehend_game * game, struct file_buf * fb) {
	struct game_header *header = &game->info->header;
	uint16 dummy, addr_dictionary_end;
	uint8 dummy8;

	file_buf_set_pos(fb, 0);
	file_buf_get_le16(fb, &header->magic);
	switch (header->magic) {
	case 0x2000: /* Transylvania, Crimson Crown disk one */
	case 0x4800: /* Crimson Crown disk two */
		game->info->comprehend_version = 1;
		magic_offset = (uint16)(-0x5a00 + 0x4);
		break;

	case 0x93f0: /* OO-Topos */
		game->info->comprehend_version = 2;
		magic_offset = (uint16)-0x5a00;
		break;

	case 0xa429: /* Talisman */
		game->info->comprehend_version = 2;
		magic_offset = (uint16)-0x5a00;
		break;

	default:
		fatal_error("Unknown game_data magic %.4x\n", header->magic);
		break;
	}

	/* FIXME - Second word in header has unknown usage */
	parse_header_le16(fb, &dummy);

	/*
	* Action tables.
	*
	* Layout depends on the comprehend version.
	*/
	if (game->info->comprehend_version == 1) {
		parse_header_le16(fb, &header->addr_actions_vvnn);
		parse_header_le16(fb, &header->addr_actions_unknown);
		parse_header_le16(fb, &header->addr_actions_vnjn);
		parse_header_le16(fb, &header->addr_actions_vjn);
		parse_header_le16(fb, &header->addr_actions_vdn);
	}
	if (game->info->comprehend_version >= 2) {
		parse_header_le16(fb, &header->addr_actions_vnjn);
		parse_header_le16(fb, &header->addr_actions_vjn);
		parse_header_le16(fb, &header->addr_actions_vnn);
	}
	parse_header_le16(fb, &header->addr_actions_vn);
	parse_header_le16(fb, &header->addr_actions_v);

	parse_header_le16(fb, &header->addr_vm);
	parse_header_le16(fb, &header->addr_dictionary);

	parse_header_le16(fb, &header->addr_word_map);
	/* FIXME - what is this for? */
	parse_header_le16(fb, &dummy);
	addr_dictionary_end = header->addr_word_map;

	/* Rooms */
	parse_header_le16(fb, &header->room_desc_table);
	parse_header_le16(fb, &header->room_direction_table[DIRECTION_NORTH]);
	parse_header_le16(fb, &header->room_direction_table[DIRECTION_SOUTH]);
	parse_header_le16(fb, &header->room_direction_table[DIRECTION_EAST]);
	parse_header_le16(fb, &header->room_direction_table[DIRECTION_WEST]);
	parse_header_le16(fb, &header->room_direction_table[DIRECTION_UP]);
	parse_header_le16(fb, &header->room_direction_table[DIRECTION_DOWN]);
	parse_header_le16(fb, &header->room_direction_table[DIRECTION_IN]);
	parse_header_le16(fb, &header->room_direction_table[DIRECTION_OUT]);
	parse_header_le16(fb, &header->room_flags_table);
	parse_header_le16(fb, &header->room_graphics_table);

	/*
	* Objects.
	*
	* Layout is dependent on comprehend version.
	*/
	if (game->info->comprehend_version == 1) {
		parse_header_le16(fb, &header->addr_item_locations);
		parse_header_le16(fb, &header->addr_item_flags);
		parse_header_le16(fb, &header->addr_item_word);
		parse_header_le16(fb, &header->addr_item_strings);
		parse_header_le16(fb, &header->addr_item_graphics);

		header->nr_items = (header->addr_item_word -
			                header->addr_item_flags);

	} else {
		parse_header_le16(fb, &header->addr_item_strings);
		parse_header_le16(fb, &header->addr_item_word);
		parse_header_le16(fb, &header->addr_item_locations);
		parse_header_le16(fb, &header->addr_item_flags);
		parse_header_le16(fb, &header->addr_item_graphics);

		header->nr_items = (header->addr_item_flags -
			                header->addr_item_locations);
	}

	parse_header_le16(fb, &header->addr_strings);
	parse_header_le16(fb, &dummy);
	parse_header_le16(fb, &header->addr_strings_end);

	file_buf_get_u8(fb, &dummy8);
	file_buf_get_u8(fb, &game->info->start_room);
	file_buf_get_u8(fb, &dummy8);

	parse_variables(game, fb);
	parse_flags(game, fb);

	game->info->nr_rooms = header->room_direction_table[DIRECTION_SOUTH] -
		                    header->room_direction_table[DIRECTION_NORTH];

	game->info->nr_words = (addr_dictionary_end -
		                    header->addr_dictionary) /
		                    8;
}

static void load_extra_string_file(comprehend_game * game,
	                                string_file * string_file) {
	struct file_buf fb;
	unsigned end;

	file_buf_map(string_file->filename, &fb);

	if (string_file->end_offset)
		end = string_file->end_offset;
	else
		end = fb.size;

	parse_string_table(&fb, string_file->base_offset,
		                end, &game->info->strings2);

	file_buf_unmap(&fb);
}

static void load_extra_string_files(comprehend_game * game) {
	int i;

	memset(&game->info->strings2, 0, sizeof(game->info->strings2));

	for (i = 0; i < ARRAY_SIZE(game->string_files); i++) {
		if (!game->string_files[i].filename)
			break;

		// HACK - get string offsets correct
		game->info->strings2.nr_strings = 0x40 * i;
		if (game->info->strings2.nr_strings == 0)
			game->info->strings2.nr_strings++;

		load_extra_string_file(game, &game->string_files[i]);
	}
}

static void load_game_data(struct comprehend_game * game) {
	struct file_buf fb;

	memset(game->info, 0, sizeof(*game->info));
	file_buf_map(game->game_data_file, &fb);

	parse_header(game, &fb);
	parse_rooms(game, &fb);
	parse_items(game, &fb);
	parse_dictionary(game, &fb);
	parse_word_map(game, &fb);
	memset(&game->info->strings, 0, sizeof(game->info->strings));
	parse_string_table(&fb, game->info->header.addr_strings,
		                game->info->header.addr_strings_end,
		                &game->info->strings);
	load_extra_string_files(game);
	parse_vm(game, &fb);
	parse_action_table(game, &fb);
	parse_replace_words(game, &fb);

	file_buf_unmap(&fb);
}

void comprehend_load_game(struct comprehend_game * game) {
	/* Load the main game data file */
	load_game_data(game);

	if (g_enabled()) {
		comprehend_load_images(game);
		if (game->color_table)
			g_set_color_table(game->color_table);
	}

	/* FIXME - This can be merged, don't need to keep start room around */
	game->info->current_room = game->info->start_room;
}

#ifdef TODO
static void patch_string_desc(uint16 * desc) {
	/*
	* String descriptors in the save file sometimes are encoded as a
	* table/index value like the instruction opcodes used, and other
	* times the are encoded as an absolute index. We fix them up to
	* all be the former type.
	*/
	if (!(*desc & 0x8000) && *desc >= 0x100) {
		*desc -= 0x100;
		*desc |= 0x8100;
	}
}
#endif

void comprehend_save_game(struct comprehend_game * game, const char *filename) {
#ifdef TODO
	FILE *fd;
	uint8 bitmask;
	int dir, bit, flag_index, i;
	size_t nr_rooms, nr_items;

	fd = fopen(filename, "w");
	if (!fd) {
		printf("Error: Failed to open save file '%s': %s\n",
			    filename, strerror(errno));
		return;
	}

	nr_rooms = game->info->nr_rooms;
	nr_items = game->info->header.nr_items;

	file_buf_put_u8(fd, 0);
	file_buf_put_u8(fd, game->info->current_room);
	file_buf_put_u8(fd, 0);

	/* Variables */
	for (i = 0; i < ARRAY_SIZE(game->info->variable); i++)
		file_buf_put_le16(fd, game->info->variable[i]);

	/* Flags */
	for (flag_index = 0, i = 0; i < ARRAY_SIZE(game->info->flags) / 8; i++) {
		bitmask = 0;
		for (bit = 7; bit >= 0; bit--) {
			bitmask |= (!!game->info->flags[flag_index]) << bit;
			flag_index++;
		}

		file_buf_put_u8(fd, bitmask);
	}

	/*
	* Re-Comprehend doesn't need this since the number of items is
	* determined by the currently loaded game, but the original games
	* won't load the file properly without it.
	*/
	file_buf_put_skip(fd, 0x12c - ftell(fd));
	file_buf_put_u8(fd, nr_items);

	if (game->info->comprehend_version == 1)
		file_buf_put_skip(fd, 0x230 - ftell(fd));
	else
		file_buf_put_skip(fd, 0x130 - ftell(fd));

	/* Rooms */
	file_buf_put_array_le16(fd, 1, game->info->rooms,
		                    string_desc, nr_rooms);
	for (dir = 0; dir < NR_DIRECTIONS; dir++)
		file_buf_put_array_u8(fd, 1, game->info->rooms,
			                    direction[dir], nr_rooms);
	file_buf_put_array_u8(fd, 1, game->info->rooms, flags, nr_rooms);
	file_buf_put_array_u8(fd, 1, game->info->rooms, graphic, nr_rooms);

	/*
	* Objects
	*
	* Layout differs depending on Comprehend version. Version 2 also
	* has long string descriptions for each object.
	*/
	file_buf_put_array_le16(fd, 0, game->info->item, string_desc, nr_items);
	if (game->info->comprehend_version == 1) {
		file_buf_put_array_u8(fd, 0, game->info->item, room, nr_items);
		file_buf_put_array_u8(fd, 0, game->info->item, flags, nr_items);
		file_buf_put_array_u8(fd, 0, game->info->item, word, nr_items);
		file_buf_put_array_u8(fd, 0, game->info->item, graphic, nr_items);
	} else {
		file_buf_put_array_le16(fd, 0, game->info->item, long_string, nr_items);
		file_buf_put_array_u8(fd, 0, game->info->item, word, nr_items);
		file_buf_put_array_u8(fd, 0, game->info->item, room, nr_items);
		file_buf_put_array_u8(fd, 0, game->info->item, flags, nr_items);
		file_buf_put_array_u8(fd, 0, game->info->item, graphic, nr_items);
	}

	fclose(fd);
#else
	error("Save");
#endif
}

void comprehend_restore_game(struct comprehend_game * game, const char *filename) {
#ifdef TODO
	struct file_buf fb;
	size_t nr_rooms, nr_items;
	uint err, dir, i;

	err = file_buf_map_may_fail(filename, &fb);
	if (err) {
		printf("Error: Failed to open save file '%s': %s\n",
			    filename, strerror(-err));
		return;
	}

	nr_rooms = game->info->nr_rooms;
	nr_items = game->info->header.nr_items;

	/* Restore starting room */
	file_buf_set_pos(&fb, 1);
	file_buf_get_u8(&fb, &game->info->current_room);

	/* Restore flags and variables */
	file_buf_set_pos(&fb, 3);
	parse_variables(game, &fb);
	parse_flags(game, &fb);

	/* FIXME - unknown restore data, skip over it */
	if (game->info->comprehend_version == 1)
		file_buf_set_pos(&fb, 0x230);
	else
		file_buf_set_pos(&fb, 0x130);

	/* Restore rooms */
	file_buf_get_array_le16(&fb, 1, game->info->rooms,
		                    string_desc, nr_rooms);
	for (dir = 0; dir < NR_DIRECTIONS; dir++)
		file_buf_get_array_u8(&fb, 1, game->info->rooms,
			                    direction[dir], nr_rooms);
	file_buf_get_array_u8(&fb, 1, game->info->rooms, flags, nr_rooms);
	file_buf_get_array_u8(&fb, 1, game->info->rooms, graphic, nr_rooms);

	/*
	* Restore objects
	*
	* Layout differs depending on Comprehend version. Version 2 also
	* has long string descriptions for each object.
	*/
	file_buf_get_array_le16(&fb, 0, game->info->item, string_desc, nr_items);
	if (game->info->comprehend_version == 1) {
		file_buf_get_array_u8(&fb, 0, game->info->item, room, nr_items);
		file_buf_get_array_u8(&fb, 0, game->info->item, flags, nr_items);
		file_buf_get_array_u8(&fb, 0, game->info->item, word, nr_items);
		file_buf_get_array_u8(&fb, 0, game->info->item, graphic, nr_items);
	} else {
		file_buf_get_array_le16(&fb, 0, game->info->item, long_string, nr_items);
		file_buf_get_array_u8(&fb, 0, game->info->item, word, nr_items);
		file_buf_get_array_u8(&fb, 0, game->info->item, room, nr_items);
		file_buf_get_array_u8(&fb, 0, game->info->item, flags, nr_items);
		file_buf_get_array_u8(&fb, 0, game->info->item, graphic, nr_items);
	}

	/*
	* FIXME - The save file has some string descriptors masked with 0x8000.
	*         Not sure what this means, so just mask it out for now.
	*/
	for (i = 1; i <= nr_rooms; i++)
		patch_string_desc(&game->info->rooms[i].string_desc);
	for (i = 0; i < nr_items; i++)
		patch_string_desc(&game->info->item[i].string_desc);

	file_buf_unmap(&fb);
#else
	error("load");
#endif
}

} // namespace Comprehend
} // namespace Glk
