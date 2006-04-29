/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
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
 * $URL$
 * $Id$
 *
 */

#ifndef __lure_hotspots_h__
#define __lure_hotspots_h__

#include "lure/luredefs.h"
#include "lure/screen.h"
#include "lure/disk.h"
#include "lure/res_struct.h"

namespace Lure {

#define MAX_NUM_IMPINGING 10

class Hotspot;

class Support {
public:
	static int findIntersectingCharacters(Hotspot &h, uint16 *charList); 
	static bool checkForIntersectingCharacter(Hotspot &h);
	static void checkRoomChange(Hotspot &h);
	static void characterChangeRoom(Hotspot &h, uint16 roomNumber, 
								  int16 newX, int16 newY, Direction dir);
	static bool charactersIntersecting(HotspotData *hotspot1, HotspotData *hotspot2);
};

typedef void(*HandlerMethodPtr)(Hotspot &h);

class HotspotTickHandlers {
private:
	// Handler methods
	static void defaultHandler(Hotspot &h);
	static void standardAnimHandler(Hotspot &h);
	static void standardCharacterAnimHandler(Hotspot &h);
	static void roomExitAnimHandler(Hotspot &h);
	static void playerAnimHandler(Hotspot &h);
	static void droppingTorchAnimHandler(Hotspot &h);
	static void fireAnimHandler(Hotspot &h);
	static void talkAnimHandler(Hotspot &h);
	static void headAnimationHandler(Hotspot &h);

public:
	static HandlerMethodPtr getHandler(uint16 procOffset);
};

enum CurrentAction {NO_ACTION, START_WALKING, DISPATCH_ACTION, EXEC_HOTSPOT_SCRIPT, 
	PROCESSING_PATH, WALKING};
	
class CurrentActionEntry {
private:
	CurrentAction _action;
	Action _hotspotAction;
	uint16 _hotspotId;
	uint16 _usedId;
public:
	CurrentActionEntry(CurrentAction newAction) { _action = newAction; }
	CurrentActionEntry(CurrentAction newAction, Action hsAction, uint16 id) { 
		_action = newAction; 
		_hotspotAction = hsAction;
		_hotspotId = id;
	}
	CurrentActionEntry(CurrentAction newAction, Action hsAction, uint16 id, uint16 uId) { 
		_action = newAction; 
		_hotspotAction = hsAction;
		_hotspotId = id;
		_usedId = uId;
	}

	CurrentAction action() { return _action; }
	Action hotspotAction() { return _hotspotAction; }
	uint16 hotspotId() { return _hotspotId; }
	uint16 usedId() { return _usedId; }
};

class CurrentActionStack {
private:
	ManagedList<CurrentActionEntry *> _actions;
public:
	CurrentActionStack() { _actions.clear(); }

	bool isEmpty() { return _actions.begin() == _actions.end(); }
	void clear() { _actions.clear(); }
	CurrentActionEntry &top() { return **_actions.begin(); }
	CurrentAction action() { return isEmpty() ? NO_ACTION : top().action(); }
	void pop() { _actions.erase(_actions.begin()); }
	void addBack(CurrentAction newAction) {
		_actions.push_back(new CurrentActionEntry(newAction));
	}
	void addBack(CurrentAction newAction, Action hsAction, uint16 id) {
		_actions.push_back(new CurrentActionEntry(newAction, hsAction, id));
	}
	void addFront(CurrentAction newAction) {
		_actions.push_front(new CurrentActionEntry(newAction));
	}
	void addFront(CurrentAction newAction, Action hsAction, uint16 id) {
		_actions.push_front(new CurrentActionEntry(newAction, hsAction, id));
	}
	void addFront(CurrentAction newAction, Action hsAction, uint16 id, uint16 usedId) {
		_actions.push_front(new CurrentActionEntry(newAction, hsAction, id, usedId));
	}
};

class WalkingActionEntry {
private:
	Direction _direction;
	int _numSteps;
public:
	WalkingActionEntry(Direction dir, int steps): _direction(dir), _numSteps(steps) {};
	Direction direction() { return _direction; }
	int &rawSteps() { return _numSteps; }
	int numSteps();
};

enum PathFinderResult {PF_OK, PF_DEST_OCCUPIED, PF_NO_PATH, PF_NO_WALK};

class PathFinder {
private:
	Hotspot *_hotspot;
	ManagedList<WalkingActionEntry *> _list;
	RoomPathsDecompressedData _layer;
	int _stepCtr;
	bool _inProgress;
	int _countdownCtr;
	int16 _destX, _destY;
	int16 _xPos, _yPos;
	int16 _xCurrent, _yCurrent;
	int16 _xDestPos, _yDestPos;
	int16 _xDestCurrent, _yDestCurrent;
	bool _destOccupied;
	bool _cellPopulated;
	PathFinderResult _result;
	uint16 *_pSrc, *_pDest;
	int _xChangeInc, _xChangeStart;
	int _yChangeInc, _yChangeStart;
	int _xCtr, _yCtr;

	void initVars();
	void processCell(uint16 *p);
	void scanLine(int numScans, int changeAmount, uint16 *&pEnd, int &v);

	void add(Direction dir, int steps) { 
		_list.push_front(new WalkingActionEntry(dir, steps)); 
	}
	void addBack(Direction dir, int steps) { 
		_list.push_back(new WalkingActionEntry(dir, steps)); 
	}
	void clear() { _list.clear(); }
public:
	PathFinder(Hotspot *h);
	void reset(RoomPathsData &src);
	bool process();

	void pop() { _list.erase(_list.begin()); }
	WalkingActionEntry &top() { return **_list.begin(); }
	bool isEmpty() { return _list.empty(); }
	int &stepCtr() { return _stepCtr; }
};

enum HotspotPrecheckResult {PC_EXECUTE, PC_NOT_IN_ROOM, PC_UNKNOWN, PC_INITIAL, PC_EXCESS};

class Hotspot {
private:
	HotspotData *_data;
	HotspotAnimData *_anim;
	HandlerMethodPtr _tickHandler;
	Surface *_frames;
	uint16 _hotspotId;
	uint16 _roomNumber;
	int16 _startX, _startY;
	uint16 _height, _width;
	uint16 _heightCopy, _widthCopy;
	uint16 _yCorrection;
	uint16 _charRectY;
	int8 _talkX, _talkY;
	uint16 _numFrames;
	uint16 _frameNumber;
	Direction _direction;
	uint8 _layer;
	uint16 _sequenceOffset;
	uint16 _tickCtr;
	uint32 _actions;
	uint8 _colourOffset;
	bool _persistant;
	HotspotOverrideData *_override;
	bool _skipFlag;

	CurrentActionStack _currentActions;
	PathFinder _pathFinder;

	uint16 _frameCtr;
	uint8 _actionCtr;
	int16 _destX, _destY;
	uint16 _destHotspotId;

	// Support methods
	void startTalk(HotspotData *charHotspot);

	// Action support methods
	HotspotPrecheckResult actionPrecheck(HotspotData *hotspot);
	HotspotPrecheckResult actionPrecheck2(HotspotData *hotspot);
	void actionPrecheck3(HotspotData *hotspot);
	bool characterWalkingCheck(HotspotData *hotspot);
	bool doorCloseCheck(uint16 doorId);

	// Action set
	void doGet(HotspotData *hotspot);
	void doOperate(HotspotData *hotspot, Action action);
	void doOpen(HotspotData *hotspot);
	void doClose(HotspotData *hotspot);
	void doLockUnlock(HotspotData *hotspot);
	void doUse(HotspotData *hotspot);
	void doGive(HotspotData *hotspot);
	void doTalkTo(HotspotData *hotspot);
	void doTell(HotspotData *hotspot);
	void doLook();
	void doLookAt(HotspotData *hotspot);
	void doLookThrough(HotspotData *hotspot);
	void doAsk(HotspotData *hotspot);
	void doDrink(HotspotData *hotspot);
	void doStatus();
	void doBribe(HotspotData *hotspot);
	void doExamine(HotspotData *hotspot);
	void doSimple(HotspotData *hotspot, Action action);
public:
	Hotspot(HotspotData *res);
	Hotspot(Hotspot *character, uint16 objType);
	~Hotspot();

	void setAnimation(uint16 newAnimId);
	void setAnimation(HotspotAnimData *newRecord);
	uint16 hotspotId() { return _hotspotId; }
	Surface &frames() { return *_frames; }
	HotspotAnimData &anim() { return *_anim; }
	HotspotData *resource() { return _data; }
	uint16 numFrames() { return _numFrames; }
	uint16 frameNumber() { return _frameNumber; }
	void setFrameNumber(uint16 v) { _frameNumber = v; }
	void incFrameNumber();
	Direction direction() { return _direction; }
	uint16 frameWidth() { return _width; }
	int16 x() { return _startX; }
	int16 y() { return _startY; }
	int16 destX() { return _destX; }
	int16 destY() { return _destY; }
	int8 talkX() { return _talkX; }
	int8 talkY() { return _talkY; }
	uint16 destHotspotId() { return _destHotspotId; }
	uint16 width() { return _width; }
	uint16 height() { return _height; }
	uint16 widthCopy() { return _widthCopy; }
	uint16 heightCopy() { return _heightCopy; }
	uint16 yCorrection() { return _yCorrection; }
	uint16 charRectY() { return _charRectY; }
	uint16 roomNumber() { return _roomNumber; }
	uint16 script() { return _sequenceOffset; }
	uint8 layer() { return _layer; }
	uint16 tickCtr() { return _tickCtr; }
	bool skipFlag() { return _skipFlag; }
	void setTickCtr(uint16 newVal) { _tickCtr = newVal; }
	void setTickProc(uint16 newVal);
	bool persistant() { return _persistant; }
	void setPersistant(bool value) { _persistant = value; }
	void setRoomNumber(uint16 roomNum) { 
		_roomNumber = roomNum; 
		if (_data) _data->roomNumber = roomNum;
	}
	uint16 nameId();
	bool isActiveAnimation();
	void setPosition(int16 newX, int16 newY);
	void setDestPosition(int16 newX, int16 newY) { _destX = newX; _destY = newY; }
	void setDestHotspot(uint16 id) { _destHotspotId = id; }
	void setSize(uint16 newWidth, uint16 newHeight);
	void setScript(uint16 offset) {
		_sequenceOffset = offset;
		_data->sequenceOffset = offset; 
	}
	void setActions(uint32 newActions) { _actions = newActions; }
	void setCharRectY(uint16 value) { _charRectY = value; }
	void setSkipFlag(bool value) { _skipFlag = value; }

	void copyTo(Surface *dest);
	bool executeScript();
	void tick();
	bool isRoomExit(uint16 id);

	// Walking
	void walkTo(int16 endPosX, int16 endPosY, uint16 destHotspot = 0);
	void stopWalking();
	void setDirection(Direction dir);
	void faceHotspot(HotspotData *hotspot);
	void setOccupied(bool occupiedFlag);
	bool walkingStep();

	// Actions
	void doAction(Action action, HotspotData *hotspot);
	void setCurrentAction(CurrentAction currAction) { 
		_currentActions.addFront(currAction); 
	}
	void setCurrentAction(CurrentAction currAction, Action hsAction, uint16 id) { 
		_currentActions.addFront(currAction, hsAction, id); 
	}
	void setCurrentAction(CurrentAction currAction, Action hsAction, uint16 id, uint16 usedId) { 
		_currentActions.addFront(currAction, hsAction, id, usedId);
	}
	CurrentActionStack &currentActions() { return _currentActions; }
	PathFinder &pathFinder() { return _pathFinder; }
	uint16 frameCtr() { return _frameCtr; }
	void setFrameCtr(uint16 value) { _frameCtr = value; }
	void decrFrameCtr() { if (_frameCtr > 0) --_frameCtr; }
	uint8 actionCtr() { return _actionCtr; }
	void setActionCtr(uint8 v) { _actionCtr = v; }
};

typedef ManagedList<Hotspot *> HotspotList;

} // End of namespace Lure

#endif
