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

#include "illusions/illusions_bbdou.h"
#include "illusions/actor.h"
#include "illusions/actorresource.h"
#include "illusions/backgroundresource.h"
#include "illusions/camera.h"
#include "illusions/cursor.h"
#include "illusions/dictionary.h"
#include "illusions/fontresource.h"
#include "illusions/graphics.h"
#include "illusions/input.h"
#include "illusions/resourcesystem.h"
#include "illusions/screen.h"
#include "illusions/screentext.h"
#include "illusions/scriptopcodes_bbdou.h"
#include "illusions/scriptresource.h"
#include "illusions/scriptman.h"
#include "illusions/soundresource.h"
#include "illusions/specialcode.h"
#include "illusions/bbdou/bbdou_specialcode.h"
#include "illusions/talkresource.h"
#include "illusions/thread.h"
#include "illusions/time.h"
#include "illusions/updatefunctions.h"

#include "illusions/abortablethread.h"
#include "illusions/scriptthread.h"
#include "illusions/talkthread.h"
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

// TriggerFunction

TriggerFunction::TriggerFunction(uint32 sceneId, uint32 verbId, uint32 objectId2, uint32 objectId, TriggerFunctionCallback *callback)
	: _sceneId(sceneId), _verbId(verbId), _objectId2(objectId2), _objectId(objectId), _callback(callback) {
}

TriggerFunction::~TriggerFunction() {
	delete _callback;
}

void TriggerFunction::run(uint32 callingThreadId) {
	(*_callback)(this, callingThreadId);
}

// TriggerFunctions

void TriggerFunctions::add(uint32 sceneId, uint32 verbId, uint32 objectId2, uint32 objectId, TriggerFunctionCallback *callback) {
	ItemsIterator it = findInternal(sceneId, verbId, objectId2, objectId);
	if (it != _triggerFunctions.end()) {
		delete *it;
		_triggerFunctions.erase(it);
	}
	_triggerFunctions.push_back(new TriggerFunction(sceneId, verbId, objectId2, objectId, callback));
}

TriggerFunction *TriggerFunctions::find(uint32 sceneId, uint32 verbId, uint32 objectId2, uint32 objectId) {
	ItemsIterator it = findInternal(sceneId, verbId, objectId2, objectId);
	if (it != _triggerFunctions.end())
		return (*it);
	return 0;
}

void TriggerFunctions::removeBySceneId(uint32 sceneId) {
	ItemsIterator it = _triggerFunctions.begin();
	while (it != _triggerFunctions.end()) {
		if ((*it)->_sceneId == sceneId) {
			delete *it;
			it = _triggerFunctions.erase(it);
		} else
			++it;
	}
}

TriggerFunctions::ItemsIterator TriggerFunctions::findInternal(uint32 sceneId, uint32 verbId, uint32 objectId2, uint32 objectId) {
	ItemsIterator it = _triggerFunctions.begin();
	for (; it != _triggerFunctions.end(); ++it) {
		TriggerFunction *triggerFunction = *it;
		if (triggerFunction->_sceneId == sceneId && triggerFunction->_verbId == verbId &&
			triggerFunction->_objectId2 == objectId2 && triggerFunction->_objectId == objectId)
			break;
	}
	return it;		
}

// ActiveScenes

ActiveScenes::ActiveScenes() {
	clear();
}

void ActiveScenes::clear() {
	_stack.clear();
}

void ActiveScenes::push(uint32 sceneId) {
	ActiveScene activeScene;
	activeScene._sceneId = sceneId;
	activeScene._pauseCtr = 0;
	_stack.push(activeScene);
}

void ActiveScenes::pop() {
	_stack.pop();
}

void ActiveScenes::pauseActiveScene() {
	++_stack.top()._pauseCtr;
}

void ActiveScenes::unpauseActiveScene() {
	--_stack.top()._pauseCtr;
}

uint ActiveScenes::getActiveScenesCount() {
	return _stack.size();
}

void ActiveScenes::getActiveSceneInfo(uint index, uint32 *sceneId, int *pauseCtr) {
	if (sceneId)
		*sceneId = _stack[index - 1]._sceneId;
	if (pauseCtr)
		*pauseCtr = _stack[index - 1]._pauseCtr;
}

uint32 ActiveScenes::getCurrentScene() {
	if (_stack.size() > 0)
		return _stack.top()._sceneId;
	return 0;
}

bool ActiveScenes::isSceneActive(uint32 sceneId) {
	for (uint i = 0; i < _stack.size(); ++i)
		if (_stack[i]._sceneId == sceneId && _stack[i]._pauseCtr <= 0)
			return true;
	return false;
}

// IllusionsEngine_BBDOU

IllusionsEngine_BBDOU::IllusionsEngine_BBDOU(OSystem *syst, const IllusionsGameDescription *gd)
	: IllusionsEngine(syst, gd) {
}

Common::Error IllusionsEngine_BBDOU::run() {

	// Init search paths
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "music");
	SearchMan.addSubDirectoryMatching(gameDataDir, "resource");
	SearchMan.addSubDirectoryMatching(gameDataDir, "resrem");
	SearchMan.addSubDirectoryMatching(gameDataDir, "savegame");
	SearchMan.addSubDirectoryMatching(gameDataDir, "sfx");
	SearchMan.addSubDirectoryMatching(gameDataDir, "video");
	SearchMan.addSubDirectoryMatching(gameDataDir, "voice");

	_dict = new Dictionary();

	_resSys = new ResourceSystem(this);
	_resSys->addResourceLoader(0x00060000, new ActorResourceLoader(this));
	_resSys->addResourceLoader(0x00080000, new SoundGroupResourceLoader(this));
	_resSys->addResourceLoader(0x000D0000, new ScriptResourceLoader(this));
	_resSys->addResourceLoader(0x000F0000, new TalkResourceLoader(this));
	_resSys->addResourceLoader(0x00100000, new ActorResourceLoader(this));
	_resSys->addResourceLoader(0x00110000, new BackgroundResourceLoader(this));
	_resSys->addResourceLoader(0x00120000, new FontResourceLoader(this));
	_resSys->addResourceLoader(0x00170000, new SpecialCodeLoader(this));

	_screen = new Screen(this, 640, 480, 16);
	_screenText = new ScreenText(this);
	_input = new Input();	
	_scriptMan = new ScriptMan(this);
	_actorItems = new ActorItems(this);
	_backgroundItems = new BackgroundItems(this);
	_camera = new Camera(this);
	_controls = new Controls(this);
	_cursor = new Cursor(this);
	_talkItems = new TalkItems(this);
	_triggerFunctions = new TriggerFunctions();
	_threads = new ThreadList(this);

	_scriptOpcodes = new ScriptOpcodes_BBDOU(this);
	_stack = new ScriptStack();
	
	// TODO Move to own class
	_resGetCtr = 0;
	_unpauseControlActorFlag = false;
	_lastUpdateTime = 0;

	_pauseCtr = 0;
	_field8 = 1;
	_fieldA = 0;
	_fieldE = 240;
	
	_globalSceneId = 0x00010003;	
	
    setDefaultTextCoords();
	
	_resSys->loadResource(0x000D0001, 0, 0);

	_doScriptThreadInit = false;
	startScriptThread(0x00020004, 0, 0, 0, 0);
	_doScriptThreadInit = true;

	while (!shouldQuit()) {
		_threads->updateThreads();
		updateActors();
		updateSequences();
		updateGraphics();
		_screen->updateSprites();
		_system->updateScreen();
		updateEvents();
		_system->delayMillis(10);
	}

	delete _stack;
	delete _scriptOpcodes;

	delete _threads;
	delete _triggerFunctions;
	delete _talkItems;
	delete _cursor;
	delete _controls;
	delete _camera;
	delete _backgroundItems;
	delete _actorItems;
	delete _scriptMan;
	delete _input;
	delete _screenText;
	delete _screen;
	delete _resSys;
	delete _dict;
	
	debug("Ok");
	
	return Common::kNoError;
}

bool IllusionsEngine_BBDOU::hasFeature(EngineFeature f) const {
	return
		false;
		/*
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
		*/
}

bool IllusionsEngine_BBDOU::causeIsDeclared(uint32 sceneId, uint32 verbId, uint32 objectId2, uint32 objectId) {
	uint32 codeOffs;
	return 
		_triggerFunctions->find(sceneId, verbId, objectId2, objectId) ||
		findTriggerCause(sceneId, verbId, objectId2, objectId, codeOffs);
}

void IllusionsEngine_BBDOU::causeDeclare(uint32 verbId, uint32 objectId2, uint32 objectId, TriggerFunctionCallback *callback) {
	_triggerFunctions->add(getCurrentScene(), verbId, objectId2, objectId, callback);
}

uint32 IllusionsEngine_BBDOU::causeTrigger(uint32 sceneId, uint32 verbId, uint32 objectId2, uint32 objectId, uint32 callingThreadId) {
	uint32 codeOffs;
	uint32 causeThreadId = 0;
	TriggerFunction *triggerFunction = _triggerFunctions->find(sceneId, verbId, objectId2, objectId);
	if (triggerFunction) {
		triggerFunction->run(callingThreadId);
	} else if (findTriggerCause(sceneId, verbId, objectId2, objectId, codeOffs)) {
		//debug("Run cause at %08X", codeOffs);
		causeThreadId = startTempScriptThread(_scriptResource->getCode(codeOffs),
			callingThreadId, verbId, objectId2, objectId);
	}
	return causeThreadId;
}

void IllusionsEngine_BBDOU::setDefaultTextCoords() {
	WidthHeight dimensions;
	dimensions._width = 480;
	dimensions._height = 48;
	Common::Point pt(320, 448);
	setDefaultTextDimensions(dimensions);
	setDefaultTextPosition(pt);
}

void IllusionsEngine_BBDOU::loadSpecialCode(uint32 resId) {
	_specialCode = new BbdouSpecialCode(this);
	_specialCode->init();
}

void IllusionsEngine_BBDOU::unloadSpecialCode(uint32 resId) {
	delete _specialCode;
	_specialCode = 0;
}

void IllusionsEngine_BBDOU::notifyThreadId(uint32 &threadId) {
	if (threadId) {
		uint32 tempThreadId = threadId;
		threadId = 0;
		_threads->notifyId(tempThreadId);
	}
}

bool IllusionsEngine_BBDOU::testMainActorFastWalk(Control *control) {
	return false;
}

bool IllusionsEngine_BBDOU::testMainActorCollision(Control *control) {
	// Not used in BBDOU
	return false;
}

Control *IllusionsEngine_BBDOU::getObjectControl(uint32 objectId) {
	return _dict->getObjectControl(objectId);
}

Common::Point IllusionsEngine_BBDOU::getNamedPointPosition(uint32 namedPointId) {
	Common::Point pt;
	if (_backgroundItems->findActiveBackgroundNamedPoint(namedPointId, pt) ||
		_actorItems->findNamedPoint(namedPointId, pt) ||
		_controls->findNamedPoint(namedPointId, pt))
		return pt;
	// TODO
	//debug("getNamedPointPosition(%08X) UNKNOWN", namedPointId);
	return Common::Point(0, 0);
}

uint32 IllusionsEngine_BBDOU::getPriorityFromBase(int16 priority) {
	return 32000000 * priority;
}

uint32 IllusionsEngine_BBDOU::getCurrentScene() {
	return _activeScenes.getCurrentScene();
}

uint32 IllusionsEngine_BBDOU::getPrevScene() {
	return _prevSceneId;
}

bool IllusionsEngine_BBDOU::isCursorObject(uint32 actorTypeId, uint32 objectId) {
	return actorTypeId == 0x50001 && objectId == 0x40004;
}

void IllusionsEngine_BBDOU::setCursorControlRoutine(Control *control) {
	control->_actor->setControlRoutine(new Common::Functor2Mem<Control*, uint32, void, IllusionsEngine_BBDOU>
		(this, &IllusionsEngine_BBDOU::cursorControlRoutine));
}

void IllusionsEngine_BBDOU::placeCursorControl(Control *control, uint32 sequenceId) {
	_cursor->place(control, sequenceId);
}

void IllusionsEngine_BBDOU::setCursorControl(Control *control) {
	_cursor->setControl(control);
}

void IllusionsEngine_BBDOU::showCursor() {
	_cursor->show();
}

void IllusionsEngine_BBDOU::hideCursor() {
	_cursor->hide();
}

void IllusionsEngine_BBDOU::cursorControlRoutine(Control *control, uint32 deltaTime) {
	control->_actor->_seqCodeValue1 = 100 * deltaTime;
	if (control->_actor->_flags & 1) {
		switch (_cursor->_status) {
		case 2:
			// Unused nullsub_1(control);
			break;
		case 3:
			// TODO _vm->_shellMgr->handleMouse(control);
			break;
		}
	}
}

void IllusionsEngine_BBDOU::startScriptThreadSimple(uint32 threadId, uint32 callingThreadId) {
	startScriptThread(threadId, callingThreadId, 0, 0, 0);
}

void IllusionsEngine_BBDOU::startScriptThread(uint32 threadId, uint32 callingThreadId,
	uint32 value8, uint32 valueC, uint32 value10) {
	debug(2, "Starting script thread %08X", threadId);
	byte *scriptCodeIp = _scriptResource->getThreadCode(threadId);
	newScriptThread(threadId, callingThreadId, 0, scriptCodeIp, value8, valueC, value10);
}

void IllusionsEngine_BBDOU::startAnonScriptThread(int32 threadId, uint32 callingThreadId,
	uint32 value8, uint32 valueC, uint32 value10) {
	debug(2, "Starting anonymous script thread %08X", threadId);
	uint32 tempThreadId = newTempThreadId();
	byte *scriptCodeIp = _scriptResource->getThreadCode(threadId);
	scriptCodeIp = _scriptResource->getThreadCode(threadId);
	newScriptThread(tempThreadId, callingThreadId, 0, scriptCodeIp, value8, valueC, value10);
}

uint32 IllusionsEngine_BBDOU::startAbortableTimerThread(uint32 duration, uint32 threadId) {
	return newTimerThread(duration, threadId, true);
}

uint32 IllusionsEngine_BBDOU::startTimerThread(uint32 duration, uint32 threadId) {
	return newTimerThread(duration, threadId, false);
}

uint32 IllusionsEngine_BBDOU::startAbortableThread(byte *scriptCodeIp1, byte *scriptCodeIp2, uint32 callingThreadId) {
	uint32 tempThreadId = newTempThreadId();
	debug(2, "Starting abortable thread %08X", tempThreadId);
	uint32 scriptThreadId = startTempScriptThread(scriptCodeIp1, tempThreadId, 0, 0, 0);
	AbortableThread *abortableThread = new AbortableThread(this, tempThreadId, callingThreadId, 0,
		scriptThreadId, scriptCodeIp2);
	_threads->startThread(abortableThread);
	return tempThreadId;
}

uint32 IllusionsEngine_BBDOU::startTalkThread(int16 duration, uint32 objectId, uint32 talkId, uint32 sequenceId1,
	uint32 sequenceId2, uint32 namedPointId, uint32 callingThreadId) {
	debug(2, "Starting talk thread");
	uint32 tempThreadId = newTempThreadId();
	_threads->endTalkThreadsNoNotify();
	TalkThread *talkThread = new TalkThread(this, tempThreadId, callingThreadId, 0,
		duration, objectId, talkId, sequenceId1, sequenceId2, namedPointId);
	_threads->startThread(talkThread);
	return tempThreadId;
}

uint32 IllusionsEngine_BBDOU::startTempScriptThread(byte *scriptCodeIp, uint32 callingThreadId,
	uint32 value8, uint32 valueC, uint32 value10) {
	uint32 tempThreadId = newTempThreadId();
	debug(2, "Starting temp script thread %08X", tempThreadId);
	newScriptThread(tempThreadId, callingThreadId, 0, scriptCodeIp, value8, valueC, value10);
	return tempThreadId;
}

void IllusionsEngine_BBDOU::newScriptThread(uint32 threadId, uint32 callingThreadId, uint notifyFlags,
	byte *scriptCodeIp, uint32 value8, uint32 valueC, uint32 value10) {
	ScriptThread *scriptThread = new ScriptThread(this, threadId, callingThreadId, notifyFlags,
		scriptCodeIp, value8, valueC, value10);
	_threads->startThread(scriptThread);
	if (_pauseCtr > 0)
		scriptThread->pause();
	if (_doScriptThreadInit) {
		int updateResult = kTSRun;
		while (scriptThread->_pauseCtr <= 0 && updateResult != kTSTerminate && updateResult != kTSYield)
			updateResult = scriptThread->update();
	}
}

uint32 IllusionsEngine_BBDOU::newTimerThread(uint32 duration, uint32 callingThreadId, bool isAbortable) {
	uint32 tempThreadId = newTempThreadId();
	TimerThread *timerThread = new TimerThread(this, tempThreadId, callingThreadId, 0,
		duration, isAbortable);
	_threads->startThread(timerThread);
	return tempThreadId;
}

uint32 IllusionsEngine_BBDOU::newTempThreadId() {
	uint32 threadId = _nextTempThreadId + 2 * _scriptResource->_codeCount;
	if (threadId > 65535) {
		_nextTempThreadId = 0;
		threadId = 2 * _scriptResource->_codeCount;
	}
	++_nextTempThreadId;
	return 0x00020000 | threadId;
}

bool IllusionsEngine_BBDOU::enterScene(uint32 sceneId, uint32 threadId) {
	ProgInfo *progInfo = _scriptResource->getProgInfo(sceneId & 0xFFFF);
	if (!progInfo) {
		dumpActiveScenes(_globalSceneId, threadId);
		sceneId = _theSceneId;
	}
	_activeScenes.push(sceneId);
	return progInfo != 0;
}

void IllusionsEngine_BBDOU::exitScene(uint32 threadId) {
	uint32 sceneId = _activeScenes.getCurrentScene();
	// TODO krnfileDump(sceneId);
	// TODO UpdateFunctions_disableByTag__TODO_maybe(sceneId);
	_threads->terminateThreadsByTag(sceneId, threadId);
	_controls->destroyControlsByTag(sceneId);
	_triggerFunctions->removeBySceneId(sceneId);
	_resSys->unloadResourcesByTag(sceneId);
	_activeScenes.pop();
}

void IllusionsEngine_BBDOU::enterPause(uint32 threadId) {
	uint32 sceneId = _activeScenes.getCurrentScene();
	_camera->pushCameraMode();
	_threads->suspendThreadsByTag(sceneId, threadId);
	_controls->pauseControlsByTag(sceneId);
	_actorItems->pauseByTag(sceneId);
	_backgroundItems->pauseByTag(sceneId);
	_activeScenes.pauseActiveScene();
}

void IllusionsEngine_BBDOU::leavePause(uint32 threadId) {
	uint32 sceneId = _activeScenes.getCurrentScene();
	_backgroundItems->unpauseByTag(sceneId);
	_actorItems->unpauseByTag(sceneId);
	_controls->unpauseControlsByTag(sceneId);
	_threads->notifyThreadsByTag(sceneId, threadId);
	_camera->popCameraMode();
	_activeScenes.unpauseActiveScene();
}

void IllusionsEngine_BBDOU::dumpActiveScenes(uint32 sceneId, uint32 threadId) {
	uint activeScenesCount = _activeScenes.getActiveScenesCount();
	while (activeScenesCount > 0) {
		uint32 activeSceneId;
		_activeScenes.getActiveSceneInfo(activeScenesCount, &activeSceneId, 0);
		if (activeSceneId == sceneId)
			break;
		exitScene(threadId);
		--activeScenesCount;
	}
	_camera->clearCameraModeStack();
}

void IllusionsEngine_BBDOU::setSceneIdThreadId(uint32 theSceneId, uint32 theThreadId) {
	_theSceneId = theSceneId;
	_theThreadId = theThreadId;
}

bool IllusionsEngine_BBDOU::findTriggerCause(uint32 sceneId, uint32 verbId, uint32 objectId2, uint32 objectId, uint32 &codeOffs) {
	ProgInfo *progInfo = _scriptResource->getProgInfo(sceneId & 0xFFFF);
	if (progInfo)
		return progInfo->findTriggerCause(verbId, objectId2, objectId, codeOffs);
	return false;
}

void IllusionsEngine_BBDOU::reset() {
	_scriptResource->_blockCounters.clear();
	_scriptResource->_properties.clear();
	// TODO script_sub_417FF0(1, 0);
}

} // End of namespace Illusions
