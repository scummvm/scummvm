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

#include "common/random.h"
#include "common/rect.h"

#include "engines/engine.h"

#include "gui/debugger.h"

namespace Common {
class ReadStream;
class SeekableReadStream;
}

namespace Adl {

class Display;
class Parser;
class Console;
struct AdlGameDescription;

enum GameType {
	kGameTypeNone = 0,
	kGameTypeHires1
};

struct StringOffset {
	int stringIdx;
	uint offset;
};

// Messages used outside of scripts
enum EngineMessage {
	IDI_MSG_CANT_GO_THERE,
	IDI_MSG_DONT_UNDERSTAND,
	IDI_MSG_ITEM_DOESNT_MOVE,
	IDI_MSG_ITEM_NOT_HERE,
	IDI_MSG_THANKS_FOR_PLAYING
};

// Strings embedded in the executable
enum EngineString {
	IDI_STR_ENTER_COMMAND,
	IDI_STR_VERB_ERROR,
	IDI_STR_NOUN_ERROR,
	IDI_STR_PLAY_AGAIN,

	IDI_STR_TOTAL
};

struct Room {
	byte description;
	byte connections[6];
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
	bool isDrawing;
	Common::Point position;
	int state;
	byte description;
	Common::Array<byte> roomPictures;
};

typedef Common::List<Command> Commands;

class AdlEngine : public Engine {
public:
	AdlEngine(OSystem *syst, const AdlGameDescription *gd);
	virtual ~AdlEngine();

	const AdlGameDescription *_gameDescription;
	uint32 getFeatures() const;
	const char *getGameId() const;

	static AdlEngine *create(GameType type, OSystem *syst, const AdlGameDescription *gd);

	Common::Error run();
	virtual Common::String getEngineString(int str);

protected:
	virtual void runGame() = 0;
	virtual uint getEngineMessage(EngineMessage msg) = 0;
	Common::String readString(Common::ReadStream &stream, byte until = 0);
	void printStrings(Common::SeekableReadStream &stream, int count = 1);
	virtual void printMessage(uint idx, bool wait = true);
	void wordWrap(Common::String &str);
	void readCommands(Common::ReadStream &stream, Commands &commands);
	bool checkCommand(const Command &command, byte verb, byte noun);
	bool doOneCommand(const Commands &commands, byte verb, byte noun);
	void doAllCommands(const Commands &commands, byte verb, byte noun);
	void doActions(const Command &command, byte noun, byte offset);
	void clearScreen();
	void takeItem(byte noun);
	void dropItem(byte noun);

	Display *_display;
	Parser *_parser;

	Common::Array<Common::String> _strings;
	Common::Array<Common::String> _messages;
	Common::Array<Picture> _pictures;
	Common::Array<Item> _inventory;
	Common::Array<Common::Point> _itemOffsets;
	Common::Array<Common::Array<byte> > _drawings;
	Commands _roomCommands;
	Commands _globalCommands;

	// Game state
	Common::Array<Room> _rooms;
	byte _room;
	uint16 _steps;
	Common::Array<byte> _variables;
	bool _isDark;

private:
	void printEngineMessage(EngineMessage);
};

AdlEngine *HiRes1Engine__create(OSystem *syst, const AdlGameDescription *gd);

} // End of namespace Adl

#endif
