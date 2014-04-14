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

#include "illusions/illusions_duckman.h"
#include "illusions/actor.h"
#include "illusions/actorresource.h"
#include "illusions/backgroundresource.h"
#include "illusions/camera.h"
#include "illusions/cursor.h"
#include "illusions/dictionary.h"
#include "illusions/fontresource.h"
#include "illusions/graphics.h"
#include "illusions/input.h"
#include "illusions/midiresource.h"
#include "illusions/resourcesystem.h"
#include "illusions/screen.h"
#include "illusions/screentext.h"
#include "illusions/scriptopcodes_duckman.h"
#include "illusions/scriptresource.h"
#include "illusions/scriptman.h"
#include "illusions/soundresource.h"
#include "illusions/specialcode.h"
//TODO#include "illusions/bbdou/bbdou_specialcode.h"
#include "illusions/talkresource.h"
#include "illusions/textdrawer.h"
#include "illusions/thread.h"
#include "illusions/time.h"
#include "illusions/updatefunctions.h"

#include "illusions/abortablethread.h"
#include "illusions/causethread_duckman.h"
#include "illusions/scriptthread.h"
#include "illusions/talkthread_duckman.h"
#include "illusions/timerthread.h"

#include "audio/audiostream.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/timer.h"
#include "engines/util.h"
#include "graphics/cursorman.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/palette.h"
#include "graphics/surface.h"

namespace Illusions {

// IllusionsEngine_Duckman

IllusionsEngine_Duckman::IllusionsEngine_Duckman(OSystem *syst, const IllusionsGameDescription *gd)
	: IllusionsEngine(syst, gd) {
}

Common::Error IllusionsEngine_Duckman::run() {

	// Init search paths
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "music");
	SearchMan.addSubDirectoryMatching(gameDataDir, "sfx");
	SearchMan.addSubDirectoryMatching(gameDataDir, "video");
	SearchMan.addSubDirectoryMatching(gameDataDir, "voice");
	SearchMan.addSubDirectoryMatching(gameDataDir, "x");// DEBUG until gam reader is done

	_dict = new Dictionary();

	_resSys = new ResourceSystem(this);
	_resSys->addResourceLoader(0x00060000, new ActorResourceLoader(this));
	_resSys->addResourceLoader(0x00080000, new SoundGroupResourceLoader(this));
	_resSys->addResourceLoader(0x000A0000, new MidiGroupResourceLoader(this));
	_resSys->addResourceLoader(0x000D0000, new ScriptResourceLoader(this));
	_resSys->addResourceLoader(0x000F0000, new TalkResourceLoader(this));
	_resSys->addResourceLoader(0x00100000, new ActorResourceLoader(this));
	_resSys->addResourceLoader(0x00110000, new BackgroundResourceLoader(this));
	_resSys->addResourceLoader(0x00120000, new FontResourceLoader(this));

	_screen = new Screen(this, 320, 200, 8);
	_screenText = new ScreenText(this);
	_input = new Input();	
	_actorItems = new ActorItems(this);
	_backgroundItems = new BackgroundItems(this);
	_camera = new Camera(this);
	_controls = new Controls(this);
	_talkItems = new TalkItems(this);
	_threads = new ThreadList(this);

	_scriptOpcodes = new ScriptOpcodes_Duckman(this);
	_stack = new ScriptStack();
	
	// TODO Move to own class
	_resGetCtr = 0;
	_unpauseControlActorFlag = false;
	_lastUpdateTime = 0;

	_pauseCtr = 0;
	_doScriptThreadInit = false;
	_field8 = 1;
	_fieldA = 0;
	_fieldE = 240;
	
	_globalSceneId = 0x00010003;

	initInventory();
	initSpecialCode();
	setDefaultTextCoords();
	initCursor();
	initActiveScenes();

	_resSys->loadResource(0x120001, 0x00010001, 0);
	_resSys->loadResource(0x120002, 0x00010001, 0);
	_resSys->loadResource(0x120003, 0x00010001, 0);

	_resSys->loadResource(0x000D0001, 0x00010001, 0);
	startScriptThread(0x00020004, 0);
	_doScriptThreadInit = true;

	while (!shouldQuit()) {
		_threads->updateThreads();
		updateActors();
		updateSequences();
		updateGraphics();
		_screen->updateSprites();
		_screen->updatePalette();
		_system->updateScreen();
		updateEvents();
		_system->delayMillis(10);
	}

	delete _stack;
	delete _scriptOpcodes;

	delete _threads;
	delete _talkItems;
	delete _controls;
	delete _camera;
	delete _backgroundItems;
	delete _actorItems;
	delete _input;
	delete _screenText;
	delete _screen;
	delete _resSys;
	delete _dict;
	
	debug("Ok");
	
	return Common::kNoError;
}

bool IllusionsEngine_Duckman::hasFeature(EngineFeature f) const {
	return
		false;
		/*
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
		*/
}

void IllusionsEngine_Duckman::setDefaultTextCoords() {
	WidthHeight dimensions;
	dimensions._width = 300;
	dimensions._height = 32;
	Common::Point pt(160, 176);
	setDefaultTextDimensions(dimensions);
	setDefaultTextPosition(pt);
}

void IllusionsEngine_Duckman::loadSpecialCode(uint32 resId) {
//TODO	_specialCode = new BbdouSpecialCode(this);
//TODO	_specialCode->init();
}

void IllusionsEngine_Duckman::unloadSpecialCode(uint32 resId) {
//TODO	delete _specialCode;
//TODO	_specialCode = 0;
}

void IllusionsEngine_Duckman::notifyThreadId(uint32 &threadId) {
	if (threadId) {
		uint32 tempThreadId = threadId;
		threadId = 0;
		_threads->notifyId(tempThreadId);
	}
}

bool IllusionsEngine_Duckman::testMainActorFastWalk(Control *control) {
	return
		control->_objectId == _scriptResource->getMainActorObjectId() &&
		_input->pollButton(0x20);
}

bool IllusionsEngine_Duckman::testMainActorCollision(Control *control) {
	// TODO
	return false;
}

Control *IllusionsEngine_Duckman::getObjectControl(uint32 objectId) {
	return _dict->getObjectControl(objectId);
}

Common::Point IllusionsEngine_Duckman::getNamedPointPosition(uint32 namedPointId) {
	Common::Point pt;
	Common::Point currPan = _camera->getCurrentPan();
	if (_backgroundItems->findActiveBackgroundNamedPoint(namedPointId, pt)) {
		return pt;
	} else if (namedPointId - 0x00070001 > 209) {
		if (_controls->findNamedPoint(namedPointId, pt)) {
			return pt;
		} else {
			return currPan;
		}
	} else {
		// TODO
		debug("getNamedPointPosition(%08X) UNKNOWN", namedPointId);
		return Common::Point(0, 0);
	}
}

uint32 IllusionsEngine_Duckman::getPriorityFromBase(int16 priority) {
	return 32000000 * priority;
}

uint32 IllusionsEngine_Duckman::getCurrentScene() {
	return _activeScenes[_activeScenesCount];
}

uint32 IllusionsEngine_Duckman::getPrevScene() {
	uint index = _activeScenesCount - 1;
	if (_activeScenesCount == 1)
		index = 5;
	return _activeScenes[index];
}

bool IllusionsEngine_Duckman::isCursorObject(uint32 actorTypeId, uint32 objectId) {
	return actorTypeId == 0x50001;
}

void IllusionsEngine_Duckman::setCursorControlRoutine(Control *control) {
	control->_actor->setControlRoutine(new Common::Functor2Mem<Control*, uint32, void, IllusionsEngine_Duckman>
		(this, &IllusionsEngine_Duckman::cursorControlRoutine));
}

void IllusionsEngine_Duckman::placeCursorControl(Control *control, uint32 sequenceId) {
	_cursor._gameState = 2;
	_cursor._control = control;
	_cursor._actorIndex = 1;
	_cursor._savedActorIndex = 1;
	_cursor._currOverlappedControl = 0;
	_cursor._sequenceId1 = sequenceId;
	_cursor._field14[0] = true;
	_cursor._field14[1] = true;
	_cursor._field14[2] = false;
	_cursor._field14[3] = false;
	_cursor._field14[4] = false;
	_cursor._field14[5] = false;
	_cursor._field14[9] = false;
	_cursor._field14[10] = false;
	_cursor._field14[11] = false;
	_cursor._field14[12] = false;
	_cursor._field14[6] = _cursor._sequenceId2 != 0 && _cursor._objectId != 0;
	_cursor._field14[7] = false;
	_cursor._field14[8] = false;
	_cursor._op113_choiceOfsPtr = 0;
	_cursor._notifyThreadId30 = 0;
	_cursor._dialogItemsCount = 0;
	_cursor._overlappedObjectId = 0;
	_cursor._field40 = 0;
	control->_flags |= 8;
	setCursorActorIndex(_cursor._actorIndex, 1, 0);
	// TODO Input_setMousePos(cursorControl->actor->position);
	// TODO
	//control->_actor->_actorIndex = 2;
	// TODO _cursor->place(control, sequenceId);
}

void IllusionsEngine_Duckman::setCursorControl(Control *control) {
	_cursor._control = control;
}

void IllusionsEngine_Duckman::showCursor() {
	// TODO
}

void IllusionsEngine_Duckman::hideCursor() {
	// TODO
}

void IllusionsEngine_Duckman::initCursor() {
	_cursor._gameState = 1;
	_cursor._control = 0;
	_cursor._position.x = 160;
	_cursor._position.y = 100;
	_cursor._objectId = 0;
	_cursor._actorIndex = 1;
	_cursor._savedActorIndex = 1;
	_cursor._currOverlappedControl = 0;
	_cursor._sequenceId1 = 0;
	_cursor._sequenceId2 = 0;
	_cursor._field14[0] = true;
	_cursor._field14[1] = true;
	_cursor._field14[2] = false;
	_cursor._field14[3] = false;
	_cursor._field14[4] = false;
	_cursor._field14[5] = false;
	_cursor._field14[6] = false;
	_cursor._field14[7] = false;
	_cursor._field14[8] = false;
	_cursor._field14[9] = false;
	_cursor._field14[10] = false;
	_cursor._field14[11] = false;
	_cursor._field14[12] = false;
	_cursor._op113_choiceOfsPtr = 0;
	_cursor._notifyThreadId30 = 0;
	_cursor._dialogItemsCount = 0;
	_cursor._overlappedObjectId = 0;
	_cursor._field40 = 0;
}

void IllusionsEngine_Duckman::setCursorActorIndex(int actorIndex, int a, int b) {
	static int kCursorMap[13][2][2] = {
		{{ 1,  2}, { 0,  0}},
		{{ 3,  4}, { 0,  0}},
		{{ 5,  6}, {13, 14}},
		{{ 7,  8}, { 0,  0}},
		{{ 9, 10}, { 0,  0}},
		{{11, 12}, { 0,  0}},
		{{ 1,  2}, { 0,  0}},
		{{ 0,  0}, { 0,  0}},
		{{ 0,  0}, { 0,  0}},
		{{15, 16}, { 0,  0}},
		{{17, 18}, { 0,  0}},
		{{19, 20}, { 0,  0}},
		{{21, 22}, { 0,  0}}
	};
	_cursor._control->_actor->_actorIndex = kCursorMap[actorIndex - 1][b][a - 1];
}

void IllusionsEngine_Duckman::enableCursorVerb(int verbNum) {
	if (verbNum != 7 || _cursor._sequenceId2)
		_cursor._field14[verbNum - 1] = true;
}

void IllusionsEngine_Duckman::disableCursorVerb(int verbNum) {
	_cursor._field14[verbNum - 1] = false;
	if (_cursor._actorIndex == verbNum) {
		_cursor._actorIndex = getCursorActorIndex();
		setCursorActorIndex(_cursor._actorIndex, 1, 0);
		startCursorSequence();
		_cursor._currOverlappedControl = 0;
	}
}

void IllusionsEngine_Duckman::setCursorHandMode(int mode) {
	if (mode == 1) {
		enableCursorVerb(4);
		disableCursorVerb(1);
		disableCursorVerb(2);
		disableCursorVerb(7);
		_cursor._actorIndex = 4;
	} else {
		enableCursorVerb(1);
		enableCursorVerb(2);
		enableCursorVerb(7);
		disableCursorVerb(4);
		_cursor._actorIndex = 1;
	}
	_cursor._control->startSequenceActor(_cursor._sequenceId1, 2, 0);
	if (_cursor._currOverlappedControl)
		setCursorActorIndex(_cursor._actorIndex, 2, 0);
	else
		setCursorActorIndex(_cursor._actorIndex, 1, 0);
}

void IllusionsEngine_Duckman::setCursorInventoryMode(int mode, int value) {
	_cursor._control = _cursor._control;
	if (mode == 1) {
		_savedInventoryActorIndex = _cursor._actorIndex;
		if (_cursor._actorIndex == 3 || _cursor._actorIndex == 10 || _cursor._actorIndex == 11 || _cursor._actorIndex == 12 || _cursor._actorIndex == 13) {
			_cursor._savedActorIndex = _cursor._savedActorIndex;
			if (_cursor._savedActorIndex == 1 || _cursor._savedActorIndex == 2 || _cursor._savedActorIndex == 7)
				_savedInventoryActorIndex = _cursor._savedActorIndex;
			else
				_savedInventoryActorIndex = 0;
		}
		if (value == 1 && _cursor._objectId && _savedInventoryActorIndex != 7) {
			_cursor._actorIndex = 7;
			stopCursorHoldingObject();
			_cursor._actorIndex = _savedInventoryActorIndex;
		}
	} else if (mode == 2) {
		if (_savedInventoryActorIndex)
			_cursor._actorIndex = _savedInventoryActorIndex;
		else
			_cursor._actorIndex = 1;
		if (_cursor._actorIndex == 7)
			_cursor._control->startSequenceActor(_cursor._sequenceId2, 2, 0);
		else
			_cursor._control->startSequenceActor(_cursor._sequenceId1, 2, 0);
		if (_cursor._currOverlappedControl)
			setCursorActorIndex(_cursor._actorIndex, 2, 0);
		else
			setCursorActorIndex(_cursor._actorIndex, 1, 0);
		_savedInventoryActorIndex = 0;
	}
}

void IllusionsEngine_Duckman::startCursorHoldingObject(uint32 objectId, uint32 sequenceId) {
	_cursor._objectId = objectId;
	_cursor._sequenceId2 = sequenceId;
	_cursor._actorIndex = 7;
	_cursor._savedActorIndex = 7;
	_cursor._field14[_cursor._actorIndex - 1] = true;
	_cursor._control->startSequenceActor(sequenceId, 2, 0);
	setCursorActorIndex(_cursor._actorIndex, 1, 0);
	_cursor._currOverlappedControl = 0;
}

void IllusionsEngine_Duckman::stopCursorHoldingObject() {
	_cursor._field14[6] = false;
	_cursor._objectId = 0;
	_cursor._sequenceId2 = 0;
	if (_cursor._actorIndex == 7) {
		_cursor._actorIndex = getCursorActorIndex();
		_cursor._control->startSequenceActor(_cursor._sequenceId1, 2, 0);
		if (_cursor._currOverlappedControl)
			setCursorActorIndex(_cursor._actorIndex, 2, 0);
		else
			setCursorActorIndex(_cursor._actorIndex, 1, 0);
	}
}

void IllusionsEngine_Duckman::cursorControlRoutine(Control *control, uint32 deltaTime) {
	control->_actor->_seqCodeValue1 = 100 * deltaTime;
	if (control->_actor->_flags & 1) {
		switch (_cursor._gameState) {
		case 2:
			updateGameState2();
			break;
		case 3:
			updateDialogState();
			break;
		case 4:
			// TODO ShellMgr_update(_cursor._control);
			break;
		}
	}
}

void IllusionsEngine_Duckman::startScriptThreadSimple(uint32 threadId, uint32 callingThreadId) {
	startScriptThread(threadId, callingThreadId);
}

void IllusionsEngine_Duckman::startScriptThread(uint32 threadId, uint32 callingThreadId) {
	debug(2, "Starting script thread %08X", threadId);
	byte *scriptCodeIp = _scriptResource->getThreadCode(threadId);
	newScriptThread(threadId, callingThreadId, 0, scriptCodeIp);
}

uint32 IllusionsEngine_Duckman::startAbortableTimerThread(uint32 duration, uint32 threadId) {
	return newTimerThread(duration, threadId, true);
}

uint32 IllusionsEngine_Duckman::startTimerThread(uint32 duration, uint32 threadId) {
	return newTimerThread(duration, threadId, false);
}

uint32 IllusionsEngine_Duckman::startAbortableThread(byte *scriptCodeIp1, byte *scriptCodeIp2, uint32 callingThreadId) {
	uint32 tempThreadId = newTempThreadId();
	debug(2, "Starting abortable thread %08X", tempThreadId);
	uint32 scriptThreadId = startTempScriptThread(scriptCodeIp1, tempThreadId, 0, 0, 0);
	AbortableThread *abortableThread = new AbortableThread(this, tempThreadId, callingThreadId, 0,
		scriptThreadId, scriptCodeIp2);
	_threads->startThread(abortableThread);
	return tempThreadId;
}

uint32 IllusionsEngine_Duckman::startTalkThread(uint32 objectId, uint32 talkId, uint32 sequenceId1,
	uint32 sequenceId2, uint32 callingThreadId) {
	debug(2, "Starting talk thread");
	uint32 tempThreadId = newTempThreadId();
	TalkThread_Duckman *talkThread = new TalkThread_Duckman(this, tempThreadId, callingThreadId, 0,
		objectId, talkId, sequenceId1, sequenceId2);
	_threads->startThread(talkThread);
	return tempThreadId;
}

uint32 IllusionsEngine_Duckman::startTempScriptThread(byte *scriptCodeIp, uint32 callingThreadId,
	uint32 value8, uint32 valueC, uint32 value10) {
	uint32 tempThreadId = newTempThreadId();
	debug(2, "Starting temp script thread %08X", tempThreadId);
	newScriptThread(tempThreadId, callingThreadId, 0, scriptCodeIp);
	return tempThreadId;
}

void IllusionsEngine_Duckman::newScriptThread(uint32 threadId, uint32 callingThreadId, uint notifyFlags,
	byte *scriptCodeIp) {
	ScriptThread *scriptThread = new ScriptThread(this, threadId, callingThreadId, notifyFlags,
		scriptCodeIp, 0, 0, 0);
	_threads->startThread(scriptThread);
	if (_pauseCtr > 0)
		scriptThread->pause();
	if (_doScriptThreadInit) {
		int updateResult = kTSRun;
		while (scriptThread->_pauseCtr <= 0 && updateResult != kTSTerminate && updateResult != kTSYield)
			updateResult = scriptThread->update();
	}
}

uint32 IllusionsEngine_Duckman::newTimerThread(uint32 duration, uint32 callingThreadId, bool isAbortable) {
	uint32 tempThreadId = newTempThreadId();
	TimerThread *timerThread = new TimerThread(this, tempThreadId, callingThreadId, 0,
		duration, isAbortable);
	_threads->startThread(timerThread);
	return tempThreadId;
}

uint32 IllusionsEngine_Duckman::newTempThreadId() {
	uint32 threadId = _nextTempThreadId + 2 * _scriptResource->_codeCount;
	if (threadId > 65535) {
		_nextTempThreadId = 0;
		threadId = 2 * _scriptResource->_codeCount;
	}
	++_nextTempThreadId;
	return 0x00020000 | threadId;
}

void IllusionsEngine_Duckman::initActiveScenes() {
	_activeScenesCount = 0;
	_activeScenes[0] = 0xEFEF;
	pushActiveScene(0x10000);
}

void IllusionsEngine_Duckman::pushActiveScene(uint32 sceneId) {
	++_activeScenesCount;
	if (_activeScenesCount >= 6)
		_activeScenesCount = 1;
	_activeScenes[_activeScenesCount] = sceneId;
}

void IllusionsEngine_Duckman::popActiveScene() {
	--_activeScenesCount;
	if (_activeScenesCount == 0)
		_activeScenesCount = 5;
}

bool IllusionsEngine_Duckman::loadScene(uint32 sceneId) {
	ProgInfo *progInfo = _scriptResource->getProgInfo(sceneId & 0xFFFF);
	if (!progInfo)
		return false;
	pushActiveScene(sceneId);
	uint resourcesCount;
	uint32 *resources;
	progInfo->getResources(resourcesCount, resources);
	for (uint i = 0; i < resourcesCount; ++i)
		_resSys->loadResource(resources[i], sceneId, 0);
	return true;
}

bool IllusionsEngine_Duckman::enterScene(uint32 sceneId, uint32 threadId) {
	if (loadScene(sceneId)) {
		if (threadId)
			startScriptThread(threadId, 0);
		return true;
	}
	// TODO startScriptThread2(0x10002, 0x20001, 0);
	return false;
}

void IllusionsEngine_Duckman::exitScene() {
	popActiveScene();
}

bool IllusionsEngine_Duckman::changeScene(uint32 sceneId, uint32 threadId, uint32 callerThreadId) {
	uint32 currSceneId = getCurrentScene();
	if (currSceneId != 0x10003)
		dumpCurrSceneFiles(currSceneId, callerThreadId);
	_threads->terminateThreads(callerThreadId);
	_controls->destroyControls();
	_resSys->unloadSceneResources(0x10003, 0x10001);
	if (enterScene(sceneId, threadId)) {
		// TODO GameStates_writeStates(sceneId, threadId);
		return true;
	}
	return false;
}

void IllusionsEngine_Duckman::enterPause(uint32 sceneId, uint32 threadId) {
	_threads->suspendThreads(threadId);
	_controls->pauseControls();
	_actorItems->pauseByTag(sceneId);
	_backgroundItems->pauseByTag(sceneId);
}

void IllusionsEngine_Duckman::leavePause(uint32 sceneId, uint32 threadId) {
	_backgroundItems->unpauseByTag(sceneId);
	_actorItems->unpauseByTag(sceneId);
	_controls->unpauseControls();
	_threads->notifyThreads(threadId);
}

void IllusionsEngine_Duckman::dumpActiveScenes(uint32 sceneId, uint32 threadId) {
	// TODO
}

void IllusionsEngine_Duckman::dumpCurrSceneFiles(uint32 sceneId, uint32 threadId) {
	// TODO UpdateFunctions_disableByTag(sceneId);
	_threads->terminateActiveThreads(threadId);
	_threads->terminateThreadsByTag(sceneId, threadId);
	_controls->destroyActiveControls();
	_resSys->unloadResourcesByTag(sceneId);
}

void IllusionsEngine_Duckman::setSceneIdThreadId(uint32 theSceneId, uint32 theThreadId) {
	_theSceneId = theSceneId;
	_theThreadId = theThreadId;
}

bool IllusionsEngine_Duckman::findTriggerCause(uint32 sceneId, uint32 verbId, uint32 objectId2, uint32 objectId, uint32 &codeOffs) {
	ProgInfo *progInfo = _scriptResource->getProgInfo(sceneId & 0xFFFF);
	if (progInfo)
		return progInfo->findTriggerCause(verbId, objectId2, objectId, codeOffs);
	return false;
}

void IllusionsEngine_Duckman::reset() {
	_scriptResource->_blockCounters.clear();
	_scriptResource->_properties.clear();
	// TODO script_sub_417FF0(1, 0);
}

uint32 IllusionsEngine_Duckman::getObjectActorTypeId(uint32 objectId) {
	return _scriptResource->getObjectActorTypeId(objectId);
}

Common::Point IllusionsEngine_Duckman::convertMousePos(Common::Point mousePos) {
	Common::Point screenOffsPt = _camera->getScreenOffset();
	mousePos.x += screenOffsPt.x;
	mousePos.y += screenOffsPt.y;
	return mousePos;
}

void IllusionsEngine_Duckman::startCursorSequence() {
	// NOTE Calls to startCursorSequence were put after calls to setCursorActorIndex
	// to make the cursor switch more immediate. In the original these calls are swapped.
	if (_cursor._actorIndex == 7)
		_cursor._control->startSequenceActor(_cursor._sequenceId2, 2, 0);
	else
		_cursor._control->startSequenceActor(_cursor._sequenceId1, 2, 0);
}

int IllusionsEngine_Duckman::getCursorActorIndex() {
	int result = _cursor._actorIndex;
	do {
		++result;
		if (result > 13)
			result = 1;
	} while (!_cursor._field14[result - 1]);
	return result;
}

void IllusionsEngine_Duckman::updateGameState2() {
	Common::Point cursorPos = _input->getCursorPosition();
	Common::Point convMousePos = convertMousePos(cursorPos);
	int trackCursorIndex = -1;
	bool foundOverlapped;
	Control *overlappedControl;

	_cursor._control->_actor->_position = cursorPos;

	foundOverlapped = _controls->getOverlappedObject(_cursor._control, convMousePos, &overlappedControl, 0);

	if (cursorPos.y < 8 && !_camera->isAtPanLimit(1)) {
		trackCursorIndex = 10;
	} else if (cursorPos.y >= 192 && !_camera->isAtPanLimit(2)) {
		trackCursorIndex = 11;
	} else if (cursorPos.x < 8 && !_camera->isAtPanLimit(3)) {
		trackCursorIndex = 12;
	} else if (cursorPos.x >= 312 && !_camera->isAtPanLimit(4)) {
		trackCursorIndex = 13;
	} else if (_cursor._actorIndex == 10 || _cursor._actorIndex == 11 || _cursor._actorIndex == 12 || _cursor._actorIndex == 13) {
		_cursor._actorIndex = _cursor._savedActorIndex;
		if (_cursor._currOverlappedControl)
			setCursorActorIndex(_cursor._actorIndex, 2, 0);
		else
			setCursorActorIndex(_cursor._actorIndex, 1, 0);
		startCursorSequence();
	}

	if (trackCursorIndex >= 0) {
		if (_cursor._actorIndex != 10 && _cursor._actorIndex != 11 && _cursor._actorIndex != 12 && _cursor._actorIndex != 13 && _cursor._actorIndex != 3)
			_cursor._savedActorIndex = _cursor._actorIndex;
		if (_cursor._actorIndex != trackCursorIndex) {
			_cursor._actorIndex = trackCursorIndex;
			setCursorActorIndex(_cursor._actorIndex, 1, 0);
			startCursorSequence();
		}
		_cursor._currOverlappedControl = 0;
		foundOverlapped = false;
	}

	if (foundOverlapped) {
		if (_cursor._currOverlappedControl != overlappedControl) {
			int cursorValue2 = 0;
			if (overlappedControl->_flags & 2) {
				if (_cursor._actorIndex != 3) {
					_cursor._savedActorIndex = _cursor._actorIndex;
					_cursor._actorIndex = 3;
				}
				if (overlappedControl->_flags & 0x40)
					cursorValue2 = 1;
			} else if (_cursor._actorIndex == 3) {
				_cursor._actorIndex = _cursor._savedActorIndex;
			}
			setCursorActorIndex(_cursor._actorIndex, 2, cursorValue2);
			startCursorSequence();
			_cursor._currOverlappedControl = overlappedControl;
		}
	} else if (_cursor._currOverlappedControl) {
		if (_cursor._actorIndex == 3)
			_cursor._actorIndex = _cursor._savedActorIndex;
		setCursorActorIndex(_cursor._actorIndex, 1, 0);
		startCursorSequence();
		_cursor._currOverlappedControl = 0;
	}

	if (_input->pollButton(1)) {
		if (_cursor._currOverlappedControl) {
			runTriggerCause(_cursor._actorIndex, _cursor._objectId, _cursor._currOverlappedControl->_objectId);
		} else {
			_cursor._position = convertMousePos(_cursor._control->_actor->_position);
			// TODO clipMousePos(&_cursor._position);
			if (_cursor._actorIndex == 10 || _cursor._actorIndex == 11 || _cursor._actorIndex == 12 || _cursor._actorIndex == 13)
				runTriggerCause(1, _cursor._objectId, 0x40003);
			else
				runTriggerCause(_cursor._actorIndex, _cursor._objectId, 0x40003);
		}
	} else if (_input->pollButton(2)) {
		if (_cursor._actorIndex != 3 && _cursor._actorIndex != 10 && _cursor._actorIndex != 11 && _cursor._actorIndex != 12 && _cursor._actorIndex != 13) {
			int newActorIndex = getCursorActorIndex();
			if (newActorIndex != _cursor._actorIndex) {
				_cursor._actorIndex = newActorIndex;
				if (_cursor._currOverlappedControl)
					setCursorActorIndex(_cursor._actorIndex, 2, 0);
				else
					setCursorActorIndex(_cursor._actorIndex, 1, 0);
				startCursorSequence();
			}
		}
	} else if (_input->pollButton(8)) {
		if (_cursor._field14[0] == 1) {
			runTriggerCause(1, 0, _scriptResource->getMainActorObjectId());
		} else if (_cursor._field14[1] == 1) {
			runTriggerCause(2, 0, _scriptResource->getMainActorObjectId());
		}
	}

}

void IllusionsEngine_Duckman::playSoundEffect(int index) {
	// TODO
}

bool IllusionsEngine_Duckman::getTriggerCause(uint32 verbId, uint32 objectId2, uint32 objectId, uint32 &outThreadId) {
	ProgInfo *progInfo = _scriptResource->getProgInfo(getCurrentScene() & 0xFFFF);
	bool found =
		progInfo->findTriggerCause(verbId, objectId2, objectId, outThreadId) ||
		progInfo->findTriggerCause(verbId, objectId2, 0x40001, outThreadId);
	if (!found) {
		progInfo = _scriptResource->getProgInfo(3);
		found =
			progInfo->findTriggerCause(verbId, objectId2, objectId, outThreadId) ||
			progInfo->findTriggerCause(verbId, objectId2, 0x40001, outThreadId);
	}
	return found;
}

uint32 IllusionsEngine_Duckman::runTriggerCause(uint32 verbId, uint32 objectId2, uint32 objectId) {
	// TODO
	debug("runTriggerCause(%08X, %08X, %08X)", verbId, objectId2, objectId);
	uint32 triggerThreadId;

	if (!getTriggerCause(verbId, objectId2, objectId, triggerThreadId))
		return 0;

	bool flag = false;
	if (_scriptResource->_properties.get(0x000E003C)) {
		if (verbId == 7 && objectId == 0x40003) {
			playSoundEffect(7);
			flag = true;
		} else if (objectId == 0x40003) {
			playSoundEffect(14);
			flag = true;
		} else if (verbId == 3) {
			playSoundEffect(16);
			flag = true;
		} else if (verbId == 2) {
			flag = true;
		}
	}

	if (!flag) {
		if (objectId == 0x40003) {
			playSoundEffect(14);
		} else if ((verbId == 1 || verbId == 2) && _scriptResource->getMainActorObjectId() == objectId) {
			playSoundEffect(15);
		} else if (verbId == 7 && _scriptResource->getMainActorObjectId() == objectId) {
			playSoundEffect(15);
		} else if (verbId == 1) {
			playSoundEffect(1);
		} else if (verbId == 2) {
			playSoundEffect(2);
		} else if (verbId == 3) {
			playSoundEffect(3);
		} else if (verbId == 4 || verbId == 7) {
			playSoundEffect(4);
		} else if (verbId == 9) {
			playSoundEffect(5);
		}
	}

	uint32 tempThreadId = newTempThreadId();
	debug(2, "Starting cause thread %08X", tempThreadId);
	CauseThread_Duckman *causeThread = new CauseThread_Duckman(this, tempThreadId, 0, 0,
		triggerThreadId);
	_threads->startThread(causeThread);

	return tempThreadId;
}

void IllusionsEngine_Duckman::addDialogItem(int16 choiceJumpOffs, uint32 sequenceId) {
	DialogItem dialogItem;
	dialogItem._choiceJumpOffs = choiceJumpOffs;
	dialogItem._sequenceId = sequenceId;
	_dialogItems.push_back(dialogItem);
}

void IllusionsEngine_Duckman::startDialog(int16 *choiceOfsPtr, uint32 actorTypeId, uint32 callerThreadId) {
	static const uint32 kDialogSequenceIds[] = {
		0,
		0x6049C, 0x6049C, 0x6047A, 0x6049D,
		0x60479, 0x6049E, 0x6049F, 0x60468
	};
	if (_dialogItems.size() == 1) {
		*choiceOfsPtr = _dialogItems[0]._choiceJumpOffs;
		notifyThreadId(callerThreadId);
	} else {
		if (!_cursor._control) {
			Common::Point pos = getNamedPointPosition(0x70001);
			_controls->placeActor(0x50001, pos, 0x60001, 0x40004, 0);
			_cursor._control = _dict->getObjectControl(0x40004);
		}
		_cursor._control->appearActor();
		setCursorActorIndex(6, 1, 0);

		_cursor._gameState = 3;
		_cursor._notifyThreadId30 = callerThreadId;
		_cursor._dialogItemsCount = 0;
		_cursor._overlappedObjectId = 0;
		_cursor._op113_choiceOfsPtr = choiceOfsPtr;
		_cursor._currOverlappedControl = 0;

		/* TODO?
		if (!_input->getCursorMouseMode())
			_input->setMousePos((Point)0xBC0014);
		*/

		_cursor._dialogItemsCount = _dialogItems.size();
		Common::Point placePt(20, 188);

		for (uint i = 1; i <= _dialogItems.size(); ++i) {
			DialogItem &dialogItem = _dialogItems[_dialogItems.size() - i];
			_controls->placeDialogItem(i + 1, actorTypeId, dialogItem._sequenceId, placePt, dialogItem._choiceJumpOffs);
			placePt.x += 40;
		}

		Common::Point placePt2 = getNamedPointPosition(0x700C3);
		_controls->placeActor(0x5006E, placePt2, kDialogSequenceIds[_dialogItems.size()], 0x40148, 0);
		Control *control = _dict->getObjectControl(0x40148);
		control->_flags |= 8;
		playSoundEffect(8);
	}

	_dialogItems.clear();

}

void IllusionsEngine_Duckman::updateDialogState() {
	Common::Point mousePos = _input->getCursorPosition();
	// TODO Handle keyboard input
	_cursor._control->_actor->_position = mousePos;
	mousePos = convertMousePos(mousePos);

	Control *currOverlappedControl = _cursor._currOverlappedControl;
	Control *newOverlappedControl;
	
	if (_controls->getDialogItemAtPos(_cursor._control, mousePos, &newOverlappedControl)) {
		if (currOverlappedControl != newOverlappedControl) {
			newOverlappedControl->setActorIndex(2);
			newOverlappedControl->startSequenceActor(newOverlappedControl->_actor->_sequenceId, 2, 0);
			if (currOverlappedControl) {
				currOverlappedControl->setActorIndex(1);
				currOverlappedControl->startSequenceActor(currOverlappedControl->_actor->_sequenceId, 2, 0);
			}
			playSoundEffect(10);
			startCursorSequence();
			setCursorActorIndex(6, 2, 0);
			_cursor._currOverlappedControl = newOverlappedControl;
			_cursor._overlappedObjectId = newOverlappedControl->_objectId;
		}
	} else if (currOverlappedControl) {
		currOverlappedControl->setActorIndex(1);
		currOverlappedControl->startSequenceActor(currOverlappedControl->_actor->_sequenceId, 2, 0);
		playSoundEffect(10);
		_cursor._currOverlappedControl = 0;
		_cursor._overlappedObjectId = 0;
		startCursorSequence();
		setCursorActorIndex(6, 1, 0);
	}

	if (_input->pollButton(1)) {
		if (_cursor._currOverlappedControl) {
			playSoundEffect(9);
			*_cursor._op113_choiceOfsPtr = _cursor._currOverlappedControl->_actor->_choiceJumpOffs;
			_controls->destroyDialogItems();
			Control *control = _dict->getObjectControl(0x40148);
			_controls->destroyControl(control);
			debug("_cursor._notifyThreadId30: %08X", _cursor._notifyThreadId30);
			notifyThreadId(_cursor._notifyThreadId30);
			_cursor._notifyThreadId30 = 0;
			_cursor._gameState = 2;
			_cursor._dialogItemsCount = 0;
			_cursor._overlappedObjectId = 0;
			_cursor._op113_choiceOfsPtr = 0;
			_cursor._control->disappearActor();
		}
	}

}

void IllusionsEngine_Duckman::initInventory() {
	_inventorySlots.push_back(DMInventorySlot( 64,  52));
	_inventorySlots.push_back(DMInventorySlot(112,  52));
	_inventorySlots.push_back(DMInventorySlot(160,  52));
	_inventorySlots.push_back(DMInventorySlot(208,  52));
	_inventorySlots.push_back(DMInventorySlot(255,  52));
	_inventorySlots.push_back(DMInventorySlot( 64,  84));
	_inventorySlots.push_back(DMInventorySlot(112,  84));
	_inventorySlots.push_back(DMInventorySlot(160,  84));
	_inventorySlots.push_back(DMInventorySlot(208,  84));
	_inventorySlots.push_back(DMInventorySlot(255,  84));
	_inventorySlots.push_back(DMInventorySlot( 64, 116));
	_inventorySlots.push_back(DMInventorySlot(112, 116));
	_inventorySlots.push_back(DMInventorySlot(160, 116));
	_inventorySlots.push_back(DMInventorySlot(208, 116));
	_inventorySlots.push_back(DMInventorySlot(255, 116));
	_inventorySlots.push_back(DMInventorySlot( 64, 148));
	_inventorySlots.push_back(DMInventorySlot(112, 148));
	_inventorySlots.push_back(DMInventorySlot(160, 148));
	_inventorySlots.push_back(DMInventorySlot(208, 148));
	_inventorySlots.push_back(DMInventorySlot(255, 148));
	_inventoyItems.push_back(DMInventoryItem(0x40011, 0xE005B));
	_inventoyItems.push_back(DMInventoryItem(0x40099, 0xE001B));
	_inventoyItems.push_back(DMInventoryItem(0x4000F, 0xE000C));
	_inventoyItems.push_back(DMInventoryItem(0x40042, 0xE0012));
	_inventoyItems.push_back(DMInventoryItem(0x40044, 0xE000F));
	_inventoyItems.push_back(DMInventoryItem(0x40029, 0xE000D));
	_inventoyItems.push_back(DMInventoryItem(0x400A7, 0xE005D));
	_inventoyItems.push_back(DMInventoryItem(0x40096, 0xE001C));
	_inventoyItems.push_back(DMInventoryItem(0x40077, 0xE0010));
	_inventoyItems.push_back(DMInventoryItem(0x4008A, 0xE0033));
	_inventoyItems.push_back(DMInventoryItem(0x4004B, 0xE0045));
	_inventoyItems.push_back(DMInventoryItem(0x40054, 0xE0021));
	_inventoyItems.push_back(DMInventoryItem(0x400C6, 0xE005A));
	_inventoyItems.push_back(DMInventoryItem(0x4000B, 0xE005E));
	_inventoyItems.push_back(DMInventoryItem(0x4005F, 0xE0016));
	_inventoyItems.push_back(DMInventoryItem(0x40072, 0xE0017));
	_inventoyItems.push_back(DMInventoryItem(0x400AA, 0xE005F));
	_inventoyItems.push_back(DMInventoryItem(0x400B8, 0xE0050));
	_inventoyItems.push_back(DMInventoryItem(0x4001F, 0xE001A));
	_inventoyItems.push_back(DMInventoryItem(0x40095, 0xE0060));
	_inventoyItems.push_back(DMInventoryItem(0x40041, 0xE0053));
	_savedInventoryActorIndex = 0;
}

void IllusionsEngine_Duckman::openInventory() {

	for (uint i = 0; i < _inventorySlots.size(); ++i) {
		DMInventorySlot *inventorySlot = &_inventorySlots[i];
		if (inventorySlot->_objectId) {
			DMInventoryItem *inventoryItem = findInventoryItem(inventorySlot->_objectId);
			if (!_scriptResource->_properties.get(inventoryItem->_propertyId))
				inventorySlot->_objectId = 0;
		}
	}

	for (uint i = 0; i < _inventoyItems.size(); ++i) {
		DMInventoryItem *inventoryItem = &_inventoyItems[i];
		if (_scriptResource->_properties.get(inventoryItem->_propertyId)) {
			DMInventorySlot *inventorySlot = findInventorySlot(inventoryItem->_objectId);
			if (inventorySlot) {
				Control *control = getObjectControl(inventoryItem->_objectId);
				control->setActorPosition(inventorySlot->_position);
				control->appearActor();
			} else {
				addInventoryItem(inventoryItem->_objectId);
			}
		}
	}

}

void IllusionsEngine_Duckman::addInventoryItem(uint32 objectId) {
	DMInventorySlot *DMInventorySlot = findInventorySlot(0);
	DMInventorySlot->_objectId = objectId;
	Control *control = getObjectControl(objectId);
	control->setActorPosition(DMInventorySlot->_position);
	control->appearActor();
}

DMInventorySlot *IllusionsEngine_Duckman::findInventorySlot(uint32 objectId) {
	for (uint i = 0; i < _inventorySlots.size(); ++i)
		if (_inventorySlots[i]._objectId == objectId)
			return &_inventorySlots[i];
	return 0;
}

DMInventoryItem *IllusionsEngine_Duckman::findInventoryItem(uint32 objectId) {
	for (uint i = 0; i < _inventoyItems.size(); ++i)
		if (_inventoyItems[i]._objectId == objectId)
			return &_inventoyItems[i];
	return 0;
}

// Special code

typedef Common::Functor1Mem<OpCall&, void, IllusionsEngine_Duckman> SpecialCodeFunctionDM;
#define SPECIAL(id, func) _specialCodeMap[id] = new SpecialCodeFunctionDM(this, &IllusionsEngine_Duckman::func);

void IllusionsEngine_Duckman::initSpecialCode() {
	SPECIAL(0x00160002, spcSetCursorHandMode);
	SPECIAL(0x00160005, spcOpenInventory);
	SPECIAL(0x00160010, spcCenterNewspaper);
	SPECIAL(0x00160014, spcUpdateObject272Sequence);
	SPECIAL(0x0016001C, spcSetCursorInventoryMode);
}

void IllusionsEngine_Duckman::runSpecialCode(uint32 specialCodeId, OpCall &opCall) {
	SpecialCodeMapIterator it = _specialCodeMap.find(specialCodeId);
	if (it != _specialCodeMap.end()) {
		(*(*it)._value)(opCall);
	} else {
		debug("IllusionsEngine_Duckman::runSpecialCode() Unimplemented special code %08X", specialCodeId);
		notifyThreadId(opCall._threadId);
	}
}

void IllusionsEngine_Duckman::spcSetCursorHandMode(OpCall &opCall) {
	ARG_BYTE(mode);
	setCursorHandMode(mode);
	notifyThreadId(opCall._threadId);
}

void IllusionsEngine_Duckman::spcOpenInventory(OpCall &opCall) {
	openInventory();
	notifyThreadId(opCall._threadId);
}

void IllusionsEngine_Duckman::spcCenterNewspaper(OpCall &opCall) {
	Control *control = getObjectControl(0x40017);
	control->_flags |= 8;
	control->_actor->_position.x = 160;
	control->_actor->_position.y = 100;
	notifyThreadId(opCall._threadId);
}

void IllusionsEngine_Duckman::spcSetCursorInventoryMode(OpCall &opCall) {
	ARG_BYTE(mode);
	ARG_BYTE(value);
	setCursorInventoryMode(mode, value);
	notifyThreadId(opCall._threadId);
}

void IllusionsEngine_Duckman::spcUpdateObject272Sequence(OpCall &opCall) {
	byte flags = 0;
	uint32 sequenceId;
	if (_scriptResource->_properties.get(0x000E0085))
		flags |= 1;
	if (_scriptResource->_properties.get(0x000E0083))
		flags |= 2;
	if (_scriptResource->_properties.get(0x000E0084))
		flags |= 4;
	switch (flags) {
	case 0:
		sequenceId = 0x603C1;
		break;
	case 1:
		sequenceId = 0x603BF;
		break;
	case 2:
		sequenceId = 0x603C2;
		break;
	case 3:
		sequenceId = 0x603C0;
		break;
	case 4:
		sequenceId = 0x603C3;
		break;
	case 5:
		sequenceId = 0x603C5;
		break;
	case 6:
		sequenceId = 0x603C4;
		break;
	case 7:
		sequenceId = 0x603C6;
		break;
	default:
		sequenceId = 0x603C1;
		break;
	}
	Control *control = getObjectControl(0x40110);
	control->startSequenceActor(sequenceId, 2, opCall._threadId);
}

} // End of namespace Illusions
