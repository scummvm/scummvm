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
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#ifndef LAB_LAB_H
#define LAB_LAB_H

#include "common/system.h"
#include "common/random.h"
#include "common/rect.h"
#include "common/savefile.h"
#include "engines/engine.h"
#include "engines/savestate.h"

#include "lab/console.h"
#include "lab/image.h"
#include "lab/labsets.h"
#include "lab/detection.h"

struct ADGameDescription;

namespace Lab {

struct MapData;
struct Action;
struct CloseData;
struct Button;
struct IntuiMessage;
struct InventoryData;
struct RoomData;
struct Rule;
struct TextFont;
struct ViewData;

class Anim;
class DisplayMan;
class EventManager;
class Interface;
class Image;
class Music;
class Resource;
class SpecialLocks;
class Utils;

struct SaveGameHeader {
	byte _version;
	SaveStateDescriptor _descr;
	uint16 _roomNumber;
	uint16 _direction;
};

typedef Common::List<Button *> ButtonList;

struct CrumbData {
	uint16 _crumbRoomNum;
	uint16 _crumbDirection;
};

#define MAX_CRUMBS          128

typedef Common::List<Rule> RuleList;
typedef Common::List<Action> ActionList;
typedef Common::List<CloseData> CloseDataList;
typedef Common::List<ViewData> ViewDataList;

enum Direction {
	kDirectionNorth,
	kDirectionSouth,
	kDirectionEast,
	kDirectionWest
};

enum MainButton {
	kButtonNone = -1,
	kButtonPickup,
	kButtonUse,
	kButtonOpen,
	kButtonClose,
	kButtonLook,
	kButtonInventory,
	kButtonLeft,
	kButtonForward,
	kButtonRight,
	kButtonMap
};

enum MessageClass {
	kMessageLeftClick,
	kMessageRightClick,
	kMessageButtonUp,
	kMessageRawKey
};

class LabEngine : public Engine {
	friend class Console;

private:
	bool _isCrumbWaiting;
	bool _lastTooLong;
	bool _lastPage;
	bool _mainDisplay;
	bool _noUpdateDiff;
	bool _quitLab;

	byte *_blankJournal;

	int _lastWaitTOFTicks;

	uint16 _direction;
	uint16 _highPalette[20];
	uint16 _journalPage;
	uint16 _maxRooms;
	uint16 _monitorPage;
	uint16 _monitorButtonHeight;

	uint32 _extraGameFeatures;

	Common::String _journalText;
	Common::String _journalTextTitle;
	Common::String _nextFileName;
	Common::String _newFileName;
	Common::String _monitorTextFilename;

	const CloseData *_closeDataPtr;
	ButtonList _journalButtonList;
	ButtonList _mapButtonList;
	Image *_imgMap, *_imgRoom, *_imgUpArrowRoom, *_imgDownArrowRoom, *_imgBridge;
	Image *_imgHRoom, *_imgVRoom, *_imgMaze, *_imgHugeMaze, *_imgPath;
	Image *_imgMapX[4];
	InventoryData *_inventory;
	MapData *_maps;
	Image *_monitorButton;
	Image *_journalBackImage;
	TextFont *_journalFont;
	bool _introPlaying;

public:
	bool _alternate;
	bool _droppingCrumbs;
	bool _followingCrumbs;
	bool _followCrumbsFast;
	bool _isCrumbTurning;
	bool _isHiRes;

	int _roomNum;

	uint16 _highestCondition;
	uint16 _manyRooms;
	uint16 _numCrumbs;
	uint16 _numInv;

	uint32 _crumbTimestamp;

	Common::String _curFileName;

	Anim *_anim;
	CrumbData _breadCrumbs[MAX_CRUMBS];
	DisplayMan *_graphics;
	EventManager *_event;
	Interface *_interface;
	ButtonList _invButtonList;
	ButtonList _moveButtonList;
	Image *_invImages[10];
	Image *_moveImages[20];
	LargeSet *_conditions, *_roomsFound;
	Music *_music;
	Resource *_resource;
	RoomData *_rooms;
	TextFont *_msgFont;
	SpecialLocks *_specialLocks;
	Utils *_utils;

public:
	LabEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~LabEngine() override;

	Common::Error run() override;
	void go();

	const ADGameDescription *_gameDescription;
	Common::Platform getPlatform() const;
	uint32 getFeatures() const;

	bool hasFeature(EngineFeature f) const override;

	void changeVolume(int delta);
	uint16 getDirection() { return _direction; }

	/**
	 * Returns the current picture name.
	 */
	Common::String getPictName(bool useClose);
	uint16 getQuarters();
	void setQuarters(uint16 quarters);
	void updateEvents();
	void waitTOF();

	Common::Error loadGameState(int slot) override;
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;
	bool canLoadGameStateCurrently() override;
	bool canSaveGameStateCurrently() override;

	bool isMainDisplay() const { return _mainDisplay; }

private:
	/**
	 * Checks whether all the conditions in a condition list are met.
	 */
	bool checkConditions(const Common::Array<int16> &cond);

	/**
	 * Decrements the current inventory number.
	 */
	void decIncInv(uint16 *CurInv, bool dec);

	/**
	 * Processes the action list.
	 */
	void doActions(const ActionList &actionList);

	/**
	 * Goes through the rules if an action is taken.
	 */
	bool doActionRule(Common::Point pos, int16 action, int16 roomNum);

	/**
	 * Does the work for doActionRule.
	 */
	bool doActionRuleSub(int16 action, int16 roomNum, const CloseData *closePtr, bool allowDefaults);

	/**
	 * Handles monitor closeups
	 */
	void handleMonitorCloseup();

	/**
	 * Goes through the rules if the user tries to go forward.
	 */
	bool doGoForward();

	/**
	 * Does the journal processing.
	 */
	void doJournal();

	/**
	 * Goes through the rules if the user tries to go to the main view
	 */
	bool doMainView();

	/**
	 * Does the map processing.
	 */
	void doMap();

	/**
	 * Does what's necessary for the monitor.
	 */
	void doMonitor(const Common::String background, const Common::String textfile, bool isinteractive, Common::Rect textRect);

	/**
	 * Does the things to properly set up the detective notes.
	 */
	void doNotes();

	/**
	 * Does the work for doActionRule.
	 */
	bool doOperateRuleSub(int16 itemNum, int16 roomNum, const CloseData *closePtr, bool allowDefaults);

	/**
	 * Goes through the rules if the user tries to operate an item on an object.
	 */
	bool doOperateRule(Common::Point pos, int16 ItemNum);

	/**
	 * Goes through the rules if the user tries to turn.
	 */
	bool doTurn(uint16 from, uint16 to);

	/**
	 * If the user hits the "Use" button; things that can get used on themselves.
	 */
	bool doUse(uint16 curInv);

	/**
	 * Does the things to properly set up the old west newspaper.  Assumes that
	 * OpenHiRes already called.
	 */
	void doWestPaper();

	/**
	 * Draws the current direction to the screen.
	 */
	void drawDirection(const CloseData *closePtr);

	/**
	 * Draws the journal from page x.
	 */
	void drawJournal(uint16 wipenum, bool needFade);

	/**
	 * Draws the text to the back journal screen to the appropriate Page number
	 */
	void drawJournalText();

	/**
	 * Draws the map
	 */
	void drawMap(uint16 curRoom, uint16 curMsg, uint16 floorNum, bool fadeIn);

	/**
	 * Draws the text for the monitor.
	 */
	void drawMonText(const char *text, TextFont *monitorFont, Common::Rect textRect, bool isinteractive);

	/**
	 * Draws a room map.
	 */
	void drawRoomMap(uint16 curRoom, bool drawMarkFl);

	/**
	 * Draws the message for the room.
	 */
	void drawRoomMessage(uint16 curInv, const CloseData *closePtr);
	void drawStaticMessage(byte index);

	/**
	 * Eats all the available messages.
	 */
	void eatMessages();

	/**
	 * Goes through the list of closeups to find a match.
	 * @note Known bug here.  If there are two objects that have closeups, and
	 * some of the closeups have the same hit boxes, then this returns the first
	 * occurrence of the object with the same hit box.
	 */
	const CloseData *findClosePtrMatch(const CloseData *closePtr, const CloseDataList &list);

	/**
	 * Checks if a floor has been visited.
	 */
	bool floorVisited(uint16 floorNum);

	/**
	 * New code to allow quick(er) return navigation in game.
	 */
	MainButton followCrumbs();
	void freeMapData();
	void freeScreens();
	bool processEvent(MessageClass tmpClass, uint16 code, uint16 qualifier, Common::Point tmpPos,
		uint16 &curInv, IntuiMessage *curMsg, bool &forceDraw, uint16 buttonId, uint16 &actionMode);

	/**
	 * Gets the current inventory name.
	 */
	Common::String getInvName(uint16 curInv);

	/**
	 * Returns the floor to show when the down arrow is pressed
	 * @note The original did not show all the visited floors, but we do
	 */
	uint16 getLowerFloor(uint16 floorNum);

	/**
	 * Gets an object, if any, from the user's click on the screen.
	 */
	const CloseData *getObject(Common::Point pos, const CloseData *closePtr);

	/**
	 * Returns the floor to show when the up arrow is pressed
	 * @note The original did not show all the visited floors, but we do
	 */
	uint16 getUpperFloor(uint16 floorNum);

	/**
	 * Gets the current ViewDataPointer.
	 */
	ViewData *getViewData(uint16 roomNum, uint16 direction);

	/**
	 * Turns the interface off.
	 */
	void interfaceOff();

	/**
	 * Turns the interface on.
	 */
	void interfaceOn();

	/**
	 * Loads in the data for the journal.
	 */
	void loadJournalData();

	/**
	 * Loads in the map data.
	 */
	void loadMapData();

	/**
	 * The main game loop.
	 */
	void mainGameLoop();
	void showLab2Teaser();

	/**
	 * Permanently flips the imagery of a button.
	 */
	void perFlipButton(uint16 buttonId);

	/**
	 * process a arrow button movement.
	 */
	uint16 processArrow(uint16 curDirection, uint16 arrow);

	/**
	 * Processes user input.
	 */
	void processJournal();

	/**
	 * Processes the map.
	 */
	void processMap(uint16 curRoom);

	/**
	 * Processes user input.
	 */
	void processMonitor(const Common::String &ntext, TextFont *monitorFont, bool isInteractive, Common::Rect textRect);

	/**
	 * Figures out what a room's coordinates should be.
	 */
	Common::Rect roomCoords(uint16 curRoom);
	bool saveRestoreGame();

	/**
	 * Sets the current close up data.
	 */
	void setCurrentClose(Common::Point pos, const CloseData **closePtrList, bool useAbsoluteCoords, bool next=false);

	/**
	 * Takes the currently selected item.
	 */
	bool takeItem(Common::Point pos);

	/**
	 * Does the turn page wipe.
	 */
	void turnPage(bool fromLeft);
	bool processKey(IntuiMessage *curMsg, uint32 msgClass, uint16 &qualifier, Common::Point &curPos, uint16 &curInv, bool &forceDraw, uint16 code);
	void processMainButton(uint16 &curInv, uint16 &lastInv, uint16 &oldDirection, bool &forceDraw, uint16 buttonId, uint16 &actionMode);
	void processAltButton(uint16 &curInv, uint16 &lastInv, uint16 buttonId, uint16 &actionMode);
	void performAction(uint16 actionMode, Common::Point curPos, uint16 &curInv);

	/**
	 * Writes the game out to disk.
	 */
	bool saveGame(int slot, const Common::String desc);

	/**
	 * Reads the game from disk.
	 */
	bool loadGame(int slot);
	void writeSaveGameHeader(Common::OutSaveFile *out, const Common::String &saveName);

	void handleTrialWarning();
};

WARN_UNUSED_RESULT bool readSaveGameHeader(Common::InSaveFile *in, SaveGameHeader &header, bool skipThumbnail = true);

} // End of namespace Lab

#endif // LAB_LAB_H
