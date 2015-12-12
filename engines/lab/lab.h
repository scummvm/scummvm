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

#ifndef LAB_H
#define LAB_H

#include "common/system.h"
#include "common/rect.h"

#include "engines/engine.h"

struct ADGameDescription;

namespace Lab {

struct MapData;
struct Action;
struct CloseData;
struct Gadget;
struct IntuiMessage;
struct InventoryData;
struct RoomData;
struct Rule;
struct TextFont;
struct ViewData;

class Anim;
class DisplayMan;
class EventManager;
class Image;
class LargeSet;
class Music;
class Resource;
class TilePuzzle;
class Utils;

enum GameFeatures {
	GF_LOWRES = 1 << 0,
	GF_WINDOWS_TRIAL = 1 << 1
};

#define ONESECOND      1000
#define BRICKOPEN      115
#define COMBINATIONUNLOCKED  130
#define LEFTSCROLL     1
#define RIGHTSCROLL    2
#define UPSCROLL       3
#define DOWNSCROLL     4

#define QUARTERNUM  30

typedef Common::List<Gadget *> GadgetList;

struct CrumbData {
	uint16 _roomNum;
	uint16 _direction;
};

#define MAX_CRUMBS          128

typedef CloseData *CloseDataPtr;
typedef Common::List<Rule *> RuleList;

// Direction defines
#define NORTH   0
#define SOUTH   1
#define EAST    2
#define WEST    3

class LabEngine : public Engine {
public:
	LabEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~LabEngine();

	virtual Common::Error run();
	void go();

	const ADGameDescription *_gameDescription;
	Common::Platform getPlatform() const;
	uint32 getFeatures() const;

	bool hasFeature(EngineFeature f) const;
	Common::String generateSaveFileName(uint slot);

	LargeSet *_conditions, *_roomsFound;

	// timing.cpp
	void getTime(uint32 *secs, uint32 *micros);
	void addCurTime(uint32 sec, uint32 micros, uint32 *timeSec, uint32 *timeMicros);
	void anyTimeDiff(uint32 sec1, uint32 micros1, uint32 sec2, uint32 micros2, uint32 *diffSecs, uint32 *diffMicros);
	void timeDiff(uint32 sec, uint32 micros, uint32 *diffSec, uint32 *diffMicros);
	void waitForTime(uint32 sec, uint32 micros);

private:
	uint32 _extraGameFeatures;
	bool _interfaceOff;
	bool _noUpdateDiff;
	bool _quitLab;
	bool _mainDisplay;
	// timing.cpp
	void microDelay(uint32 secs, uint32 micros);

public:
	EventManager *_event;
	Resource *_resource;
	Music *_music;
	Anim *_anim;
	DisplayMan *_graphics;
	RoomData *_rooms;
	TilePuzzle *_tilePuzzle;
	Utils *_utils;

	int _roomNum;
	CrumbData _breadCrumbs[MAX_CRUMBS];
	uint16 _numCrumbs;
	bool _droppingCrumbs;
	bool _followingCrumbs;
	bool _followCrumbsFast;
	bool _isCrumbTurning;
	uint32 _crumbSecs, _crumbMicros;
	bool _isCrumbWaiting;
	bool _alternate;
	bool _isHiRes;
	uint16 _numInv;
	uint16 _manyRooms;
	uint16 _direction;
	uint16 _highestCondition;

	const char *_curFileName;
	const char *_nextFileName;
	// When ProcessRoom.c decides to change the filename of the current picture.
	const char *_newFileName;
	TextFont *_msgFont;
	GadgetList _moveGadgetList;
	GadgetList _invGadgetList;
	Image *_moveImages[20];
	Image *_invImages[10];

private:
	int _lastWaitTOFTicks;
	bool _lastTooLong;
	CloseDataPtr _closeDataPtr;
	InventoryData *_inventory;
	GadgetList _journalGadgetList;
	GadgetList _mapGadgetList;
	Image *_imgMap, *_imgRoom, *_imgUpArrowRoom, *_imgDownArrowRoom, *_imgBridge;
	Image *_imgHRoom, *_imgVRoom, *_imgMaze, *_imgHugeMaze, *_imgPath, *_imgMapNorth;
	Image *_imgMapEast, *_imgMapSouth, *_imgMapWest, *_imgXMark;
	uint16 _maxRooms;
	MapData *_maps;

private:
	bool fromCrumbs(uint32 tmpClass, uint16 code, uint16 qualifier, Common::Point tmpPos,
			uint16 &curInv, IntuiMessage *curMsg, bool &forceDraw, uint16 gadgetId, uint16 &actionMode);

public:
	void waitTOF();
	void drawRoomMessage(uint16 curInv, CloseDataPtr closePtr);
	void interfaceOff();
	void interfaceOn();
	void decIncInv(uint16 *CurInv, bool dec);
	Gadget *checkNumGadgetHit(GadgetList *gadgetList, uint16 key);
	IntuiMessage *getMsg();
	void loadMapData();
	void drawMap(uint16 curRoom, uint16 curMsg, uint16 floorNum, bool fadeOut, bool fadeIn);
	void processMap(uint16 curRoom);
	void doMap(uint16 curRoom);
	void freeMapData();
	void loadJournalData();
	void drawJournal(uint16 wipenum, bool needFade);
	void processJournal();
	void doJournal();
	void drawMonText(char *text, TextFont *monitorFont, uint16 x1, uint16 y1, uint16 x2, uint16 y2, bool isinteractive);
	void processMonitor(char *ntext, TextFont *monitorFont, bool isinteractive, uint16 x1, uint16 y1, uint16 x2, uint16 y2);
	void doMonitor(char *background, char *textfile, bool isinteractive, uint16 x1, uint16 y1, uint16 x2, uint16 y2);
	void doNotes();
	void doWestPaper();
	void eatMessages();
	void drawStaticMessage(byte index);
	void drawDirection(CloseDataPtr closePtr);
	int followCrumbs();

	void changeVolume(int delta);
	char *getPictName(CloseDataPtr *closePtrList);

private:
	void freeScreens();
	void perFlipGadget(uint16 gadID);
	bool doCloseUp(CloseDataPtr closePtr);
	void mainGameLoop();
	bool doUse(uint16 curInv);
	void mayShowCrumbIndicator();
	void mayShowCrumbIndicatorOff();
	const char *getInvName(uint16 curInv);
	bool saveRestoreGame();
	Common::Rect roomCoords(uint16 curRoom);
	void drawRoomMap(uint16 curRoom, bool drawMarkFl);
	bool floorVisited(uint16 floorNum);
	uint16 getUpperFloor(uint16 floorNum);
	uint16 getLowerFloor(uint16 floorNum);
	bool checkConditions(int16 *condition);
	ViewData *getViewData(uint16 roomNum, uint16 direction);
	CloseData *getObject(Common::Point pos, CloseDataPtr closePtr);
	CloseDataPtr findClosePtrMatch(CloseDataPtr closePtr, CloseDataPtr closePtrList);
	uint16 processArrow(uint16 curDirection, uint16 arrow);
	void setCurrentClose(Common::Point pos, CloseDataPtr *closePtrList, bool useAbsoluteCoords);
	bool takeItem(uint16 x, uint16 y, CloseDataPtr *closePtrList);
	bool doActionRuleSub(int16 action, int16 roomNum, CloseDataPtr closePtr, CloseDataPtr *setCloseList, bool allowDefaults);
	bool doActionRule(Common::Point pos, int16 action, int16 roomNum, CloseDataPtr *closePtrList);
	bool doOperateRuleSub(int16 itemNum, int16 roomNum, CloseDataPtr closePtr, CloseDataPtr *setCloseList, bool allowDefaults);
	bool doOperateRule(Common::Point pos, int16 ItemNum, CloseDataPtr *closePtrList);
	bool doGoForward(CloseDataPtr *closePtrList);
	bool doTurn(uint16 from, uint16 to, CloseDataPtr *closePtrList);
	bool doMainView(CloseDataPtr *closePtrList);

public:
	void doActions(Action *actionList, CloseDataPtr *closePtrList);

};

extern LabEngine *g_lab;

} // End of namespace Lab

#endif // LAB_H
