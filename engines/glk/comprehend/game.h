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
#include "common/array.h"
#include "common/serializer.h"

namespace Glk {
namespace Comprehend {

#define ROOM_IS_NORMAL 0
#define ROOM_IS_DARK 1
#define ROOM_IS_TOO_BRIGHT 2
#define INPUT_LINE_SIZE 1024

enum NounState { NOUNSTATE_STANDARD = 0, NOUNSTATE_QUERY = 1, NOUNSTATE_INITIAL = 2 };

enum RedoLine { REDO_NONE, REDO_PROMPT, REDO_TURN };

struct GameStrings;
struct Sentence;

struct Sentence {
	Word _words[20];
	size_t _nr_words;
	byte _formattedWords[6];
	byte _specialOpcodeVal2;

	Sentence() {
		clear();
	}

	bool empty() const {
		return !_formattedWords[0];
	}

	/**
	 * Clears the sentence
	 */
	void clear();

	/**
	 * Copies from another sentence to this one
	 */
	void copyFrom(const Sentence &src, bool copyNoun = true);

	/**
	 * Splits up the array of _words into a _formattedWords
	 * array, placing the words in appropriate noun, verb, etc.
	 * positions appropriately
	 */
	void format();
};

class ComprehendGame : public GameData {
protected:
	bool _ended;
	NounState _nounState;
	Sentence _sentence;
	char _inputLine[INPUT_LINE_SIZE];
	int _inputLineIndex;
	int _currentRoomCopy;
	int _functionNum;
	int _specialOpcode;
	RedoLine _redoLine;
public:
	const GameStrings *_gameStrings;

private:
	void describe_objects_in_current_room();
	void eval_instruction(FunctionState *func_state,
		const Function &func, uint functionOffset,
		const Sentence *sentence);
	void skip_whitespace(const char **p);
	void skip_non_whitespace(const char **p);
	bool handle_sentence(Sentence *sentence);
	bool handle_sentence(uint tableNum, Sentence *sentence, Common::Array<byte> &words);
	void read_sentence(Sentence *sentence);
	void parse_sentence_word_pairs(Sentence *sentence);
	void read_input();
	void doBeforeTurn();

protected:
	void game_save();
	void game_restore();
	void game_restart() {
		_ended = true;
	}
	virtual bool handle_restart();

	virtual void execute_opcode(const Instruction *instr, const Sentence *sentence,
		FunctionState *func_state) = 0;

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
	void eval_function(uint functionNum, const Sentence *sentence);

	void parse_header(FileBuffer *fb) override {
		GameData::parse_header(fb);
	}

	Item *get_item_by_noun(byte noun);
	int get_item_id(byte noun);
	void weighInventory();
	size_t num_objects_in_room(int room);
	void doMovementVerb(uint verbNum);

public:
	ComprehendGame();
	virtual ~ComprehendGame();

	/**
	 * Called before the game starts
	 */
	virtual void beforeGame() {}

	/**
	 * Called just before the prompt for user input
	 */
	virtual void beforePrompt() {}

	/**
	 * Called after input has been entered.
	 */
	virtual void afterPrompt() {}

	/**
	 * Called before the start of a game turn
	 */
	virtual void beforeTurn();

	/**
	 * Called at the end of a game turn
	 */
	virtual void afterTurn() {}

	/**
	 * Called when an action function has been selected
	 */
	virtual void handleAction(Sentence *sentence);

	virtual int roomIsSpecial(unsigned room_index, unsigned *room_desc_string) {
		return ROOM_IS_NORMAL;
	}
	virtual void handleSpecialOpcode() {}

	virtual void synchronizeSave(Common::Serializer &s);

	virtual ScriptOpcode getScriptOpcode(const Instruction *instr) = 0;

	Common::String stringLookup(uint16 index);
	Common::String instrStringLookup(uint8 index, uint8 table);

	virtual void playGame();

	void move_to(uint8 room);
	Room *get_room(uint16 index);
	Item *get_item(uint16 index);
	void updateRoomDesc();
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
