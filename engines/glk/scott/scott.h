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

#ifndef GLK_SCOTT
#define GLK_SCOTT

/*
 *  Controlling block
 */

#include "common/scummsys.h"
#include "glk/glk_api.h"

namespace Glk {
namespace Scott {

#define LIGHT_SOURCE    9   // Always 9 how odd
#define CARRIED     255     // Carried
#define DESTROYED   0       // Destroyed
#define DARKBIT     15
#define LIGHTOUTBIT 16      // Light gone out

enum GameOption {
	YOUARE      = 1,        ///< You are not I am
	SCOTTLIGHT  = 2,        ///< Authentic Scott Adams light messages
	DEBUGGING   = 4,        ///< Info from database load
	TRS80_STYLE = 8,        ///< Display in style used on TRS-80
	PREHISTORIC_LAMP = 16   ///< Destroy the lamp (very old databases)
};

#define TRS80_LINE  "\n<------------------------------------------------------------>\n"
#define MY_LOC   (_gameHeader._playerRoom)

struct Header {
	int _unknown;
	int _numItems;
	int _numActions;
	int _numWords;           ///< Smaller of verb/noun is padded to same size
	int _numRooms;
	int _maxCarry;
	int _playerRoom;
	int _treasures;
	int _wordLength;
	int _lightTime;
	int _numMessages;
	int _treasureRoom;

	Header() : _unknown(0), _numItems(0), _numActions(0), _numWords(0), _numRooms(0),
		_maxCarry(0), _playerRoom(0), _treasures(0), _wordLength(0), _lightTime(0),
		_numMessages(0), _treasureRoom(0) {}
};

struct Action {
	int _vocab;
	int _condition[5];
	int _action[2];

	Action() : _vocab(0) {
		Common::fill(&_condition[0], &_condition[5], 0);
		Common::fill(&_action[0], &_action[2], 0);
	}
};

struct Room {
	Common::String _text;
	int _exits[6];

	Room() {
		Common::fill(&_exits[0], &_exits[6], 0);
	}
};

struct Item {
	Common::String _text;
	byte _location;
	byte _initialLoc;
	Common::String _autoGet;

	Item() : _location(0), _initialLoc(0) {}
};

struct Tail {
	int _version;
	int _adventureNumber;
	int _unknown;

	Tail() : _version(0), _adventureNumber(0), _unknown(0) {}
};

/**
 * Scott Adams game interpreter
 */
class Scott : public GlkAPI {
private:
	Header _gameHeader;
	Common::Array<Item> _items;
	Common::Array<Room> _rooms;
	Common::StringArray _verbs;
	Common::StringArray _nouns;
	Common::StringArray _messages;
	Common::Array<Action> _actions;
	int _lightRefill;
	char _nounText[16];
	int _counters[16];   ///< Range unknown
	int _currentCounter;
	int _savedRoom;
	int _roomSaved[16];  ///< Range unknown
	int _options;        ///< Option flags set
	int _width;          ///< Terminal width
	int _topHeight;      ///< Height of top window

	bool _splitScreen;
	winid_t _bottomWindow, _topWindow;
	uint32 _bitFlags;    ///< Might be >32 flags - I haven't seen >32 yet
	int _saveSlot;		 ///< Save slot when loading savegame from launcher
private:
	/**
	 * Initialization code
	 */
	void initialize();

	void display(winid_t w, const char *fmt, ...);
	void delay(int seconds);
	void fatal(const char *x);
	void clearScreen(void);
	bool randomPercent(uint n);
	int countCarried(void);
	const char *mapSynonym(const char *word);
	int matchUpItem(const char *text, int loc);
	Common::String readString(Common::SeekableReadStream *f);
	void loadDatabase(Common::SeekableReadStream *f, bool loud);
	void output(const Common::String &a);
	void outputNumber(int a);
	void look(void);
	int whichWord(const char *word, const Common::StringArray &list);
	void lineInput(char *buf, size_t n);
	int getInput(int *vb, int *no);
	int performLine(int ct);
	int performActions(int vb, int no);

	void readInts(Common::SeekableReadStream *f, size_t count, ...);
public:
	/**
	 * Constructor
	 */
	Scott(OSystem *syst, const GlkGameDescription &gameDesc);

	/**
	 * Returns the running interpreter type
	 */
	virtual InterpreterType getInterpreterType() const override { return INTERPRETER_SCOTT; }

	/**
	 * Execute the game
	 */
	virtual void runGame() override;

	/**
	 * Load a savegame from the passed Quetzal file chunk stream
	 */
	virtual Common::Error readSaveData(Common::SeekableReadStream *rs) override;

	/**
	 * Save the game. The passed write stream represents access to the UMem chunk
	 * in the Quetzal save file that will be created
	 */
	virtual Common::Error writeGameData(Common::WriteStream *ws) override;
};

} // End of namespace Scott
} // End of namespace Glk

#endif
