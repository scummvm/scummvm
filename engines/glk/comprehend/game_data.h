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

#ifndef GLK_COMPREHEND_GAME_DATA_H
#define GLK_COMPREHEND_GAME_DATA_H

#include "glk/comprehend/file_buf.h"
#include "common/serializer.h"
#include "common/str-array.h"

namespace Glk {
namespace Comprehend {

#define MAX_FLAGS 256
#define MAX_VARIABLES 128
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

class ComprehendGame;

enum {
	DIRECTION_NORTH,
	DIRECTION_SOUTH,
	DIRECTION_EAST,
	DIRECTION_WEST,
	DIRECTION_UP,
	DIRECTION_DOWN,
	DIRECTION_IN,
	DIRECTION_OUT,
	NR_DIRECTIONS
};


enum ScriptOpcode {
	OPCODE_UNKNOWN,
	OPCODE_HAVE_OBJECT,
	OPCODE_OR,
	OPCODE_IN_ROOM,
	OPCODE_VAR_EQ1,
	OPCODE_VAR_EQ2,
	OPCODE_VAR_GT1,
	OPCODE_VAR_GT2,
	OPCODE_VAR_GTE1,
	OPCODE_VAR_GTE2,
	OPCODE_CURRENT_IS_OBJECT,
	OPCODE_OBJECT_PRESENT,
	OPCODE_ELSE,
	OPCODE_OBJECT_IN_ROOM,
	OPCODE_CURRENT_OBJECT_NOT_VALID,
	OPCODE_INVENTORY_FULL,
	OPCODE_INVENTORY_FULL_X,
	OPCODE_TEST_FLAG,
	OPCODE_CURRENT_OBJECT_IN_ROOM,
	OPCODE_HAVE_CURRENT_OBJECT,
	OPCODE_OBJECT_IS_NOT_NOWHERE,
	OPCODE_CURRENT_OBJECT_PRESENT,
	OPCODE_TEST_ROOM_FLAG,
	OPCODE_NOT_HAVE_OBJECT,
	OPCODE_NOT_IN_ROOM,
	OPCODE_CURRENT_OBJECT_NOT_IN_ROOM,
	OPCODE_OBJECT_NOT_IN_ROOM,
	OPCODE_TEST_NOT_FLAG,
	OPCODE_NOT_HAVE_CURRENT_OBJECT,
	OPCODE_OBJECT_IS_NOWHERE,
	OPCODE_OBJECT_NOT_PRESENT,
	OPCODE_CURRENT_OBJECT_IS_NOWHERE,
	OPCODE_CURRENT_OBJECT_NOT_PRESENT,
	OPCODE_CURRENT_OBJECT_NOT_TAKEABLE,
	OPCODE_TEST_NOT_ROOM_FLAG,
	OPCODE_INVENTORY,
	OPCODE_TAKE_OBJECT,
	OPCODE_MOVE_OBJECT_TO_ROOM,
	OPCODE_SAVE_ACTION,
	OPCODE_CLEAR_LINE,
	OPCODE_MOVE_TO_ROOM,
	OPCODE_VAR_ADD,
	OPCODE_SET_ROOM_DESCRIPTION,
	OPCODE_MOVE_OBJECT_TO_CURRENT_ROOM,
	OPCODE_VAR_SUB,
	OPCODE_SET_OBJECT_DESCRIPTION,
	OPCODE_SET_OBJECT_LONG_DESCRIPTION,
	OPCODE_MOVE_DEFAULT,
	OPCODE_PRINT,
	OPCODE_REMOVE_OBJECT,
	OPCODE_SET_FLAG,
	OPCODE_CALL_FUNC,
	OPCODE_CALL_FUNC2,
	OPCODE_TURN_TICK,
	OPCODE_CLEAR_FLAG,
	OPCODE_INVENTORY_ROOM,
	OPCODE_TAKE_CURRENT_OBJECT,
	OPCODE_SPECIAL,
	OPCODE_DROP_OBJECT,
	OPCODE_DROP_CURRENT_OBJECT,
	OPCODE_SET_ROOM_GRAPHIC,
	OPCODE_SET_OBJECT_GRAPHIC,
	OPCODE_REMOVE_CURRENT_OBJECT,
	OPCODE_MOVE_DIR,
	OPCODE_VAR_INC,
	OPCODE_VAR_DEC,
	OPCODE_MOVE_CURRENT_OBJECT_TO_ROOM,
	OPCODE_DESCRIBE_CURRENT_OBJECT,
	OPCODE_SET_STRING_REPLACEMENT1,
	OPCODE_SET_STRING_REPLACEMENT2,
	OPCODE_SET_STRING_REPLACEMENT3,
	OPCODE_SET_CURRENT_NOUN_STRING_REPLACEMENT,
	OPCODE_DRAW_ROOM,
	OPCODE_DRAW_OBJECT,
	OPCODE_WAIT_KEY,
	OPCODE_TEST_FALSE,
	OPCODE_CAN_TAKE,
	OPCODE_TOO_HEAVY,
	OPCODE_OBJECT_TAKEABLE,
	OPCODE_OBJECT_CAN_TAKE,
	OPCODE_CLEAR_INVISIBLE,
	OPCODE_SET_INVISIBLE,
	OPCODE_CLEAR_CAN_TAKE,
	OPCODE_SET_CAN_TAKE,
	OPCODE_CLEAR_FLAG40,
	OPCODE_SET_FLAG40,
	OPCODE_RANDOM_MSG,
	OPCODE_SET_WORD,
	OPCODE_CLEAR_WORD
};

/* Game state update flags */
#define UPDATE_GRAPHICS (1 << 0) /* Implies UPDATE_GRAPHICS_ITEMS */
#define UPDATE_GRAPHICS_ITEMS (1 << 1)
#define UPDATE_ROOM_DESC (1 << 2)
#define UPDATE_ITEM_LIST (1 << 3)
#define UPDATE_ALL (~0U)

/* Action types */
enum {
	ACTION_VERB_VERB_NOUN_NOUN,
	ACTION_VERB_NOUN_JOIN_NOUN,
	ACTION_VERB_JOIN_NOUN,
	ACTION_VERB_DIR_NOUN,
	ACTION_VERB_NOUN_NOUN,
	ACTION_VERB_NOUN,
	ACTION_VERB_OPT_NOUN
};

/* Standard strings (main string table) */
#define STRING_CANT_GO 0
#define STRING_DONT_UNDERSTAND 1
#define STRING_YOU_SEE 2
#define STRING_INVENTORY 3
#define STRING_INVENTORY_EMPTY 4
#define STRING_BEFORE_CONTINUE 5
#define STRING_SAVE_GAME 6
#define STRING_RESTORE_GAME 7

/* Special variables */
#define VAR_INVENTORY_WEIGHT 0
#define VAR_INVENTORY_LIMIT 1
#define VAR_TURN_COUNT 2

/* Special rooms */
#define ROOM_INVENTORY 0x00
#define ROOM_CONTAINER 0xfe
#define ROOM_NOWHERE 0xff

/* Item flags */
enum ItemFlag {
	ITEMF_WEIGHT_MASK = 0x7,
	ITEMF_CAN_TAKE    = 1 << 3,
	ITEMF_UNKNOWN     = 1 << 6,
	ITEMF_INVISIBLE   = 1 << 7
};

/* Word types */
#define WORD_TYPE_VERB 0x01
#define WORD_TYPE_JOIN 0x02
#define WORD_TYPE_FEMALE 0x10
#define WORD_TYPE_MALE 0x20
#define WORD_TYPE_NOUN 0x40
#define WORD_TYPE_NOUN_PLURAL 0x80
#define WORD_TYPE_NOUN_MASK (WORD_TYPE_FEMALE | WORD_TYPE_MALE | \
                             WORD_TYPE_NOUN | WORD_TYPE_NOUN_PLURAL)

struct FunctionState {
	bool _testResult;
	bool _elseResult;
	uint _orCount;
	bool _and;
	bool _inCommand;
	bool _executed;
	bool _notComparison;

	FunctionState() {
		clear();
	}

	void clear();
};

struct Room {
	uint8 _direction[NR_DIRECTIONS];
	uint8 _flags;
	uint8 _graphic;
	uint16 _stringDesc;

	Room() {
		clear();
	}

	void clear();
};

struct Item {
	uint16 _stringDesc;
	uint16 _longString; /* Only used by version 2 */
	uint8 _room;
	uint8 _flags;
	uint8 _word;
	uint8 _graphic;

	Item() {
		clear();
	}

	void clear();

	void synchronize(Common::Serializer &s);
};

struct WordIndex {
	uint8 _index;
	uint8 _type;

	WordIndex() {
		clear();
	}

	void clear() {
		_index = _type = 0;
	}

	bool operator==(WordIndex &src) {
		return _index == src._index && _type == src._type;
	}

	bool operator()() const {
		return _index != 0;
	}
};

struct Word : public WordIndex {
	char _word[7];

	Word() : WordIndex() {
		Word::clear();
	}

	void clear();

	void load(FileBuffer *fb);

	Word &operator=(const WordIndex &src);
};

struct WordMap {
	/* <word[0]>, <word[1]> == <word[2]> */
	WordIndex _word[3];
	uint8 _flags;

	WordMap() {
		clear();
	}

	void clear();
};

struct Action {
	size_t _nr_words;
	uint8 _words[4];
	uint16 _function;

	Action() {
		clear();
	}

	void clear();
};

struct Instruction {
	uint8 _opcode;
	size_t _nr_operands;
	uint8 _operand[3];
	bool _isCommand;

	Instruction() {
		clear();
	}

	Instruction(byte opcode, byte op1 = 0, byte op2 = 0, byte op3 = 0);

	void clear();
};

typedef Common::Array<Instruction> Function;

typedef Common::StringArray StringTable;

struct StringFile {
	Common::String _filename;
	uint32 _baseOffset;
	uint32 _endOffset;

	StringFile() : _baseOffset(0), _endOffset(0) {
	} 
	StringFile(const char *fname, uint32 baseOfs = 0, uint32 endO = 0) :
		_filename(fname), _baseOffset(baseOfs), _endOffset(endO) {
	}
};

struct GameHeader {
	uint16 magic;

	uint16 room_desc_table;
	uint16 room_direction_table[NR_DIRECTIONS];
	uint16 room_flags_table;
	uint16 room_graphics_table;

	size_t nr_items;
	uint16 addr_item_locations;
	uint16 addr_item_flags;
	uint16 addr_item_word;
	uint16 addr_item_strings;
	uint16 addr_item_graphics;

	uint16 addr_dictionary;
	uint16 addr_word_map;
	uint16 addr_word_map_target;

	uint16 addr_strings;
	uint16 addr_strings_end;

	uint16 addr_actions[7];

	uint16 addr_vm; // FIXME - functions

	GameHeader() {
		clear();
	}

	void clear();
};

typedef Common::Array<Action> ActionTable;

class GameData {
private:
	uint16 _magicWord;
protected:
	Common::String _gameDataFile;
	Common::Array<StringFile> _stringFiles;
	Common::StringArray _locationGraphicFiles;
	Common::StringArray _itemGraphicFiles;
	Common::String _titleGraphicFile;
	uint _colorTable;

public:
	GameHeader _header;

	uint _comprehendVersion;

	Common::Array<Room> _rooms;
	uint8 _currentRoom;
	uint8 _startRoom;
	uint8 _itemCount;
	uint8 _totalInventoryWeight;

	Common::Array<Item> _items;
	Common::Array<Word> _words;

	StringTable _strings;
	StringTable _strings2;

	bool _flags[MAX_FLAGS];
	uint16 _variables[MAX_VARIABLES];

	uint8 _currentReplaceWord;
	uint8 _wordFlags;
	uint _updateFlags;

	Common::Array<WordMap> _wordMaps;
	Common::Array<ActionTable> _actions;
	Common::Array<Function> _functions;
	Common::StringArray _replaceWords;

private:
	size_t opcode_nr_operands(uint8 opcode) const {
		// Number of operands is encoded in the low 2 bits
		return opcode & 0x3;
	}

	bool opcode_is_command(uint8 opcode) const {
		/* If the MSB is set the instruction is a command */
		return opcode & 0x80;
	}

	void load_extra_string_files();
	void load_extra_string_file(const StringFile &stringFile);
	void parse_header_le16(FileBuffer *fb, uint16 *val);
	uint8 parse_vm_instruction(FileBuffer *fb, Instruction *instr);
	void parse_function(FileBuffer *fb, Function *func);
	void parse_vm(FileBuffer *fb);
	void parse_action_tables(FileBuffer *fb);
	void parse_dictionary(FileBuffer *fb);
	void parse_word_map(FileBuffer *fb);
	void parse_items(FileBuffer *fb);
	void parse_rooms(FileBuffer *fb);
	uint64 string_get_chunk(uint8 *string);
	char decode_string_elem(uint8 c, bool capital, bool special);

	void parse_string_table(FileBuffer *fb, uint start_addr,
		uint32 end_addr, StringTable *table);
	void parse_variables(FileBuffer *fb);
	void parse_flags(FileBuffer *fb);
	void parse_replace_words(FileBuffer *fb);

	void loadGameData();

protected:
	/**
	 * Game strings are stored using 5-bit characters. By default a character
	 * value maps to the lower-case letter table. If a character has the value 0x1e
	 * then the next character is upper-case. An upper-case space is used to
	 * specify that the character should be replaced at runtime (like a '%s'
	 * specifier). If a character has the value 0x1f then the next character is
	 * taken from the symbols table.
	 */
	Common::String parseString(FileBuffer *fb);

	/**
	 * The main game data file header has the offsets for where each bit of
	 * game data is. The offsets have a magic constant value added to them.
	 */
	virtual void parse_header(FileBuffer *fb);

public:
	GameData() {
		clearGame();
	}
	virtual ~GameData() {
		clearGame();
	}

	void clearGame();
	void loadGame();
};

} // namespace Comprehend
} // namespace Glk

#endif
