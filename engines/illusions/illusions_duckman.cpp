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
#include "illusions/scriptopcodes_duckman.h"
#include "illusions/scriptresource.h"
#include "illusions/scriptman.h"
#include "illusions/soundresource.h"
#include "illusions/specialcode.h"
//TODO#include "illusions/bbdou/bbdou_specialcode.h"
#include "illusions/talkresource.h"
#include "illusions/thread.h"
#include "illusions/time.h"
#include "illusions/updatefunctions.h"

#include "illusions/abortablethread.h"
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
	
	_resSys->loadResource(0x000D0001, 0x00010001, 0);

    initActiveScenes();
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
		//debug("getNamedPointPosition(%08X) UNKNOWN", namedPointId);
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
	// TODO
	control->_actor->_actorIndex = 2;
	// TODO _cursor->place(control, sequenceId);
}

void IllusionsEngine_Duckman::setCursorControl(Control *control) {
	// TODO
}

void IllusionsEngine_Duckman::showCursor() {
	// TODO
}

void IllusionsEngine_Duckman::hideCursor() {
	// TODO
}

void IllusionsEngine_Duckman::cursorControlRoutine(Control *control, uint32 deltaTime) {
	control->_actor->_seqCodeValue1 = 100 * deltaTime;
	// TODO
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

} // End of namespace Illusions
