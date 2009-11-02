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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef DRACI_GAME_H
#define DRACI_GAME_H

#include "common/str.h"
#include "common/serializer.h"
#include "draci/barchive.h"
#include "draci/script.h"
#include "draci/animation.h"
#include "draci/sprite.h"
#include "draci/walking.h"

namespace Draci {

class DraciEngine;

enum {
	kDragonObject = 0
};

// Used as a return value for Game::getObjectWithAnimation() if no object
// owns the animation in question
enum {
	kObjectNotFound = -1
};

// Used as the value of the _escRoom field of the current room if there is
// no escape room defined
enum {
	kNoEscRoom = -1
};

// Used as a value to Game::_currentIcon and means there is no item selected
// and a "real" cursor image is used
enum {
	kNoItem = -1
};

enum {
	kNoDialogue = -1,
	kDialogueLines = 4
};

enum {
	kBlackPalette = -1
};

// Constants tuned such that with ScummVM's default talkspeed kStandardSpeed, the speed
// computed by equation (kBaseSpeechDuration + kSpeechTimeUnit * #characters) /
// talkspeed is equal to the original game.
enum SpeechConstants {
	kBaseSpeechDuration = 12000,
	kSpeechTimeUnit = 2640,
	kStandardSpeed = 60
};

// One fading phase is 50ms.
enum FadeConstants {
	kFadingTimeUnit = 50
};

/** Inventory related magical constants */
enum InventoryConstants {
  kInventoryItemWidth = 25,
  kInventoryItemHeight = 25,
  kInventoryColumns = 7,
  kInventoryLines = 5,
  kInventoryX = 70, ///< Used for positioning of the inventory sprite on the X axis
  kInventoryY = 30, ///< Used for positioning of the inventory sprite on the Y axis
  kInventorySlots = kInventoryLines * kInventoryColumns
};

struct GameObject {
	uint _init, _look, _use, _canUse;
	bool _imInit, _imLook, _imUse;
	int _walkDir;
	byte _z;
	uint _lookX, _lookY, _useX, _useY;
	SightDirection _lookDir, _useDir;
	uint _absNum;
	Common::Array<int> _anim;
	GPL2Program _program;
	Common::String _title;
	int _location;
	bool _visible;
};

struct GameInfo {
	int _startRoom;
	int _mapRoom;
	uint _numObjects;
	uint _numItems;
	byte _numVariables;
	byte _numPersons;
	byte _numDialogues;
	uint _maxItemWidth, _maxItemHeight;
	uint _musicLength;
	uint _crc[4];
	uint _numDialogueBlocks;
};

struct GameItem {
	uint _init, _look, _use, _canUse;
	bool _imInit, _imLook, _imUse;
	GPL2Program _program;
	Common::String _title;
};

struct Person {
	uint _x, _y;
	byte _fontColour;
};

struct Dialogue {
	int _canLen;
	byte *_canBlock;
	Common::String _title;
	GPL2Program _program;
};

struct Room {
	int _roomNum;
	byte _music;
	int _mapID;
	int _palette;
	int _numOverlays;
	int _init, _look, _use, _canUse;
	bool _imInit, _imLook, _imUse;
	bool _mouseOn, _heroOn;
	double _pers0, _persStep;
	int _escRoom;
	byte _numGates;
	Common::Array<int> _gates;
	GPL2Program _program;
};

enum LoopStatus {
	kStatusOrdinary,	// normal game-play: everything allowed
	kStatusGate,		// during running init-scripts when entering a room: disable interactivity
	kStatusInventory,	// inventory is open: cannot change the room or go to map
	kStatusDialogue		// during a dialogue: cannot change the room, go to inventory
};

enum LoopSubstatus {
	kOuterLoop,		// outer loop: everything is allowed
	kInnerWhileTalk,	// playing a voice: inner loop will exit afterwards
	kInnerWhileFade,	// fading a palette: inner loop will exit when done
	kInnerDuringDialogue,	// selecting continuation block: inner block will exit afterwards
	kInnerUntilExit		// other inner loop: either immediately exiting or waiting for an animation to end (whose callback ends the loop)
};

class Game {
public:
	Game(DraciEngine *vm);
	~Game();

	void init();
	void start();
	void loop(LoopSubstatus substatus, bool shouldExit);

	// HACK: this is only for testing
	int nextRoomNum() const {
		int n = _currentRoom._roomNum;
		n = n < 37 ? n+1 : n;

		// disable former distributor logo
		if (n == 30)
			++n;

		return n;
	}

	// HACK: same as above
	int prevRoomNum() const {
		int n = _currentRoom._roomNum;
		n = n > 0 ? n-1 : n;

		// disable former distributor logo
		if (n == 30)
			--n;

		return n;
	}

	void walkHero(int x, int y, SightDirection dir);
	int getHeroX() const { return _hero.x; }
	int getHeroY() const { return _hero.y; }
	void positionAnimAsHero(Animation *anim);
	void playHeroAnimation(int anim_index);

	int loadAnimation(uint animNum, uint z);
	void loadOverlays();
	void loadObject(uint numObj);
	void loadWalkingMap(int mapID);		// but leaves _currentRoom._mapID untouched
	void loadItem(int itemID);

	uint getNumObjects() const { return _info._numObjects; }
	GameObject *getObject(uint objNum) { return _objects + objNum; }
	int getObjectWithAnimation(int animID) const;
	void deleteObjectAnimations();
	void deleteAnimationsAfterIndex(int lastAnimIndex);
	void stopObjectAnimations(const GameObject *obj);
	int playingObjectAnimation(const GameObject *obj) const;

	int getVariable(int varNum) const { return _variables[varNum]; }
	void setVariable(int varNum, int value) { _variables[varNum] = value; }

	const Person *getPerson(int personID) const { return &_persons[personID]; }

	int getRoomNum() const { return _currentRoom._roomNum; }
	void setRoomNum(int num) { _currentRoom._roomNum = num; }
	int getPreviousRoomNum() const { return _previousRoom; }
	void rememberRoomNumAsPrevious() { _previousRoom = getRoomNum(); }
	void scheduleEnteringRoomUsingGate(int room, int gate) { _newRoom = room; _newGate = gate; }
	void pushNewRoom();
	void popNewRoom();

	double getPers0() const { return _currentRoom._pers0; }
	double getPersStep() const { return _currentRoom._persStep; }
	int getMusicTrack() const { return _currentRoom._music; }
	void setMusicTrack(int num) { _currentRoom._music = num; }

	int getItemStatus(int itemID) const { return _itemStatus[itemID]; }
	void setItemStatus(int itemID, int status) { _itemStatus[itemID] = status; }
	int getCurrentItem() const { return _currentItem; }
	void setCurrentItem(int itemID) { _currentItem = itemID; }
	void removeItem(int itemID);
	void putItem(int itemID, int position);
	void addItem(int itemID);

	int getEscRoom() const { return _currentRoom._escRoom; }
	int getMapRoom() const { return _info._mapRoom; }
	int getMapID() const { return _currentRoom._mapID; }

	/**
	 * The GPL command Mark sets the animation index (which specifies the
	 * order in which animations were loaded in) which is then used by the
	 * Release command to delete all animations that have an index greater
	 * than the one marked.
	 */
	int getMarkedAnimationIndex() const { return _markedAnimationIndex; }
	void setMarkedAnimationIndex(int index) { _markedAnimationIndex = index; }

	void setLoopStatus(LoopStatus status) { _loopStatus = status; }
	void setLoopSubstatus(LoopSubstatus status) { _loopSubstatus = status; }
	LoopStatus getLoopStatus() const { return _loopStatus; }
	LoopSubstatus getLoopSubstatus() const { return _loopSubstatus; }

	bool shouldQuit() const { return _shouldQuit; }
	void setQuit(bool quit) { _shouldQuit = quit; }
	bool shouldExitLoop() const { return _shouldExitLoop; }
	void setExitLoop(bool exit) { _shouldExitLoop = exit; }
	bool isReloaded() const { return _isReloaded; }
	void setIsReloaded(bool value) { _isReloaded = value; }

	void setSpeechTiming(uint tick, uint duration);
	void shiftSpeechAndFadeTick(int delta);

	void updateTitle(int x, int y);
	void updateCursor();

	void inventoryInit();
	void inventoryDraw();
	void inventoryDone();
	void inventoryReload();

	void dialogueMenu(int dialogueID);
	int dialogueDraw();
	void dialogueInit(int dialogID);
	void dialogueDone();
	void runDialogueProg(GPL2Program, int offset);

	bool isDialogueBegin() const { return _dialogueBegin; }
	bool shouldExitDialogue() const { return _dialogueExit; }
	void setDialogueExit(bool exit) { _dialogueExit = exit; }
	int getDialogueBlockNum() const { return _blockNum; }
	int getDialogueVar(int dialogueID) const { return _dialogueVars[dialogueID]; }
	void setDialogueVar(int dialogueID, int value) { _dialogueVars[dialogueID] = value; }
	int getCurrentDialogue() const { return _currentDialogue; }
	int getDialogueCurrentBlock() const { return _currentBlock; }
	int getDialogueLastBlock() const { return _lastBlock; }
	int getDialogueLinesNum() const { return _dialogueLinesNum; }
	int getCurrentDialogueOffset() const { return _dialogueOffsets[_currentDialogue]; }

	void schedulePalette(int paletteID) { _scheduledPalette = paletteID; }
	int getScheduledPalette() const { return _scheduledPalette; }
	void initializeFading(int phases);
	void setEnableQuickHero(bool value) { _enableQuickHero = value; }
	bool getEnableQuickHero() const { return _enableQuickHero; }
	void setWantQuickHero(bool value) { _wantQuickHero = value; }
	bool getWantQuickHero() const { return _wantQuickHero; }
	// TODO: after proper walking is implemented, do super-fast animation when walking
	void setEnableSpeedText(bool value) { _enableSpeedText = value; }
	bool getEnableSpeedText() const { return _enableSpeedText; }

	void DoSync(Common::Serializer &s);

private:
	void updateOrdinaryCursor();
	void updateInventoryCursor();
	void handleOrdinaryLoop();
	void handleInventoryLoop();
	void handleDialogueLoop();
	void advanceAnimationsAndTestLoopExit();

	bool enterNewRoom();	// Returns false if another room change has been triggered and therefore loop() shouldn't be called yet.
	void loadRoom(int roomNum);
	void runGateProgram(int gate);
	void redrawWalkingPath(int id, byte colour, const WalkingMap::Path &path);

	DraciEngine *_vm;

	GameInfo _info;

	Common::Point _hero;

	int *_variables;
	Person *_persons;
	GameObject *_objects;

	byte *_itemStatus;
	GameItem *_items;
	int _currentItem;
	int _itemUnderCursor;

	int _inventory[kInventorySlots];
	bool _inventoryExit;

	Room _currentRoom;
	WalkingMap _walkingMap;
	int _newRoom;
	int _newGate;
	int _previousRoom;
	int _pushedNewRoom;	// used in GPL programs
	int _pushedNewGate;

	uint *_dialogueOffsets;
	int _currentDialogue;
	int *_dialogueVars;
	BArchive *_dialogueArchive;
	Dialogue *_dialogueBlocks;
	bool _dialogueBegin;
	bool _dialogueExit;
	int _currentBlock;
	int _lastBlock;
	int _dialogueLinesNum;
	int _blockNum;
	int _lines[kDialogueLines];
	Animation *_dialogueAnims[kDialogueLines];

	LoopStatus _loopStatus;
	LoopSubstatus _loopSubstatus;

	bool _shouldQuit;
	bool _shouldExitLoop;
	bool _isReloaded;

	uint _speechTick;
	uint _speechDuration;

	int _objUnderCursor;
	int _animUnderCursor;

	int _markedAnimationIndex; ///< Used by the Mark GPL command

	int _scheduledPalette;
	int _fadePhases;
	int _fadePhase;
	uint _fadeTick;

	bool _enableQuickHero;
	bool _wantQuickHero;
	bool _enableSpeedText;
};

} // End of namespace Draci

#endif // DRACI_GAME_H
