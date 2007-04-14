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

#ifndef LURE_HOTSPOTS_H
#define LURE_HOTSPOTS_H

#include "lure/luredefs.h"
#include "lure/screen.h"
#include "lure/disk.h"
#include "lure/res_struct.h"

namespace Lure {

#define MAX_NUM_IMPINGING 10

class Hotspot;

class Support {
private:
	static bool changeRoomCheckBumped(Hotspot &h);
public:
	static int findIntersectingCharacters(Hotspot &h, uint16 *charList); 
	static bool checkForIntersectingCharacter(Hotspot &h);
	static bool checkRoomChange(Hotspot &h);
	static void characterChangeRoom(Hotspot &h, uint16 roomNumber, 
								  int16 newX, int16 newY, Direction dir);
	static bool charactersIntersecting(HotspotData *hotspot1, HotspotData *hotspot2);
	static bool isCharacterInList(uint16 *lst, int numEntries, uint16 charId);
};

typedef void(*HandlerMethodPtr)(Hotspot &h);

class HotspotTickHandlers {
private:
	// Support methods
	static void npcRoomChange(Hotspot &h);

	// Handler methods
	static void defaultHandler(Hotspot &h);
	static void voiceBubbleAnimHandler(Hotspot &h);
	static void standardAnimHandler(Hotspot &h);
	static void standardAnimHandler2(Hotspot &h);
	static void standardCharacterAnimHandler(Hotspot &h);
	static void puzzledAnimHandler(Hotspot &h);
	static void roomExitAnimHandler(Hotspot &h);
	static void playerAnimHandler(Hotspot &h);
	static void followerAnimHandler(Hotspot &h);
	static void skorlAnimHandler(Hotspot &h);
	static void droppingTorchAnimHandler(Hotspot &h);
	static void playerSewerExitAnimHandler(Hotspot &h);
	static void fireAnimHandler(Hotspot &h);
	static void goewinCaptiveAnimHandler(Hotspot &h);
	static void prisonerAnimHandler(Hotspot &h);
	static void catrionaAnimHandler(Hotspot &h);
	static void morkusAnimHandler(Hotspot &h);
	static void talkAnimHandler(Hotspot &h);
	static void headAnimHandler(Hotspot &h);
	static void barmanAnimHandler(Hotspot &h);
	static void skorlGaurdAnimHandler(Hotspot &h);
	static void rackSerfAnimHandler(Hotspot &h);

public:
	static HandlerMethodPtr getHandler(uint16 procOffset);
};

enum CurrentAction {NO_ACTION, START_WALKING, DISPATCH_ACTION, EXEC_HOTSPOT_SCRIPT, 
	PROCESSING_PATH, WALKING};
	
class CurrentActionEntry {
private:
	CurrentAction _action;
	CharacterScheduleEntry *_supportData;
	uint16 _roomNumber;
	bool _dynamicSupportData;
public:
	CurrentActionEntry(CurrentAction newAction, uint16 roomNum);
	CurrentActionEntry(CurrentAction newAction, CharacterScheduleEntry *data, uint16 roomNum);
	CurrentActionEntry(Action newAction, uint16 roomNum, uint16 param1, uint16 param2);
	virtual ~CurrentActionEntry() {
		if (_dynamicSupportData) delete _supportData;
	}

	CurrentAction action() { return _action; }
	CharacterScheduleEntry &supportData() { 
		if (!_supportData) error("Access made to non-defined action support record");
		return *_supportData;
	}
	bool hasSupportData() { return _supportData != NULL; }
	uint16 roomNumber() { return _roomNumber; }
	void setAction(CurrentAction newAction) { _action = newAction; }
	void setRoomNumber(uint16 roomNum) { _roomNumber = roomNum; }
	void setSupportData(CharacterScheduleEntry *newRec) { 
		assert((newRec == NULL) || (newRec->parent() != NULL));
		_supportData = newRec; 
	}
	void setSupportData(uint16 entryId);

	void saveToStream(WriteStream *stream);
	static CurrentActionEntry *loadFromStream(ReadStream *stream);
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
	int size() { return _actions.size(); }
	void list(char *buffer);
	void list() { list(NULL); }

	void addBack(CurrentAction newAction, uint16 roomNum) {
		_actions.push_back(new CurrentActionEntry(newAction, roomNum));
	}
	void addBack(CurrentAction newAction, CharacterScheduleEntry *rec, uint16 roomNum) {
		_actions.push_back(new CurrentActionEntry(newAction, rec, roomNum));
	}
	void addBack(Action newAction, uint16 roomNum, uint16 param1, uint16 param2) {
		_actions.push_back(new CurrentActionEntry(newAction, roomNum, param1, param2));
	}
	void addFront(CurrentAction newAction, uint16 roomNum) {
		_actions.push_front(new CurrentActionEntry(newAction, roomNum));
	}
	void addFront(CurrentAction newAction, CharacterScheduleEntry *rec, uint16 roomNum) {
		_actions.push_front(new CurrentActionEntry(newAction, rec, roomNum));
	}
	void addFront(Action newAction, uint16 roomNum, uint16 param1, uint16 param2) {
		_actions.push_front(new CurrentActionEntry(newAction, roomNum, param1, param2));
	}

	void saveToStream(WriteStream *stream);
	void loadFromStream(ReadStream *stream);
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
public:
	PathFinder(Hotspot *h);
	void clear();
	void reset(RoomPathsData &src);
	bool process();
	void list(char *buffer);
	void list() { list(NULL); }

	void pop() { _list.erase(_list.begin()); }
	WalkingActionEntry &top() { return **_list.begin(); }
	bool isEmpty() { return _list.empty(); }
	int &stepCtr() { return _stepCtr; }
	PathFinderResult result() { return _result; }

	void saveToStream(Common::WriteStream *stream);
	void loadFromStream(Common::ReadStream *stream);
};

enum HotspotPrecheckResult {PC_EXECUTE, PC_NOT_IN_ROOM, PC_UNKNOWN, PC_INITIAL, PC_EXCESS};

enum BarPlaceResult {BP_KEEP_TRYING, BP_GOT_THERE, BP_FAIL};

#define MAX_NUM_FRAMES 16

class Hotspot {
private:
	HotspotData *_data;
	uint16 _animId;
	HotspotAnimData *_anim;
	HandlerMethodPtr _tickHandler;
	Surface *_frames;
	uint16 _hotspotId;
	uint16 _originalId;
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
	uint16 _frameWidth;
	bool _frameStartsUsed;
	uint16 _frameStarts[MAX_NUM_FRAMES];
	char _nameBuffer[MAX_HOTSPOT_NAME_SIZE];

	// Runtime fields
	uint16 _frameCtr;
	uint8 _voiceCtr;
	int16 _destX, _destY;
	uint16 _destHotspotId;
	uint16 _blockedOffset;
	uint8 _exitCtr;
	bool _walkFlag;
	uint16 _startRoomNumber;
	uint16 _supportValue; 
	
	// Support methods
	uint16 getTalkId(HotspotData *charHotspot);
	void startTalk(HotspotData *charHotspot, uint16 id);
	void startTalkDialog();

	// Action support methods
	HotspotPrecheckResult actionPrecheck(HotspotData *hotspot);
	BarPlaceResult getBarPlace();
	bool findClearBarPlace();
	bool characterWalkingCheck(HotspotData *hotspot);
	bool doorCloseCheck(uint16 doorId);
	void resetDirection();

	// Action set
	void doNothing(HotspotData *hotspot);
	void doGet(HotspotData *hotspot);
	void doOperate(HotspotData *hotspot);
	void doOpen(HotspotData *hotspot);
	void doClose(HotspotData *hotspot);
	void doLockUnlock(HotspotData *hotspot);
	void doUse(HotspotData *hotspot);
	void doGive(HotspotData *hotspot);
	void doTalkTo(HotspotData *hotspot);
	void doTell(HotspotData *hotspot);
	void doLook(HotspotData *hotspot);
	void doLookAt(HotspotData *hotspot);
	void doLookThrough(HotspotData *hotspot);
	void doAsk(HotspotData *hotspot);
	void doDrink(HotspotData *hotspot);
	void doStatus(HotspotData *hotspot);
	void doGoto(HotspotData *hotspot);
	void doReturn(HotspotData *hotspot);
	void doBribe(HotspotData *hotspot);
	void doExamine(HotspotData *hotspot);
	void npcSetRoomAndBlockedOffset(HotspotData *hotspot);
	void npcHeySir(HotspotData *hotspot); 
	void npcExecScript(HotspotData *hotspot); 
	void npcResetPausedList(HotspotData *hotspot); 
	void npcSetRandomDest(HotspotData *hotspot);
	void npcWalkingCheck(HotspotData *hotspot); 
	void npcSetSupportOffset(HotspotData *hotspot); 
	void npcSupportOffsetConditional(HotspotData *hotspot);
	void npcDispatchAction(HotspotData *hotspot); 
	void npcUnknown3(HotspotData *hotspot); 
	void npcPause(HotspotData *hotspot); 
	void npcStartTalking(HotspotData *hotspot);
	void npcJumpAddress(HotspotData *hotspot);
public:
	Hotspot(HotspotData *res);
	Hotspot(Hotspot *character, uint16 objType);
	~Hotspot();

	void setAnimation(uint16 newAnimId);
	void setAnimation(HotspotAnimData *newRecord);
	uint16 hotspotId() { return _hotspotId; }
	uint16 originalId() { return _originalId; }
	Surface &frames() { return *_frames; }
	HotspotAnimData &anim() { return *_anim; }
	HotspotData *resource() { return _data; }
	uint16 numFrames() { return _numFrames; }
	uint16 frameNumber() { return _frameNumber; }
	void setFrameNumber(uint16 frameNum) { 
		assert(frameNum < _numFrames);
		_frameNumber = frameNum; 
	}
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
	uint16 blockedOffset() { return _blockedOffset; }
	uint8 exitCtr() { return _exitCtr; }
	bool walkFlag() { return _walkFlag; }
	uint16 startRoomNumber() { return _startRoomNumber; }
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
	const char *getName();
	bool isActiveAnimation();
	void setPosition(int16 newX, int16 newY);
	void setDestPosition(int16 newX, int16 newY) { _destX = newX; _destY = newY; }
	void setDestHotspot(uint16 id) { _destHotspotId = id; }
	void setExitCtr(uint8 value) { _exitCtr = value; }
	BlockedState blockedState() { 
		assert(_data);
		return _data->blockedState; 
	}
	void setBlockedState(BlockedState newState) { 
		assert(_data);
		_data->blockedState = newState; 
	}
	bool blockedFlag() {
		assert(_data);
		return _data->blockedFlag;
	}
	void setBlockedFlag(bool newValue) {
		assert(_data);
		_data->blockedFlag = newValue;
	}
	void setWalkFlag(bool value) { _walkFlag = value; }
	void setStartRoomNumber(uint16 value) { _startRoomNumber = value; }
	void setSize(uint16 newWidth, uint16 newHeight);
	void setWidth(uint16 newWidth) {
		_width = newWidth;
		_frameWidth = newWidth;
	}
	void setHeight(uint16 newHeight) { 
		_height = newHeight;
	}
	void setScript(uint16 offset) {
		assert(_data != NULL);
		_sequenceOffset = offset;
		_data->sequenceOffset = offset; 
	}
	void setLayer(uint8 newLayer) {
		assert(_data != NULL);
		_layer = newLayer;
		_data->layer = newLayer;
	}
	void setActions(uint32 newActions) { _actions = newActions; }
	void setCharRectY(uint16 value) { _charRectY = value; }
	void setSkipFlag(bool value) { _skipFlag = value; }
	CharacterMode characterMode() {
		assert(_data != NULL);
		return _data->characterMode;
	}
	void setCharacterMode(CharacterMode value) {
		assert(_data != NULL);
		_data->characterMode = value;
	}
	uint16 delayCtr() { 
		assert(_data);
		return _data->delayCtr; 
	}
	void setDelayCtr(uint16 value) { 
		assert(_data);
		_data->delayCtr = value;	
	}
	uint16 pauseCtr() { 
		assert(_data);
		return _data->pauseCtr;
	}
	void setPauseCtr(uint16 value) { 
		assert(_data);
		_data->pauseCtr = value;
	}
	VariantBool coveredFlag() { 
		assert(_data);
		return _data->coveredFlag;
	}
	void setCoveredFlag(VariantBool value) { 
		assert(_data);
		_data->coveredFlag = value;
	}
	uint16 useHotspotId() { 
		assert(_data);
		return _data->useHotspotId; 
	}
	void setUseHotspotId(uint16 value) {
		assert(_data);
		_data->useHotspotId = value;
	}
	uint16 v2b() {
		assert(_data);
		return _data->v2b;
	}
	void setV2b(uint16 value) {
		assert(_data);
		_data->v2b = value;
	}
	uint16 supportValue() { return _supportValue; }
	void setSupportValue(uint16 value) { _supportValue = value; }

	void copyTo(Surface *dest);
	bool executeScript();
	void tick();
	bool isRoomExit(uint16 id);

	// Walking
	void walkTo(int16 endPosX, int16 endPosY, uint16 destHotspot = 0);
	void stopWalking();
	void endAction();
	void setDirection(Direction dir);
	void faceHotspot(HotspotData *hotspot);
	void faceHotspot(uint16 hotspotId);
	void setRandomDest();
	void setOccupied(bool occupiedFlag);
	bool walkingStep();
	void updateMovement();
	void updateMovement2(CharacterMode value);
	void resetPosition();

	void doAction();
	void doAction(Action action, HotspotData *hotspot);
	CurrentActionStack &currentActions() { return _currentActions; }
	PathFinder &pathFinder() { return _pathFinder; }
	uint16 frameCtr() { return _frameCtr; }
	void setFrameCtr(uint16 value) { _frameCtr = value; }
	void decrFrameCtr() { if (_frameCtr > 0) --_frameCtr; }
	uint8 actionCtr() { 
		assert(_data);
		return _data->actionCtr; 
	}
	void setActionCtr(uint8 v) { 
		assert(_data);
		_data->actionCtr = v; 
	}
	uint8 voiceCtr() { return _voiceCtr; }
	void setVoiceCtr(uint8 v) { _voiceCtr = v; }

	// Miscellaneous
	void converse(uint16 destCharacterId, uint16 messageId, bool standStill);
	void converse(uint16 destCharacterId, uint16 messageId) {
		converse(destCharacterId, messageId, false);
	}
	void showMessage(uint16 messageId, uint16 destCharacterId = NOONE_ID);
	void scheduleConverse(uint16 destHotspot, uint16 messageId);
	void handleTalkDialog();

	void saveToStream(Common::WriteStream *stream);
	void loadFromStream(Common::ReadStream *stream);
};

class HotspotList: public ManagedList<Hotspot *> {
public:
	void saveToStream(WriteStream *stream);
	void loadFromStream(ReadStream *stream);
};

} // End of namespace Lure

#endif
