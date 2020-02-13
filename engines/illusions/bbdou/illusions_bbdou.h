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
#include "illusions/bbdou/bbdou_triggerfunctions.h"
#include "common/algorithm.h"
#include "common/stack.h"

namespace Illusions {

class Dictionary;
class ScriptMan;
class ScriptStack;
class BBDOUVideoPlayer;
class BBDOUMenuKeys;
class BBDOUMenuSystem;

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
	Common::Error run() override;
	bool hasFeature(EngineFeature f) const override;
public:
	ScriptMan *_scriptMan;
	TriggerFunctions *_triggerFunctions;
	Cursor *_cursor;

	ActiveScenes _activeScenes;
	uint32 _prevSceneId;
	uint32 _theSceneId;
	uint32 _theThreadId;
	uint32 _globalSceneId;

	bool _loadGameResult, _saveGameResult;

	BBDOUMenuSystem *_menuSystem;
	BBDOUVideoPlayer *_videoPlayer;
	BBDOUMenuKeys *_menuKeys;

	bool _walkthroughStarted;
	bool _canResumeFromSavegame;

	void initInput();

	void initUpdateFunctions();
	int updateScript(uint flags);
	int updateMenuKeys(uint flags);

	bool causeIsDeclared(uint32 sceneId, uint32 verbId, uint32 objectId2, uint32 objectId);
	void causeDeclare(uint32 verbId, uint32 objectId2, uint32 objectId, TriggerFunctionCallback *callback);
	uint32 causeTrigger(uint32 sceneId, uint32 verbId, uint32 objectId2, uint32 objectId, uint32 callingThreadId);

	int updateVideoPlayer(uint flags);
	void playVideo(uint32 videoId, uint32 objectId, uint32 priority, uint32 callingThreadId);
	bool isVideoPlaying() override;

	void setDefaultTextCoords() override;

	void loadSpecialCode(uint32 resId) override;
	void unloadSpecialCode(uint32 resId) override;
	void notifyThreadId(uint32 &threadId) override;
	bool testMainActorFastWalk(Control *control) override;
	bool testMainActorCollision(Control *control) override;
	Control *getObjectControl(uint32 objectId) override;
	Common::Point getNamedPointPosition(uint32 namedPointId) override;
	uint32 getPriorityFromBase(int16 priority) override;
	uint32 getCurrentScene() override;
	uint32 getPrevScene() override;

	bool isCursorObject(uint32 actorTypeId, uint32 objectId) override;
	void setCursorControlRoutine(Control *control) override;
	void placeCursorControl(Control *control, uint32 sequenceId) override;
	void setCursorControl(Control *control) override;
	void showCursor() override;
	void hideCursor() override;
	void cursorControlRoutine(Control *control, uint32 deltaTime);

	void startScriptThreadSimple(uint32 threadId, uint32 callingThreadId) override;
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
		uint32 value8, uint32 valueC, uint32 value10) override;

	void newScriptThread(uint32 threadId, uint32 callingThreadId, uint notifyFlags,
		byte *scriptCodeIp, uint32 value8, uint32 valueC, uint32 value10);
	uint32 newTimerThread(uint32 duration, uint32 callingThreadId, bool isAbortable);
	uint32 newTempThreadId();

	bool enterScene(uint32 sceneId, uint32 threadId);
	void exitScene(uint32 threadId);
	void enterPause(uint32 threadId);
	void leavePause(uint32 threadId);
	void dumpActiveScenes(uint32 sceneId, uint32 threadId);

	void pause(uint32 callerThreadId);
	void unpause(uint32 callerThreadId);

	void enterMenuPause();
	void leaveMenuPause();

	void setSceneIdThreadId(uint32 theSceneId, uint32 theThreadId);
	bool findTriggerCause(uint32 sceneId, uint32 verbId, uint32 objectId2, uint32 objectId, uint32 &codeOffs);
	void reset();

	void loadSavegameFromScript(int16 slotNum, uint32 callingThreadId);
	void saveSavegameFromScript(int16 slotNum, uint32 callingThreadId);
	void activateSavegame(uint32 callingThreadId);
	void resumeFromSavegame();

};

} // End of namespace Illusions

#endif // ILLUSIONS_ILLUSIONS_H
