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

#ifndef ILLUSIONS_ILLUSIONS_BBDOU_H
#define ILLUSIONS_ILLUSIONS_BBDOU_H

#include "illusions/illusions.h"
#include "common/algorithm.h"
#include "common/stack.h"

namespace Illusions {

class Dictionary;
class ScriptMan;
class ScriptStack;
class TriggerFunctions;
class TriggerFunction;

typedef Common::Functor2<TriggerFunction*, uint32, void> TriggerFunctionCallback;

struct TriggerFunction {
	uint32 _sceneId;
	uint32 _verbId;
	uint32 _objectId2;
	uint32 _objectId;
	TriggerFunctionCallback *_callback;
	TriggerFunction(uint32 sceneId, uint32 verbId, uint32 objectId2, uint32 objectId, TriggerFunctionCallback *callback);
	~TriggerFunction();
	void run(uint32 callingThreadId);
};

class TriggerFunctions {
public:
	void add(uint32 sceneId, uint32 verbId, uint32 objectId2, uint32 objectId, TriggerFunctionCallback *callback);
	TriggerFunction *find(uint32 sceneId, uint32 verbId, uint32 objectId2, uint32 objectId);
	void removeBySceneId(uint32 sceneId);
public:
	typedef Common::List<TriggerFunction*> Items;
	typedef Items::iterator ItemsIterator;
	Items _triggerFunctions;
	ItemsIterator findInternal(uint32 sceneId, uint32 verbId, uint32 objectId2, uint32 objectId);
};

struct ActiveScene {
	uint32 _sceneId;
	int _pauseCtr;
};

class ActiveScenes {
public:
	ActiveScenes();
	void clear();
	void push(uint32 sceneId);
	void pop();
	void pauseActiveScene();
	void unpauseActiveScene();
	uint getActiveScenesCount();
	void getActiveSceneInfo(uint index, uint32 *sceneId, int *pauseCtr);
	uint32 getCurrentScene();
	bool isSceneActive(uint32 sceneId);
protected:
	Common::FixedStack<ActiveScene, 16> _stack;
};

class IllusionsEngine_BBDOU : public IllusionsEngine {
public:
	IllusionsEngine_BBDOU(OSystem *syst, const IllusionsGameDescription *gd);
protected:
	virtual Common::Error run();
	virtual bool hasFeature(EngineFeature f) const;
public:	
	ScriptMan *_scriptMan;
	TriggerFunctions *_triggerFunctions;
	Cursor *_cursor;

	ActiveScenes _activeScenes;
	uint32 _prevSceneId;
	uint32 _theSceneId;
	uint32 _theThreadId;
	uint32 _globalSceneId;

	void initUpdateFunctions();
	int updateScript(uint flags);

	bool causeIsDeclared(uint32 sceneId, uint32 verbId, uint32 objectId2, uint32 objectId);
	void causeDeclare(uint32 verbId, uint32 objectId2, uint32 objectId, TriggerFunctionCallback *callback);
	uint32 causeTrigger(uint32 sceneId, uint32 verbId, uint32 objectId2, uint32 objectId, uint32 callingThreadId);

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
	void cursorControlRoutine(Control *control, uint32 deltaTime);

	void startScriptThreadSimple(uint32 threadId, uint32 callingThreadId);
	void startScriptThread(uint32 threadId, uint32 callingThreadId,
		uint32 value8, uint32 valueC, uint32 value10);
	void startAnonScriptThread(int32 threadId, uint32 callingThreadId,
		uint32 value8, uint32 valueC, uint32 value10);
	uint32 startAbortableTimerThread(uint32 duration, uint32 threadId);
	uint32 startTimerThread(uint32 duration, uint32 threadId);
	uint32 startAbortableThread(byte *scriptCodeIp1, byte *scriptCodeIp2, uint32 callingThreadId);
	uint32 startTalkThread(int16 duration, uint32 objectId, uint32 talkId, uint32 sequenceId1,
		uint32 sequenceId2, uint32 namedPointId, uint32 callingThreadId);
	uint32 startTempScriptThread(byte *scriptCodeIp, uint32 callingThreadId,
		uint32 value8, uint32 valueC, uint32 value10);
	void newScriptThread(uint32 threadId, uint32 callingThreadId, uint notifyFlags,
		byte *scriptCodeIp, uint32 value8, uint32 valueC, uint32 value10);
	uint32 newTimerThread(uint32 duration, uint32 callingThreadId, bool isAbortable);
	uint32 newTempThreadId();

	bool enterScene(uint32 sceneId, uint32 threadId);
	void exitScene(uint32 threadId);
	void enterPause(uint32 threadId);
	void leavePause(uint32 threadId);
	void dumpActiveScenes(uint32 sceneId, uint32 threadId);

	void setSceneIdThreadId(uint32 theSceneId, uint32 theThreadId);
	bool findTriggerCause(uint32 sceneId, uint32 verbId, uint32 objectId2, uint32 objectId, uint32 &codeOffs);
	void reset();
	
};

} // End of namespace Illusions

#endif // ILLUSIONS_ILLUSIONS_H
