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
#include "glk/comprehend/draw_surface.h"
#include "glk/comprehend/file_buf.h"
#include "glk/comprehend/game.h"
#include "glk/comprehend/pics.h"

namespace Glk {
namespace Comprehend {

static const char CHARSET[] = "..abcdefghijklmnopqrstuvwxyz .";
static const char SPECIAL_CHARSET[] = "[]\n!\"#$%&'(),-/0123456789:;?<>";

void FunctionState::clear() {
	test_result = true;
	else_result = false;
	or_count = 0;
	_and = false;
	in_command = false;
	executed = false;
}

/*-------------------------------------------------------*/

void Room::clear() {
	flags = 0;
	graphic = 0;
	string_desc = 0;
	Common::fill(&direction[0], &direction[NR_DIRECTIONS], 0);
}

/*-------------------------------------------------------*/

void Item::clear() {
	string_desc = 0;
	long_string = 0;
	room = 0;
	flags = 0;
	word = 0;
	graphic = 0;
}

void Item::synchronize(Common::Serializer &s) {
	s.syncAsUint16LE(string_desc);
	s.syncAsUint16LE(long_string);
	s.syncAsByte(room);
	s.syncAsByte(flags);
	s.syncAsByte(word);
	s.syncAsByte(graphic);
}

/*-------------------------------------------------------*/

void Word::clear() {
	_index = 0;
	_type = 0;
	Common::fill(&_word[0], &_word[7], '\0');
}

void Word::load(FileBuffer *fb) {
	fb->read(_word, 6);

	// Decode
	for (int j = 0; j < 6; j++)
		_word[j] ^= 0x8a;
	_word[6] = '\0';

	_index = fb->readByte();
	_type = fb->readByte();
}

/*-------------------------------------------------------*/

void WordMap::clear() {
	flags = 0;
	for (int idx = 0; idx < 3; ++idx)
		word[idx].clear();
}

/*-------------------------------------------------------*/

void Action::clear() {
	type = 0;
	nr_words = 0;
	function = 0;
	Common::fill(&word[0], &word[4], 0);
	Common::fill(&word_type[0], &word_type[4], 0);
}

/*-------------------------------------------------------*/

void Instruction::clear() {
	opcode = 0;
	nr_operands = 0;
	is_command = false;
	Common::fill(&operand[0], &operand[3], 0);
}

/*-------------------------------------------------------*/

void Function::clear() {
	nr_instructions = 0;
	for (int idx = 0; idx < 0x100; ++idx)
		instructions[idx].clear();
}

/*-------------------------------------------------------*/

void GameHeader::clear() {
	magic = 0;
	room_desc_table = 0;
	room_flags_table = 0;
	room_graphics_table = 0;
	nr_items = 0;
	addr_item_locations = 0;
	addr_item_flags = 0;
	addr_item_word = 0;
	addr_item_strings = 0;
	addr_item_graphics = 0;
	addr_dictionary = 0;
	addr_word_map = 0;
	addr_strings = 0;
	addr_strings_end = 0;
	addr_actions_vvnn = 0;
	addr_actions_unknown = 0;
	addr_actions_vnjn = 0;
	addr_actions_vjn = 0;
	addr_actions_vdn = 0;
	addr_actions_vnn = 0;
	addr_actions_vn = 0;
	addr_actions_v = 0;
	addr_vm = 0;
	Common::fill(&room_direction_table[0], &room_direction_table[NR_DIRECTIONS], 0);
}

/*-------------------------------------------------------*/

void GameData::clearGame() {
	_header.clear();
	_magicWord = 0;
	_comprehendVersion = 0;
	_startRoom = 0;
	_currentRoom = 0;
	_words = nullptr;
	_nr_words = 0;
	_currentReplaceWord = 0;
	_updateFlags = 0;
	_colorTable = 0;

	_strings.clear();
	_strings2.clear();
	_rooms.clear();
	_items.clear();
	_wordMaps.clear();
	_actions.clear();
	_functions.clear();
	_replaceWords.clear();

	Common::fill(&_flags[0], &_flags[MAX_FLAGS], false);
	Common::fill(&_variables[0], &_variables[MAX_VARIABLES], 0);
}

void GameData::parse_header_le16(FileBuffer *fb, uint16 *val) {
	*val = fb->readUint16LE();
	*val += _magicWord;
}

uint8 GameData::parse_vm_instruction(FileBuffer *fb,
                                  Instruction *instr) {
	uint i;

	/* Get the opcode */
	instr->opcode = fb->readByte();
	instr->nr_operands = opcode_nr_operands(instr->opcode);

	/* Get the operands */
	for (i = 0; i < instr->nr_operands; i++)
		instr->operand[i] = fb->readByte();

	instr->is_command = opcode_is_command(instr->opcode);

	return instr->opcode;
}

void GameData::parse_function(FileBuffer *fb, Function *func) {
	Instruction *instruction;
	const uint8 *p;
	uint8 opcode;

	p = (const uint8 *)memchr(fb->dataPtr(), 0x00, fb->size() - fb->pos());
	if (!p)
		error("bad function @ %.4x", fb->pos());

	while (1) {
		instruction = &func->instructions[func->nr_instructions];

		opcode = parse_vm_instruction(fb, instruction);
		if (opcode == 0)
			break;

		func->nr_instructions++;
		if (func->nr_instructions >= ARRAY_SIZE(func->instructions))
			error("Function has too many instructions");
	}
}

void GameData::parse_vm(FileBuffer *fb) {
	fb->seek(_header.addr_vm);

	while (1) {
		Function func;

		parse_function(fb, &func);
		if (func.nr_instructions == 0)
			break;

		_functions.push_back(func);
	}
}

void GameData::parse_action_table_vvnn(FileBuffer *fb) {
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
	fb->seek(_header.addr_actions_vvnn);
	while (1) {
		verb = fb->readByte();
		if (verb == 0)
			break;
		count = fb->readByte();

		for (i = 0; i < count; i++) {
			Action action;
			action.type = ACTION_VERB_VERB_NOUN_NOUN;

			action.nr_words = 4;
			action.word_type[0] = WORD_TYPE_VERB;
			action.word_type[1] = WORD_TYPE_VERB;
			action.word_type[2] = WORD_TYPE_NOUN_MASK;
			action.word_type[3] = WORD_TYPE_NOUN_MASK;

			action.word[0] = verb;

			for (j = 0; j < 3; j++)
				action.word[j + 1] = fb->readByte();
			action.function = fb->readUint16LE();

			_actions.push_back(action);
		}
	}
}

void GameData::parse_action_table_vnjn(FileBuffer *fb) {
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
	fb->seek(_header.addr_actions_vnjn);
	while (1) {
		join = fb->readByte();
		if (join == 0)
			break;
		count = fb->readByte();

		for (i = 0; i < count; i++) {
			Action action;
			action.type = ACTION_VERB_NOUN_JOIN_NOUN;

			action.nr_words = 4;
			action.word_type[0] = WORD_TYPE_VERB;
			action.word_type[1] = WORD_TYPE_NOUN_MASK;
			action.word_type[2] = WORD_TYPE_JOIN;
			action.word_type[3] = WORD_TYPE_NOUN_MASK;

			action.word[2] = join;

			action.word[0] = fb->readByte();
			action.word[1] = fb->readByte();
			action.word[3] = fb->readByte();
			action.function = fb->readUint16LE();

			_actions.push_back(action);
		}
	}
}

void GameData::parse_action_table_vjn(FileBuffer *fb) {
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
	fb->seek(_header.addr_actions_vjn);
	while (1) {
		join = fb->readByte();
		if (join == 0)
			break;
		count = fb->readByte();

		for (i = 0; i < count; i++) {
			Action action;
			action.type = ACTION_VERB_JOIN_NOUN;
			action.word[1] = join;

			action.nr_words = 3;
			action.word_type[0] = WORD_TYPE_VERB;
			action.word_type[1] = WORD_TYPE_JOIN;
			action.word_type[2] = WORD_TYPE_NOUN_MASK;

			action.word[0] = fb->readByte();
			action.word[2] = fb->readByte();
			action.function = fb->readUint16LE();

			_actions.push_back(action);
		}
	}
}

void GameData::parse_action_table_vdn(FileBuffer *fb) {
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
	fb->seek(_header.addr_actions_vdn);
	while (1) {
		verb = fb->readByte();
		if (verb == 0)
			break;
		count = fb->readByte();

		for (i = 0; i < count; i++) {
			Action action;
			action.type = ACTION_VERB_JOIN_NOUN;
			action.word[0] = verb;

			action.nr_words = 3;
			action.word_type[0] = WORD_TYPE_VERB;
			action.word_type[1] = WORD_TYPE_VERB;
			action.word_type[2] = WORD_TYPE_NOUN_MASK;

			action.word[1] = fb->readByte();
			action.word[2] = fb->readByte();
			action.function = fb->readUint16LE();

			_actions.push_back(action);
		}
	}
}

void GameData::parse_action_table_vnn(FileBuffer *fb) {
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
	fb->seek(_header.addr_actions_vnn);
	while (1) {
		/* 2-byte header */
		verb = fb->readByte();
		if (verb == 0)
			break;
		count = fb->readByte();

		for (i = 0; i < count; i++) {
			Action action;
			action.type = ACTION_VERB_NOUN_NOUN;
			action.word[0] = verb;

			action.nr_words = 3;
			action.word_type[0] = WORD_TYPE_VERB;
			action.word_type[1] = WORD_TYPE_NOUN_MASK;
			action.word_type[2] = WORD_TYPE_NOUN_MASK;

			action.word[1] = fb->readByte();
			action.word[2] = fb->readByte();
			action.function = fb->readUint16LE();

			_actions.push_back(action);
		}
	}
}

void GameData::parse_action_table_vn(FileBuffer *fb) {
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
	fb->seek(_header.addr_actions_vn);
	while (1) {
		/* 2-byte header */
		verb = fb->readByte();
		if (verb == 0)
			break;
		count = fb->readByte();

		for (i = 0; i < count; i++) {
			Action action;
			action.type = ACTION_VERB_NOUN;
			action.word[0] = verb;

			action.nr_words = 2;
			action.word_type[0] = WORD_TYPE_VERB;
			action.word_type[1] = WORD_TYPE_NOUN_MASK;

			action.word[1] = fb->readByte();
			action.function = fb->readUint16LE();

			_actions.push_back(action);
		}
	}
}

void GameData::parse_action_table_v(FileBuffer *fb) {
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
	fb->seek(_header.addr_actions_v);
	while (1) {
		verb = fb->readByte();
		if (verb == 0)
			break;

		Action action;
		action.type = ACTION_VERB_OPT_NOUN;
		action.word[0] = verb;

		/* Can take an optional noun (nr_words here is maximum) */
		action.nr_words = 1;
		action.word_type[0] = WORD_TYPE_VERB;

		/*
		* Default actions can have more than one function, but only
		* the first one actually seems to be used?
		*/
		nr_funcs = fb->readByte();
		for (i = 0; i < nr_funcs; i++) {
			func = fb->readUint16LE();
			if (i == 0)
				action.function = func;
		}

		_actions.push_back(action);
	}
}

void GameData::parse_action_table(FileBuffer *fb) {
	_actions.clear();

	if (_comprehendVersion == 1) {
		parse_action_table_vvnn(fb);
		parse_action_table_vdn(fb);
	}
	if (_comprehendVersion >= 2) {
		parse_action_table_vnn(fb);
	}

	parse_action_table_vnjn(fb);
	parse_action_table_vjn(fb);
	parse_action_table_vn(fb);
	parse_action_table_v(fb);
}

void GameData::parse_dictionary(FileBuffer *fb) {
	uint i;

	// FIXME - fixed size 0xff array?
	_words = (Word *)malloc(_nr_words * sizeof(Word));

	fb->seek(_header.addr_dictionary);
	for (i = 0; i < _nr_words; i++)
		_words[i].load(fb);
}

void GameData::parse_word_map(FileBuffer *fb) {
	uint8 index, type;
	uint i;

	_wordMaps.clear();
	fb->seek(_header.addr_word_map);

	/*
	* Parse the word pair table. Each entry has a pair of dictionary
	* index/type values for a first and second word.
	*/
	while (1) {
		WordMap map;

		index = fb->readByte();
		type = fb->readByte();
		if (type == 0 && index == 0) {
			/* End of pairs */
			break;
		}

		map.word[0].index = index;
		map.word[0].type = type;
		map.flags = fb->readByte();
		map.word[1].index = fb->readByte();
		map.word[1].type = fb->readByte();

		_wordMaps.push_back(map);
	}

	/* Consume two more null bytes (type and index were also null) */
	fb->skip(2);

	/*
	* Parse the target word table. Each entry has a dictionary
	* index/type. The first and second words from above map to the
	* target word here. E.g. 'go north' -> 'north'.
	*/
	for (i = 0; i < _wordMaps.size(); i++) {
		WordMap &map = _wordMaps[i];

		map.word[2].index = fb->readByte();
		map.word[2].type = fb->readByte();
	}
}

void GameData::parse_items(FileBuffer *fb) {
	size_t nr_items = _header.nr_items;
	_items.resize(nr_items);

	/* Item descriptions */
	fb->seek(_header.addr_item_strings);
	file_buf_get_array_le16(fb, 0, _items, string_desc, nr_items);

	if (_comprehendVersion == 2) {
		/* Comprehend version 2 adds long string descriptions */
		fb->seek(_header.addr_item_strings +
		         (_items.size() * sizeof(uint16)));
		file_buf_get_array_le16(fb, 0, _items, long_string, nr_items);
	}

	/* Item flags */
	fb->seek(_header.addr_item_flags);
	file_buf_get_array_u8(fb, 0, _items, flags, nr_items);

	/* Item word */
	fb->seek(_header.addr_item_word);
	file_buf_get_array_u8(fb, 0, _items, word, nr_items);

	/* Item locations */
	fb->seek(_header.addr_item_locations);
	file_buf_get_array_u8(fb, 0, _items, room, nr_items);

	/* Item graphic */
	fb->seek(_header.addr_item_graphics);
	file_buf_get_array_u8(fb, 0, _items, graphic, nr_items);
}

void GameData::parse_rooms(FileBuffer *fb) {
	size_t nr_rooms = _rooms.size() - 1;
	int i;

	/* Room exit directions */
	for (i = 0; i < NR_DIRECTIONS; i++) {
		fb->seek(_header.room_direction_table[i]);
		file_buf_get_array_u8(fb, 1, _rooms,
		                      direction[i], nr_rooms);
	}

	/* Room string descriptions */
	fb->seek(_header.room_desc_table);
	file_buf_get_array_le16(fb, 1, _rooms, string_desc, nr_rooms);

	/* Room flags */
	fb->seek(_header.room_flags_table);
	file_buf_get_array_u8(fb, 1, _rooms, flags, nr_rooms);

	/* Room graphic */
	fb->seek(_header.room_graphics_table);
	file_buf_get_array_u8(fb, 1, _rooms, graphic, nr_rooms);
}

uint64 GameData::string_get_chunk(uint8 *string) {
	uint64 c, val = 0;
	int i;

	for (i = 0; i < 5; i++) {
		c = string[i] & 0xff;
		val |= (c << ((4 - i) * 8));
	}

	return val;
}

char GameData::decode_string_elem(uint8 c, bool capital, bool special) {
	if (special) {
		if (c < sizeof(SPECIAL_CHARSET) - 1)
			return SPECIAL_CHARSET[c];
	} else {
		if (c < sizeof(CHARSET) - 1) {
			c = CHARSET[c];
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

	// Unknown character
	g_comprehend->print("Unknown char %d, caps=%d, special=%d\n", c, capital, special);
	return '*';
}

Common::String GameData::parseString(FileBuffer *fb) {
	bool capital_next = false, special_next = false;
	unsigned i, j;
	uint64 chunk;
	uint8 elem, *encoded;
	char c;
	size_t encoded_len;
	Common::String string;

	encoded_len = fb->strlen();

	/* Get the encoded string */
	encoded = (uint8 *)malloc(encoded_len + 5);
	Common::fill(encoded, encoded + encoded_len + 5, 0);
	fb->read(encoded, encoded_len);

	/* Skip over the zero byte */
	if (fb->pos() < fb->size())
		fb->skip(1);

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
				string += c;
			}
		}
	}

done:
	free(encoded);

	return string;
}

void GameData::parse_string_table(FileBuffer *fb, unsigned start_addr,
                               uint32 end_addr, StringTable *table) {
	fb->seek(start_addr);
	while (1) {
		table->push_back(parseString(fb));
		if (fb->pos() >= (int32)end_addr)
			break;
	}
}

void GameData::parse_variables(FileBuffer *fb) {
	uint i;

	for (i = 0; i < ARRAY_SIZE(_variables); i++)
		_variables[i] = fb->readUint16LE();
}

void GameData::parse_flags(FileBuffer *fb) {
	uint i, flag_index = 0;
	int bit;
	uint8 bitmask;

	for (i = 0; i < ARRAY_SIZE(_flags) / 8; i++) {
		bitmask = fb->readByte();
		for (bit = 7; bit >= 0; bit--) {
			_flags[flag_index] = !!(bitmask & (1 << bit));
			flag_index++;
		}
	}
}

void GameData::parse_replace_words(FileBuffer *fb) {
	size_t len;
	bool eof;
	int i;

	/* FIXME - Rename addr_strings_end */
	fb->seek(_header.addr_strings_end);

	/* FIXME - what is this for */
	fb->skip(2);

	for (i = 0;; i++) {
		len = fb->strlen(&eof);
		if (len == 0)
			break;

		_replaceWords.push_back(Common::String((const char *)fb->dataPtr(), len));
		fb->skip(len + (eof ? 0 : 1));
		if (eof)
			break;
	}
}

void GameData::parse_header(FileBuffer *fb) {
	GameHeader *header = &_header;
	uint16 dummy, addr_dictionary_end;

	fb->seek(0);
	header->magic = fb->readUint16LE();
	switch (header->magic) {
	case 0x2000: /* Transylvania, Crimson Crown disk one */
	case 0x4800: /* Crimson Crown disk two */
		_comprehendVersion = 1;
		_magicWord = (uint16)(-0x5a00 + 0x4);
		break;

	case 0x93f0: /* OO-Topos */
		_comprehendVersion = 2;
		_magicWord = (uint16)-0x5a00;
		break;

	case 0xa429: /* Talisman */
		_comprehendVersion = 2;
		_magicWord = (uint16)-0x5a00;
		break;

	default:
		error("Unknown game_data magic %.4x\n", header->magic);
		break;
	}

	/* FIXME - Second word in header has unknown usage */
	parse_header_le16(fb, &dummy);

	/*
	* Action tables.
	*
	* Layout depends on the comprehend version.
	*/
	if (_comprehendVersion == 1) {
		parse_header_le16(fb, &header->addr_actions_vvnn);
		parse_header_le16(fb, &header->addr_actions_unknown);
		parse_header_le16(fb, &header->addr_actions_vnjn);
		parse_header_le16(fb, &header->addr_actions_vjn);
		parse_header_le16(fb, &header->addr_actions_vdn);
	}
	if (_comprehendVersion >= 2) {
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
	if (_comprehendVersion == 1) {
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

	fb->skip(1);
	_startRoom = fb->readByte();
	fb->skip(1);

	parse_variables(fb);
	parse_flags(fb);

	_rooms.resize(header->room_direction_table[DIRECTION_SOUTH] -
	                    header->room_direction_table[DIRECTION_NORTH] + 1);

	_nr_words = (addr_dictionary_end -
	                   header->addr_dictionary) /
	                  8;
}

void GameData::load_extra_string_file(StringFile *string_file) {
	FileBuffer fb(string_file->filename);
	unsigned end;

	if (string_file->end_offset)
		end = string_file->end_offset;
	else
		end = fb.size();

	parse_string_table(&fb, string_file->base_offset,
	                   end, &_strings2);
}

void GameData::load_extra_string_files() {
	uint i;

	for (i = 0; i < _stringFiles.size(); i++) {
		// HACK - get string offsets correct
		_strings2.resize(0x40 * i);
		if (_strings2.empty())
			_strings2.push_back("");

		load_extra_string_file(&_stringFiles[i]);
	}
}

void GameData::loadGameData() {
	FileBuffer fb(_gameDataFile);

	clearGame();

	parse_header(&fb);
	parse_rooms(&fb);
	parse_items(&fb);
	parse_dictionary(&fb);
	parse_word_map(&fb);
	parse_string_table(&fb, _header.addr_strings,
	                   _header.addr_strings_end,
	                   &_strings);
	load_extra_string_files();
	parse_vm(&fb);
	parse_action_table(&fb);
	parse_replace_words(&fb);
}

void GameData::loadGame() {
	/* Load the main game data file */
	loadGameData();

	if (g_comprehend->_graphicsEnabled) {
		// Set up the picture archive
		g_comprehend->_pics->load(_locationGraphicFiles,
		                          _itemGraphicFiles, _titleGraphicFile);

		if (_colorTable)
			g_comprehend->_drawSurface->setColorTable(_colorTable);
	}

	// FIXME: This can be merged, don't need to keep start room around
	_currentRoom = _startRoom;
}

} // namespace Comprehend
} // namespace Glk
