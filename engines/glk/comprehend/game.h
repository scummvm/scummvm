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

#ifndef GLK_COMPREHEND_GAME_H
#define GLK_COMPREHEND_GAME_H

#include "glk/comprehend/game_data.h"
#include "glk/comprehend/opcode_map.h"
#include "common/array.h"
#include "common/serializer.h"

namespace Glk {
namespace Comprehend {

#define ROOM_IS_NORMAL 0
#define ROOM_IS_DARK 1
#define ROOM_IS_TOO_BRIGHT 2

struct GameStrings;
struct Sentence;

class ComprehendGame : public GameInfo, public OpcodeMap {
public:
	Common::String _gameDataFile;
	Common::Array<StringFile> _stringFiles;
	Common::StringArray _locationGraphicFiles;
	Common::StringArray _itemGraphicFiles;
	Common::String _titleGraphicFile;
	unsigned _colorTable;

	const GameStrings *_gameStrings;

private:
	WordIndex *is_word_pair(Word *word1, Word *word2);
	Item *get_item_by_noun(Word *noun);
	void update_graphics();
	void describe_objects_in_current_room();
	void update();
	void move_to(uint8 room);
	void func_set_test_result(FunctionState *func_state, bool value);
	size_t num_objects_in_room(int room);
	void eval_instruction(FunctionState *func_state, Instruction *instr,
		Word *verb, Word *noun);
	void skip_whitespace(char **p);
	void skip_non_whitespace(char **p);
	bool handle_sentence(Sentence *sentence);
	void read_sentence(char **line, Sentence *sentence);
	void doBeforeTurn();
	void doAfterTurn();
	void read_input();

protected:
	void game_save();
	void game_restore();
	void game_restart();
	int console_get_key();
	void console_println(const char *text);
	Room *get_room(uint16 index);
	Item *get_item(uint16 index);
	void move_object(Item *item, int new_room);

	/*
	 * Comprehend functions consist of test and command instructions (if the MSB
	 * of the opcode is set then it is a command). Functions are parsed by
	 * evaluating each test until a command instruction is encountered. If the
	 * overall result of the tests was true then the command instructions are
	 * executed until either a test instruction is found or the end of the function
	 * is reached. Otherwise the commands instructions are skipped over and the
	 * next test sequence (if there is one) is tried.
	 */
	void eval_function(Function *func, Word *verb, Word *noun);

public:
	ComprehendGame();
	virtual ~ComprehendGame();

	virtual void beforeGame() {}
	virtual void beforePrompt() {}
	virtual bool beforeTurn() {
		return false;
	}
	virtual bool afterTurn() {
		return false;
	}
	virtual int roomIsSpecial(unsigned room_index, unsigned *room_desc_string) {
		return ROOM_IS_NORMAL;
	}
	virtual void handleSpecialOpcode(uint8 operand) {}

	void synchronizeSave(Common::Serializer &s);

	Common::String stringLookup(uint16 index);
	Common::String instrStringLookup(uint8 index, uint8 table);

	void playGame();
};

void console_println(ComprehendGame *game, const char *text);
int console_get_key(void);

Item *get_item(ComprehendGame *game, uint16 index);
void move_object(ComprehendGame *game, Item *item, int new_room);
void eval_function(ComprehendGame *game, Function *func,
                   Word *verb, Word *noun);

void comprehend_play_game(ComprehendGame *game);
void game_save(ComprehendGame *game);
void game_restore(ComprehendGame *game);
void game_restart(ComprehendGame *game);

} // namespace Comprehend
} // namespace Glk

#endif
