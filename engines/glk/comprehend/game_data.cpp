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

#define STRING_FILE_COUNT 64

void FunctionState::clear() {
	_testResult = true;
	_elseResult = false;
	_orCount = 0;
	_and = false;
	_inCommand = false;
	_executed = false;
	_notComparison = false;
}

/*-------------------------------------------------------*/

void Room::clear() {
	_flags = 0;
	_graphic = 0;
	_stringDesc = 0;
	Common::fill(&_direction[0], &_direction[NR_DIRECTIONS], 0);
}

/*-------------------------------------------------------*/

void Item::clear() {
	_stringDesc = 0;
	_longString = 0;
	_room = 0;
	_flags = 0;
	_word = 0;
	_graphic = 0;
}

void Item::synchronize(Common::Serializer &s) {
	s.syncAsUint16LE(_stringDesc);
	s.syncAsUint16LE(_longString);
	s.syncAsByte(_room);
	s.syncAsByte(_flags);
	s.syncAsByte(_word);
	s.syncAsByte(_graphic);
}

/*-------------------------------------------------------*/

void Word::clear() {
	WordIndex::clear();
	Common::fill(&_word[0], &_word[7], '\0');
}

Word &Word::operator=(const WordIndex &src) {
	_index = src._index;
	_type = src._type;
	Common::fill(&_word[0], &_word[7], '\0');
	return *this;
}

void Word::load(FileBuffer *fb) {
	fb->read(_word, 6);

	// Decode
	for (int j = 0; j < 6; j++)
		_word[j] = tolower((char)(_word[j] ^ 0xaa));

	// Strip off trailing spaces
	_word[6] = '\0';
	for (int j = 5; j > 0 && _word[j] == ' '; --j)
		_word[j] = '\0';

	_index = fb->readByte();
	_type = fb->readByte();
}

/*-------------------------------------------------------*/

void WordMap::clear() {
	_flags = 0;
	for (int idx = 0; idx < 3; ++idx)
		_word[idx].clear();
}

/*-------------------------------------------------------*/

void Action::clear() {
	_nr_words = 0;
	_function = 0;
	Common::fill(&_words[0], &_words[4], 0);
}

/*-------------------------------------------------------*/

Instruction::Instruction(byte opcode, byte op1, byte op2, byte op3) : _opcode(opcode) {
	_operand[0] = op1;
	_operand[1] = op2;
	_operand[2] = op3;
}

void Instruction::clear() {
	_opcode = 0;
	_nr_operands = 0;
	_isCommand = false;
	Common::fill(&_operand[0], &_operand[3], 0);
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

	Common::fill(&addr_actions[0], &addr_actions[7], 0);
	Common::fill(&room_direction_table[0], &room_direction_table[NR_DIRECTIONS], 0);
}

/*-------------------------------------------------------*/

void GameData::clearGame() {
	_header.clear();
	_magicWord = 0;
	_comprehendVersion = 0;
	_startRoom = 0;
	_currentRoom = 0;
	_currentReplaceWord = 0;
	_wordFlags = 0;
	_updateFlags = 0;
	_colorTable = 0;
	_itemCount = 0;
	_totalInventoryWeight = 0;

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
	instr->_opcode = fb->readByte();
	instr->_nr_operands = opcode_nr_operands(instr->_opcode);

	/* Get the operands */
	for (i = 0; i < instr->_nr_operands; i++)
		instr->_operand[i] = fb->readByte();

	instr->_isCommand = opcode_is_command(instr->_opcode);

	return instr->_opcode;
}

#define MAX_FUNCTION_SIZE 0x100

void GameData::parse_function(FileBuffer *fb, Function *func) {
	const uint8 *p;
	uint8 opcode;

	p = (const uint8 *)memchr(fb->dataPtr(), 0x00, fb->size() - fb->pos());
	if (!p)
		error("bad function @ %.4x", fb->pos());

	for (;;) {
		Instruction instruction;

		opcode = parse_vm_instruction(fb, &instruction);
		if (opcode == 0)
			break;

		func->push_back(instruction);
		assert(func->size() <= MAX_FUNCTION_SIZE);
	}

	assert(fb->dataPtr() == (p + 1));
}

void GameData::parse_vm(FileBuffer *fb) {
	fb->seek(_header.addr_vm);

	while (1) {
		Function func;

		parse_function(fb, &func);
		if (func.empty())
			break;

		_functions.push_back(func);

		// WORKAROUND: Parsing functions for Talisman
		if (_functions.size() == 0x1d8 && g_vm->getGameID() == "talisman")
			break;
	}
}

void GameData::parse_action_tables(FileBuffer *fb) {
	uint8 verb, count;
	uint i, j;

	_actions.clear();
	_actions.resize(7);

	const byte NUM_WORDS[7] = { 3, 2, 3, 2, 2, 1, 0 };

	for (int tableNum = 0; tableNum < 7; ++tableNum) {
		ActionTable &table = _actions[tableNum];

		fb->seek(_header.addr_actions[tableNum]);
		while (1) {
			verb = fb->readByte();
			if (verb == 0)
				break;

			count = fb->readByte();

			for (i = 0; i < count; i++) {
				Action action;
				action._nr_words = NUM_WORDS[tableNum] + 1;
				action._words[0] = verb;

				for (j = 1; j < action._nr_words; j++)
					action._words[j] = fb->readByte();
				action._function = fb->readUint16LE();

				table.push_back(action);
			}
		}
	}
}

void GameData::parse_dictionary(FileBuffer *fb) {
	fb->seek(_header.addr_dictionary);

	for (uint i = 0; i < _words.size(); i++)
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

		map._word[0]._index = index;
		map._word[0]._type = type;
		map._flags = fb->readByte();
		map._word[1]._index = fb->readByte();
		map._word[1]._type = fb->readByte();

		_wordMaps.push_back(map);
	}

	/*
	* Parse the target word table. Each entry has a dictionary
	* index/type. The first and second words from above map to the
	* target word here. E.g. 'go north' -> 'north'.
	*/
	fb->seek(_header.addr_word_map_target);

	for (i = 0; i < _wordMaps.size(); i++) {
		WordMap &map = _wordMaps[i];

		map._word[2]._index = fb->readByte();
		map._word[2]._type = fb->readByte();
	}
}

void GameData::parse_items(FileBuffer *fb) {
	size_t nr_items = _header.nr_items;
	_items.resize(nr_items);

	/* Item descriptions */
	fb->seek(_header.addr_item_strings);
	file_buf_get_array_le16(fb, 0, _items, _stringDesc, nr_items);

	if (_comprehendVersion == 2) {
		/* Comprehend version 2 adds long string descriptions */
		fb->seek(_header.addr_item_strings +
		         (_items.size() * sizeof(uint16)));
		file_buf_get_array_le16(fb, 0, _items, _longString, nr_items);
	}

	/* Item flags */
	fb->seek(_header.addr_item_flags);
	file_buf_get_array_u8(fb, 0, _items, _flags, nr_items);

	/* Item word */
	fb->seek(_header.addr_item_word);
	file_buf_get_array_u8(fb, 0, _items, _word, nr_items);

	/* Item locations */
	fb->seek(_header.addr_item_locations);
	file_buf_get_array_u8(fb, 0, _items, _room, nr_items);

	/* Item graphic */
	fb->seek(_header.addr_item_graphics);
	file_buf_get_array_u8(fb, 0, _items, _graphic, nr_items);
}

void GameData::parse_rooms(FileBuffer *fb) {
	size_t nr_rooms = _rooms.size() - 1;
	int i;

	/* Room exit directions */
	for (i = 0; i < NR_DIRECTIONS; i++) {
		fb->seek(_header.room_direction_table[i]);
		file_buf_get_array_u8(fb, 1, _rooms, _direction[i], nr_rooms);
	}

	/* Room string descriptions */
	fb->seek(_header.room_desc_table);
	file_buf_get_array_le16(fb, 1, _rooms, _stringDesc, nr_rooms);

	/* Room flags */
	fb->seek(_header.room_flags_table);
	file_buf_get_array_u8(fb, 1, _rooms, _flags, nr_rooms);

	/* Room graphic */
	fb->seek(_header.room_graphics_table);
	file_buf_get_array_u8(fb, 1, _rooms, _graphic, nr_rooms);
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
	if (start_addr < end_addr) {
		fb->seek(start_addr);
		while (1) {
			table->push_back(parseString(fb));
			if (fb->pos() >= (int32)end_addr)
				break;
		}
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
	fb->skip(2);		// Unknown in earlier versions

	switch (header->magic) {
	case 0x2000: /* Transylvania, Crimson Crown disk one */
	case 0x4800: /* Crimson Crown disk two */
		_comprehendVersion = 1;
		_magicWord = (uint16)(-0x5a00 + 0x4);
		break;

	case 0x8bc3: /* Transylvania v2 */
	case 0x93f0: /* OO-Topos */
	case 0xa429: /* Talisman */
		_comprehendVersion = 2;
		_magicWord = (uint16)-0x5a00;

		// Actions table starts right at the start of the file
		fb->seek(0);
		break;

	default:
		error("Unknown game_data magic %.4x\n", header->magic);
		break;
	}

	/* Basic data */
	for (int idx = 0; idx < 7; ++idx)
		parse_header_le16(fb, &header->addr_actions[idx]);

	parse_header_le16(fb, &header->addr_vm);
	parse_header_le16(fb, &header->addr_dictionary);

	parse_header_le16(fb, &header->addr_word_map);
	parse_header_le16(fb, &header->addr_word_map_target);
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

	fb->skip(9);
	_itemCount = fb->readByte();

	_rooms.resize(header->room_direction_table[DIRECTION_SOUTH] -
	                    header->room_direction_table[DIRECTION_NORTH] + 1);

	_words.resize((addr_dictionary_end - header->addr_dictionary) / 8);
}

void GameData::load_extra_string_file(const StringFile &stringFile) {
	FileBuffer fb(stringFile._filename);

	if (stringFile._baseOffset > 0) {
		// Explicit offset specified, so read the strings in sequentially
		uint endOffset = stringFile._endOffset;
		if (!endOffset)
			endOffset = fb.size();

		parse_string_table(&fb, stringFile._baseOffset, endOffset, &_strings2);
	} else {
		// Standard strings file. Has a 4-byte header we can ignore,
		// followed by 64 2-byte string offsets
		fb.seek(4);
		uint fileSize = fb.size();

		// Read in the index
		uint16 index[STRING_FILE_COUNT];
		Common::fill(&index[0], &index[STRING_FILE_COUNT], 0);

		for (int i = 0; i < STRING_FILE_COUNT; ++i) {
			uint v = fb.readUint16LE();
			if (v > fileSize)
				break;

			index[i] = v;
		}

		// Iterate through parsing the strings
		for (int i = 0; i < STRING_FILE_COUNT; ++i) {
			if (index[i]) {
				fb.seek(index[i] + 4);
				_strings2.push_back(parseString(&fb));
			} else {
				_strings2.push_back("");
			}
		}
	}
}

void GameData::load_extra_string_files() {
	_strings2.clear();
	_strings2.reserve(STRING_FILE_COUNT * _stringFiles.size() + 1);

	for (uint i = 0; i < _stringFiles.size(); i++) {
		// TODO: Is this needed for other than OO-Topos?
		if (_comprehendVersion == 2 && (i == 0 || i == 4))
			_strings2.push_back("");

		load_extra_string_file(_stringFiles[i]);
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
	if (g_comprehend->getGameID() != "talisman")
		parse_string_table(&fb, _header.addr_strings, _header.addr_strings_end, &_strings);
	load_extra_string_files();
	parse_vm(&fb);
	parse_action_tables(&fb);
	parse_replace_words(&fb);
}

void GameData::loadGame() {
	/* Load the main game data file */
	loadGameData();

	if (g_comprehend->isGraphicsEnabled()) {
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
