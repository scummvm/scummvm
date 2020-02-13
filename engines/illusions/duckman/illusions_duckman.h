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
class DuckmanDialogSystem;
class DuckmanMenuSystem;
class DuckmanVideoPlayer;

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

struct OpCall;

class IllusionsEngine_Duckman : public IllusionsEngine {
public:
	IllusionsEngine_Duckman(OSystem *syst, const IllusionsGameDescription *gd);
protected:
	Common::Error run() override;
	bool hasFeature(EngineFeature f) const override;
public:

	uint32 _prevSceneId;
	uint32 _theSceneId;
	uint32 _theThreadId;
	uint32 _globalSceneId;

	uint _activeScenesCount;
	uint32 _activeScenes[6];

	Cursor_Duckman _cursor;
	Control *_currWalkOverlappedControl;

	DuckmanDialogSystem *_dialogSys;

	int _savedInventoryActorIndex;

	ScreenShaker *_screenShaker;
	DuckmanMenuSystem *_menuSystem;
	DuckmanVideoPlayer *_videoPlayer;

	void initInput();

	void initUpdateFunctions();
	int updateScript(uint flags);

	void startScreenShaker(uint pointsCount, uint32 duration, const ScreenShakerPoint *points, uint32 threadId);
	void stopScreenShaker();
	int updateScreenShaker(uint flags);

	void startFader(int duration, int minValue, int maxValue, int firstIndex, int lastIndex, uint32 threadId);
	void updateFader() override;
	void clearFader() override;
	void pauseFader() override;
	void unpauseFader() override;

	int updateVideoPlayer(uint flags);
	void playVideo(uint32 videoId, uint32 callingThreadId);
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
	void initCursor();
	void setCursorActorIndex(int actorIndex, int a, int b);
	void enableCursorVerb(int verbNum);
	void disableCursorVerb(int verbNum);
	void setCursorHandMode(int mode);
	void setCursorInventoryMode(int mode, int value);
	void startCursorHoldingObject(uint32 objectId, uint32 sequenceId);
	void stopCursorHoldingObject();
	void cursorControlRoutine(Control *control, uint32 deltaTime);

	void startScriptThreadSimple(uint32 threadId, uint32 callingThreadId) override;
	void startScriptThread(uint32 threadId, uint32 callingThreadId);
	void startScriptThread2(uint32 threadId, uint32 callingThreadId, uint32 unk);
	uint32 startAbortableTimerThread(uint32 duration, uint32 threadId);
	uint32 startTimerThread(uint32 duration, uint32 threadId);
	uint32 startAbortableThread(byte *scriptCodeIp1, byte *scriptCodeIp2, uint32 callingThreadId);
	uint32 startTalkThread(uint32 objectId, uint32 talkId, uint32 sequenceId1,
		uint32 sequenceId2, uint32 callingThreadId);
	uint32 startTempScriptThread(byte *scriptCodeIp, uint32 callingThreadId,
		uint32 value8, uint32 valueC, uint32 value10) override;
	void resumeFromSavegame(uint32 callingThreadId);

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

	void pause(uint32 callerThreadId);
	void unpause(uint32 callerThreadId);

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
	void playTriggerCauseSound(uint32 verbId, uint32 objectId2, uint32 objectId);

	bool loadSavegameFromScript(int16 slotNum, uint32 callingThreadId);
	bool saveSavegameFromScript(int16 slotNum, uint32 callingThreadId);
	void activateSavegame(uint32 callingThreadId);

};

} // End of namespace Illusions

#endif // ILLUSIONS_ILLUSIONS_H
