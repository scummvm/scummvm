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
#include "avalanche/dialogs.h"
#include "avalanche/background.h"
#include "avalanche/sequence.h"
#include "avalanche/timer.h"
#include "avalanche/animation.h"
#include "avalanche/dropdown.h"
#include "avalanche/closing.h"
#include "avalanche/sound.h"
#include "avalanche/nim.h"
#include "avalanche/clock.h"
#include "avalanche/ghostroom.h"
#include "avalanche/help.h"
#include "avalanche/shootemup.h"
#include "avalanche/mainmenu.h"
#include "avalanche/highscore.h"

#include "common/error.h"
#include "common/serializer.h"

#include "engines/engine.h"

#include "graphics/cursorman.h"

namespace Common {
class RandomSource;
}

namespace Avalanche {

struct AvalancheGameDescription;

static const int kSavegameVersion = 2;

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
	GraphicManager *_graphics;
	Parser *_parser;
	Dialogs *_dialogs;
	Background *_background;
	Sequence *_sequence;
	Timer *_timer;
	Animation *_animation;
	DropDownMenu *_dropdown;
	Closing *_closing;
	SoundHandler *_sound;
	Nim *_nim;
	GhostRoom *_ghostroom;
	Help *_help;
	HighScore *_highscore;

	OSystem *_system;

	AvalancheEngine(OSystem *syst, const AvalancheGameDescription *gd);
	~AvalancheEngine() override;

	Common::ErrorCode initialize();

	Common::RandomSource *_rnd;

	const AvalancheGameDescription *_gameDescription;
	uint32 getFeatures() const;
	const char *getGameId() const;
	Common::Platform getPlatform() const;
	bool hasFeature(EngineFeature f) const override;
	const char *getCopyrightString() const;

	void synchronize(Common::Serializer &sz);
	bool canSaveGameStateCurrently() override;
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;
	bool saveGame(const int16 slot, const Common::String &desc);
	bool canLoadGameStateCurrently() override;
	Common::Error loadGameState(int slot) override;
	bool loadGame(const int16 slot);
	Common::String expandDate(int d, int m, int y);
	uint32 getTimeInSeconds();

	void updateEvents();
	bool getEvent(Common::Event &event); // A wrapper around _eventMan->pollEvent(), so we can use it in Scrolls::normscroll() for example.
	Common::Point getMousePos();

protected:
	// Engine APIs
	Common::Error run() override;

public:
	// For Thinkabout:
	static const bool kThing = true;
	static const bool kPerson = false;

	static const char kSpludwicksOrder[3];

	static const uint16 kNotes[12];

	bool _holdLeftMouse;

	// If this is greater than zero, the next line you type is stored in the DNA in a position dictated by the value.
	// If a scroll comes up, or you leave the room, it's automatically set to zero.
	byte _interrogation;

	// Former DNA structure
	byte _carryNum; // How many objects you're carrying...
	bool _objects[kObjectNum]; // ...and which ones they are.
	int16 _score; // your score, of course
	int32 _money; // your current amount of dosh
	Room _room; // your current room
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
	bool _arrowInTheDoor;  // Did the arrow hit the wall?
	Common::String _favoriteDrink, _favoriteSong, _worstPlaceOnEarth, _spareEvening; // Personalisation str's
	uint32 _startTime; // When did you start playing this session?
	uint32 _totalTime; // Your total time playing this game, in seconds. Updated only at saving and loading.
	byte _jumpStatus; // Fixes how high you're jumping.
	bool _mushroomGrowing; // Is the mushroom growing in 42?
	bool _crapulusWillTell; // Will Crapulus tell you about Spludwick being away?
	bool _enterCatacombsFromLustiesRoom;
	bool _teetotal; // Are we touching any more drinks?
	byte _malagauche; // Position of Malagauche. See Celer for more info.
	char _drinking; // What's he getting you?
	bool _enteredLustiesRoomAsMonk;
	byte _catacombX, _catacombY;   // XY coords in the catacombs.
	bool _avvysInTheCupboard; // On screen 22.
	bool _geidaFollows; // Is Geida following you?
	bool _givenPotionToGeida; // Does Geida have the potion?
	bool _lustieIsAsleep; // Is BDL asleep?
	bool _beenTiedUp; // In r__Robins.
	bool _sittingInPub; // Are you sitting down in the pub?
	byte _spurgeTalkCount; // Count for talking to Spurge.
	bool _metAvaroid;
	bool _takenMushroom, _givenPenToAyles, _askedDogfoodAboutNim;
	// End of former DNA Structure

	bool _showDebugLines;
	byte _lineNum; // Number of lines.
	LineType _lines[50]; // For Also.
	bool _dropsOk;
	bool _cheat; // CHECKME: Currently unused
	bool _letMeOut;
	byte _thinks;
	bool _thinkThing;
	bool _animationsEnabled; // If set to TRUE, it stops the animation system working. This prevents display of the new sprites before the new picture is loaded or during the display of a scroll. Original name: seescroll.
	char _objectList[10];
	// Called .free() for them in ~Gyro().

	byte _currentMouse; // current mouse-void
	Common::String *_also[31][2];
	PedType _peds[15];
	MagicType _magics[15];
	MagicType _portals[7];
	FieldType _fields[30];
	byte _fieldNum;
	Common::String _listen;
	byte _cp, _ledStatus;
	FontType _font;
	bool _alive;
	byte _subjectNum; // The same thing.
	People _him, _her;
	byte _it;
	uint32 _roomCycles; // Set to 0 when you enter a room, added to in every loop. Cycles since you've been in this room.

	bool _doingSpriteRun; // Only set to True if we're doing a sprite_run at this moment. This stops the trippancy system from moving any of the sprites.
	bool _soundFx;

	bool _isLoaded; // Is it a loaded gamestate?

	void callVerb(VerbCode id);
	void loadBackground(byte num);
	void loadRoom(byte num);
	void thinkAbout(byte object, bool type); // Hey!!! Get it and put it!!!
	void incScore(byte num); // Add on no. of points
	void fxToggle();
	void refreshObjectList();
	void errorLed();
	void fadeOut();
	void fadeIn();
	void drawDirection(); // Draws the little icon at the left end of the text input field.
	void gameOver();
	uint16 bearing(byte whichPed); // Returns the bearing from ped 'whichped' to Avvy, in degrees.

	// There are two kinds of redraw: Major and Minor. Minor is what happens when you load a game, etc.
	// Major was replaced with GraphicManager::refreshScreen(), it redraws EVERYTHING.
	void minorRedraw();

	void spriteRun();

	Common::String intToStr(int32 num);
	void newGame(); // This sets up the DNA for a completely new game.
	bool getFlag(char x);
	bool decreaseMoney(uint16 amount); // Called pennycheck in the original.

	Common::String getName(People whose);
	Common::String getItem(byte which); // Called get_better in the original.
	Common::String f5Does(); // This procedure determines what f5 does.

	void openDoor(Room whither, byte ped, byte magicnum); // Handles slidey-open doors.
	void flipRoom(Room room, byte ped);

	void setRoom(People persId, Room roomId);
	Room getRoom(People persId);
private:
	static const int16 kMaxSprites = 2; // Current max no. of sprites.
	static Room _whereIs[29];

	// Will be used in dusk() and dawn().
	bool _fxHidden;
	byte _fxPal[16][16][3];

	bool _spludwickAtHome; // Is Spludwick at home?
	bool _passedCwytalotInHerts; // Have you passed Cwytalot in Herts?
	bool _holdTheDawn; // If this is true, calling Dawn will do nothing. It's used, for example, at the start, to stop Load from dawning.
	byte _lastRoom;
	byte _lastRoomNotMap;
	byte _roomCount[100]; // Add one to each every time you enter a room
	Common::String _mouseText;
	Common::String _flags;
	Common::String _roomnName; // Name of actual room
	int8 _scoreToDisplay[3];

	Common::String readAlsoStringFromFile(Common::File &file);
	void runAvalot();
	void init();
	void initVariables();
	void setup();
	void scram(Common::String &str);
	void unScramble();
	void handleKeyDown(Common::Event &event); // To replace Basher::keyboard_link() and Basher::typein().
	void enterNewTown();
	void findPeople(byte room);
	void putGeidaAt(byte whichPed, byte ped);
	void guideAvvy(Common::Point cursorPos);
	void enterRoom(Room room, byte ped);
	void exitRoom(byte x);
	void drawToolbar();
	void drawScore();
	void useCompass(const Common::Point &cursorPos); // Click on the compass on the toolbar to control Avvy's movement.
	void checkClick();
	void fixFlashers();
	void loadAlso(byte num);
	void resetAllVariables();
	void resetVariables();
};

} // End of namespace Avalanche

#endif // AVALANCHE_AVALANCHE_H
