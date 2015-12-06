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

#include "common/array.h"
#include "common/events.h"

#include "engines/engine.h"
#include "lab/labfun.h"
#include "lab/interface.h"
#include "lab/eventman.h"
#include "lab/music.h"
#include "lab/resource.h"
#include "lab/anim.h"
#include "lab/graphics.h"
#include "lab/labsets.h"

struct ADGameDescription;

namespace Lab {

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

	// timing.cpp
	void microDelay(uint32 secs, uint32 micros);

public:
	EventManager *_event;
	Resource *_resource;
	Music *_music;
	Anim *_anim;
	DisplayMan *_graphics;

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
	const char *_newFileName;  /* When ProcessRoom.c decides to change the filename
                                    of the current picture. */
	TextFont *_msgFont;
	Gadget *_moveGadgetList;
	Gadget *_invGadgetList;
	Image *_moveImages[20];
	Image *_invImages[10];
	Image *_numberImages[10];
	uint16 _curTile[4][4];
	byte _combination[6];

private:
	int _lastWaitTOFTicks;
	bool _lastTooLong;
	CloseDataPtr _cptr;
	InventoryData *_inventory;
	Image *_tiles[16];

private:
	bool from_crumbs(uint32 tmpClass, uint16 code, uint16 Qualifier, Common::Point tmpPos, uint16 &curInv, IntuiMessage * curMsg, bool &forceDraw, uint16 gadgetId, uint16 &actionMode);

public:
	void waitTOF();
	void drawRoomMessage(uint16 CurInv, CloseDataPtr cptr);
	void interfaceOff();
	void interfaceOn();
	void decIncInv(uint16 *CurInv, bool dec);
	Gadget *checkNumGadgetHit(Gadget *gadlist, uint16 key);
	IntuiMessage *getMsg();
	void drawMap(uint16 CurRoom, uint16 CurMsg, uint16 Floor, bool fadeout, bool fadein);
	void processMap(uint16 CurRoom);
	void doMap(uint16 CurRoom);
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
	void drawDirection(CloseDataPtr lcPtr);
	int followCrumbs();

	void changeVolume(int delta);

private:
	// engine.cpp
	void freeScreens();
	void perFlipGadget(uint16 gadID);
	bool doCloseUp(CloseDataPtr cptr);
	void mainGameLoop();
	bool doUse(uint16 curInv);
	void mayShowCrumbIndicator();
	void mayShowCrumbIndicatorOff();
	const char *getInvName(uint16 curInv);
	int VGAUnScaleX(int x);
	int VGAUnScaleY(int y);
	void mouseTile(Common::Point pos);
	void changeTile(uint16 col, uint16 row);
	void mouseCombination(Common::Point pos);
	void doTile(bool showsolution);
	void showTile(const char *filename, bool showsolution);
	void doTileScroll(uint16 col, uint16 row, uint16 scrolltype);
	void changeCombination(uint16 number);
	void scrollRaster(int16 dx, int16 dy, uint16 x1, uint16 y1, uint16 x2, uint16 y2);
	void doCombination();
	void showCombination(const char *filename);
	void initTilePuzzle();

	bool saveRestoreGame();

public:
	void doActions(Action *aPtr, CloseDataPtr *lcptr);

};

extern LabEngine *g_lab;

} // End of namespace Lab

#endif // LAB_H
