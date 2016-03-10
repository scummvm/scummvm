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

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/error.h"
#include "common/file.h"
#include "common/system.h"
#include "common/events.h"
#include "common/stream.h"
#include "common/savefile.h"

#include "engines/util.h"

#include "graphics/palette.h"
#include "graphics/thumbnail.h"

#include "adl/adl.h"
#include "adl/display.h"
#include "adl/detection.h"

namespace Adl {

AdlEngine::~AdlEngine() {
	delete _display;
}

AdlEngine::AdlEngine(OSystem *syst, const AdlGameDescription *gd) :
		Engine(syst),
		_display(nullptr),
		_gameDescription(gd),
		_isRestarting(false),
		_isRestoring(false),
		_saveVerb(0),
		_saveNoun(0),
		_restoreVerb(0),
		_restoreNoun(0),
		_canSaveNow(false),
		_canRestoreNow(false) {
}

Common::String AdlEngine::readString(Common::ReadStream &stream, byte until) const {
	Common::String str;

	while (1) {
		byte b = stream.readByte();

		if (stream.eos() || stream.err())
			error("Error reading string");

		if (b == until)
			break;

		str += b;
	};

	return str;
}

Common::String AdlEngine::readStringAt(Common::SeekableReadStream &stream, uint offset, byte until) const {
	stream.seek(offset);
	return readString(stream, until);
}

void AdlEngine::printMessage(uint idx, bool wait) const {
	Common::String msg = _messages[idx - 1];
	wordWrap(msg);
	_display->printString(msg);

	if (wait)
		delay(14 * 166018 / 1000);
}

void AdlEngine::delay(uint32 ms) const {
	Common::EventManager *ev = g_system->getEventManager();

	uint32 start = g_system->getMillis();

	while (!g_engine->shouldQuit() && g_system->getMillis() - start < ms) {
		Common::Event event;
		if (ev->pollEvent(event)) {
			if (event.type == Common::EVENT_KEYDOWN && (event.kbd.flags & Common::KBD_CTRL)) {
				switch(event.kbd.keycode) {
				case Common::KEYCODE_q:
					g_engine->quitGame();
					break;
				default:
					break;
				}
			}
		}
		g_system->delayMillis(16);
	}
}

Common::String AdlEngine::inputString(byte prompt) const {
	Common::String s;

	if (prompt > 0)
		_display->printString(Common::String(prompt));

	while (1) {
		byte b = inputKey();

		if (g_engine->shouldQuit() || _isRestoring)
			return 0;

		if (b == 0)
			continue;

		if (b == ('\r' | 0x80)) {
			s += b;
			_display->printString(Common::String(b));
			return s;
		}

		if (b < 0xa0) {
			switch (b) {
			case Common::KEYCODE_BACKSPACE | 0x80:
				if (!s.empty()) {
					_display->moveCursorBackward();
					_display->setCharAtCursor(APPLECHAR(' '));
					s.deleteLastChar();
				}
				break;
			};
		} else {
			if (s.size() < 255) {
				s += b;
				_display->printString(Common::String(b));
			}
		}
	}
}

byte AdlEngine::inputKey() const {
	Common::EventManager *ev = g_system->getEventManager();

	byte key = 0;

	_display->showCursor(true);

	while (!g_engine->shouldQuit() && !_isRestoring && key == 0) {
		Common::Event event;
		if (ev->pollEvent(event)) {
			if (event.type != Common::EVENT_KEYDOWN)
				continue;

			if (event.kbd.flags & Common::KBD_CTRL) {
				if (event.kbd.keycode == Common::KEYCODE_q)
					g_engine->quitGame();
				continue;
			}

			switch (event.kbd.keycode) {
			case Common::KEYCODE_BACKSPACE:
			case Common::KEYCODE_RETURN:
				key = convertKey(event.kbd.keycode);
				break;
			default:
				if (event.kbd.ascii >= 0x20 && event.kbd.ascii < 0x80)
					key = convertKey(event.kbd.ascii);
			};
		}

		_display->updateTextScreen();
		g_system->delayMillis(16);
	}

	_display->showCursor(false);

	return key;
}

void AdlEngine::loadWords(Common::ReadStream &stream, WordMap &map) const {
	uint index = 0;

	while (1) {
		++index;

		byte buf[IDI_WORD_SIZE];

		if (stream.read(buf, IDI_WORD_SIZE) < IDI_WORD_SIZE)
			error("Error reading word list");

		Common::String word((char *)buf, IDI_WORD_SIZE);

		if (!map.contains(word))
			map[word] = index;

		byte synonyms = stream.readByte();

		if (stream.err() || stream.eos())
			error("Error reading word list");

		if (synonyms == 0xff)
			break;

		for (uint i = 0; i < synonyms; ++i) {
			if (stream.read((char *)buf, IDI_WORD_SIZE) < IDI_WORD_SIZE)
				error("Error reading word list");

			word = Common::String((char *)buf, IDI_WORD_SIZE);

			if (!map.contains(word))
				map[word] = index;
		}
	}
}

void AdlEngine::readCommands(Common::ReadStream &stream, Commands &commands) {
	while (1) {
		Command command;
		command.room = stream.readByte();

		if (command.room == 0xff)
			return;

		command.verb = stream.readByte();
		command.noun = stream.readByte();

		byte scriptSize = stream.readByte() - 6;

		command.numCond = stream.readByte();
		command.numAct = stream.readByte();

		for (uint i = 0; i < scriptSize; ++i)
			command.script.push_back(stream.readByte());

		if (stream.eos() || stream.err())
			error("Failed to read commands");

		if (command.numCond == 0 && command.script[0] == IDO_ACT_SAVE) {
			_saveVerb = command.verb;
			_saveNoun = command.noun;
		}

		if (command.numCond == 0 && command.script[0] == IDO_ACT_LOAD) {
			_restoreVerb = command.verb;
			_restoreNoun = command.noun;
		}

		commands.push_back(command);
	}
}

void AdlEngine::clearScreen() const {
	_display->setMode(DISPLAY_MODE_MIXED);
	_display->clear(0x00);
}

void AdlEngine::drawItems() const {
	Common::Array<Item>::const_iterator item;

	uint dropped = 0;

	for (item = _state.items.begin(); item != _state.items.end(); ++item) {
		if (item->room != _state.room)
			continue;

		if (item->state == IDI_ITEM_MOVED) {
			if (getCurRoom().picture == getCurRoom().curPicture) {
				const Common::Point &p =  _itemOffsets[dropped];
				if (item->isLineArt)
					drawLineArt(_lineArt[item->picture - 1], p);
				else
					drawPic(item->picture, p);
				++dropped;
			}
			continue;
		}

		Common::Array<byte>::const_iterator pic;

		for (pic = item->roomPictures.begin(); pic != item->roomPictures.end(); ++pic) {
			if (*pic == getCurRoom().curPicture) {
				if (item->isLineArt)
					drawLineArt(_lineArt[item->picture - 1], item->position);
				else
					drawPic(item->picture, item->position);
				continue;
			}
		}
	}
}

void AdlEngine::drawNextPixel(Common::Point &p, byte color, byte bits, byte quadrant) const {
	if (bits & 4)
		_display->putPixel(p, color);

	bits += quadrant;

	if (bits & 1)
		p.x += (bits & 2 ? -1 : 1);
	else
		p.y += (bits & 2 ? 1 : -1);
}

void AdlEngine::drawLineArt(const Common::Array<byte> &lineArt, const Common::Point &pos, byte rotation, byte scaling, byte color) const {
	const byte stepping[] = {
		0xff, 0xfe, 0xfa, 0xf4, 0xec, 0xe1, 0xd4, 0xc5,
		0xb4, 0xa1, 0x8d, 0x78, 0x61, 0x49, 0x31, 0x18,
		0xff
	};

	byte quadrant = rotation >> 4;
	rotation &= 0xf;
	byte xStep = stepping[rotation];
	byte yStep = stepping[(rotation ^ 0xf) + 1] + 1;

	Common::Point p(pos);

	for (uint i = 0; i < lineArt.size(); ++i) {
		byte b = lineArt[i];

		do {
			byte xFrac = 0x80;
			byte yFrac = 0x80;
			for (uint j = 0; j < scaling; ++j) {
				if (xFrac + xStep + 1 > 255)
					drawNextPixel(p, color, b, quadrant);
				xFrac += xStep + 1;
				if (yFrac + yStep > 255)
					drawNextPixel(p, color, b, quadrant + 1);
				yFrac += yStep;
			}
			b >>= 3;
		} while (b != 0);
	}
}

const Room &AdlEngine::getRoom(uint i) const {
	if (i < 1 || i > _state.rooms.size())
		error("Room %i out of range [1, %i]", i, _state.rooms.size());

	return _state.rooms[i - 1];
}

Room &AdlEngine::getRoom(uint i) {
	if (i < 1 || i > _state.rooms.size())
		error("Room %i out of range [1, %i]", i, _state.rooms.size());

	return _state.rooms[i - 1];
}

const Room &AdlEngine::getCurRoom() const {
	return getRoom(_state.room);
}

Room &AdlEngine::getCurRoom() {
	return getRoom(_state.room);
}

const Item &AdlEngine::getItem(uint i) const {
	if (i < 1 || i > _state.items.size())
		error("Item %i out of range [1, %i]", i, _state.items.size());

	return _state.items[i - 1];
}

Item &AdlEngine::getItem(uint i) {
	if (i < 1 || i > _state.items.size())
		error("Item %i out of range [1, %i]", i, _state.items.size());

	return _state.items[i - 1];
}

byte AdlEngine::getVar(uint i) const {
	if (i >= _state.vars.size())
		error("Variable %i out of range [0, %i]", i, _state.vars.size() - 1);

	return _state.vars[i];
}

void AdlEngine::setVar(uint i, byte value) {
	if (i >= _state.vars.size())
		error("Variable %i out of range [0, %i]", i, _state.vars.size() - 1);

	_state.vars[i] = value;
}

void AdlEngine::takeItem(byte noun) {
	Common::Array<Item>::iterator item;

	for (item = _state.items.begin(); item != _state.items.end(); ++item) {
		if (item->noun != noun || item->room != _state.room)
			continue;

		if (item->state == IDI_ITEM_DOESNT_MOVE) {
			printMessage(_messageIds.itemDoesntMove);
			return;
		}

		if (item->state == IDI_ITEM_MOVED) {
			item->room = IDI_NONE;
			return;
		}

		Common::Array<byte>::const_iterator pic;
		for (pic = item->roomPictures.begin(); pic != item->roomPictures.end(); ++pic) {
			if (*pic == getCurRoom().curPicture) {
				item->room = IDI_NONE;
				item->state = IDI_ITEM_MOVED;
				return;
			}
		}
	}

	printMessage(_messageIds.itemNotHere);
}

void AdlEngine::dropItem(byte noun) {
	Common::Array<Item>::iterator item;

	for (item = _state.items.begin(); item != _state.items.end(); ++item) {
		if (item->noun != noun || item->room != IDI_NONE)
			continue;

		item->room = _state.room;
		item->state = IDI_ITEM_MOVED;
		return;
	}

	printMessage(_messageIds.dontUnderstand);
}

Common::Error AdlEngine::run() {
	_display = new Display();

	loadData();

	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot >= 0) {
		if (loadGameState(saveSlot).getCode() != Common::kNoError)
			error("Failed to load save game from slot %i", saveSlot);
		_display->moveCursorTo(Common::Point(0, 23));
		_isRestoring = true;
	} else {
		runIntro();
		initState();
	}

	_display->setMode(DISPLAY_MODE_MIXED);
	_display->printAsciiString("\r\r\r\r\r");

	while (1) {
		uint verb = 0, noun = 0;

		// When restoring from the launcher, we don't read
		// input on the first iteration. This is needed to
		// ensure that restoring from the launcher and
		// restoring in-game brings us to the same game state.
		// (Also see comment below.)
		if (!_isRestoring) {
			clearScreen();
			showRoom();

			_canSaveNow = _canRestoreNow = true;
			getInput(verb, noun);
			_canSaveNow = _canRestoreNow = false;

			if (shouldQuit())
				break;

			// If we just restored from the GMM, we skip this command
			// set, as no command has been input by the user
			if (!_isRestoring)
				if (!doOneCommand(_roomCommands, verb, noun))
					printMessage(_messageIds.dontUnderstand);
		}

		if (_isRestoring) {
			// We restored from the GMM or launcher. As restoring
			// with "RESTORE GAME" does not end command processing,
			// we don't break it off here either. This essentially
			// means that restoring a game will always run through
			// the global commands and increase the move counter
			// before the first user input.
			_display->printAsciiString("\r");
			_isRestoring = false;
			verb = _restoreVerb;
			noun = _restoreNoun;
		}

		// Restarting does end command processing
		if (_isRestarting) {
			_isRestarting = false;
			continue;
		}

		doAllCommands(_globalCommands, verb, noun);
		_state.moves++;
	}

	return Common::kNoError;
}

bool AdlEngine::hasFeature(EngineFeature f) const {
	switch (f) {
	case kSupportsLoadingDuringRuntime:
	case kSupportsSavingDuringRuntime:
	case kSupportsRTL:
		return true;
	default:
		return false;
	}
}

Common::Error AdlEngine::loadGameState(int slot) {
	Common::String fileName = Common::String::format("%s.s%02d", _targetName.c_str(), slot);
	Common::InSaveFile *inFile = getSaveFileManager()->openForLoading(fileName);

	if (!inFile) {
		warning("Failed to open file '%s'", fileName.c_str());
		return Common::kUnknownError;
	}

	if (inFile->readUint32BE() != MKTAG('A', 'D', 'L', ':')) {
		warning("No header found in '%s'", fileName.c_str());
		delete inFile;
		return Common::kUnknownError;
	}

	byte saveVersion = inFile->readByte();
	if (saveVersion != SAVEGAME_VERSION) {
		warning("Save game version %i not supported", saveVersion);
		delete inFile;
		return Common::kUnknownError;
	}

	// Skip description
	inFile->seek(SAVEGAME_NAME_LEN, SEEK_CUR);
	// Skip save time
	inFile->seek(6, SEEK_CUR);

	uint32 playTime = inFile->readUint32BE();

	Graphics::skipThumbnail(*inFile);

	initState();

	_state.room = inFile->readByte();
	_state.moves = inFile->readByte();
	_state.isDark = inFile->readByte();

	uint32 size = inFile->readUint32BE();
	if (size != _state.rooms.size())
		error("Room count mismatch (expected %i; found %i)", _state.rooms.size(), size);

	for (uint i = 0; i < size; ++i) {
		_state.rooms[i].picture = inFile->readByte();
		_state.rooms[i].curPicture = inFile->readByte();
	}

	size = inFile->readUint32BE();
	if (size != _state.items.size())
		error("Item count mismatch (expected %i; found %i)", _state.items.size(), size);

	for (uint i = 0; i < size; ++i) {
		_state.items[i].room = inFile->readByte();
		_state.items[i].picture = inFile->readByte();
		_state.items[i].position.x = inFile->readByte();
		_state.items[i].position.y = inFile->readByte();
		_state.items[i].state = inFile->readByte();
	}

	size = inFile->readUint32BE();
	if (size != _state.vars.size())
		error("Variable count mismatch (expected %i; found %i)", _state.vars.size(), size);

	for (uint i = 0; i < size; ++i)
		_state.vars[i] = inFile->readByte();

	if (inFile->err() || inFile->eos())
		error("Failed to load game '%s'", fileName.c_str());

	delete inFile;

	setTotalPlayTime(playTime);

	_isRestoring = true;
	return Common::kNoError;
}

bool AdlEngine::canLoadGameStateCurrently() {
	return _canRestoreNow;
}

Common::Error AdlEngine::saveGameState(int slot, const Common::String &desc) {
	Common::String fileName = Common::String::format("%s.s%02d", _targetName.c_str(), slot);
	Common::OutSaveFile *outFile = getSaveFileManager()->openForSaving(fileName);

	if (!outFile) {
		warning("Failed to open file '%s'", fileName.c_str());
		return Common::kUnknownError;
	}

	outFile->writeUint32BE(MKTAG('A', 'D', 'L', ':'));
	outFile->writeByte(SAVEGAME_VERSION);

	char name[SAVEGAME_NAME_LEN] = { };

	if (!desc.empty())
		strncpy(name, desc.c_str(), sizeof(name) - 1);
	else {
		Common::String defaultName("Save ");
		defaultName += 'A' + slot;
		strncpy(name, defaultName.c_str(), sizeof(name) - 1);
	}

	outFile->write(name, sizeof(name));

	TimeDate t;
	g_system->getTimeAndDate(t);

	outFile->writeUint16BE(t.tm_year);
	outFile->writeByte(t.tm_mon);
	outFile->writeByte(t.tm_mday);
	outFile->writeByte(t.tm_hour);
	outFile->writeByte(t.tm_min);

	uint32 playTime = getTotalPlayTime();
	outFile->writeUint32BE(playTime);

	_display->saveThumbnail(*outFile);

	outFile->writeByte(_state.room);
	outFile->writeByte(_state.moves);
	outFile->writeByte(_state.isDark);

	outFile->writeUint32BE(_state.rooms.size());
	for (uint i = 0; i < _state.rooms.size(); ++i) {
		outFile->writeByte(_state.rooms[i].picture);
		outFile->writeByte(_state.rooms[i].curPicture);
	}

	outFile->writeUint32BE(_state.items.size());
	for (uint i = 0; i < _state.items.size(); ++i) {
		outFile->writeByte(_state.items[i].room);
		outFile->writeByte(_state.items[i].picture);
		outFile->writeByte(_state.items[i].position.x);
		outFile->writeByte(_state.items[i].position.y);
		outFile->writeByte(_state.items[i].state);
	}

	outFile->writeUint32BE(_state.vars.size());
	for (uint i = 0; i < _state.vars.size(); ++i)
		outFile->writeByte(_state.vars[i]);

	outFile->finalize();

	if (outFile->err()) {
		delete outFile;
		warning("Failed to save game '%s'", fileName.c_str());
		return Common::kUnknownError;
	}

	delete outFile;
	return Common::kNoError;
}

bool AdlEngine::canSaveGameStateCurrently() {
	if (!_canSaveNow)
		return false;

	Commands::const_iterator cmd;

	// Here we check whether or not the game currently accepts the command
	// "SAVE GAME". This prevents saving via the GMM in situations where
	// it wouldn't otherwise be possible to do so.
	for (cmd = _roomCommands.begin(); cmd != _roomCommands.end(); ++cmd) {
		uint offset;
		if (matchCommand(*cmd, _saveVerb, _saveNoun, &offset))
			return cmd->script[offset] == IDO_ACT_SAVE;
	}

	return false;
}

void AdlEngine::wordWrap(Common::String &str) const {
	uint end = 39;

	while (1) {
		if (str.size() <= end)
			return;

		while (str[end] != APPLECHAR(' '))
			--end;

		str.setChar(APPLECHAR('\r'), end);
		end += 40;
	}
}

byte AdlEngine::convertKey(uint16 ascii) const {
	ascii = toupper(ascii);

	if (ascii >= 0x80)
		return 0;

	ascii |= 0x80;

	if (ascii >= 0x80 && ascii <= 0xe0)
		return ascii;

	return 0;
}

Common::String AdlEngine::getLine() const {
	// Original engine uses a global here, which isn't reset between
	// calls and may not match actual mode
	bool textMode = false;

	while (1) {
		Common::String line = inputString(APPLECHAR('?'));

		if (shouldQuit() || _isRestoring)
			return "";

		if ((byte)line[0] == ('\r' | 0x80)) {
			textMode = !textMode;
			_display->setMode(textMode ? DISPLAY_MODE_TEXT : DISPLAY_MODE_MIXED);
			continue;
		}

		// Remove the return
		line.deleteLastChar();
		return line;
	}
}

Common::String AdlEngine::getWord(const Common::String &line, uint &index) const {
	Common::String str;

	for (uint i = 0; i < 8; ++i)
		str += APPLECHAR(' ');

	int copied = 0;

	// Skip initial whitespace
	while (1) {
		if (index == line.size())
			return str;
		if (line[index] != APPLECHAR(' '))
			break;
		++index;
	}

	// Copy up to 8 characters
	while (1) {
		if (copied < 8)
			str.setChar(line[index], copied++);

		index++;

		if (index == line.size() || line[index] == APPLECHAR(' '))
			return str;
	}
}

void AdlEngine::getInput(uint &verb, uint &noun) {
	while (1) {
		_display->printString(_strings.enterCommand);
		Common::String line = getLine();

		if (shouldQuit() || _isRestoring)
			return;

		uint index = 0;
		Common::String verbStr = getWord(line, index);

		if (!_verbs.contains(verbStr)) {
			Common::String err = _strings.verbError;
			for (uint i = 0; i < verbStr.size(); ++i)
				err.setChar(verbStr[i], i + 19);
			_display->printString(err);
			continue;
		}

		verb = _verbs[verbStr];

		Common::String nounStr = getWord(line, index);

		if (!_nouns.contains(nounStr)) {
			Common::String err = _strings.nounError;
			for (uint i = 0; i < verbStr.size(); ++i)
				err.setChar(verbStr[i], i + 19);
			for (uint i = 0; i < nounStr.size(); ++i)
				err.setChar(nounStr[i], i + 30);
			_display->printString(err);
			continue;
		}

		noun = _nouns[nounStr];
		return;
	}
}

#define ARG(N) (command.script[offset + (N)])

bool AdlEngine::matchCommand(const Command &command, byte verb, byte noun, uint *actions) const {
	if (command.room != IDI_NONE && command.room != _state.room)
		return false;

	if (command.verb != IDI_NONE && command.verb != verb)
		return false;

	if (command.noun != IDI_NONE && command.noun != noun)
		return false;

	uint offset = 0;
	for (uint i = 0; i < command.numCond; ++i) {
		switch (ARG(0)) {
		case IDO_CND_ITEM_IN_ROOM:
			if (getItem(ARG(1)).room != ARG(2))
				return false;
			offset += 3;
			break;
		case IDO_CND_MOVES_GE:
			if (ARG(1) > _state.moves)
				return false;
			offset += 2;
			break;
		case IDO_CND_VAR_EQ:
			if (getVar(ARG(1)) != ARG(2))
				return false;
			offset += 3;
			break;
		case IDO_CND_CUR_PIC_EQ:
			if (getCurRoom().curPicture != ARG(1))
				return false;
			offset += 2;
			break;
		case IDO_CND_ITEM_PIC_EQ:
			if (getItem(ARG(1)).picture != ARG(2))
				return false;
			offset += 3;
			break;
		default:
			error("Invalid condition opcode %02x", command.script[offset]);
		}
	}

	if (actions)
		*actions = offset;

	return true;
}

void AdlEngine::doActions(const Command &command, byte noun, byte offset) {
	for (uint i = 0; i < command.numAct; ++i) {
		switch (ARG(0)) {
		case IDO_ACT_VAR_ADD:
			setVar(ARG(2), getVar(ARG(2) + ARG(1)));
			offset += 3;
			break;
		case IDO_ACT_VAR_SUB:
			setVar(ARG(2), getVar(ARG(2)) - ARG(1));
			offset += 3;
			break;
		case IDO_ACT_VAR_SET:
			setVar(ARG(1), ARG(2));
			offset += 3;
			break;
		case IDO_ACT_LIST_ITEMS: {
			Common::Array<Item>::const_iterator item;

			for (item = _state.items.begin(); item != _state.items.end(); ++item)
				if (item->room == IDI_NONE)
					printMessage(item->description);

			++offset;
			break;
		}
		case IDO_ACT_MOVE_ITEM:
			getItem(ARG(1)).room = ARG(2);
			offset += 3;
			break;
		case IDO_ACT_SET_ROOM:
			getCurRoom().curPicture = getCurRoom().picture;
			_state.room = ARG(1);
			offset += 2;
			break;
		case IDO_ACT_SET_CUR_PIC:
			getCurRoom().curPicture = ARG(1);
			offset += 2;
			break;
		case IDO_ACT_SET_PIC:
			getCurRoom().picture = getCurRoom().curPicture = ARG(1);
			offset += 2;
			break;
		case IDO_ACT_PRINT_MSG:
			printMessage(ARG(1));
			offset += 2;
			break;
		case IDO_ACT_SET_LIGHT:
			_state.isDark = false;
			++offset;
			break;
		case IDO_ACT_SET_DARK:
			_state.isDark = true;
			++offset;
			break;
		case IDO_ACT_SAVE:
			saveGameState(0, "");
			++offset;
			break;
		case IDO_ACT_LOAD:
			loadGameState(0);
			++offset;
			// Original engine does not jump out of the loop,
			// so we don't either.
			// We reset the restore flag, as the restore game
			// process is complete
			_isRestoring = false;
			break;
		case IDO_ACT_RESTART: {
			_display->printString(_strings.playAgain);
			Common::String input = inputString();
			if (input.size() == 0 || input[0] != APPLECHAR('N')) {
				_isRestarting = true;
				_display->clear(0x00);
				_display->updateHiResScreen();
				restartGame();
				return;
			}
			// Fall-through
		}
		case IDO_ACT_QUIT:
			printMessage(_messageIds.thanksForPlaying);
			quitGame();
			return;
		case IDO_ACT_PLACE_ITEM:
			getItem(ARG(1)).room = ARG(2);
			getItem(ARG(1)).position.x = ARG(3);
			getItem(ARG(1)).position.y = ARG(4);
			offset += 5;
			break;
		case IDO_ACT_SET_ITEM_PIC:
			getItem(ARG(2)).picture = ARG(1);
			offset += 3;
			break;
		case IDO_ACT_RESET_PIC:
			getCurRoom().curPicture = getCurRoom().picture;
			++offset;
			break;
		case IDO_ACT_GO_NORTH:
		case IDO_ACT_GO_SOUTH:
		case IDO_ACT_GO_EAST:
		case IDO_ACT_GO_WEST:
		case IDO_ACT_GO_UP:
		case IDO_ACT_GO_DOWN: {
			byte room = getCurRoom().connections[ARG(0) - IDO_ACT_GO_NORTH];

			if (room == 0) {
				printMessage(_messageIds.cantGoThere);
				return;
			}

			getCurRoom().curPicture = getCurRoom().picture;
			_state.room = room;
			return;
		}
		case IDO_ACT_TAKE_ITEM:
			takeItem(noun);
			++offset;
			break;
		case IDO_ACT_DROP_ITEM:
			dropItem(noun);
			++offset;
			break;
		case IDO_ACT_SET_ROOM_PIC:
			getRoom(ARG(1)).picture = getRoom(ARG(1)).curPicture = ARG(2);
			offset += 3;
			break;
		default:
			error("Invalid action opcode %02x", ARG(0));
		}
	}
}

#undef ARG

bool AdlEngine::doOneCommand(const Commands &commands, byte verb, byte noun) {
	Commands::const_iterator cmd;

	for (cmd = commands.begin(); cmd != commands.end(); ++cmd) {
		uint offset = 0;
		if (matchCommand(*cmd, verb, noun, &offset)) {
			doActions(*cmd, noun, offset);
			return true;
		}
	}

	return false;
}

void AdlEngine::doAllCommands(const Commands &commands, byte verb, byte noun) {
	Commands::const_iterator cmd;

	for (cmd = commands.begin(); cmd != commands.end(); ++cmd) {
		uint offset = 0;
		if (matchCommand(*cmd, verb, noun, &offset))
			doActions(*cmd, noun, offset);
	}
}

} // End of namespace Adl
