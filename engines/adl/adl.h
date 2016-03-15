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

// Conditional opcodes
#define IDO_CND_ITEM_IN_ROOM   0x03
#define IDO_CND_MOVES_GE       0x05
#define IDO_CND_VAR_EQ         0x06
#define IDO_CND_CUR_PIC_EQ     0x09
#define IDO_CND_ITEM_PIC_EQ    0x0a

// Action opcodes
#define IDO_ACT_VAR_ADD        0x01
#define IDO_ACT_VAR_SUB        0x02
#define IDO_ACT_VAR_SET        0x03
#define IDO_ACT_LIST_ITEMS     0x04
#define IDO_ACT_MOVE_ITEM      0x05
#define IDO_ACT_SET_ROOM       0x06
#define IDO_ACT_SET_CUR_PIC    0x07
#define IDO_ACT_SET_PIC        0x08
#define IDO_ACT_PRINT_MSG      0x09
#define IDO_ACT_SET_LIGHT      0x0a
#define IDO_ACT_SET_DARK       0x0b
#define IDO_ACT_QUIT           0x0d
#define IDO_ACT_SAVE           0x0f
#define IDO_ACT_LOAD           0x10
#define IDO_ACT_RESTART        0x11
#define IDO_ACT_PLACE_ITEM     0x12
#define IDO_ACT_SET_ITEM_PIC   0x13
#define IDO_ACT_RESET_PIC      0x14
#define IDO_ACT_GO_NORTH       0x15
#define IDO_ACT_GO_SOUTH       0x16
#define IDO_ACT_GO_EAST        0x17
#define IDO_ACT_GO_WEST        0x18
#define IDO_ACT_GO_UP          0x19
#define IDO_ACT_GO_DOWN        0x1a
#define IDO_ACT_TAKE_ITEM      0x1b
#define IDO_ACT_DROP_ITEM      0x1c
#define IDO_ACT_SET_ROOM_PIC   0x1d

#define IDI_WORD_SIZE 8

struct Room {
	byte description;
	byte connections[6];
	byte track;
	byte sector;
	byte offset;
	byte picture;
	byte curPicture;
};

struct Picture {
	byte block;
	uint16 offset;
};

struct Command {
	byte room;
	byte verb, noun;
	byte numCond, numAct;
	Common::Array<byte> script;
};

enum {
	IDI_ITEM_NOT_MOVED,
	IDI_ITEM_MOVED,
	IDI_ITEM_DOESNT_MOVE
};

#define IDI_NONE 0xfe

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

	State() : room(1), moves(0), isDark(false) { }
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

	virtual void printMessage(uint idx, bool wait = true) const;
	void delay(uint32 ms) const;

	Common::String inputString(byte prompt = 0) const;
	byte inputKey(bool showCursor = true) const;

	void loadWords(Common::ReadStream &stream, WordMap &map) const;
	void readCommands(Common::ReadStream &stream, Commands &commands);

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
	bool matchCommand(const Command &command, byte verb, byte noun, uint *actions = nullptr) const;
	void doActions(const Command &command, byte noun, byte offset);
	bool doOneCommand(const Commands &commands, byte verb, byte noun);
	void doAllCommands(const Commands &commands, byte verb, byte noun);

	Display *_display;
	GraphicsMan *_graphics;
	Speaker *_speaker;

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
