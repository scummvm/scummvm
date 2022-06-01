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

/* Based on ScottFree interpreter version 1.14 */

#ifndef GLK_SCOTT_H
#define GLK_SCOTT_H

/*
 *  Controlling block
 */

#include "common/scummsys.h"
#include "glk/glk_api.h"
#include "glk/scott/definitions.h"
#include "glk/scott/globals.h"
#include "glk/scott/types.h"

namespace Glk {
namespace Scott {

struct Command;

#define LIGHT_SOURCE 9 // Always 9 how odd
#define CARRIED 255    // Carried
#define DESTROYED 0    // Destroyed
#define DARKBIT 15
#define LIGHTOUTBIT 16 // Light gone out

enum GameOption {
	YOUARE = 1,                ///< You are not I am
	SCOTTLIGHT = 2,            ///< Authentic Scott Adams light messages
	DEBUGGING = 4,             ///< Info from database load
	TRS80_STYLE = 8,           ///< Display in style used on TRS-80
	PREHISTORIC_LAMP = 16,     ///< Destroy the lamp (very old databases)
	SPECTRUM_STYLE = 32,       ///< Display in style used on ZX Spectrum
	TI994A_STYLE = 64,         ///< Display in style used on TI-99/4A
	NO_DELAYS = 128,           ///< Skip all pauses
	FORCE_PALETTE_ZX = 256,    ///< Force ZX Spectrum image palette
	FORCE_PALETTE_C64 = 512,   ///< Force CBM 64 image palette
	FORCE_INVENTORY = 1024,    ///< Inventory in upper window always on
	FORCE_INVENTORY_OFF = 2048 ///< Inventory in upper window always off
};

#define GLK_BUFFER_ROCK 1
#define GLK_STATUS_ROCK 1010
#define GLK_GRAPHICS_ROCK 1020

#define TRS80_LINE "\n<------------------------------------------------------------>\n"
#define MY_LOC (_G(_gameHeader)->_playerRoom)
#define CURRENT_GAME (_G(_game->_gameID))

struct Header {
	int _unknown;
	int _numItems;
	int _numActions;
	int _numWords; ///< Smaller of verb/noun is padded to same size
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
	byte _image;

	Room() : _image(255) {
		Common::fill(&_exits[0], &_exits[6], 0);
	}
};

struct Item {
	Common::String _text;
	byte _location;
	byte _initialLoc;
	Common::String _autoGet;
	byte _flag;
	byte _image;

	Item() : _location(0), _initialLoc(0), _flag(0), _image(0) {}
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
	Globals _globals;
	char _nounText[16];
	int _options = 0;   ///< Option flags set
	//int _width = 0;     ///< Terminal width
	int _topHeight = 0; ///< Height of top window
	int _topWidth = 0;

	bool _splitScreen = true;
	int _saveSlot = -1;   ///< Save slot when loading savegame from launcher
	Common::String _titleScreen;

	int _shouldLookInTranscript = 0;
	int _printLookToTranscript = 0;
	int _pauseNextRoomDescription = 0;

	strid_t _roomDescriptionStream = nullptr;

private:
	/**
	 * Initialization code
	 */
	void initialize();

	void updateSettings();
	void delay(int seconds);
	void clearScreen(void);
	bool randomPercent(uint n);
	int countCarried(void);
	int matchUpItem(int noun, int loc);
	Common::String readString(Common::SeekableReadStream *f);
	void loadDatabase(Common::SeekableReadStream *f, bool loud);
	void outputNumber(int a);
	void look(void);
	int whichWord(const char *word, const Common::StringArray &list);

	ActionResultType performLine(int ct);
	ExplicitResultType performActions(int vb, int no);

	void readInts(Common::SeekableReadStream *f, size_t count, ...);
	void writeToRoomDescriptionStream(const char *fmt, ...);
	void flushRoomDescription(char *buf);
	void printWindowDelimiter();
	void listExits();
	void listExitsSpectrumStyle();
	void listInventoryInUpperWindow();
	int itemEndsWithPeriod(int item);
	void closeGraphicsWindow();
	winid_t findGlkWindowWithRock(glui32 rock);
	void openGraphicsWindow();
	glui32 optimalPictureSize(uint *width, uint *height);
	void openTopWindow();
	void cleanupAndExit();
	void drawBlack();
	void drawRoomImage();
	void restartGame();
	void transcriptOn();
	void transcriptOff();
	int yesOrNo();
	void listInventory();
	void lookWithPause();
	void doneIt();
	int printScore();
	void printNoun();
	void moveItemAToLocOfItemB(int itemA, int itemB);
	void goToStoredLoc();
	void swapLocAndRoomFlag(int index);
	void swapItemLocations(int itemA, int itemB);
	void putItemAInRoomB(int itemA, int roomB);
	void swapCounters(int index);
	void printMessage(int index);
	void playerIsDead();
	void printTakenOrDropped(int index);
	void printTitleScreenBuffer();
	void printTitleScreenGrid();

public:
	void drawImage(int image);
	void output(const Common::String &a);
	void output(const Common::U32String &a);
	void display(winid_t w, const char *fmt, ...);
	void display(winid_t w, const Common::U32String fmt, ...);
	void fatal(const char *x);
	void hitEnter();
	void updates(event_t ev);
	const char *mapSynonym(int noun);
	int performExtraCommand(int extraStopTime);

public:
	/**
	 * Constructor
	 */
	Scott(OSystem *syst, const GlkGameDescription &gameDesc);

	/**
	 * Returns the running interpreter type
	 */
	InterpreterType getInterpreterType() const override { return INTERPRETER_SCOTT; }

	/**
	 * Execute the game
	 */
	void runGame() override;

	/**
	 * Load a savegame from the passed Quetzal file chunk stream
	 */
	Common::Error readSaveData(Common::SeekableReadStream *rs) override;

	/**
	 * Save the game. The passed write stream represents access to the UMem chunk
	 * in the Quetzal save file that will be created
	 */
	Common::Error writeGameData(Common::WriteStream *ws) override;
};

extern Scott *g_scott;

} // End of namespace Scott
} // End of namespace Glk

#endif
