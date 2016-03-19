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

#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/func.h"

#include "engines/engine.h"

#include "audio/mixer.h"
#include "audio/softsynth/pcspk.h"

namespace Common {
class ReadStream;
class SeekableReadStream;
class File;
struct Event;
}

namespace Adl {

class Display;
class GraphicsMan;
class Speaker;
struct AdlGameDescription;
struct ScriptEnv;

// Save and restore opcodes
#define IDO_ACT_SAVE           0x0f
#define IDO_ACT_LOAD           0x10

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
	byte description;
	byte connections[IDI_DIR_TOTAL];
	byte track;
	byte sector;
	byte offset;
	byte picture;
	byte curPicture;
	bool isFirstTime;
};

struct Picture {
	byte block;
	uint16 offset;
};

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
	byte noun;
	byte room;
	byte picture;
	bool isLineArt;
	Common::Point position;
	int state;
	byte description;
	Common::Array<byte> roomPictures;
};

struct State {
	Common::Array<Room> rooms;
	Common::Array<Item> items;
	Common::Array<byte> vars;

	byte room;
	uint16 moves;
	bool isDark;

	State() : room(1), moves(1), isDark(false) { }
};

typedef Common::List<Command> Commands;
typedef Common::HashMap<Common::String, uint> WordMap;

class AdlEngine : public Engine {
public:
	virtual ~AdlEngine();

	static bool pollEvent(Common::Event &event);

protected:
	AdlEngine(OSystem *syst, const AdlGameDescription *gd);

	Common::String readString(Common::ReadStream &stream, byte until = 0) const;
	Common::String readStringAt(Common::SeekableReadStream &stream, uint offset, byte until = 0) const;
	void openFile(Common::File &file, const Common::String &name) const;

	virtual void printMessage(uint idx, bool wait = true);
	void delay(uint32 ms) const;

	Common::String inputString(byte prompt = 0) const;
	byte inputKey(bool showCursor = true) const;

	void loadWords(Common::ReadStream &stream, WordMap &map) const;
	void readCommands(Common::ReadStream &stream, Commands &commands);
	virtual void checkInput(byte verb, byte noun);

	virtual void setupOpcodeTables();
	virtual bool matchesCurrentPic(byte pic) const;

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
	void clearScreen() const;
	void drawItems() const;

	// Sound
	void bell(uint count = 1) const;

	// Game state functions
	const Room &getRoom(uint i) const;
	Room &getRoom(uint i);
	const Room &getCurRoom() const;
	Room &getCurRoom();
	const Item &getItem(uint i) const;
	Item &getItem(uint i);
	byte getVar(uint i) const;
	void setVar(uint i, byte value);
	void takeItem(byte noun);
	void dropItem(byte noun);
	bool matchCommand(ScriptEnv &env) const;
	void doActions(ScriptEnv &env);
	bool doOneCommand(const Commands &commands, byte verb, byte noun);
	void doAllCommands(const Commands &commands, byte verb, byte noun);

	Display *_display;
	GraphicsMan *_graphics;
	Speaker *_speaker;

	// Opcodes
	typedef Common::Functor1<ScriptEnv &, int> Opcode;
	Common::Array<const Opcode *> _condOpcodes, _actOpcodes;
	// Message strings in data file
	Common::Array<Common::String> _messages;
	// Picture data
	Common::Array<Picture> _pictures;
	// Dropped item screen offsets
	Common::Array<Common::Point> _itemOffsets;
	// <room, verb, noun, script> lists
	Commands _roomCommands;
	Commands _globalCommands;

	WordMap _verbs;
	WordMap _nouns;

	struct {
		Common::String enterCommand;
		Common::String verbError;
		Common::String nounError;
		Common::String playAgain;
		Common::String pressReturn;
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

private:
	virtual void runIntro() const { }
	virtual void init() = 0;
	virtual void initState() = 0;
	virtual void restartGame() = 0;
	virtual void drawItem(const Item &item, const Common::Point &pos) const = 0;
	virtual void showRoom() = 0;

	// Engine
	Common::Error run();
	bool hasFeature(EngineFeature f) const;
	Common::Error loadGameState(int slot);
	bool canLoadGameStateCurrently();
	Common::Error saveGameState(int slot, const Common::String &desc);
	bool canSaveGameStateCurrently();

	// Text output
	void wordWrap(Common::String &str) const;

	// Text input
	byte convertKey(uint16 ascii) const;
	Common::String getLine() const;
	Common::String getWord(const Common::String &line, uint &index) const;
	void getInput(uint &verb, uint &noun);

	const AdlGameDescription *_gameDescription;
	bool _isRestarting, _isRestoring;
	byte _saveVerb, _saveNoun, _restoreVerb, _restoreNoun;
	bool _canSaveNow, _canRestoreNow;
};

AdlEngine *HiRes1Engine__create(OSystem *syst, const AdlGameDescription *gd);

} // End of namespace Adl

#endif
