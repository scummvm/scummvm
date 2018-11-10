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

#ifndef GARGOYLE_SCOTT
#define GARGOYLE_SCOTT

/*
 *  Controlling block
 */

#include "common/scummsys.h"
#include "gargoyle/glk.h"

namespace Gargoyle {
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
#define MyLoc   (GameHeader.PlayerRoom)

struct Header {
	int Unknown;
	int NumItems;
	int NumActions;
	int NumWords;           ///< Smaller of verb/noun is padded to same size
	int NumRooms;
	int MaxCarry;
	int PlayerRoom;
	int Treasures;
	int WordLength;
	int LightTime;
	int NumMessages;
	int TreasureRoom;

	Header() : Unknown(0), NumItems(0), NumActions(0), NumWords(0), NumRooms(0),
		MaxCarry(0), PlayerRoom(0), Treasures(0), WordLength(0), LightTime(0),
		NumMessages(0), TreasureRoom(0) {}
};

struct Action {
	uint Vocab;
	uint Condition[5];
	uint action[2];

	Action() : Vocab(0) {
		Common::fill(&Condition[0], &Condition[5], 0);
		Common::fill(&action[0], &action[2], 0);
	}
};

struct Room {
	char *Text;
	short Exits[6];

	Room() : Text(0) {
		Common::fill(&Exits[0], &Exits[6], 0);
	}
};

struct Item {
	char *Text;     // PORTABILITY WARNING: THESE TWO MUST BE 8 BIT VALUES.
	byte Location;
	byte InitialLoc;
	char *AutoGet;

	Item() : Text(nullptr), Location(0), InitialLoc(0), AutoGet(nullptr) {}
};

struct Tail {
	int Version;
	int AdventureNumber;
	int Unknown;

	Tail() : Version(0), AdventureNumber(0), Unknown(0) {}
};

/**
 * Scott Adams game interpreter
 */
class Scott : public Glk {
private:
	Header GameHeader;
	Item *Items;
	Room *Rooms;
	const char **Verbs;
	const char **Nouns;
	const char **Messages;
	Action *Actions;
	int LightRefill;
	char NounText[16];
	int Counters[16];   ///< Range unknown
	int CurrentCounter;
	int SavedRoom;
	int RoomSaved[16];  ///< Range unknown
	int Options;        ///< Option flags set
	int Width;          ///< Terminal width
	int TopHeight;      ///< Height of top window

	bool split_screen;
	winid_t Bottom, Top;
	uint32 BitFlags;    ///< Might be >32 flags - I haven't seen >32 yet
	int _saveSlot;		///< Save slot when loading savegame from launcher
private:
	/**
	 * Initialization code
	 */
	void initialize();

	void display(winid_t w, const char *fmt, ...);
	void delay(int seconds);
	void fatal(const char *x);
	void clearScreen(void);
	void *memAlloc(int size);
	bool randomPercent(uint n);
	int countCarried(void);
	const char *mapSynonym(const char *word);
	int matchUpItem(const char *text, int loc);
	char *readString(Common::SeekableReadStream *f);
	void loadDatabase(Common::SeekableReadStream *f, bool loud);
	void output(const char *a);
	void outputNumber(int a);
	void look(void);
	int whichWord(const char *word, const char **list);
	void lineInput(char *buf, size_t n);
	void saveGame(void);
	void loadGame(void);
	int getInput(int *vb, int *no);
	int performLine(int ct);
	int performActions(int vb, int no);

	int xstrcasecmp(const char *, const char *);
	int xstrncasecmp(const char *, const char *, size_t);
	void readInts(Common::SeekableReadStream *f, size_t count, ...);
public:
	/**
	 * Constructor
	 */
	Scott(OSystem *syst, const GargoyleGameDescription *gameDesc);

	/**
	 * Execute the game
	 */
	virtual void runGame(Common::SeekableReadStream *gameFile) override;

	/**
	 * Load a savegame
	 */
	virtual Common::Error loadGameState(int slot) override;

	/**
	 * Save the game
	 */
	virtual Common::Error saveGameState(int slot, const Common::String &desc) override;
};

} // End of namespace Scott
} // End of namespace Gargoyle

#endif
