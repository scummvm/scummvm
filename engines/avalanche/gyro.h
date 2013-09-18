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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

/* GYRO		It all revolves around this bit! */

#ifndef AVALANCHE_GYRO2_H
#define AVALANCHE_GYRO2_H

#include "common/str.h"
#include "common/scummsys.h"
#include "common/file.h"

#include "graphics/surface.h"

#include "avalanche/roomnums.h"
#include "avalanche/color.h"

namespace Avalanche {
class AvalancheEngine;

static const byte kObjectNum = 18; // always preface with a #
static const int16 kCarryLimit = 12;  // carry limit

static const int16 kNumlockCode = 32;  // Code for Num Lock
static const int16 kMouseSize = 134;

struct MouseHotspotType { // mouse-void
	int16 _horizontal, _vertical;
};

struct PedType {
	int16 _x, _y;
	byte _direction;
};

struct MagicType {
	byte _operation; // one of the operations
	uint16 _data; // data for them
};

class FieldType {
public:
	int16 _x1, _y1, _x2, _y2;
};

struct ByteField {
	byte _x1, _y1, _x2, _y2;
};

class LineType : public FieldType {
public:
	byte _color;
};

struct DemoType {
	uint16 _delay;
	char _key, _extd;
};

typedef byte TuneType[31];

struct QuasipedType {
	byte _whichPed, _foregroundColor, _room, _backgroundColor;
	uint16 _who;
};

#if 0
struct Sundry { // Things which must be saved over a backtobootstrap, outside DNA.
	Common::String _qEnidFilename;
	bool _qSoundFx;
	byte _qThinks;
	bool _qThinkThing;
};

struct ednahead { // Edna header
	// This header starts at byte offset 177 in the .ASG file.
	char id[9];   // signature
	uint16 revision; // EDNA revision, here 2 (1=dna256)
	Common::String game; // Long name, eg Lord Avalot D'Argent
	Common::String shortname; // Short name, eg Avalot
	uint16 number; // Game's code number, here 2
	uint16 ver; // Version number as int16 (eg 1.00 = 100)
	Common::String verstr; // Vernum as Common::String (eg 1.00 = "1.00")
	Common::String filename; // Filename, eg AVALOT.EXE
	byte osbyte; // Saving OS (here 1=DOS. See below for others.
	Common::String os; // Saving OS in text format.

	// Info on this particular game

	Common::String fn; // Filename (not extension ('cos that's .ASG))
	byte d, m; // D, M, Y are the Day, Month & Year this game was...
	uint16 y;  // ...saved on.
	Common::String desc; // Description of game (same as in Avaricius!)
	uint16 len; // Length of DNA (it's not going to be above 65535!)

	// Quick reference & miscellaneous

	uint16 saves; // no. of times this game has been saved
	int16 cash; // contents of your wallet in numerical form
	Common::String money; // ditto in Common::String form (eg 5/-, or 1 denarius)
	uint16 points; // your score

	// DNA values follow, then footer (which is ignored)
};
#endif

class Gyro {
public:
	static const char *kVersionNum;
	static const char *kCopyright;
	static const int16 kVersionCode = 130; // Same as kVersionCode, but numerically & without the ".".
	static const int16 kGameCode = 2; // Avalot's code number

	// Objects you can hold:
	enum Object {
		kObjectWine = 1,
		kObjectMoney,
		kObjectBodkin,
		kObjectPotion,
		kObjectChastity,
		kObjectBolt,
		kObjectCrossbow,
		kObjectLute,
		kObjectBadge,
		kObjectMushroom,
		kObjectKey,
		kObjectBell,
		kObjectPrescription,
		kObjectPen,
		kObjectInk,
		kObjectClothes,
		kObjectHabit,
		kObjectOnion
	};

	// People who hang around this game.
	enum People {
		// Boys:
		kPeopleAvalot = 150,
		kPeopleSpludwick = 151,
		kPeopleCrapulus = 152,
		kPeopleDrDuck = 153,
		kPeopleMalagauche = 154,
		kPeopleFriarTuck = 155,
		kPeopleRobinHood = 156,
		kPeopleCwytalot = 157,
		kPeopleDuLustie = 158,
		kPeopleDuke = 159,
		kPeopleDogfood = 160,
		kPeopleTrader = 161,
		kPeopleIbythneth = 162,
		kPeopleAyles = 163,
		kPeoplePort = 164,
		kPeopleSpurge = 165,
		kPeopleJacques = 166,
		// Girls:
		kPeopleArkata = 175,
		kPeopleGeida = 176,
		kPeopleInvisible = 177,
		kPeopleWisewoman = 178
	};

	static const int16 kXW = 30;
	static const int16 kYW = 36; // x width & y whatsit
	static const int16 kMargin = 5;
	static const MouseHotspotType kMouseHotSpots[9];
	static const int16 kMaxSprites = 2; // Current max no. of sprites.

	// For Thinkabout:
	static const bool kThing = true;
	static const bool kPerson = false;

	// Magic/portal constants:
	enum Magics {
		kMagicNothing, // Ignore it if this line is touched.
		kMagicBounce, // Bounce off this line. Not valid for portals.
		kMagicExclaim, // Put up a chain of scrolls.
		kMagicTransport, // Enter new room.
		kMagicUnfinished, // Unfinished connection.
		kMagicSpecial, // Special function.
		kMagicOpenDoor // Opening door.
	};

	// These following static constants should be included in CFG when it's written.

	static const bool kSlowComputer = false; // Stops walking when mouse touches toolbar.
	static const int16 kBorder = 1; // size of border on shadowboxes
	static const int16 kWalk = 3;
	static const int16 kRun = 5;
	static const int32 kCatacombMap[8][8];
	static const char kSpludwicksOrder[3];
	static const QuasipedType kQuasipeds[16];

	enum Pitch {
		kPitchLower,
		kPitchSame,
		kPitchHigher
	};

	static const char kMusicKeys[];
	static const uint16 kNotes[12];

	static const TuneType kTune;



	// If this is greater than zero, the next line you type is stored in the DNA in a position dictated by the value.
	// If a scroll comes up, or you leave the room, it's automatically set to zero.
	byte _interrogation;

	static byte _whereIs[29];

	// Variable static constant for overriding the ability of On to switch pages.
	// You may know better than On which page to switch to.
	bool _onCanDoPageSwap;

	// Former DNA structure
	byte _carryNum; // How many objects you're carrying...
	bool _objects[kObjectNum]; // ...and which ones they are.
	int16 _dnascore; // your score, of course
	int32 _money; // your current amount of dosh
	byte _room; // your current room
	byte _wearing; // what you're wearing
	byte _sworeNum; // number of times you've sworn
	byte _saveNum; // number of times this game has been saved
	byte _roomCount[100]; // Add one to each every time you enter a room
	byte _alcoholLevel; // Your blood alcohol level.
	byte _playedNim; // How many times you've played Nim.
	bool _wonNim; // Have you *won* Nim? (That's harder.)
	byte _wineState; // 0=good (Notts), 1=passable(Argent) ... 3=vinegar.
	bool _cwytalotGone; // Has Cwytalot rushed off to Jerusalem yet?
	byte _passwordNum; // Number of the passw for this game.
	bool _aylesIsAwake; // pretty obvious!
	byte _drawbridgeOpen; // Between 0 (shut) and 4 (open).
	byte _avariciusTalk; // How much Avaricius has said to you.
	bool _boughtOnion; // Have you bought an onion yet?
	bool _rottenOnion; // And has it rotted?
	bool _onionInVinegar; // Is the onion in the vinegar?
	byte _givenToSpludwick; // 0 = nothing given, 1 = onion...
	byte _brummieStairs; // Progression through the stairs trick.
	byte _cardiffQuestionNum; // Things you get asked in Cardiff.
	bool _passedCwytalotInHerts; // Have you passed Cwytalot in Herts?
	bool _avvyIsAwake; // Well? Is Avvy awake? (Screen 1 only.)
	bool _avvyInBed; // True if Avvy's in bed, but awake.
	bool _userMovesAvvy; // If this is false, the user has no control over Avvy's movements.
	byte _npcFacing; // If there's an NPC in the current room which turns it's head according to Avvy's movement (keep looking at him), this variable tells which way it's facing at the moment.
	bool _givenBadgeToIby; // Have you given the badge to Iby yet?
	bool _friarWillTieYouUp; // If you're going to get tied up.
	bool _tiedUp; // You ARE tied up!
	byte _boxContent; // 0 = money (sixpence), 254 = empty, any other number implies the contents of the box.
	bool _talkedToCrapulus; // Pretty self-explanatory.
	byte _jacquesState; // 0=asleep, 1=awake, 2=gets up, 3=gone.
	bool _bellsAreRinging; // Is Jacques ringing the bells?
	bool _standingOnDais; // In room 71, inside Cardiff Castle.
	bool _takenPen; // Have you taken the pen (in Cardiff?)
	bool _arrowTriggered; // And has the arrow been triggered?
	bool _arrowInTheDoor;  // Did the arrow hit the wall?
	Common::String _favouriteDrink, _favouriteSong, _worstPlaceOnEarth, _spareEvening; // Personalisation str's
	uint32 _totalTime; // Your total time playing this game, in ticks.
	byte _jumpStatus; // Fixes how high you're jumping.
	bool _mushroomGrowing; // Is the mushroom growing in 42?
	bool _spludwickAtHome; // Is Spludwick at home?
	byte _lastRoom;
	byte _lastRoomNotMap;
	bool _crapulusWillTell; // Will Crapulus tell you about Spludwick being away?
	bool _enterCatacombsFromLustiesRoom;
	bool _teetotal; // Are we touching any more drinks?
	byte _malagauche; // Position of Malagauche. See Celer for more info.
	char _drinking; // What's he getting you?
	bool _enteredLustiesRoomAsMonk;
	byte _catacombX, _catacombY;   // XY coords in the catacombs.
	bool _avvysInTheCupboard; // On screen 22.
	bool _geidaFollows; // Is Geida following you?
	byte _geidaSpin, _geidaTime; // For the making "Geida dizzy" joke.
	byte _nextBell; // For the ringing.
	bool _givenPotionToGeida; // Does Geida have the potion?
	bool _lustieIsAsleep; // Is BDL asleep?
	byte _flipToWhere, _flipToPed; // For the sequencer.
	bool _beenTiedUp; // In r__Robins.
	bool _sittingInPub; // Are you sitting down in the pub?
	byte _spurgeTalkCount; // Count for talking to Spurge.
	bool _metAvaroid;
	bool _takenMushroom, _givenPenToAyles, _askedDogfoodAboutNim;
	// End of former DNA Structure

	byte _lineNum; // Number of lines.
	LineType _lines[50]; // For Also.
	enum MouseState { kMouseStateNo, kMouseStateYes, kMouseStateVirtual } _mouse;
	bool _dropsOk, _scReturn, _soundFx, _cheat;
	Common::String _mouseText;
	bool _weirdWord;
	bool _letMeOut;
	Common::String _scroll[15];
	byte _scrollNum, _whichwas;
	byte _thinks;
	bool _thinkThing;
	int16 _talkX, _talkY;
	byte _talkBackgroundColor, _talkFontColor;
	byte _scrollBells; // no. of times to ring the bell
	bool _onToolbar, _seeScroll; // TODO: maybe this means we're interacting with the toolbar / a scroll?
	char _objectList[10];
	::Graphics::Surface _digits[10]; // digitsize and rwlitesize are defined in Lucerna::load_digits() !!!
	::Graphics::Surface _directions[9]; // Maybe it will be needed to move them to the class itself instead.
	// Called .free() for them in ~Gyro().
	int8 _scoreToDisplay[3];
	byte _currentMouse; // current mouse-void
	Common::String _verbStr; // what you can do with your object. :-)
	Common::String *_also[31][2];
	PedType _peds[15];
	MagicType _magics[15];
	MagicType _portals[7];
	FieldType _fields[30];
	byte _fieldNum;
	Common::String _flags;
	Common::String _listen;
	Common::String _atKey; // For XTs, set to "alt-". For ATs, set to "f1".
	byte _cp, _ledStatus, _defaultLed;
	FontType _font;
	bool _alive;
	byte _buffer[2000];
	uint16 _bufSize;
	int16 _underScroll; // Y-coord of just under the scroll text.
	Common::String _roomnName; // Name of actual room
	Common::String _subject; // What you're talking to them about.
	byte _subjectNum; // The same thing.
	bool _keyboardClick; // Is a keyboard click noise wanted?
	byte _him, _her, _it;
	int32 _roomTime; // Set to 0 when you enter a room, added to in every loop.

	// For the demo:
	DemoType _demoRec;
	Common::File _demoFile; // of demo_type
	Common::DumpFile _demoFileSave; // uruk added it - first use located in constructor of Basher

	byte _lastPerson; // Last person to have been selected using the People menu.
	bool _doingSpriteRun; // Only set to True if we're doing a sprite_run at this moment. This stops the trippancy system from moving any of the sprites.
	bool _holdTheDawn; // If this is true, calling Dawn will do nothing. It's used, for example, at the start, to stop Load from dawning.
	bool _isLoaded; // Is it a loaded gamestate?
	Common::String _enidFilename;

	Gyro(AvalancheEngine *vm);
	~Gyro();

	Common::String intToStr(int32 num);
	void newMouse(byte id);
	void setMousePointerWait();    // Makes hourglass.
	void loadMouse(byte which);

	void note(uint16 hertz);
	void blip();
	void click(); // "Audio keyboard feedback"

	void setBackgroundColor(byte x);
	void drawShadowBox(int16 x1, int16 y1, int16 x2, int16 y2, Common::String t);

	void resetVariables();
	void newGame(); // This sets up the DNA for a completely new game.
	void slowDown();
	bool setFlag(char x);
	bool decreaseMoney(uint16 howmuchby); // Called pennycheck in the original.
	void hangAroundForAWhile();
	
	Common::String getName(byte whose);
	byte getNameChar(byte whose);
	Common::String getThing(byte which);
	char getThingChar(byte which);
	Common::String getItem(byte which); // Called get_better in the original.
	Common::String f5Does(); // This procedure determines what f5 does.

	void init();
private:
	AvalancheEngine *_vm;

	void drawShadow(int16 x1, int16 y1, int16 x2, int16 y2, byte hc, byte sc);
};

} // End of namespace Avalanche

#endif // AVALANCHE_GYRO2_H
