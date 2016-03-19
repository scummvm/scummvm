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
#include "adl/graphics.h"
#include "adl/speaker.h"

namespace Adl {

AdlEngine::~AdlEngine() {
	delete _display;
	delete _graphics;
	delete _speaker;
}

AdlEngine::AdlEngine(OSystem *syst, const AdlGameDescription *gd) :
		Engine(syst),
		_display(nullptr),
		_graphics(nullptr),
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

bool AdlEngine::pollEvent(Common::Event &event) {
	if (g_system->getEventManager()->pollEvent(event)) {
		if (event.type != Common::EVENT_KEYDOWN)
			return false;

		if (event.kbd.flags & Common::KBD_CTRL) {
			if (event.kbd.keycode == Common::KEYCODE_q) {
				quitGame();
				return false;
			}
		}

		return true;
	}

	return false;
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

void AdlEngine::openFile(Common::File &file, const Common::String &name) const {
	if (!file.open(name))
		error("Error opening '%s'", name.c_str());
}

void AdlEngine::printMessage(uint idx, bool wait) {
	Common::String msg = _messages[idx - 1];
	wordWrap(msg);
	_display->printString(msg);

	if (wait)
		delay(14 * 166018 / 1000);
}

void AdlEngine::delay(uint32 ms) const {
	uint32 start = g_system->getMillis();

	while (!shouldQuit() && g_system->getMillis() - start < ms) {
		Common::Event event;
		pollEvent(event);
		g_system->delayMillis(16);
	}
}

Common::String AdlEngine::inputString(byte prompt) const {
	Common::String s;

	if (prompt > 0)
		_display->printString(Common::String(prompt));

	while (1) {
		byte b = inputKey();

		if (shouldQuit() || _isRestoring)
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

byte AdlEngine::inputKey(bool showCursor) const {
	byte key = 0;

	if (showCursor)
		_display->showCursor(true);

	while (!shouldQuit() && !_isRestoring && key == 0) {
		Common::Event event;
		if (pollEvent(event)) {
			if (event.type != Common::EVENT_KEYDOWN)
				continue;

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

void AdlEngine::checkInput(byte verb, byte noun) {
	if (!doOneCommand(_roomCommands, verb, noun))
		printMessage(_messageIds.dontUnderstand);
}

typedef Common::Functor1Mem<ScriptEnv &, int, AdlEngine> OpcodeV1;
#define SetOpcodeTable(x) table = &x;
#define Opcode(x) table->push_back(new OpcodeV1(this, &AdlEngine::x))
#define OpcodeUnImpl() table->push_back(new OpcodeV1(this, 0))

void AdlEngine::setupOpcodeTables() {
	Common::Array<const Opcode *> *table = 0;

	SetOpcodeTable(_condOpcodes);
	// 0x00
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();
	Opcode(o1_isItemInRoom);
	// 0x04
	OpcodeUnImpl();
	Opcode(o1_isMovesGT);
	Opcode(o1_isVarEQ);
	OpcodeUnImpl();
	// 0x08
	OpcodeUnImpl();
	Opcode(o1_isCurPicEQ);
	Opcode(o1_isItemPicEQ);

	SetOpcodeTable(_actOpcodes);
	// 0x00
	OpcodeUnImpl();
	Opcode(o1_varAdd);
	Opcode(o1_varSub);
	Opcode(o1_varSet);
	// 0x04
	Opcode(o1_listInv);
	Opcode(o1_moveItem);
	Opcode(o1_setRoom);
	Opcode(o1_setCurPic);
	// 0x08
	Opcode(o1_setPic);
	Opcode(o1_printMsg);
	Opcode(o1_setLight);
	Opcode(o1_setDark);
	// 0x0c
	OpcodeUnImpl();
	Opcode(o1_quit);
	OpcodeUnImpl();
	Opcode(o1_save);
	// 0x10
	Opcode(o1_restore);
	Opcode(o1_restart);
	Opcode(o1_placeItem);
	Opcode(o1_setItemPic);
	// 0x14
	Opcode(o1_resetPic);
	Opcode(o1_goDirection<IDI_DIR_NORTH>);
	Opcode(o1_goDirection<IDI_DIR_SOUTH>);
	Opcode(o1_goDirection<IDI_DIR_EAST>);
	// 0x18
	Opcode(o1_goDirection<IDI_DIR_WEST>);
	Opcode(o1_goDirection<IDI_DIR_UP>);
	Opcode(o1_goDirection<IDI_DIR_DOWN>);
	Opcode(o1_takeItem);
	// 0x1c
	Opcode(o1_dropItem);
	Opcode(o1_setRoomPic);
}

bool AdlEngine::matchesCurrentPic(byte pic) const {
	return pic == getCurRoom().curPicture;
}

byte AdlEngine::roomArg(byte room) const {
	return room;
}

void AdlEngine::clearScreen() const {
	_display->setMode(DISPLAY_MODE_MIXED);
	_display->clear(0x00);
}

void AdlEngine::drawItems() const {
	Common::Array<Item>::const_iterator item;

	uint dropped = 0;

	for (item = _state.items.begin(); item != _state.items.end(); ++item) {
		// Skip items not in this room
		if (item->room != _state.room)
			continue;

		if (item->state == IDI_ITEM_DROPPED) {
			// Draw dropped item if in normal view
			if (getCurRoom().picture == getCurRoom().curPicture) {
				drawItem(*item, _itemOffsets[dropped]);
				++dropped;
			}
		} else {
			// Draw fixed item if current view is in the pic list
			Common::Array<byte>::const_iterator pic;

			for (pic = item->roomPictures.begin(); pic != item->roomPictures.end(); ++pic) {
				if (matchesCurrentPic(*pic)) {
					drawItem(*item, item->position);
					break;
				}
			}
		}
	}
}

void AdlEngine::bell(uint count) const {
	_speaker->bell(count);
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

		if (item->state == IDI_ITEM_DROPPED) {
			item->room = IDI_ANY;
			return;
		}

		Common::Array<byte>::const_iterator pic;
		for (pic = item->roomPictures.begin(); pic != item->roomPictures.end(); ++pic) {
			if (matchesCurrentPic(*pic)) {
				item->room = IDI_ANY;
				item->state = IDI_ITEM_DROPPED;
				return;
			}
		}
	}

	printMessage(_messageIds.itemNotHere);
}

void AdlEngine::dropItem(byte noun) {
	Common::Array<Item>::iterator item;

	for (item = _state.items.begin(); item != _state.items.end(); ++item) {
		if (item->noun != noun || item->room != IDI_ANY)
			continue;

		item->room = _state.room;
		item->state = IDI_ITEM_DROPPED;
		return;
	}

	printMessage(_messageIds.dontUnderstand);
}

Common::Error AdlEngine::run() {
	_speaker = new Speaker();
	_display = new Display();

	setupOpcodeTables();

	init();

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
				checkInput(verb, noun);
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
		ScriptEnv env(*cmd, _state.room, _saveVerb, _saveNoun);
		if (matchCommand(env))
			return env.op() == IDO_ACT_SAVE;
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

typedef Common::Functor1Mem<ScriptEnv &, int, AdlEngine> OpcodeV1;

int AdlEngine::o1_isItemInRoom(ScriptEnv &e) {
	if (getItem(e.arg(1)).room == roomArg(e.arg(2)))
		return 2;
	return -1;
}

int AdlEngine::o1_isMovesGT(ScriptEnv &e) {
	if (_state.moves > e.arg(1))
		return 1;
	return -1;
}

int AdlEngine::o1_isVarEQ(ScriptEnv &e) {
	if (getVar(e.arg(1)) == e.arg(2))
		return 2;
	return -1;
}

int AdlEngine::o1_isCurPicEQ(ScriptEnv &e) {
	if (getCurRoom().curPicture == e.arg(1))
		return 1;
	return -1;
}

int AdlEngine::o1_isItemPicEQ(ScriptEnv &e) {
	if (getItem(e.arg(1)).picture == e.arg(2))
		return 2;
	return -1;
}

int AdlEngine::o1_varAdd(ScriptEnv &e) {
	setVar(e.arg(2), getVar(e.arg(2) + e.arg(1)));
	return 2;
}

int AdlEngine::o1_varSub(ScriptEnv &e) {
	setVar(e.arg(2), getVar(e.arg(2)) - e.arg(1));
	return 2;
}

int AdlEngine::o1_varSet(ScriptEnv &e) {
	setVar(e.arg(1), e.arg(2));
	return 2;
}

int AdlEngine::o1_listInv(ScriptEnv &e) {
	Common::Array<Item>::const_iterator item;

	for (item = _state.items.begin(); item != _state.items.end(); ++item)
		if (item->room == IDI_ANY)
			printMessage(item->description);

	return 0;
}

int AdlEngine::o1_moveItem(ScriptEnv &e) {
	getItem(e.arg(1)).room = e.arg(2);
	return 2;
}

int AdlEngine::o1_setRoom(ScriptEnv &e) {
	getCurRoom().curPicture = getCurRoom().picture;
	_state.room = e.arg(1);
	return 1;
}

int AdlEngine::o1_setCurPic(ScriptEnv &e) {
	getCurRoom().curPicture = e.arg(1);
	return 1;
}

int AdlEngine::o1_setPic(ScriptEnv &e) {
	getCurRoom().picture = getCurRoom().curPicture = e.arg(1);
	return 1;
}

int AdlEngine::o1_printMsg(ScriptEnv &e) {
	printMessage(e.arg(1));
	return 1;
}

int AdlEngine::o1_setLight(ScriptEnv &e) {
	_state.isDark = false;
	return 0;
}

int AdlEngine::o1_setDark(ScriptEnv &e) {
	_state.isDark = true;
	return 0;
}

int AdlEngine::o1_save(ScriptEnv &e) {
	saveGameState(0, "");
	return 0;
}

int AdlEngine::o1_restore(ScriptEnv &e) {
	loadGameState(0);
	_isRestoring = false;
	return 0;
}

int AdlEngine::o1_restart(ScriptEnv &e) {
	_display->printString(_strings.playAgain);
	Common::String input = inputString();

	if (input.size() == 0 || input[0] != APPLECHAR('N')) {
		_isRestarting = true;
		_display->clear(0x00);
		_display->updateHiResScreen();
		restartGame();
		return -1;
	}

	return o1_quit(e);
}

int AdlEngine::o1_quit(ScriptEnv &e) {
	printMessage(_messageIds.thanksForPlaying);
	quitGame();
	return -1;
}

int AdlEngine::o1_placeItem(ScriptEnv &e) {
	Item &item = getItem(e.arg(1));

	item.room = roomArg(e.arg(2));
	item.position.x = e.arg(3);
	item.position.y = e.arg(4);
	return 4;
}

int AdlEngine::o1_setItemPic(ScriptEnv &e) {
	getItem(e.arg(2)).picture = e.arg(1);
	return 2;
}

int AdlEngine::o1_resetPic(ScriptEnv &e) {
	getCurRoom().curPicture = getCurRoom().picture;
	return 0;
}

template <Direction D>
int AdlEngine::o1_goDirection(ScriptEnv &e) {
	byte room = getCurRoom().connections[D];

	if (room == 0) {
		printMessage(_messageIds.cantGoThere);
		return -1;
	}

	getCurRoom().curPicture = getCurRoom().picture;
	_state.room = room;
	return -1;
}

int AdlEngine::o1_takeItem(ScriptEnv &e) {
	takeItem(e.getNoun());
	return 0;
}

int AdlEngine::o1_dropItem(ScriptEnv &e) {
	dropItem(e.getNoun());
	return 0;
}

int AdlEngine::o1_setRoomPic(ScriptEnv &e) {
	getRoom(e.arg(1)).picture = getRoom(e.arg(1)).curPicture = e.arg(2);
	return 2;
}

bool AdlEngine::matchCommand(ScriptEnv &env) const {
	if (!env.isMatch())
		return false;

	for (uint i = 0; i < env.getCondCount(); ++i) {
		byte op = env.op();

		if (!_condOpcodes[op] || !_condOpcodes[op]->isValid())
			error("Unimplemented condition opcode %02x", op);

		int numArgs = (*_condOpcodes[op])(env);

		if (numArgs < 0)
			return false;

		env.skip(numArgs + 1);
	}

	return true;
}

void AdlEngine::doActions(ScriptEnv &env) {
	for (uint i = 0; i < env.getActCount(); ++i) {
		byte op = env.op();

		if (!_actOpcodes[op] || !_actOpcodes[op]->isValid())
			error("Unimplemented action opcode %02x", op);

		int numArgs = (*_actOpcodes[op])(env);

		if (numArgs < 0)
			return;

		env.skip(numArgs + 1);
	}
}

bool AdlEngine::doOneCommand(const Commands &commands, byte verb, byte noun) {
	Commands::const_iterator cmd;

	for (cmd = commands.begin(); cmd != commands.end(); ++cmd) {
		ScriptEnv env(*cmd, _state.room, verb, noun);
		if (matchCommand(env)) {
			doActions(env);
			return true;
		}
	}

	return false;
}

void AdlEngine::doAllCommands(const Commands &commands, byte verb, byte noun) {
	Commands::const_iterator cmd;

	for (cmd = commands.begin(); cmd != commands.end(); ++cmd) {
		ScriptEnv env(*cmd, _state.room, verb, noun);
		if (matchCommand(env))
			doActions(env);
	}
}

} // End of namespace Adl
