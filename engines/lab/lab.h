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

#include "lab/image.h"

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
class Image;
class LargeSet;
class Music;
class Resource;
class TilePuzzle;
class Utils;

struct SaveGameHeader {
	byte _version;
	SaveStateDescriptor _descr;
	uint16 _roomNumber;
	uint16 _direction;
};

enum GameFeatures {
	GF_LOWRES = 1 << 0,
	GF_WINDOWS_TRIAL = 1 << 1
};

typedef Common::List<Button *> ButtonList;

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
private:
	bool _interfaceOff;
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

	char *_journalText;
	char *_journalTextTitle;
	const char *_nextFileName;
	const char *_newFileName;
	const char *_monitorTextFilename;

	CloseDataPtr _closeDataPtr;
	ButtonList _journalButtonList;
	ButtonList _mapButtonList;
	Image *_imgMap, *_imgRoom, *_imgUpArrowRoom, *_imgDownArrowRoom, *_imgBridge;
	Image *_imgHRoom, *_imgVRoom, *_imgMaze, *_imgHugeMaze, *_imgPath;
	Image *_imgMapX[4];
	InventoryData *_inventory;
	MapData *_maps;
	Image *_monitorButton;
	Image *_journalBackImage;
	Image *_screenImage;
	TextFont *_journalFont;

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

	const char *_curFileName;

	Anim *_anim;
	CrumbData _breadCrumbs[MAX_CRUMBS];
	DisplayMan *_graphics;
	EventManager *_event;
	ButtonList _invButtonList;
	ButtonList _moveButtonList;
	Image *_invImages[10];
	Image *_moveImages[20];
	LargeSet *_conditions, *_roomsFound;
	Music *_music;
	Resource *_resource;
	RoomData *_rooms;
	TextFont *_msgFont;
	TilePuzzle *_tilePuzzle;
	Utils *_utils;

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

	void changeVolume(int delta);
	uint16 getDirection() { return _direction; }
	char *getPictName(CloseDataPtr *closePtrList);
	uint16 getQuarters();
	void setDirection(uint16 direction) { _direction = direction; };
	void setQuarters(uint16 quarters);
	void waitTOF();

private:
	bool checkConditions(int16 *condition);
	void decIncInv(uint16 *CurInv, bool dec);
	void doActions(Action *actionList, CloseDataPtr *closePtrList);
	bool doActionRule(Common::Point pos, int16 action, int16 roomNum, CloseDataPtr *closePtrList);
	bool doActionRuleSub(int16 action, int16 roomNum, CloseDataPtr closePtr, CloseDataPtr *setCloseList, bool allowDefaults);
	bool doCloseUp(CloseDataPtr closePtr);
	bool doGoForward(CloseDataPtr *closePtrList);
	void doJournal();
	bool doMainView(CloseDataPtr *closePtrList);
	void doMap(uint16 curRoom);
	void doMonitor(char *background, char *textfile, bool isinteractive, uint16 x1, uint16 y1, uint16 x2, uint16 y2);
	void doNotes();
	bool doOperateRuleSub(int16 itemNum, int16 roomNum, CloseDataPtr closePtr, CloseDataPtr *setCloseList, bool allowDefaults);
	bool doOperateRule(Common::Point pos, int16 ItemNum, CloseDataPtr *closePtrList);
	bool doTurn(uint16 from, uint16 to, CloseDataPtr *closePtrList);
	bool doUse(uint16 curInv);
	void doWestPaper();
	void drawDirection(CloseDataPtr closePtr);
	void drawJournal(uint16 wipenum, bool needFade);
	void drawJournalText();
	void drawMap(uint16 curRoom, uint16 curMsg, uint16 floorNum, bool fadeOut, bool fadeIn);
	void drawMonText(char *text, TextFont *monitorFont, uint16 x1, uint16 y1, uint16 x2, uint16 y2, bool isinteractive);
	void drawRoomMap(uint16 curRoom, bool drawMarkFl);
	void drawRoomMessage(uint16 curInv, CloseDataPtr closePtr);
	void drawStaticMessage(byte index);
	void eatMessages();
	CloseDataPtr findClosePtrMatch(CloseDataPtr closePtr, CloseDataPtr closePtrList);
	bool floorVisited(uint16 floorNum);
	int followCrumbs();
	void freeMapData();
	void freeScreens();
	bool fromCrumbs(uint32 tmpClass, uint16 code, uint16 qualifier, Common::Point tmpPos,
		uint16 &curInv, IntuiMessage *curMsg, bool &forceDraw, uint16 buttonId, uint16 &actionMode);
	const char *getInvName(uint16 curInv);
	uint16 getLowerFloor(uint16 floorNum);
	CloseData *getObject(Common::Point pos, CloseDataPtr closePtr);
	uint16 getUpperFloor(uint16 floorNum);
	ViewData *getViewData(uint16 roomNum, uint16 direction);
	void interfaceOff();
	void interfaceOn();
	void loadJournalData();
	void loadMapData();
	void mainGameLoop();
	void showLab2Teaser();
	void mayShowCrumbIndicator();
	void mayShowCrumbIndicatorOff();
	void perFlipButton(uint16 gadID);
	uint16 processArrow(uint16 curDirection, uint16 arrow);
	void processJournal();
	void processMap(uint16 curRoom);
	void processMonitor(char *ntext, TextFont *monitorFont, bool isInteractive, uint16 x1, uint16 y1, uint16 x2, uint16 y2);
	Common::Rect roomCoords(uint16 curRoom);
	bool saveRestoreGame();
	void setCurrentClose(Common::Point pos, CloseDataPtr *closePtrList, bool useAbsoluteCoords);
	bool takeItem(uint16 x, uint16 y, CloseDataPtr *closePtrList);
	void turnPage(bool fromLeft);
	bool processKey(IntuiMessage *curMsg, uint32 &msgClass, uint16 &qualifier, Common::Point &curPos, uint16 &curInv, bool &forceDraw, uint16 code);
	void processMainButton(CloseDataPtr wrkClosePtr, uint16 &curInv, uint16 &lastInv, uint16 &oldDirection, bool &forceDraw, uint16 buttonId, uint16 &actionMode);
	void processAltButton(uint16 &curInv, uint16 &lastInv, uint16 buttonId, uint16 &actionMode);

private:
	bool saveGame(int slot, Common::String desc);
	bool loadGame(int slot);
	void writeSaveGameHeader(Common::OutSaveFile *out, const Common::String &saveName);
};

bool readSaveGameHeader(Common::InSaveFile *in, SaveGameHeader &header);

} // End of namespace Lab

#endif // LAB_LAB_H
