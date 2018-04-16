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

#ifndef ADL_ADL_H
#define ADL_ADL_H

#include "common/debug-channels.h"
#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/func.h"
#include "common/scummsys.h"

#include "engines/engine.h"

#include "audio/mixer.h"
#include "audio/softsynth/pcspk.h"

#include "adl/console.h"
#include "adl/disk.h"
#include "adl/sound.h"
#include "adl/detection.h"

namespace Common {
class ReadStream;
class WriteStream;
class SeekableReadStream;
class File;
struct Event;
}

namespace Adl {

class Console;
class Display;
class GraphicsMan;
struct AdlGameDescription;
class ScriptEnv;

enum kDebugChannels {
	kDebugChannelScript = 1 << 0
};

// Save and restore opcodes
#define IDO_ACT_SAVE           0x0f
#define IDO_ACT_LOAD           0x10

#define IDI_CUR_ROOM 0xfc
#define IDI_VOID_ROOM 0xfd
#define IDI_ANY 0xfe

#define IDI_WORD_SIZE 8

enum Direction {
	IDI_DIR_NORTH,
	IDI_DIR_SOUTH,
	IDI_DIR_EAST,
	IDI_DIR_WEST,
	IDI_DIR_UP,
	IDI_DIR_DOWN,
	IDI_DIR_TOTAL
};

struct Room {
	Room() :
			description(0),
			picture(0),
			curPicture(0),
			isFirstTime(true) {
		memset(connections, 0, sizeof(connections));
	}

	byte description;
	byte connections[IDI_DIR_TOTAL];
	DataBlockPtr data;
	byte picture;
	byte curPicture;
	bool isFirstTime;
};

typedef Common::HashMap<byte, DataBlockPtr> PictureMap;

typedef Common::Array<byte> Script;

struct Command {
	byte room;
	byte verb, noun;
	byte numCond, numAct;
	Script script;
};

class ScriptEnv {
public:
	ScriptEnv(const Command &cmd, byte room, byte verb, byte noun) :
			_cmd(cmd), _room(room), _verb(verb), _noun(noun), _ip(0) { }

	byte op() const { return _cmd.script[_ip]; }
	// We keep this 1-based for easier comparison with the original engine
	byte arg(uint i) const { return _cmd.script[_ip + i]; }
	void skip(uint i) { _ip += i; }

	bool isMatch() const {
		return (_cmd.room == IDI_ANY || _cmd.room == _room) &&
		       (_cmd.verb == IDI_ANY || _cmd.verb == _verb) &&
		       (_cmd.noun == IDI_ANY || _cmd.noun == _noun);
	}

	byte getCondCount() const { return _cmd.numCond; }
	byte getActCount() const { return _cmd.numAct; }
	byte getNoun() const { return _noun; }
	const Command &getCommand() const { return _cmd; }

private:
	const Command &_cmd;
	const byte _room, _verb, _noun;
	byte _ip;
};

enum {
	IDI_ITEM_NOT_MOVED,
	IDI_ITEM_DROPPED,
	IDI_ITEM_DOESNT_MOVE
};

struct Item {
	byte id;
	byte noun;
	byte region;
	byte room;
	byte picture;
	bool isShape;
	Common::Point position;
	int state;
	byte description;
	Common::Array<byte> roomPictures;
	bool isOnScreen;

	Item() : id(0), noun(0), region(0), room(0), picture(0), isShape(false), state(0), description(0), isOnScreen(false) { }
};

struct Time {
	byte hours, minutes;

	Time() : hours(12), minutes(0) { }
};

struct RoomState {
	byte picture;
	byte isFirstTime;
};

struct Region {
	Common::Array<byte> vars;
	Common::Array<RoomState> rooms;
};

struct State {
	Common::Array<Region> regions;
	Common::Array<Room> rooms;
	Common::List<Item> items;
	Common::Array<byte> vars;

	byte region, prevRegion;
	byte room;
	byte curPicture;
	uint16 moves;
	bool isDark;
	Time time;

	State() : region(0), prevRegion(0), room(1), curPicture(0), moves(1), isDark(false) { }
};

typedef Common::List<Command> Commands;
typedef Common::HashMap<Common::String, uint> WordMap;

struct RoomData {
	Common::String description;
	PictureMap pictures;
	Commands commands;
};

// Opcode debugging macros
#define OP_DEBUG_0(F) do { \
	if (DebugMan.isDebugChannelEnabled(kDebugChannelScript) && op_debug(F)) \
		return 0; \
} while (0)

#define OP_DEBUG_1(F, P1) do { \
	if (DebugMan.isDebugChannelEnabled(kDebugChannelScript) && op_debug(F, P1)) \
		return 1; \
} while (0)

#define OP_DEBUG_2(F, P1, P2) do { \
	if (DebugMan.isDebugChannelEnabled(kDebugChannelScript) && op_debug(F, P1, P2)) \
		return 2; \
} while (0)

#define OP_DEBUG_3(F, P1, P2, P3) do { \
	if (DebugMan.isDebugChannelEnabled(kDebugChannelScript) && op_debug(F, P1, P2, P3)) \
		return 3; \
} while (0)

#define OP_DEBUG_4(F, P1, P2, P3, P4) do { \
	if (DebugMan.isDebugChannelEnabled(kDebugChannelScript) && op_debug(F, P1, P2, P3, P4)) \
		return 4; \
} while (0)

class AdlEngine : public Engine {
friend class Console;
public:
	virtual ~AdlEngine();

	bool pollEvent(Common::Event &event) const;
	void bell(uint count = 1) const;

protected:
	AdlEngine(OSystem *syst, const AdlGameDescription *gd);

	// Engine
	Common::Error loadGameState(int slot);
	Common::Error saveGameState(int slot, const Common::String &desc);
	bool canSaveGameStateCurrently();

	Common::String getDiskImageName(byte volume) const { return Adl::getDiskImageName(*_gameDescription, volume); }
	GameType getGameType() const { return Adl::getGameType(*_gameDescription); }
	GameVersion getGameVersion() const { return Adl::getGameVersion(*_gameDescription); }
	virtual void gameLoop();
	virtual void loadState(Common::ReadStream &stream);
	virtual void saveState(Common::WriteStream &stream);
	Common::String readString(Common::ReadStream &stream, byte until = 0) const;
	Common::String readStringAt(Common::SeekableReadStream &stream, uint offset, byte until = 0) const;
	void openFile(Common::File &file, const Common::String &name) const;

	virtual void printString(const Common::String &str) = 0;
	virtual Common::String loadMessage(uint idx) const = 0;
	virtual void printMessage(uint idx);
	virtual Common::String getItemDescription(const Item &item) const;
	void delay(uint32 ms) const;

	virtual Common::String getLine();
	Common::String inputString(byte prompt = 0) const;
	byte inputKey(bool showCursor = true) const;
	void getInput(uint &verb, uint &noun);

	virtual Common::String formatVerbError(const Common::String &verb) const;
	virtual Common::String formatNounError(const Common::String &verb, const Common::String &noun) const;
	void loadWords(Common::ReadStream &stream, WordMap &map, Common::StringArray &pri) const;
	void readCommands(Common::ReadStream &stream, Commands &commands);
	void removeCommand(Commands &commands, uint idx);
	Command &getCommand(Commands &commands, uint idx);
	void checkInput(byte verb, byte noun);
	virtual bool isInputValid(byte verb, byte noun, bool &is_any);
	virtual bool isInputValid(const Commands &commands, byte verb, byte noun, bool &is_any);
	virtual void applyRoomWorkarounds(byte roomNr) { }
	virtual void applyRegionWorkarounds() { }

	virtual void setupOpcodeTables();
	virtual void initState();
	virtual void switchRoom(byte roomNr);
	virtual byte roomArg(byte room) const;
	virtual void advanceClock() { }
	void loadDroppedItemOffsets(Common::ReadStream &stream, byte count);

	// Opcodes
	int o1_isItemInRoom(ScriptEnv &e);
	int o1_isMovesGT(ScriptEnv &e);
	int o1_isVarEQ(ScriptEnv &e);
	int o1_isCurPicEQ(ScriptEnv &e);
	int o1_isItemPicEQ(ScriptEnv &e);

	int o1_varAdd(ScriptEnv &e);
	int o1_varSub(ScriptEnv &e);
	int o1_varSet(ScriptEnv &e);
	int o1_listInv(ScriptEnv &e);
	int o1_moveItem(ScriptEnv &e);
	int o1_setRoom(ScriptEnv &e);
	int o1_setCurPic(ScriptEnv &e);
	int o1_setPic(ScriptEnv &e);
	int o1_printMsg(ScriptEnv &e);
	int o1_setLight(ScriptEnv &e);
	int o1_setDark(ScriptEnv &e);
	int o1_save(ScriptEnv &e);
	int o1_restore(ScriptEnv &e);
	int o1_restart(ScriptEnv &e);
	int o1_quit(ScriptEnv &e);
	int o1_placeItem(ScriptEnv &e);
	int o1_setItemPic(ScriptEnv &e);
	int o1_resetPic(ScriptEnv &e);
	template <Direction D>
	int o1_goDirection(ScriptEnv &e);
	int o1_takeItem(ScriptEnv &e);
	int o1_dropItem(ScriptEnv &e);
	int o1_setRoomPic(ScriptEnv &e);

	// Graphics
	void drawPic(byte pic, Common::Point pos = Common::Point()) const;

	// Sound
	bool playTones(const Tones &tones, bool isMusic, bool allowSkip = false) const;

	// Game state functions
	const Region &getRegion(uint i) const;
	Region &getRegion(uint i);
	const Room &getRoom(uint i) const;
	Room &getRoom(uint i);
	const Region &getCurRegion() const;
	Region &getCurRegion();
	const Room &getCurRoom() const;
	Room &getCurRoom();
	const Item &getItem(uint i) const;
	Item &getItem(uint i);
	byte getVar(uint i) const;
	void setVar(uint i, byte value);
	virtual void takeItem(byte noun);
	virtual void dropItem(byte noun);
	bool matchCommand(ScriptEnv &env) const;
	void doActions(ScriptEnv &env);
	bool doOneCommand(const Commands &commands, byte verb, byte noun);
	void doAllCommands(const Commands &commands, byte verb, byte noun);

	// Debug functions
	static Common::String toAscii(const Common::String &str);
	Common::String itemStr(uint i) const;
	Common::String roomStr(uint i) const;
	Common::String itemRoomStr(uint i) const;
	Common::String verbStr(uint i) const;
	Common::String nounStr(uint i) const;
	Common::String msgStr(uint i) const;
	Common::String dirStr(Direction dir) const;
	bool op_debug(const char *fmt, ...) const;
	Common::DumpFile *_dumpFile;

	Display *_display;
	GraphicsMan *_graphics;
	bool _textMode;

	// Opcodes
	typedef Common::Functor1<ScriptEnv &, int> Opcode;
	Common::Array<const Opcode *> _condOpcodes, _actOpcodes;
	// Message strings in data file
	Common::Array<DataBlockPtr> _messages;
	// Picture data
	PictureMap _pictures;
	// Dropped item screen offsets
	Common::Array<Common::Point> _itemOffsets;
	// <room, verb, noun, script> lists
	Commands _roomCommands;
	Commands _globalCommands;
	// Data related to the current room
	RoomData _roomData;

	WordMap _verbs;
	WordMap _nouns;
	Common::StringArray _priVerbs;
	Common::StringArray _priNouns;

	struct {
		Common::String enterCommand;
		Common::String verbError;
		Common::String nounError;
		Common::String playAgain;
		Common::String pressReturn;
		Common::String lineFeeds;
	} _strings;

	struct {
		uint cantGoThere;
		uint dontUnderstand;
		uint itemDoesntMove;
		uint itemNotHere;
		uint thanksForPlaying;
	} _messageIds;

	// Game state
	State _state;

	uint _linesPrinted;
	bool _isRestarting, _isRestoring, _isQuitting;
	bool _canSaveNow, _canRestoreNow;
	bool _abortScript;

	const AdlGameDescription *_gameDescription;

private:
	virtual void runIntro() { }
	virtual void init() = 0;
	virtual void initGameState() = 0;
	virtual void drawItems() = 0;
	virtual void drawItem(Item &item, const Common::Point &pos) = 0;
	virtual void loadRoom(byte roomNr) = 0;
	virtual void showRoom() = 0;
	virtual void switchRegion(byte region) { }

	// Engine
	Common::Error run();
	bool hasFeature(EngineFeature f) const;
	bool canLoadGameStateCurrently();

	// Text input
	byte convertKey(uint16 ascii) const;
	Common::String getWord(const Common::String &line, uint &index) const;

	Console *_console;
	GUI::Debugger *getDebugger() { return _console; }
	byte _saveVerb, _saveNoun, _restoreVerb, _restoreNoun;
};

} // End of namespace Adl

#endif
