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
 */

#ifndef ILLUSIONS_ILLUSIONS_DUCKMAN_H
#define ILLUSIONS_ILLUSIONS_DUCKMAN_H

#include "illusions/illusions.h"
#include "common/algorithm.h"
#include "common/stack.h"

namespace Illusions {

class Dictionary;
class ScriptStack;

struct Cursor_Duckman {
	int _gameState;
	Control *_control;
	Common::Point _position;
	uint32 _objectId;
	int _actorIndex;
	int _savedActorIndex;
	bool _field14[14];
	Control *_currOverlappedControl;
	uint32 _sequenceId1;
	uint32 _sequenceId2;
	uint32 _notifyThreadId30;
	int16 *_op113_choiceOfsPtr;
	int _dialogItemsCount;
	uint32 _overlappedObjectId;
	uint32 _field3C;
	uint32 _field40;
};

struct DialogItem {
	int16 _choiceJumpOffs;
	uint32 _sequenceId;
};

struct DMInventorySlot {
	Common::Point _position;
	uint32 _objectId;
	DMInventorySlot() : _objectId(0) {}
	DMInventorySlot(int16 x, int16 y) : _objectId(0), _position(x, y) {}
};

struct DMInventoryItem {
	uint32 _objectId;
	uint32 _propertyId;
	DMInventoryItem() : _objectId(0) {}
	DMInventoryItem(uint32 objectId, uint32 propertyId)
		: _objectId(objectId), _propertyId(propertyId) {}
};

struct ScreenShakerPoint {
	int16 x, y;
};

struct ScreenShakeEffect {
	uint32 _duration;
	uint _pointsCount;
	const ScreenShakerPoint *_points;
};

struct ScreenShaker {
	uint _pointsIndex;
	uint _pointsCount;
	bool _finished;
	uint32 _duration;
	uint32 _nextTime;
	uint32 _notifyThreadId;
	const ScreenShakerPoint *_points;
};

struct PropertyTimer {
	uint32 _propertyId;
	uint32 _startTime;
	uint32 _duration;
	uint32 _endTime;
	PropertyTimer() : _propertyId(0) {}
};

const uint kPropertyTimersCount = 6;

struct OpCall;

typedef Common::Functor1<OpCall&, void> SpecialCodeFunction;
typedef Common::HashMap<uint32, SpecialCodeFunction*> SpecialCodeMap;
typedef SpecialCodeMap::iterator SpecialCodeMapIterator;

class IllusionsEngine_Duckman : public IllusionsEngine {
public:
	IllusionsEngine_Duckman(OSystem *syst, const IllusionsGameDescription *gd);
protected:
	virtual Common::Error run();
	virtual bool hasFeature(EngineFeature f) const;
public:	

	// TODO ActiveScenes _activeScenes;
	uint32 _prevSceneId;
	uint32 _theSceneId;
	uint32 _theThreadId;
	uint32 _globalSceneId;

	uint _activeScenesCount;
	uint32 _activeScenes[6];

	Cursor_Duckman _cursor;
	Control *_currWalkOverlappedControl;

	Common::Array<DialogItem> _dialogItems;

	int _savedInventoryActorIndex;
	Common::Array<DMInventorySlot> _inventorySlots;
	Common::Array<DMInventoryItem> _inventoyItems;

	ScreenShaker *_screenShaker;

	PropertyTimer _propertyTimers[kPropertyTimersCount];
	bool _propertyTimersActive;
	bool _propertyTimersPaused;

	uint _chinesePuzzleIndex;
	byte _chinesePuzzleAnswers[3];

	SpecialCodeMap _specialCodeMap;

	void initUpdateFunctions();
	int updateScript(uint flags);

	void startScreenShaker(uint pointsCount, uint32 duration, const ScreenShakerPoint *points, uint32 threadId);
	int updateScreenShaker(uint flags);

	void startFader(int duration, int minValue, int maxValue, int firstIndex, int lastIndex, uint32 threadId);

	void setDefaultTextCoords();

	void loadSpecialCode(uint32 resId);
	void unloadSpecialCode(uint32 resId);
	void notifyThreadId(uint32 &threadId);
	bool testMainActorFastWalk(Control *control);
	bool testMainActorCollision(Control *control);
	Control *getObjectControl(uint32 objectId);
	Common::Point getNamedPointPosition(uint32 namedPointId);
	uint32 getPriorityFromBase(int16 priority);
	uint32 getCurrentScene();
	uint32 getPrevScene();

	bool isCursorObject(uint32 actorTypeId, uint32 objectId);
	void setCursorControlRoutine(Control *control);
	void placeCursorControl(Control *control, uint32 sequenceId);
	void setCursorControl(Control *control);
	void showCursor();
	void hideCursor();
	void initCursor();
	void setCursorActorIndex(int actorIndex, int a, int b);
	void enableCursorVerb(int verbNum);
	void disableCursorVerb(int verbNum);
	void setCursorHandMode(int mode);
	void setCursorInventoryMode(int mode, int value);
	void startCursorHoldingObject(uint32 objectId, uint32 sequenceId);
	void stopCursorHoldingObject();
	void cursorControlRoutine(Control *control, uint32 deltaTime);

	void startScriptThreadSimple(uint32 threadId, uint32 callingThreadId);
	void startScriptThread(uint32 threadId, uint32 callingThreadId);
	uint32 startAbortableTimerThread(uint32 duration, uint32 threadId);
	uint32 startTimerThread(uint32 duration, uint32 threadId);
	uint32 startAbortableThread(byte *scriptCodeIp1, byte *scriptCodeIp2, uint32 callingThreadId);
	uint32 startTalkThread(uint32 objectId, uint32 talkId, uint32 sequenceId1,
		uint32 sequenceId2, uint32 callingThreadId);
	uint32 startTempScriptThread(byte *scriptCodeIp, uint32 callingThreadId,
		uint32 value8, uint32 valueC, uint32 value10);
	void newScriptThread(uint32 threadId, uint32 callingThreadId, uint notifyFlags,
		byte *scriptCodeIp);
	uint32 newTimerThread(uint32 duration, uint32 callingThreadId, bool isAbortable);
	uint32 newTempThreadId();

	void initActiveScenes();
	void pushActiveScene(uint32 sceneId);
	void popActiveScene();
	bool loadScene(uint32 sceneId);
	bool enterScene(uint32 sceneId, uint32 threadId);
	void exitScene();
	bool changeScene(uint32 sceneId, uint32 threadId, uint32 callerThreadId);
	void enterPause(uint32 sceneId, uint32 threadId);
	void leavePause(uint32 sceneId, uint32 threadId);
	void dumpActiveScenes(uint32 sceneId, uint32 threadId);
	void dumpCurrSceneFiles(uint32 sceneId, uint32 threadId);

	void setSceneIdThreadId(uint32 theSceneId, uint32 theThreadId);
	bool findTriggerCause(uint32 sceneId, uint32 verbId, uint32 objectId2, uint32 objectId, uint32 &codeOffs);
	void reset();
	
	uint32 getObjectActorTypeId(uint32 objectId);
	
	Common::Point convertMousePos(Common::Point mousePos);
	void startCursorSequence();
	int getCursorActorIndex();
	void updateGameState2();
	void playSoundEffect(int index);
	bool getTriggerCause(uint32 verbId, uint32 objectId2, uint32 objectId, uint32 &outThreadId);
	uint32 runTriggerCause(uint32 verbId, uint32 objectId2, uint32 objectId);

	void addDialogItem(int16 choiceJumpOffs, uint32 sequenceId);
	void startDialog(int16 *choiceOfsPtr, uint32 actorTypeId, uint32 callerThreadId);
	void updateDialogState();

	void initInventory();
	void openInventory();
	void addInventoryItem(uint32 objectId);
	void clearInventorySlot(uint32 objectId);
	void putBackInventoryItem();
	DMInventorySlot *findInventorySlot(uint32 objectId);
	DMInventoryItem *findInventoryItem(uint32 objectId);
	DMInventorySlot *findClosestInventorySlot(Common::Point pos);

	void addPropertyTimer(uint32 propertyId);
	void setPropertyTimer(uint32 propertyId, uint32 duration);
	void removePropertyTimer(uint32 propertyId);
	bool findPropertyTimer(uint32 propertyId, PropertyTimer *&propertyTimer);
	int updatePropertyTimers(uint flags);

	// Special code
	void initSpecialCode();
	void runSpecialCode(uint32 specialCodeId, OpCall &opCall);
	void spcStartScreenShaker(OpCall &opCall);
	void spcSetCursorHandMode(OpCall &opCall);
	void spcResetChinesePuzzle(OpCall &opCall);
	void spcAddChinesePuzzleAnswer(OpCall &opCall);
	void spcOpenInventory(OpCall &opCall);
	void spcPutBackInventoryItem(OpCall &opCall);
	void spcClearInventorySlot(OpCall &opCall);
	void spcAddPropertyTimer(OpCall &opCall);
	void spcSetPropertyTimer(OpCall &opCall);
	void spcRemovePropertyTimer(OpCall &opCall);
	void spcCenterNewspaper(OpCall &opCall);
	void spcSetCursorInventoryMode(OpCall &opCall);
	void spcUpdateObject272Sequence(OpCall &opCall);

};

} // End of namespace Illusions

#endif // ILLUSIONS_ILLUSIONS_H
