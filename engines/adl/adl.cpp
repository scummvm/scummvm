/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
#include "common/random.h"

#include "engines/util.h"

#include "graphics/thumbnail.h"

#include "adl/adl.h"
#include "adl/display_a2.h"
#include "adl/graphics.h"
#include "adl/sound.h"

namespace Adl {

class ScriptEnv_6502 : public ScriptEnv {
public:
	ScriptEnv_6502(const Command &cmd, byte room, byte verb, byte noun) :
			ScriptEnv(cmd, room, verb, noun),
			_remCond(cmd.numCond),
			_remAct(cmd.numAct) { }

private:
	kOpType getOpType() const override {
		if (_remCond > 0)
			return kOpTypeCond;
		if (_remAct > 0)
			return kOpTypeAct;
		return kOpTypeDone;
	}

	void next(uint numArgs) override {
		_ip += numArgs + 1;
		if (_remCond > 0)
			--_remCond;
		else if (_remAct > 0)
			--_remAct;
	}

	byte _remCond, _remAct;
};

AdlEngine::~AdlEngine() {
	delete _display;
	delete _graphics;
	delete _dumpFile;
	delete _inputScript;
	delete _random;
}

AdlEngine::AdlEngine(OSystem *syst, const AdlGameDescription *gd) :
		Engine(syst),
		_dumpFile(nullptr),
		_display(nullptr),
		_graphics(nullptr),
		_textMode(false),
		_verbErrorPos(19),
		_nounErrorPos(30),
		_linesPrinted(0),
		_isRestarting(false),
		_isRestoring(false),
		_isQuitting(false),
		_abortScript(false),
		_gameDescription(gd),
		_inputScript(nullptr),
		_scriptDelay(1000),
		_scriptPaused(false),
		_messageIds(),
		_saveVerb(0),
		_saveNoun(0),
		_restoreVerb(0),
		_restoreNoun(0),
		_canSaveNow(false),
		_canRestoreNow(false) {

	_random = new Common::RandomSource("adl");
}

bool AdlEngine::pollEvent(Common::Event &event) const {
	if (g_system->getEventManager()->pollEvent(event)) {
		if (event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START) {
			if (event.customType == kADLActionQuit) {
				quitGame();
			}
			return false;
		}

		if (event.type == Common::EVENT_KEYDOWN)
			return true;
	}

	return false;
}

Common::String AdlEngine::readString(Common::ReadStream &stream, byte until, const char *key) const {
	Common::String str;
	int keyLength = strlen(key);

	while (1) {
		byte b = stream.readByte();

		if (stream.eos() || stream.err())
			error("Error reading string");

		if (b == until)
			break;

		if (keyLength)
			b ^= key[str.size() % keyLength];

		str += b;
	}

	return str;
}

Common::String AdlEngine::readStringAt(Common::SeekableReadStream &stream, uint offset, byte until) const {
	stream.seek(offset);
	return readString(stream, until);
}

void AdlEngine::extractExeStrings(Common::ReadStream &stream, uint16 printAddr, Common::StringArray &strings) const {
	uint32 window = 0;

	for (;;) {
		window <<= 8;
		window |= stream.readByte();

		if (stream.eos())
			return;

		if (stream.err())
			error("Failed to extract strings from game executable");

		if ((window & 0xffffff) == (0x200000U | printAddr))
			strings.push_back(readString(stream));
	}
}

void AdlEngine::printMessage(uint idx) {
	printString(loadMessage(idx));
}

Common::String AdlEngine::getItemDescription(const Item &item) const {
	if (item.description > 0)
		return loadMessage(item.description);
	else
		return Common::String();
}

void AdlEngine::delay(uint32 ms) const {
	if (_inputScript && !_scriptPaused)
		return;

	uint32 now = g_system->getMillis();
	const uint32 end = now + ms;

	while (!shouldQuit() && now < end) {
		Common::Event event;
		pollEvent(event);
		g_system->delayMillis(end - now < 16 ? end - now : 16);
		now = g_system->getMillis();
		g_system->updateScreen();
	}
}

Common::String AdlEngine::inputString(byte prompt) const {
	Common::String s;

	if (prompt > 0)
		_display->printString(Common::String(prompt));

	while (1) {
		byte b = inputKey();

		if (_inputScript) {
			// If debug script is active, read input line from file
			Common::String line(getScriptLine());

			// Debug script terminated, go back to keyboard input
			if (line.empty())
				continue;

			line += '\r';

			Common::String native;

			for (uint i = 0; i < line.size(); ++i)
				native += _display->asciiToNative(line[i]);

			_display->printString(native);
			// Set pause flag to activate regular behaviour of delay and inputKey
			_scriptPaused = true;

			if (_scriptDelay > 0)
				delay(_scriptDelay);
			else
				inputKey();

			_scriptPaused = false;
			return native;
		}

		if (shouldQuit() || _isRestoring)
			return Common::String();

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
					_display->setCharAtCursor(_display->asciiToNative(' '));
					s.deleteLastChar();
				}
				break;
			default:
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

	// If debug script is active, we fake a return press for the text overflow handling
	if (_inputScript && !_scriptPaused)
		return _display->asciiToNative('\r');

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

		// If debug script was activated in the meantime, abort input
		if (_inputScript && !_scriptPaused)
			return _display->asciiToNative('\r');

		_display->renderText();
		g_system->delayMillis(16);
		g_system->updateScreen();
	}

	_display->showCursor(false);

	return key;
}

void AdlEngine::waitKey(uint32 ms, Common::KeyCode keycode) const {
	uint32 start = g_system->getMillis();

	while (!shouldQuit()) {
		Common::Event event;
		if (pollEvent(event)) {
			if (event.type == Common::EVENT_KEYDOWN)
				if (keycode == Common::KEYCODE_INVALID || keycode == event.kbd.keycode)
					return;
		}

		if (ms && g_system->getMillis() - start >= ms)
			return;

		g_system->delayMillis(16);
		g_system->updateScreen();
	}
}

void AdlEngine::loadWords(Common::ReadStream &stream, WordMap &map, Common::StringArray &pri, uint count) const {
	uint index = 0;

	map.clear();
	pri.clear();

	// WORKAROUND: Several games contain one or more word lists without a terminator
	switch (getGameType()) {
	case GAME_TYPE_HIRES3:
		if (&map == &_verbs)
			count = 72;
		else
			count = 113;
		break;
	case GAME_TYPE_HIRES5:
		if (_state.region == 15 && &map == &_nouns)
			count = 81;
		break;
	default:
		break;
	}

	while (1) {
		++index;

		byte buf[IDI_WORD_SIZE];

		if (stream.read(buf, IDI_WORD_SIZE) < IDI_WORD_SIZE)
			error("Error reading word list");

		Common::String word((char *)buf, IDI_WORD_SIZE);

		if (!map.contains(word))
			map[word] = index;

		pri.push_back(Console::toAscii(word));

		byte synonyms = stream.readByte();

		if (stream.err() || stream.eos())
			error("Error reading word list");

		if (synonyms == 0xff)
			break;

		if (index == count)
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
	commands.clear();

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

void AdlEngine::removeCommand(Commands &commands, uint idx) {
	Commands::iterator cmds;
	uint i = 0;
	for (cmds = commands.begin(); cmds != commands.end(); ++cmds) {
		if (i++ == idx) {
			commands.erase(cmds);
			return;
		}
	}

	error("Command %d not found", idx);
}

Command &AdlEngine::getCommand(Commands &commands, uint idx) {
	Commands::iterator cmds;
	uint i = 0;
	for (cmds = commands.begin(); cmds != commands.end(); ++cmds) {
		if (i++ == idx)
			return *cmds;
	}

	error("Command %d not found", idx);
}

void AdlEngine::removeMessage(uint idx) {
		if (_messages[idx]) {
			_messages[idx].reset();
			return;
		}

		error("Message %d not found", idx);
}

void AdlEngine::checkInput(byte verb, byte noun) {
	// Try room-local command list first
	if (doOneCommand(_roomData.commands, verb, noun))
		return;

	// If no match was found, try the global list
	if (!doOneCommand(_roomCommands, verb, noun))
		printMessage(_messageIds.dontUnderstand);
}

bool AdlEngine::isInputValid(byte verb, byte noun, bool &is_any) {
	if (isInputValid(_roomData.commands, verb, noun, is_any))
		return true;
	return isInputValid(_roomCommands, verb, noun, is_any);
}

bool AdlEngine::isInputValid(const Commands &commands, byte verb, byte noun, bool &is_any) {
	Commands::const_iterator cmd;

	is_any = false;
	for (cmd = commands.begin(); cmd != commands.end(); ++cmd) {
		Common::ScopedPtr<ScriptEnv> env(createScriptEnv(*cmd, _state.room, verb, noun));
		if (matchCommand(*env)) {
			if (cmd->verb == IDI_ANY || cmd->noun == IDI_ANY)
				is_any = true;
			return true;
		}
	}

	return false;
}

void AdlEngine::setupOpcodeTables() {
	_condOpcodes.resize(0x0b);
	_condOpcodes[0x03] = opcode(&AdlEngine::o_isItemInRoom);
	_condOpcodes[0x05] = opcode(&AdlEngine::o_isMovesGT);
	_condOpcodes[0x06] = opcode(&AdlEngine::o_isVarEQ);
	_condOpcodes[0x09] = opcode(&AdlEngine::o_isCurPicEQ);
	_condOpcodes[0x0a] = opcode(&AdlEngine::o_isItemPicEQ);

	_actOpcodes.resize(0x1e);
	_actOpcodes[0x01] = opcode(&AdlEngine::o_varAdd);
	_actOpcodes[0x02] = opcode(&AdlEngine::o_varSub);
	_actOpcodes[0x03] = opcode(&AdlEngine::o_varSet);
	_actOpcodes[0x04] = opcode(&AdlEngine::o_listInv);
	_actOpcodes[0x05] = opcode(&AdlEngine::o_moveItem);
	_actOpcodes[0x06] = opcode(&AdlEngine::o_setRoom);
	_actOpcodes[0x07] = opcode(&AdlEngine::o_setCurPic);
	_actOpcodes[0x08] = opcode(&AdlEngine::o_setPic);
	_actOpcodes[0x09] = opcode(&AdlEngine::o_printMsg);
	_actOpcodes[0x0a] = opcode(&AdlEngine::o_setLight);
	_actOpcodes[0x0b] = opcode(&AdlEngine::o_setDark);
	_actOpcodes[0x0d] = opcode(&AdlEngine::o_quit);
	_actOpcodes[0x0f] = opcode(&AdlEngine::o_save);
	_actOpcodes[0x10] = opcode(&AdlEngine::o_restore);
	_actOpcodes[0x11] = opcode(&AdlEngine::o_restart);
	_actOpcodes[0x12] = opcode(&AdlEngine::o_placeItem);
	_actOpcodes[0x13] = opcode(&AdlEngine::o_setItemPic);
	_actOpcodes[0x14] = opcode(&AdlEngine::o_resetPic);
	_actOpcodes[0x15] = opcode(&AdlEngine::o_goNorth);
	_actOpcodes[0x16] = opcode(&AdlEngine::o_goSouth);
	_actOpcodes[0x17] = opcode(&AdlEngine::o_goEast);
	_actOpcodes[0x18] = opcode(&AdlEngine::o_goWest);
	_actOpcodes[0x19] = opcode(&AdlEngine::o_goUp);
	_actOpcodes[0x1a] = opcode(&AdlEngine::o_goDown);
	_actOpcodes[0x1b] = opcode(&AdlEngine::o_takeItem);
	_actOpcodes[0x1c] = opcode(&AdlEngine::o_dropItem);
	_actOpcodes[0x1d] = opcode(&AdlEngine::o_setRoomPic);
}

void AdlEngine::initState() {
	_state = State();

	initGameState();
}

void AdlEngine::switchRoom(byte roomNr) {
	getCurRoom().curPicture = getCurRoom().picture;
	_state.room = roomNr;
}

byte AdlEngine::roomArg(byte room) const {
	return room;
}

void AdlEngine::loadDroppedItemOffsets(Common::ReadStream &stream, byte count) {
	for (uint i = 0; i < count; ++i) {
		Common::Point p;
		p.x = stream.readByte();
		p.y = stream.readByte();
		_itemOffsets.push_back(p);
	}
}

void AdlEngine::drawPic(byte pic, Common::Point pos) const {
	if (_roomData.pictures.contains(pic)) {
		Common::StreamPtr stream(_roomData.pictures[pic]->createReadStream());
		_graphics->drawPic(*stream, pos);
	} else if (_pictures.contains(pic)) {
		Common::StreamPtr stream(_pictures[pic]->createReadStream());
		_graphics->drawPic(*stream, pos);
	} else
		error("Picture %d not found", pic);
}

void AdlEngine::bell(uint count) const {
	Tones tones;

	for (uint i = 0; i < count - 1; ++i) {
		tones.push_back(Tone(940.0, 100.0));
		tones.push_back(Tone(0.0, 12.0));
	}

	tones.push_back(Tone(940.0, 100.0));

	playTones(tones, false);
}

bool AdlEngine::playTones(const Tones &tones, bool isMusic, bool allowSkip) const {
	if (_inputScript && !_scriptPaused)
		return false;

	Audio::SoundHandle handle;
	Audio::AudioStream *stream = new Sound(tones);

	g_system->getMixer()->playStream((isMusic ? Audio::Mixer::kMusicSoundType : Audio::Mixer::kSFXSoundType), &handle, stream, -1, 25);

	while (!g_engine->shouldQuit() && g_system->getMixer()->isSoundHandleActive(handle)) {
		Common::Event event;
		pollEvent(event);

		if (allowSkip && event.type == Common::EVENT_KEYDOWN) {
			// FIXME: Preserve this event
			g_system->getMixer()->stopHandle(handle);
			return true;
		}

		g_system->delayMillis(16);
		g_system->updateScreen();
	}

	return false;
}

const Region &AdlEngine::getRegion(uint i) const {
	if (i < 1 || i > _state.regions.size())
		error("Region %i out of range [1, %i]", i, _state.regions.size());

	return _state.regions[i - 1];
}

Region &AdlEngine::getRegion(uint i) {
	if (i < 1 || i > _state.regions.size())
		error("Region %i out of range [1, %i]", i, _state.regions.size());

	return _state.regions[i - 1];
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

const Region &AdlEngine::getCurRegion() const {
	return getRegion(_state.region);
}

Region &AdlEngine::getCurRegion() {
	return getRegion(_state.region);
}

const Room &AdlEngine::getCurRoom() const {
	return getRoom(_state.room);
}

Room &AdlEngine::getCurRoom() {
	return getRoom(_state.room);
}

const Item &AdlEngine::getItem(uint i) const {
	Common::List<Item>::const_iterator item;

	for (item = _state.items.begin(); item != _state.items.end(); ++item)
		if (item->id == i)
			return *item;

	error("Item %i not found", i);
}

Item &AdlEngine::getItem(uint i) {
	Common::List<Item>::iterator item;

	for (item = _state.items.begin(); item != _state.items.end(); ++item)
		if (item->id == i)
			return *item;

	error("Item %i not found", i);
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
	Common::List<Item>::iterator item;

	for (item = _state.items.begin(); item != _state.items.end(); ++item) {
		if (item->noun == noun && item->room == _state.room && item->region == _state.region) {
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
				if (*pic == getCurRoom().curPicture) {
					item->room = IDI_ANY;
					item->state = IDI_ITEM_DROPPED;
					return;
				}
			}
		}
	}

	printMessage(_messageIds.itemNotHere);
}

void AdlEngine::dropItem(byte noun) {
	Common::List<Item>::iterator item;

	for (item = _state.items.begin(); item != _state.items.end(); ++item) {
		if (item->noun == noun && item->room == IDI_ANY) {
			item->room = _state.room;
			item->region = _state.region;
			item->state = IDI_ITEM_DROPPED;
			return;
		}
	}

	printMessage(_messageIds.dontUnderstand);
}

void AdlEngine::gameLoop() {
	uint verb = 0, noun = 0;
	_isRestarting = false;

	// When restoring from the launcher, we don't read
	// input on the first iteration. This is needed to
	// ensure that restoring from the launcher and
	// restoring in-game brings us to the same game state.
	// (Also see comment below.)
	if (!_isRestoring) {
		showRoom();

		if (_isRestarting)
			return;

		_canSaveNow = _canRestoreNow = true;
		getInput(verb, noun);
		_canSaveNow = _canRestoreNow = false;

		if (shouldQuit())
			return;

		_linesPrinted = 0;

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
	if (_isRestarting)
		return;

	doAllCommands(_globalCommands, verb, noun);

	if (_isRestarting)
		return;

	advanceClock();
	_state.moves++;
}

Common::Error AdlEngine::run() {
	_display = Display_A2_create();
	if (!_display)
		return Common::kUnsupportedColorMode;

	setDebugger(new Console(this));
	_display->init();

	setupOpcodeTables();

	init();
	g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);

	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot >= 0) {
		if (loadGameState(saveSlot).getCode() != Common::kNoError)
			error("Failed to load save game from slot %i", saveSlot);
		_display->moveCursorTo(Common::Point(0, 23));
		_isRestoring = true;
	} else {
		runIntro();
		initState();
		_display->printAsciiString(_strings.lineFeeds);
	}

	_display->setMode(Display::kModeMixed);

	while (!(_isQuitting || shouldQuit()))
		gameLoop();

	g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);

	return Common::kNoError;
}

bool AdlEngine::hasFeature(EngineFeature f) const {
	switch (f) {
	case kSupportsLoadingDuringRuntime:
	case kSupportsSavingDuringRuntime:
	case kSupportsReturnToLauncher:
		return true;
	default:
		return false;
	}
}

Common::String AdlEngine::getScriptLine() const {
	Common::String line;

	do {
		line = _inputScript->readLine();

		if (_inputScript->eos() || _inputScript->err()) {
			stopScript();
			return Common::String();
		}

		line.trim();
	} while (line.size() == 0 || line.firstChar() == ';');

	return line;
}

void AdlEngine::runScript(const char *filename) const {
	// Debug functionality to read input from a text file
	_inputScript = new Common::File;
	if (!_inputScript->open(filename)) {
		stopScript();
		return;
	}

	Common::String line(getScriptLine());

	if (!line.empty()) {
		// Read random seed
		_random->setSeed((uint32)line.asUint64());
	}
}

void AdlEngine::stopScript() const {
	delete _inputScript;
	_inputScript = nullptr;
}

void AdlEngine::loadState(Common::ReadStream &stream) {
	_state.room = stream.readByte();
	_state.moves = stream.readByte();
	_state.isDark = stream.readByte();
	_state.time.hours = stream.readByte();
	_state.time.minutes = stream.readByte();

	uint32 size = stream.readUint32BE();
	if (size != _state.rooms.size())
		error("Room count mismatch (expected %i; found %i)", _state.rooms.size(), size);

	for (uint i = 0; i < size; ++i) {
		_state.rooms[i].picture = stream.readByte();
		_state.rooms[i].curPicture = stream.readByte();
		_state.rooms[i].isFirstTime = stream.readByte();
	}

	// NOTE: _state.curPicture is part of the save state in the original engine. We
	// reconstruct it instead. This is believed to be safe for at least hires 0-2, but
	// this may need to be re-evaluated for later games.
	_state.curPicture = getCurRoom().curPicture;

	size = stream.readUint32BE();
	if (size != _state.items.size())
		error("Item count mismatch (expected %i; found %i)", _state.items.size(), size);

	Common::List<Item>::iterator item;
	for (item = _state.items.begin(); item != _state.items.end(); ++item) {
		item->room = stream.readByte();
		item->picture = stream.readByte();
		item->position.x = stream.readByte();
		item->position.y = stream.readByte();
		item->state = stream.readByte();
	}

	size = stream.readUint32BE();
	if (size != _state.vars.size())
		error("Variable count mismatch (expected %i; found %i)", _state.vars.size(), size);

	for (uint i = 0; i < size; ++i)
		_state.vars[i] = stream.readByte();
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
	loadState(*inFile);

	if (inFile->err() || inFile->eos())
		error("Failed to load game '%s'", fileName.c_str());

	delete inFile;

	setTotalPlayTime(playTime);

	_isRestoring = true;
	return Common::kNoError;
}

bool AdlEngine::canLoadGameStateCurrently(Common::U32String *msg) {
	return _canRestoreNow;
}

void AdlEngine::saveState(Common::WriteStream &stream) {
	stream.writeByte(_state.room);
	stream.writeByte(_state.moves);
	stream.writeByte(_state.isDark);
	stream.writeByte(_state.time.hours);
	stream.writeByte(_state.time.minutes);

	stream.writeUint32BE(_state.rooms.size());
	for (uint i = 0; i < _state.rooms.size(); ++i) {
		stream.writeByte(_state.rooms[i].picture);
		stream.writeByte(_state.rooms[i].curPicture);
		stream.writeByte(_state.rooms[i].isFirstTime);
	}

	stream.writeUint32BE(_state.items.size());
	Common::List<Item>::const_iterator item;
	for (item = _state.items.begin(); item != _state.items.end(); ++item) {
		stream.writeByte(item->room);
		stream.writeByte(item->picture);
		stream.writeByte(item->position.x);
		stream.writeByte(item->position.y);
		stream.writeByte(item->state);
	}

	stream.writeUint32BE(_state.vars.size());
	for (uint i = 0; i < _state.vars.size(); ++i)
		stream.writeByte(_state.vars[i]);
}

Common::String AdlEngine::getSaveStateName(int slot) const {
	return Common::String::format("%s.s%02d", _targetName.c_str(), slot);
}

Common::Error AdlEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	Common::String fileName = getSaveStateName(slot);
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

	Graphics::saveThumbnail(*outFile);
	saveState(*outFile);
	outFile->finalize();

	if (outFile->err()) {
		delete outFile;
		warning("Failed to save game '%s'", fileName.c_str());
		return Common::kUnknownError;
	}

	delete outFile;
	return Common::kNoError;
}

bool AdlEngine::canSaveGameStateCurrently(Common::U32String *msg) {
	if (!_canSaveNow)
		return false;

	Commands::const_iterator cmd;

	// Here we check whether or not the game currently accepts the command
	// "SAVE GAME". This prevents saving via the GMM in situations where
	// it wouldn't otherwise be possible to do so.
	for (cmd = _roomData.commands.begin(); cmd != _roomData.commands.end(); ++cmd) {
		Common::ScopedPtr<ScriptEnv> env(createScriptEnv(*cmd, _state.room, _saveVerb, _saveNoun));
		if (matchCommand(*env))
			return env->op() == IDO_ACT_SAVE;
	}

	for (cmd = _roomCommands.begin(); cmd != _roomCommands.end(); ++cmd) {
		Common::ScopedPtr<ScriptEnv> env(createScriptEnv(*cmd, _state.room, _saveVerb, _saveNoun));
		if (matchCommand(*env))
			return env->op() == IDO_ACT_SAVE;
	}

	return false;
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

Common::String AdlEngine::getLine() {
	while (1) {
		Common::String line = inputString(_display->asciiToNative('?'));

		if (shouldQuit() || _isRestoring)
			return Common::String();

		if ((byte)line[0] == ('\r' | 0x80)) {
			_textMode = !_textMode;
			_display->setMode(_textMode ? Display::kModeText : Display::kModeMixed);
			continue;
		}

		// Remove the return
		line.deleteLastChar();
		return line;
	}
}

Common::String AdlEngine::getWord(const Common::String &line, uint &index) const {
	Common::String str;
	const char spaceChar = _display->asciiToNative(' ');

	for (uint i = 0; i < 8; ++i)
		str += spaceChar;

	int copied = 0;

	// Skip initial whitespace
	while (1) {
		if (index == line.size())
			return str;
		if (line[index] != spaceChar)
			break;
		++index;
	}

	// Copy up to 8 characters
	while (1) {
		if (copied < 8)
			str.setChar(line[index], copied++);

		index++;

		if (index == line.size() || line[index] == spaceChar)
			return str;
	}
}

Common::String AdlEngine::formatVerbError(const Common::String &verb) const {
	Common::String err = _strings.verbError;

	if (_verbErrorPos + verb.size() > err.size())
		error("Failed to format verb error string");

	err.replace(_verbErrorPos, verb.size(), verb);

	return err;
}

Common::String AdlEngine::formatNounError(const Common::String &verb, const Common::String &noun) const {
	Common::String err = _strings.nounError;

	if (_verbErrorPos + verb.size() > err.size() || _nounErrorPos + noun.size() > err.size())
		error("Failed to format noun error string");

	err.replace(_verbErrorPos, verb.size(), verb);
	err.replace(_nounErrorPos, noun.size(), noun);

	return err;
}

void AdlEngine::getInput(uint &verb, uint &noun) {
	while (1) {
		_display->printString(_strings.enterCommand);
		Common::String line = getLine();

		if (shouldQuit() || _isRestoring)
			return;

		uint index = 0;
		Common::String verbString = getWord(line, index);

		if (!_verbs.contains(verbString)) {
			_display->printString(formatVerbError(verbString));
			continue;
		}

		verb = _verbs[verbString];

		Common::String nounString = getWord(line, index);

		if (!_nouns.contains(nounString)) {
			_display->printString(formatNounError(verbString, nounString));
			continue;
		}

		noun = _nouns[nounString];
		return;
	}
}

bool AdlEngine::op_debug(const char *fmt, ...) const {
	if (DebugMan.isDebugChannelEnabled(kDebugChannelScript)) {
		va_list va;
		va_start(va, fmt);
		Common::String output = Common::String::vformat(fmt, va);
		va_end(va);

		output += '\n';
		if (_dumpFile) {
			_dumpFile->write(output.c_str(), output.size());
			return true;
		} else
			debugN("%s", output.c_str());
	}

	return false;
}

int AdlEngine::o_isItemInRoom(ScriptEnv &e) {
	OP_DEBUG_2("\t&& GET_ITEM_ROOM(%s) == %s", itemStr(e.arg(1)).c_str(), itemRoomStr(e.arg(2)).c_str());

	if (getItem(e.arg(1)).room == roomArg(e.arg(2)))
		return 2;

	return -1;
}

int AdlEngine::o_isMovesGT(ScriptEnv &e) {
	OP_DEBUG_1("\t&& MOVES > %d", e.arg(1));

	if (_state.moves > e.arg(1))
		return 1;

	return -1;
}

int AdlEngine::o_isVarEQ(ScriptEnv &e) {
	OP_DEBUG_2("\t&& VARS[%d] == %d", e.arg(1), e.arg(2));

	if (getVar(e.arg(1)) == e.arg(2))
		return 2;

	return -1;
}

int AdlEngine::o_isCurPicEQ(ScriptEnv &e) {
	OP_DEBUG_1("\t&& GET_CURPIC() == %d", e.arg(1));

	if (_state.curPicture == e.arg(1))
		return 1;

	return -1;
}

int AdlEngine::o_isItemPicEQ(ScriptEnv &e) {
	OP_DEBUG_2("\t&& GET_ITEM_PIC(%s) == %d", itemStr(e.arg(1)).c_str(), e.arg(2));

	if (getItem(e.arg(1)).picture == e.arg(2))
		return 2;

	return -1;
}

int AdlEngine::o_varAdd(ScriptEnv &e) {
	OP_DEBUG_2("\tVARS[%d] += %d", e.arg(2), e.arg(1));

	setVar(e.arg(2), getVar(e.arg(2)) + e.arg(1));
	return 2;
}

int AdlEngine::o_varSub(ScriptEnv &e) {
	OP_DEBUG_2("\tVARS[%d] -= %d", e.arg(2), e.arg(1));

	setVar(e.arg(2), getVar(e.arg(2)) - e.arg(1));
	return 2;
}

int AdlEngine::o_varSet(ScriptEnv &e) {
	OP_DEBUG_2("\tVARS[%d] = %d", e.arg(1), e.arg(2));

	setVar(e.arg(1), e.arg(2));
	return 2;
}

int AdlEngine::o_listInv(ScriptEnv &e) {
	OP_DEBUG_0("\tLIST_INVENTORY()");

	Common::List<Item>::const_iterator item;

	for (item = _state.items.begin(); item != _state.items.end(); ++item)
		if (item->room == IDI_ANY)
			printString(getItemDescription(*item));

	return 0;
}

int AdlEngine::o_moveItem(ScriptEnv &e) {
	OP_DEBUG_2("\tSET_ITEM_ROOM(%s, %s)", itemStr(e.arg(1)).c_str(), itemRoomStr(e.arg(2)).c_str());

	getItem(e.arg(1)).room = e.arg(2);
	return 2;
}

int AdlEngine::o_setRoom(ScriptEnv &e) {
	OP_DEBUG_1("\tROOM = %d", e.arg(1));

	switchRoom(e.arg(1));
	return 1;
}

int AdlEngine::o_setCurPic(ScriptEnv &e) {
	OP_DEBUG_1("\tSET_CURPIC(%d)", e.arg(1));

	getCurRoom().curPicture = e.arg(1);
	return 1;
}

int AdlEngine::o_setPic(ScriptEnv &e) {
	OP_DEBUG_1("\tSET_PIC(%d)", e.arg(1));

	getCurRoom().picture = getCurRoom().curPicture = e.arg(1);
	return 1;
}

int AdlEngine::o_printMsg(ScriptEnv &e) {
	OP_DEBUG_1("\tPRINT(%s)", msgStr(e.arg(1)).c_str());

	printMessage(e.arg(1));
	return 1;
}

int AdlEngine::o_setLight(ScriptEnv &e) {
	OP_DEBUG_0("\tLIGHT()");

	_state.isDark = false;
	return 0;
}

int AdlEngine::o_setDark(ScriptEnv &e) {
	OP_DEBUG_0("\tDARK()");

	_state.isDark = true;
	return 0;
}

int AdlEngine::o_save(ScriptEnv &e) {
	OP_DEBUG_0("\tSAVE_GAME()");

	saveGameState(0, "");
	return 0;
}

int AdlEngine::o_restore(ScriptEnv &e) {
	OP_DEBUG_0("\tRESTORE_GAME()");

	loadGameState(0);
	_isRestoring = false;
	return 0;
}

int AdlEngine::o_restart(ScriptEnv &e) {
	OP_DEBUG_0("\tRESTART_GAME()");

	_display->printString(_strings.playAgain);
	Common::String input = inputString();

	if (input.size() == 0 || input[0] != _display->asciiToNative('N')) {
		_isRestarting = true;
		_graphics->clearScreen();
		_display->renderGraphics();
		_display->printString(_strings.pressReturn);
		initState();
		_display->printAsciiString(_strings.lineFeeds);
		return -1;
	}

	return o_quit(e);
}

int AdlEngine::o_quit(ScriptEnv &e) {
	OP_DEBUG_0("\tQUIT_GAME()");

	printMessage(_messageIds.thanksForPlaying);
	// Wait for a key here to ensure that the user gets a chance
	// to read the thank-you message
	_display->printAsciiString("PRESS ANY KEY TO QUIT");
	inputKey();

	// We use _isRestarting to abort the current game loop iteration
	_isQuitting = _isRestarting = true;
	return -1;
}

int AdlEngine::o_placeItem(ScriptEnv &e) {
	OP_DEBUG_4("\tPLACE_ITEM(%s, %s, (%d, %d))", itemStr(e.arg(1)).c_str(), itemRoomStr(e.arg(2)).c_str(), e.arg(3), e.arg(4));

	Item &item = getItem(e.arg(1));

	item.room = roomArg(e.arg(2));
	item.position.x = e.arg(3);
	item.position.y = e.arg(4);
	return 4;
}

int AdlEngine::o_setItemPic(ScriptEnv &e) {
	OP_DEBUG_2("\tSET_ITEM_PIC(%s, %d)", itemStr(e.arg(2)).c_str(), e.arg(1));

	getItem(e.arg(2)).picture = e.arg(1);
	return 2;
}

int AdlEngine::o_resetPic(ScriptEnv &e) {
	OP_DEBUG_0("\tRESET_PIC()");

	getCurRoom().curPicture = getCurRoom().picture;
	return 0;
}

int AdlEngine::goDirection(ScriptEnv &e, Direction dir) {
	OP_DEBUG_0((Common::String("\tGO_") + dirStr(dir) + "()").c_str());

	byte room = getCurRoom().connections[dir];

	if (room == 0) {
		printMessage(_messageIds.cantGoThere);
		return -1;
	}

	switchRoom(room);
	return -1;
}

int AdlEngine::o_takeItem(ScriptEnv &e) {
	OP_DEBUG_0("\tTAKE_ITEM()");

	takeItem(e.getNoun());
	return 0;
}

int AdlEngine::o_dropItem(ScriptEnv &e) {
	OP_DEBUG_0("\tDROP_ITEM()");

	dropItem(e.getNoun());
	return 0;
}

int AdlEngine::o_setRoomPic(ScriptEnv &e) {
	OP_DEBUG_2("\tSET_ROOM_PIC(%d, %d)", e.arg(1), e.arg(2));

	getRoom(e.arg(1)).picture = getRoom(e.arg(1)).curPicture = e.arg(2);
	return 2;
}

bool AdlEngine::matchCommand(ScriptEnv &env) const {
	if (!env.isMatch() && !_dumpFile)
		return false;

	if (DebugMan.isDebugChannelEnabled(kDebugChannelScript)) {
		(void)op_debug("IF\n\tROOM == %s", roomStr(env.getCommand().room).c_str());
		(void)op_debug("\t&& SAID(%s, %s)", verbStr(env.getCommand().verb).c_str(), nounStr(env.getCommand().noun).c_str());
	}

	while (env.getOpType() == ScriptEnv::kOpTypeCond) {
		byte op = env.op();

		if (op >= _condOpcodes.size() || !_condOpcodes[op] || !_condOpcodes[op]->isValid())
			error("Unimplemented condition opcode %02x", op);

		int numArgs = (*_condOpcodes[op])(env);

		if (numArgs < 0) {
			if (DebugMan.isDebugChannelEnabled(kDebugChannelScript))
				(void)op_debug("FAIL\n");
			return false;
		}

		env.next(numArgs);
	}

	return true;
}

void AdlEngine::doActions(ScriptEnv &env) {
	if (DebugMan.isDebugChannelEnabled(kDebugChannelScript))
		(void)op_debug("THEN");

	while (env.getOpType() == ScriptEnv::kOpTypeAct) {
		byte op = env.op();

		if (op >= _actOpcodes.size() || !_actOpcodes[op] || !_actOpcodes[op]->isValid())
			error("Unimplemented action opcode %02x", op);

		int numArgs = (*_actOpcodes[op])(env);

		if (numArgs < 0) {
			if (DebugMan.isDebugChannelEnabled(kDebugChannelScript))
				(void)op_debug("ABORT\n");
			return;
		}

		env.next(numArgs);
	}

	if (DebugMan.isDebugChannelEnabled(kDebugChannelScript))
		(void)op_debug("END\n");
}

bool AdlEngine::doOneCommand(const Commands &commands, byte verb, byte noun) {
	Commands::const_iterator cmd;

	for (cmd = commands.begin(); cmd != commands.end(); ++cmd) {
		Common::ScopedPtr<ScriptEnv> env(createScriptEnv(*cmd, _state.room, verb, noun));
		if (matchCommand(*env)) {
			doActions(*env);
			return true;
		}

		if (_abortScript) {
			_abortScript = false;
			return false;
		}
	}

	return false;
}

void AdlEngine::doAllCommands(const Commands &commands, byte verb, byte noun) {
	Commands::const_iterator cmd;

	for (cmd = commands.begin(); cmd != commands.end(); ++cmd) {
		Common::ScopedPtr<ScriptEnv> env(createScriptEnv(*cmd, _state.room, verb, noun));
		if (matchCommand(*env)) {
			doActions(*env);
			// The original long jumps on restart, so we need to abort here
			if (_isRestarting)
				return;
		}

		if (_abortScript) {
			_abortScript = false;
			return;
		}
	}
}

ScriptEnv *AdlEngine::createScriptEnv(const Command &cmd, byte room, byte verb, byte noun) {
	return new ScriptEnv_6502(cmd, room, verb, noun);
}

Common::String AdlEngine::toAscii(const Common::String &str) {
	Common::String ascii = Console::toAscii(str);
	if (ascii.lastChar() == '\r')
		ascii.deleteLastChar();
	// FIXME: remove '\r's inside string?
	return ascii;
}

Common::String AdlEngine::itemStr(uint i) const {
	const Item &item(getItem(i));

	Common::String name = Common::String::format("%d", i);
	if (item.noun > 0) {
		name += "/";
		name += _priNouns[item.noun - 1];
	}
	Common::String desc = getItemDescription(item);
	if (!desc.empty()) {
		name += "/";
		name += toAscii(desc);
	}
	return name;
}

Common::String AdlEngine::itemRoomStr(uint i) const {
	switch (i) {
	case IDI_ANY:
		return "CARRYING";
	case IDI_VOID_ROOM:
		return "GONE";
	case IDI_CUR_ROOM:
		return "HERE";
	default:
		return Common::String::format("%d", i);
	}
}

Common::String AdlEngine::roomStr(uint i) const {
	if (i == IDI_ANY)
		return "*";
	else
		return Common::String::format("%d", i);
}

Common::String AdlEngine::verbStr(uint i) const {
	if (i == IDI_ANY)
		return "*";
	else
		return Common::String::format("%d/%s", i, (i - 1 < _priVerbs.size() ? _priVerbs[i - 1].c_str() : "<INVALID>"));
}

Common::String AdlEngine::nounStr(uint i) const {
	if (i == IDI_ANY)
		return "*";
	else
		return Common::String::format("%d/%s", i, (i - 1 < _priNouns.size() ? _priNouns[i - 1].c_str() : "<INVALID>"));
}

Common::String AdlEngine::msgStr(uint i) const {
	return Common::String::format("%d/%s", i, toAscii(loadMessage(i)).c_str());
}

Common::String AdlEngine::dirStr(Direction dir) const {
	static const char *dirs[] = { "NORTH", "SOUTH", "EAST", "WEST", "UP", "DOWN" };
	return dirs[dir];
}

} // End of namespace Adl
