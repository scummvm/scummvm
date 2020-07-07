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

class ComprehendGame : public GameData, public OpcodeMap {
private:
	bool _ended;
public:
	const GameStrings *_gameStrings;

private:
	Item *get_item_by_noun(byte noun);
	void describe_objects_in_current_room();
	void func_set_test_result(FunctionState *func_state, bool value);
	size_t num_objects_in_room(int room);
	void eval_instruction(FunctionState *func_state, const Instruction *instr,
		const Sentence *sentence);
	void skip_whitespace(char **p);
	void skip_non_whitespace(char **p);
	bool handle_sentence(Sentence *sentence);
	bool handle_sentence(uint tableNum, Sentence *sentence, Common::Array<byte> &words);
	void read_sentence(char **line, Sentence *sentence);
	void parse_sentence_word_pairs(Sentence *sentence);
	void doBeforeTurn();
	void doAfterTurn();
	void read_input();
	void doMovementVerb(uint verbNum);
	bool isItemPresent(Item *item) const;

protected:
	void game_save();
	void game_restore();
	void game_restart() {
		_ended = true;
	}
	virtual bool handle_restart();
	int console_get_key();
	void console_println(const char *text);
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
	void eval_function(const Function &func, const Sentence *sentence);

	void parse_header(FileBuffer *fb) override {
		GameData::parse_header(fb);
		loadOpcodes(_comprehendVersion);
	}

public:
	ComprehendGame();
	virtual ~ComprehendGame();

	virtual void beforeGame() {}
	virtual void beforePrompt() {}
	virtual void beforeTurn() {}
	virtual bool afterTurn() {
		return false;
	}
	virtual int roomIsSpecial(unsigned room_index, unsigned *room_desc_string) {
		return ROOM_IS_NORMAL;
	}
	virtual void handleSpecialOpcode(uint8 operand) {}

	virtual void synchronizeSave(Common::Serializer &s);

	Common::String stringLookup(uint16 index);
	Common::String instrStringLookup(uint8 index, uint8 table);

	void playGame();

	void move_to(uint8 room);
	Room *get_room(uint16 index);
	Item *get_item(uint16 index);
	void update();
	void update_graphics();

	/**
	 * Gets a random number
	 */
	uint getRandomNumber(uint max) const;
};

} // namespace Comprehend
} // namespace Glk

#endif
