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

namespace Glk {
namespace Comprehend {

void Sentence::clear() {
	for (uint idx = 0; idx < 4; ++idx)
		_words[idx].clear();
	for (uint idx = 0; idx < 6; ++idx)
		_formattedWords[idx] = 0;

	_nr_words = 0;
	_specialOpcodeVal2 = 0;
}

void Sentence::copyFrom(const Sentence &src, bool copyNoun) {
	for (uint idx = (copyNoun ? 0 : 1); idx < 6; ++idx)
		_formattedWords[idx] = src._formattedWords[idx];
}

void Sentence::format() {
	for (uint idx = 0; idx < 6; ++idx)
		_formattedWords[idx] = 0;
	byte wordTypes[5] = { 0, 0, 0, 0, 0 };

	for (uint idx = 0; idx < _nr_words; ++idx) {
		const Word &w = _words[idx];

		if (w._type & 8) {
			if (w._type < 24) {
				int index, type;

				if (w._type & 0xf0 & wordTypes[2]) {
					index = _formattedWords[2];
					type = wordTypes[2];
				} else if (w._type & 0xf0 & wordTypes[3]) {
					index = _formattedWords[3];
					type = wordTypes[3];
				} else {
					continue;
				}

				if (!_formattedWords[2]) {
					_formattedWords[2] = index;
					wordTypes[2] = type;
				} else if (!_formattedWords[3]) {
					_formattedWords[3] = index;
					wordTypes[3] = type;
				}
			} else {
				if (w._type == 8)
					_specialOpcodeVal2 = 1;
				else if (w._type == 9)
					_specialOpcodeVal2 = 2;
			}
		} else {
			int val = w._type & 0xf0;

			if (val) {
				if ((w._type & 1) && !_formattedWords[0]) {
					_formattedWords[0] = w._index;
				} else if (!_formattedWords[2]) {
					_formattedWords[2] = w._index;
					wordTypes[2] = val;
				} else if (!_formattedWords[3]) {
					_formattedWords[3] = w._index;
					wordTypes[3] = val;
				}
			} else if (w._type & 1) {
				if (!_formattedWords[0]) {
					_formattedWords[0] = w._index;
				} else if (!_formattedWords[1]) {
					_formattedWords[1] = w._index;
				}
			} else if (w._type == 2) {
				if (!_formattedWords[4])
					_formattedWords[4] = w._index;
			} else if (w._type == 4) {
				if (!_formattedWords[5])
					_formattedWords[5] = w._index;
			}
		}
	}
}

/*-------------------------------------------------------*/


ComprehendGame::ComprehendGame() : _gameStrings(nullptr), _ended(false),
		_functionNum(0), _specialOpcode(0), _nounState(NOUNSTATE_INITIAL),
		_inputLineIndex(0), _currentRoomCopy(-1), _redoLine(REDO_NONE) {
	Common::fill(&_inputLine[0], &_inputLine[INPUT_LINE_SIZE], 0);
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
		s.syncAsUint16LE(_rooms[i]._stringDesc);
		for (dir = 0; dir < NR_DIRECTIONS; dir++)
			s.syncAsByte(_rooms[i]._direction[dir]);

		s.syncAsByte(_rooms[i]._flags);
		s.syncAsByte(_rooms[i]._graphic);
	}

	// Objects
	nr_items = _items.size();
	s.syncAsByte(nr_items);
	assert(nr_items == _items.size());

	for (i = 0; i < _items.size(); ++i)
		_items[i].synchronize(s);

	_redoLine = REDO_NONE;
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
	return g_comprehend->readChar();
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
	if (g_comprehend->shouldQuit())
		return;

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
	if (g_comprehend->shouldQuit())
		return;

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

bool ComprehendGame::handle_restart() {
	console_println(stringLookup(_gameStrings->game_restart).c_str());
	_ended = false;

	if (tolower(console_get_key()) == 'r') {
		loadGame();
		_updateFlags = UPDATE_ALL;
		return true;
	} else {
		g_comprehend->quitGame();
		return false;
	}
}

Item *ComprehendGame::get_item_by_noun(byte noun) {
	uint i;

	if (!noun)
		return nullptr;

	/*
	 * FIXME - in oo-topos the word 'box' matches more than one object
	 *         (the box and the snarl-in-a-box). The player is unable
	 *         to drop the latter because this will match the former.
	 */
	for (i = 0; i < _items.size(); i++)
		if (_items[i]._word == noun)
			return &_items[i];

	return NULL;
}

int ComprehendGame::get_item_id(byte noun) {
	for (int i = 0; i < (int)_items.size(); i++)
		if (_items[i]._word == noun)
			return i;

	return -1;
}

void ComprehendGame::update_graphics() {
	Item *item;
	Room *room;
	int type;
	uint i;

	if (!g_comprehend->isGraphicsEnabled())
		return;

	type = roomIsSpecial(_currentRoomCopy, NULL);

	switch (type) {
	case ROOM_IS_DARK:
		if (_updateFlags & UPDATE_GRAPHICS)
			g_comprehend->clearScreen(false);
		break;

	case ROOM_IS_TOO_BRIGHT:
		if (_updateFlags & UPDATE_GRAPHICS)
			g_comprehend->clearScreen(true);
		break;

	default:
		if (_updateFlags & UPDATE_GRAPHICS) {
			room = get_room(_currentRoom);
			g_comprehend->drawLocationPicture(room->_graphic - 1);
		}

		if ((_updateFlags & UPDATE_GRAPHICS) ||
		        (_updateFlags & UPDATE_GRAPHICS_ITEMS)) {
			for (i = 0; i < _items.size(); i++) {
				item = &_items[i];

				if (item->_room == _currentRoom &&
				        item->_graphic != 0)
					g_comprehend->drawItemPicture(item->_graphic - 1);
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

		if (item->_room == _currentRoom && item->_stringDesc != 0
				&& !(item->_flags & ITEMF_INVISIBLE))
			count++;
	}

	if (count > 0) {
		console_println(stringLookup(STRING_YOU_SEE).c_str());

		for (i = 0; i < _items.size(); i++) {
			item = &_items[i];

			if (item->_room == _currentRoom && item->_stringDesc != 0
					&& !(item->_flags & ITEMF_INVISIBLE))
				console_println(stringLookup(item->_stringDesc).c_str());
		}
	}
}

void ComprehendGame::updateRoomDesc() {
	Room *room = get_room(_currentRoom);
	uint room_desc_string = room->_stringDesc;
	roomIsSpecial(_currentRoom, &room_desc_string);

	Common::String desc = stringLookup(room_desc_string);
	g_comprehend->printRoomDesc(desc);
}

void ComprehendGame::update() {
	Room *room = get_room(_currentRoom);
	unsigned room_type, room_desc_string;

	update_graphics();

	/* Check if the room is special (dark, too bright, etc) */
	room_desc_string = room->_stringDesc;
	room_type = roomIsSpecial(_currentRoom,
	                                &room_desc_string);

	if (_updateFlags & UPDATE_ROOM_DESC) {
		Common::String desc = stringLookup(room_desc_string);
		console_println(desc.c_str());
		g_comprehend->printRoomDesc(desc.c_str());
	}

	if ((_updateFlags & UPDATE_ITEM_LIST) && room_type == ROOM_IS_NORMAL)
		describe_objects_in_current_room();

	_updateFlags = 0;
}

void ComprehendGame::move_to(uint8 room) {
	if (room >= (int)_rooms.size())
		error("Attempted to move to invalid room %.2x\n", room);

	_currentRoom = _currentRoomCopy = room;
	_updateFlags = (UPDATE_GRAPHICS | UPDATE_ROOM_DESC |
	                      UPDATE_ITEM_LIST);
}

size_t ComprehendGame::num_objects_in_room(int room) {
	size_t count = 0, i;

	for (i = 0; i < _items.size(); i++)
		if (_items[i]._room == room)
			count++;

	return count;
}

void ComprehendGame::move_object(Item *item, int new_room) {
	unsigned obj_weight = item->_flags & ITEMF_WEIGHT_MASK;

	if (item->_room == new_room)
		return;

	if (item->_room == ROOM_INVENTORY) {
		/* Removed from player's inventory */
		_variables[VAR_INVENTORY_WEIGHT] -= obj_weight;
	}
	if (new_room == ROOM_INVENTORY) {
		/* Moving to the player's inventory */
		_variables[VAR_INVENTORY_WEIGHT] += obj_weight;
	}

	if (item->_room == _currentRoom) {
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

	item->_room = new_room;
}

void ComprehendGame::eval_instruction(FunctionState *func_state,
		const Function &func, uint functionOffset, const Sentence *sentence) {

	const Instruction *instr = &func[functionOffset];

	if (DebugMan.isDebugChannelEnabled(kDebugScripts)) {
		Common::String line;
		if (!instr->_isCommand) {
			line += "? ";
		} else {
			if (func_state->_testResult)
				line += "+ ";
			else
				line += "- ";
		}

		line += Common::String::format("%.2x  ", functionOffset);
		line += g_debugger->dumpInstruction(this, func_state, instr);
		debugC(kDebugScripts, "%s", line.c_str());
	}

	if (func_state->_orCount)
		func_state->_orCount--;

	if (instr->_isCommand) {
		bool do_command;

		func_state->_inCommand = true;
		do_command = func_state->_testResult;

		if (func_state->_orCount != 0)
			g_comprehend->print("Warning: or_count == %d\n",
			                    func_state->_orCount);
		func_state->_orCount = 0;

		if (!do_command)
			return;

		func_state->_elseResult = false;
		func_state->_executed = true;

	} else {
		if (func_state->_inCommand) {
			/* Finished command sequence - clear test result */
			func_state->_inCommand = false;
			func_state->_testResult = false;
			func_state->_and = false;
		}
	}

	execute_opcode(instr, sentence, func_state);
}

void ComprehendGame::eval_function(uint functionNum, const Sentence *sentence) {
	FunctionState func_state;
	uint i;

	const Function &func = _functions[functionNum];
	func_state._elseResult = true;
	func_state._executed = false;

	debugC(kDebugScripts, "Start of function %.4x", functionNum);

	for (i = 0; i < func.size(); i++) {
		if (func_state._executed && !func[i]._isCommand) {
			/*
			 * At least one command has been executed and the
			 * current instruction is a test. Exit the function.
			 */
			break;
		}

		eval_instruction(&func_state, func, i, sentence);
	}

	debugC(kDebugScripts, "End of function %.4x\n", functionNum);
}

void ComprehendGame::skip_whitespace(const char **p) {
	while (**p && Common::isSpace(**p))
		(*p)++;
}

void ComprehendGame::skip_non_whitespace(const char **p) {
	while (**p && !Common::isSpace(**p) && **p != ',' && **p != '\n')
		(*p)++;
}

bool ComprehendGame::handle_sentence(Sentence *sentence) {
	if (sentence->_nr_words == 1 && !strcmp(sentence->_words[0]._word, "quit")) {
		g_comprehend->quitGame();
		return true;
	}

	// Set up default sentence
	Common::Array<byte> words;
	const byte *src = &sentence->_formattedWords[0];

	if (src[1] && src[3]) {
		words.clear();

		for (int idx = 0; idx < 4; ++idx)
			words.push_back(src[idx]);

		if (handle_sentence(0, sentence, words))
			return true;
	}

	if (src[1]) {
		words.clear();

		for (int idx = 0; idx < 3; ++idx)
			words.push_back(src[idx]);

		if (handle_sentence(1, sentence, words))
			return true;
	}

	if (src[3] && src[4]) {
		words.clear();

		words.push_back(src[4]);
		words.push_back(src[0]);
		words.push_back(src[2]);
		words.push_back(src[3]);

		if (handle_sentence(2, sentence, words))
			return true;
	}

	if (src[4]) {
		words.clear();

		words.push_back(src[4]);
		words.push_back(src[0]);
		words.push_back(src[2]);

		if (handle_sentence(3, sentence, words))
			return true;
	}

	if (src[3]) {
		words.clear();

		words.push_back(src[0]);
		words.push_back(src[2]);
		words.push_back(src[3]);

 		if (handle_sentence(4, sentence, words))
			return true;
	}

	if (src[2]) {
		words.clear();

		words.push_back(src[0]);
		words.push_back(src[2]);

		if (handle_sentence(5, sentence, words))
			return true;
	}

	if (src[0]) {
		words.clear();
		words.push_back(src[0]);

		if (handle_sentence(6, sentence, words))
			return true;
	}

	return false;
}

bool ComprehendGame::handle_sentence(uint tableNum, Sentence *sentence, Common::Array<byte> &words) {
	const ActionTable &table = _actions[tableNum];

	for (uint i = 0; i < table.size(); i++) {
		const Action &action = table[i];

		// Check for a match on the words of the action
		bool isMatch = true;
		for (uint idx = 0; idx < action._nr_words && isMatch; ++idx)
			isMatch = action._words[idx] == words[idx];

		if (isMatch) {
			// Match
			_functionNum = action._function;
			return true;
		}
	}

	// No matching action
	return false;
}

void ComprehendGame::handleAction(Sentence *sentence) {
	_specialOpcode = 0;

	if (_functionNum == 0) {
		console_println(stringLookup(STRING_DONT_UNDERSTAND).c_str());
	} else {
		eval_function(_functionNum, sentence);
		_functionNum = 0;
		eval_function(0, nullptr);
	}

	handleSpecialOpcode();
}

void ComprehendGame::read_sentence(Sentence *sentence) {
	bool sentence_end = false;
	const char *word_string, *p = &_inputLine[_inputLineIndex];
	Word *word;

	sentence->clear();
	for (;;) {
		// Get the next word
		skip_whitespace(&p);
		word_string = p;
		skip_non_whitespace(&p);

		Common::String wordStr(word_string, p);

		// Check for end of sentence
		// FIXME: The below is a hacked simplified version of how the
		// original handles cases like "get item1, item2"
		if (*p == ',' || *p == '\n' || wordStr.equalsIgnoreCase("and")) {
			// Sentence separator
			++p;
			sentence_end = true;
		} else if (*p == '\0') {
			sentence_end = true;
		}

		/* Find the dictionary word for this */
		word = dict_find_word_by_string(this, wordStr.c_str());
		if (!word)
			sentence->_words[sentence->_nr_words].clear();
		else
			sentence->_words[sentence->_nr_words] = *word;

		sentence->_nr_words++;

		if (sentence->_nr_words >= ARRAY_SIZE(sentence->_words) ||
		        sentence_end)
			break;
	}

	parse_sentence_word_pairs(sentence);
	sentence->format();

	_inputLineIndex = p - _inputLine;
}

void ComprehendGame::parse_sentence_word_pairs(Sentence *sentence) {
	if (sentence->_nr_words < 2)
		return;

	// Iterate through the pairs
	for (uint idx = 0; idx < _wordMaps.size(); ++idx) {
		for (int firstWord = 0; firstWord < (int)sentence->_nr_words - 1; ++firstWord) {
			for (int secondWord = firstWord + 1; secondWord < (int)sentence->_nr_words; ) {
				if (sentence->_words[firstWord] == _wordMaps[idx]._word[0] &&
					sentence->_words[secondWord] == _wordMaps[idx]._word[1]) {
					// Found a word pair match
					// Delete the second word
					for (; secondWord < (int)sentence->_nr_words - 1; ++secondWord)
						sentence->_words[secondWord] = sentence->_words[secondWord + 1];

					sentence->_words[sentence->_nr_words - 1].clear();
					sentence->_nr_words--;

					// Replace the first word with the target
					sentence->_words[firstWord] = _wordMaps[idx]._word[2];
				} else {
					// Move to next word
					++secondWord;
				}
			}
		}
	}
}

void ComprehendGame::doBeforeTurn() {
	// Make  a copy of the current room
	_currentRoomCopy = _currentRoom;

	beforeTurn();

	if (!_ended)
		update();
}

void ComprehendGame::beforeTurn() {
	// Run the each turn functions
	eval_function(0, nullptr);
}

void ComprehendGame::read_input() {
	Sentence tempSentence;
	bool handled;

turn:
	doBeforeTurn();
	if (_ended)
		return;

	// If we're in full screen text, we can afford a blank row between
	// any game response and the next line of text
	if (!g_comprehend->isGraphicsEnabled())
		g_comprehend->print("\n");

	beforePrompt();

	for (;;) {
		_redoLine = REDO_NONE; 
		g_comprehend->print("> ");
		g_comprehend->readLine(_inputLine, INPUT_LINE_SIZE);
		if (g_comprehend->shouldQuit())
			return;

		_inputLineIndex = 0;
		if (strlen(_inputLine) == 0) {
			// Empty line, so toggle picture window visibility
			if (!g_comprehend->toggleGraphics())
				updateRoomDesc();
			g_comprehend->print(_("Picture window toggled\n"));

			_updateFlags |= UPDATE_GRAPHICS;
			update_graphics();
			continue;
		}

		afterPrompt();

		if (_redoLine == REDO_NONE)
			break;
		else if (_redoLine == REDO_TURN)
			goto turn;
	}

	for (;;) {
		NounState prevNounState = _nounState;
		_nounState = NOUNSTATE_STANDARD;

		read_sentence(&tempSentence);
		_sentence.copyFrom(tempSentence, tempSentence._formattedWords[0] || prevNounState != NOUNSTATE_STANDARD);

		handled = handle_sentence(&_sentence);
		handleAction(&_sentence);

		if (!handled)
			return;

		/* FIXME - handle the 'before you can continue' case */
		if (_inputLine[_inputLineIndex] == '\0')
			break;
	}

	afterTurn();
}

void ComprehendGame::playGame() {
	if (!g_comprehend->loadLauncherSavegameIfNeeded())
		beforeGame();

	_updateFlags = (uint)UPDATE_ALL;
	while (!g_comprehend->shouldQuit()) {
		read_input();

		if (_ended && !handle_restart())
			break;
	}
}

uint ComprehendGame::getRandomNumber(uint max) const {
	return g_comprehend->getRandomNumber(max);
}

void ComprehendGame::doMovementVerb(uint verbNum) {
	assert(verbNum >= 1 && verbNum <= NR_DIRECTIONS);
	Room *room = get_room(_currentRoom);
	byte newRoom = room->_direction[verbNum - 1];

	if (newRoom)
		move_to(newRoom);
	else
		console_println(_strings[0].c_str());
}

void ComprehendGame::weighInventory() {
	_totalInventoryWeight = 0;
	if (!g_debugger->_invLimit)
		// Allow for an unlimited number of items in inventory
		return;

	for (int idx = _itemCount - 1; idx > 0; --idx) {
		Item *item = get_item(idx);
		if (item->_room == ROOM_INVENTORY)
			_totalInventoryWeight += item->_flags & ITEMF_WEIGHT_MASK;
	}
}

} // namespace Comprehend
} // namespace Glk
