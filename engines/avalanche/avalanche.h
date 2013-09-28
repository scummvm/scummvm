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

#ifndef AVALANCHE_AVALANCHE_H
#define AVALANCHE_AVALANCHE_H

#include "avalanche/console.h"
#include "avalanche/graphics.h"
#include "avalanche/parser.h"
#include "avalanche/avalot.h"
#include "avalanche/pingo.h"
#include "avalanche/dialogs.h"
#include "avalanche/background.h"
#include "avalanche/sequence.h"
#include "avalanche/timer.h"
#include "avalanche/animation.h"
#include "avalanche/menu.h"
#include "avalanche/closing.h"
#include "avalanche/sound.h"

#include "common/serializer.h"

#include "engines/engine.h"
#include "engines/advancedDetector.h"

#include "graphics/cursorman.h"

namespace Common {
class RandomSource;
}

namespace Avalanche {

struct AvalancheGameDescription;

static const int kSavegameVersion = 1;

enum Pitch {
	kPitchInvalid,
	kPitchLower,
	kPitchSame,
	kPitchHigher
};

class AvalancheEngine : public Engine {
public:
	byte _saveNum; // number of times this game has been saved

	Clock *_clock;
	Graphics *_graphics;
	Parser *_parser;
	Pingo *_pingo;
	Dialogs *_dialogs;
	Background *_background;
	Sequence *_sequence;
	Timer *_timer;
	Animation *_animation;
	Menu *_menu;
	Closing *_closing;
	SoundHandler *_sound;

	OSystem *_system;

	AvalancheEngine(OSystem *syst, const AvalancheGameDescription *gd);
	~AvalancheEngine();

	Common::ErrorCode initialize();
	GUI::Debugger *getDebugger();

	Common::RandomSource *_rnd;

	const AvalancheGameDescription *_gameDescription;
	uint32 getFeatures() const;
	const char *getGameId() const;
	Common::Platform getPlatform() const;
	bool hasFeature(EngineFeature f) const;
	const char *getCopyrightString() const;

	void synchronize(Common::Serializer &sz);
	virtual bool canSaveGameStateCurrently();
	Common::Error saveGameState(int slot, const Common::String &desc);
	bool saveGame(const int16 slot, const Common::String &desc);
	Common::String getSaveFileName(const int slot);
	virtual bool canLoadGameStateCurrently();
	Common::Error loadGameState(int slot);
	bool loadGame(const int16 slot);
	Common::String expandDate(int d, int m, int y);

	void updateEvents();
	bool getEvent(Common::Event &event); // A wrapper around _eventMan->pollEvent(), so we can use it in Scrolls::normscroll() for example.
	Common::Point getMousePos();

protected:
	// Engine APIs
	Common::Error run();

private:
	static AvalancheEngine *s_Engine;

	AvalancheConsole *_console;
	Common::Platform _platform;

#if 0
	struct {
		byte _operation;
		uint16 _skellern;
		byte _contents[1000];
	} _storage;

	static const int16 kRunShootemup = 1, kRunDosshell = 2, kRunGhostroom = 3, kRunGolden = 4;
	static const int16 kReset = 0;

	static const bool kJsb = true, kNoJsb = false, kBflight = true, kNoBflight = false;

	// From bootstrp:
	enum Elm {kNormal, kMusical, kElmpoyten, kRegi};

	Common::String _argsWithNoFilename;
	byte _originalMode;
	byte *_old1c;
	Common::String _segofs;
	int32 _soundcard, _speed, _baseaddr, _irq, _dma;
	bool _zoomy;

	void run(Common::String what, bool withJsb, bool withBflight, Elm how);
	void bFlightOn();
	void bFlightOff();
	Common::String elmToStr(Elm how);
	bool keyPressed();
	void flushBuffer();
	void dosShell();
	void bFlight();
	Common::String commandCom();
	void explain(byte error);
	void cursorOff();
	void cursorOn();
	void quit();
#endif

public:
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

	static const uint16 kNotes[12];
	static const TuneType kTune;

	static const char *kVersionNum;
	static const char *kCopyright;
	static const int16 kVersionCode = 130; // Same as kVersionCode, but numerically & without the ".".
	static const int16 kGameCode = 2; // Avalot's code number

	bool _holdLeftMouse;
	
	// If this is greater than zero, the next line you type is stored in the DNA in a position dictated by the value.
	// If a scroll comes up, or you leave the room, it's automatically set to zero.
	byte _interrogation;

	// Former DNA structure
	byte _carryNum; // How many objects you're carrying...
	bool _objects[kObjectNum]; // ...and which ones they are.
	int16 _dnascore; // your score, of course
	int32 _money; // your current amount of dosh
	Room _room; // your current room
	byte _roomCount[100]; // Add one to each every time you enter a room
	bool _wonNim; // Have you *won* Nim? (That's harder.)
	byte _wineState; // 0=good (Notts), 1=passable(Argent) ... 3=vinegar.
	bool _cwytalotGone; // Has Cwytalot rushed off to Jerusalem yet?
	byte _passwordNum; // Number of the passw for this game.
	bool _aylesIsAwake; // pretty obvious!
	byte _drawbridgeOpen; // Between 0 (shut) and 4 (open).
	byte _avariciusTalk; // How much Avaricius has said to you.
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
	byte _nextBell; // For the ringing.
	bool _givenPotionToGeida; // Does Geida have the potion?
	bool _lustieIsAsleep; // Is BDL asleep?
	bool _beenTiedUp; // In r__Robins.
	bool _sittingInPub; // Are you sitting down in the pub?
	byte _spurgeTalkCount; // Count for talking to Spurge.
	bool _metAvaroid;
	bool _takenMushroom, _givenPenToAyles, _askedDogfoodAboutNim;
	// End of former DNA Structure

	byte _lineNum; // Number of lines.
	LineType _lines[50]; // For Also.
	bool _dropsOk, _soundFx, _cheat;
	Common::String _mouseText;
	bool _weirdWord;
	bool _letMeOut;
	Common::String _scroll[15];
	byte _scrollNum, _whichwas;
	byte _thinks;
	bool _thinkThing;
	int16 _talkX, _talkY;
	Color _talkBackgroundColor, _talkFontColor;
	byte _scrollBells; // no. of times to ring the bell
	bool _onToolbar, _seeScroll; // TODO: maybe this means we're interacting with the toolbar / a scroll?
	char _objectList[10];
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
	People _him, _her;
	byte _it;
	int32 _roomTime; // Set to 0 when you enter a room, added to in every loop.

	People _lastPerson; // Last person to have been selected using the People menu.
	bool _doingSpriteRun; // Only set to True if we're doing a sprite_run at this moment. This stops the trippancy system from moving any of the sprites.
	bool _holdTheDawn; // If this is true, calling Dawn will do nothing. It's used, for example, at the start, to stop Load from dawning.
	bool _isLoaded; // Is it a loaded gamestate?

	void handleKeyDown(Common::Event &event); // To replace Basher::keyboard_link() and Basher::typein().
	void setup();
	void runAvalot();

	void init();
	void callVerb(VerbCode id);
	void drawAlsoLines();
	void loadRoom(byte num);
	void exitRoom(byte x);
	void enterRoom(Room room, byte ped);
	void thinkAbout(byte object, bool type); // Hey!!! Get it and put it!!!
	void drawToolbar();
	void drawScore();
	void incScore(byte num); // Add on no. of points
	void useCompass(const Common::Point &cursorPos); // Click on the compass on the toolbar to control Avvy's movement.
	void fxToggle();
	void refreshObjectList();
	void checkClick();
	void errorLed();
	void dusk();
	void dawn();
	void drawDirection(); // Draws the little icon at the left end of the text input field.
	void gameOver();
	uint16 bearing(byte whichPed); // Returns the bearing from ped 'whichped' to Avvy, in degrees.
	void fixFlashers();
	void loadAlso(byte num);

	// There are two kinds of redraw: Major and Minor. Minor is what happens when you load a game, etc. Major redraws EVERYTHING.
	void minorRedraw();
	void majorRedraw();

	void spriteRun();

	Common::String intToStr(int32 num);
	void newMouse(byte id);
	void setMousePointerWait();    // Makes hourglass.
	void loadMouse(byte which);

	void setBackgroundColor(byte x);
	
	void resetVariables();
	void newGame(); // This sets up the DNA for a completely new game.
	void slowDown();
	bool setFlag(char x);
	bool decreaseMoney(uint16 amount); // Called pennycheck in the original.
	void hangAroundForAWhile();
	
	Common::String getName(People whose);
	byte getNameChar(People whose);
	Common::String getThing(byte which);
	char getThingChar(byte which);
	Common::String getItem(byte which); // Called get_better in the original.
	Common::String f5Does(); // This procedure determines what f5 does.

	void openDoor(Room whither, byte ped, byte magicnum); // Handles slidey-open doors.
	void flipRoom(Room room, byte ped);

	void setRoom(People persId, Room roomId);
	Room getRoom(People persId);
private:
	static Room _whereIs[29];

	Common::File file;
	Common::String readAlsoStringFromFile();
	void scram(Common::String &str);
	void unScramble();

	void enterNewTown();
	void findPeople(byte room);
	void putGeidaAt(byte whichPed, byte ped);
	void guideAvvy(Common::Point cursorPos);

	// Will be used in dusk() and dawn().
	bool _fxHidden;

	int8 fades(int8 x);
	void fadeOut(byte n);
	void fadeIn(byte n);
};

} // End of namespace Avalanche

#endif // AVALANCHE_AVALANCHE_H
