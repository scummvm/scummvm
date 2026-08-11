/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "illusions/bbdou/illusions_bbdou.h"
#include "illusions/bbdou/bbdou_menukeys.h"
#include "illusions/bbdou/bbdou_videoplayer.h"
#include "illusions/bbdou/gamestate_bbdou.h"
#include "illusions/bbdou/menusystem_bbdou.h"
#include "illusions/actor.h"
#include "illusions/camera.h"
#include "illusions/console.h"
#include "illusions/cursor.h"
#include "illusions/dictionary.h"
#include "illusions/fileresourcereader.h"
#include "illusions/graphics.h"
#include "illusions/input.h"
#include "illusions/resources/actorresource.h"
#include "illusions/resources/backgroundresource.h"
#include "illusions/resources/fontresource.h"
#include "illusions/resources/scriptresource.h"
#include "illusions/resources/soundresource.h"
#include "illusions/resources/talkresource.h"
#include "illusions/resourcesystem.h"
#include "illusions/screen.h"
#include "illusions/screentext.h"
#include "illusions/scriptstack.h"
#include "illusions/bbdou/scriptopcodes_bbdou.h"
#include "illusions/sound.h"
#include "illusions/specialcode.h"
#include "illusions/bbdou/bbdou_specialcode.h"
#include "illusions/thread.h"
#include "illusions/time.h"
#include "illusions/updatefunctions.h"

#include "illusions/threads/abortablethread.h"
#include "illusions/threads/scriptthread.h"
#include "illusions/threads/talkthread.h"
#include "illusions/threads/timerthread.h"

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
#include "graphics/surface.h"

namespace Illusions {

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
	for (uint i = 0; i < _stack.size(); ++i) {
		if (_stack[i]._sceneId == sceneId && _stack[i]._pauseCtr <= 0)
			return true;
	}
	return false;
}

// IllusionsEngine_BBDOU

IllusionsEngine_BBDOU::IllusionsEngine_BBDOU(OSystem *syst, const IllusionsGameDescription *gd)
	: IllusionsEngine(syst, gd) {
}

Common::Error IllusionsEngine_BBDOU::run() {

	// Init search paths
	const Common::FSNode gameDataDir(ConfMan.getPath("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "music");
	SearchMan.addSubDirectoryMatching(gameDataDir, "resource");
	SearchMan.addSubDirectoryMatching(gameDataDir, "resrem");
	SearchMan.addSubDirectoryMatching(gameDataDir, "savegame");
	SearchMan.addSubDirectoryMatching(gameDataDir, "sfx", 0, 2);
	SearchMan.addSubDirectoryMatching(gameDataDir, "video");
	SearchMan.addSubDirectoryMatching(gameDataDir, "voice");

	_dict = new Dictionary();

	_resReader = new ResourceReaderFileReader();

	_resSys = new ResourceSystem(this);
	_resSys->addResourceLoader(0x00060000, new ActorResourceLoader(this));
	_resSys->addResourceLoader(0x00080000, new SoundGroupResourceLoader(this));
	_resSys->addResourceLoader(0x000D0000, new ScriptResourceLoader(this));
	_resSys->addResourceLoader(0x000F0000, new TalkResourceLoader(this));
	_resSys->addResourceLoader(0x00100000, new ActorResourceLoader(this));
	_resSys->addResourceLoader(0x00110000, new BackgroundResourceLoader(this));
	_resSys->addResourceLoader(0x00120000, new FontResourceLoader(this));
	_resSys->addResourceLoader(0x00170000, new SpecialCodeLoader(this));

	setDebugger(new Console(this));

	_screen = new Screen16Bit(this, 640, 480);
	_screenPalette = new NullScreenPalette();
	_screenText = new ScreenText(this);
	_input = new Input();
	_actorInstances = new ActorInstanceList(this);
	_backgroundInstances = new BackgroundInstanceList(this);
	_camera = new Camera(this);
	_controls = new Controls(this);
	_cursor = new Cursor(this);
	_talkItems = new TalkInstanceList(this);
	_triggerFunctions = new TriggerFunctions();
	_threads = new ThreadList(this);
	_updateFunctions = new UpdateFunctions();
	_soundMan = new SoundMan(this);
	_menuSystem = new BBDOUMenuSystem(this);
	_videoPlayer = new BBDOUVideoPlayer(this);
	_gameState = new BBDOU_GameState(this);
	_menuKeys = new BBDOUMenuKeys(this);

	_screen->setColorKey1(0xF81F);

	initInput();

	initUpdateFunctions();

	_fader = nullptr;

	_scriptOpcodes = new ScriptOpcodes_BBDOU(this);
	_stack = new ScriptStack();

	_resGetCtr = 0;
	_unpauseControlActorFlag = false;
	_lastUpdateTime = 0;

	_pauseCtr = 0;
	_field8 = 1;
	_fieldA = 0;
	ConfMan.registerDefault("talkspeed", 240);
	_subtitleDuration = (uint16)ConfMan.getInt("talkspeed");

	_globalSceneId = 0x00010003;

	setDefaultTextCoords();

	_resSys->loadResource(0x000D0001, 0, 0);

	_doScriptThreadInit = false;
	startScriptThread(0x00020004, 0, 0, 0, 0);
	_doScriptThreadInit = true;

	if (ConfMan.hasKey("save_slot")) {
		loadGameState(ConfMan.getInt("save_slot"));
	}

	_walkthroughStarted = false;
	_canResumeFromSavegame = false;

	while (!shouldQuit()) {
		if (_walkthroughStarted) {
			//enterScene(0x10003, 0);
			startScriptThread(0x00020404, 0, 0, 0, 0);
			_walkthroughStarted = false;
		}
		if (_resumeFromSavegameRequested && _canResumeFromSavegame) {
			resumeFromSavegame();
			_resumeFromSavegameRequested = false;
		}
		runUpdateFunctions();
		_system->updateScreen();
		updateEvents();
	}

	unloadSpecialCode(0);

	_resSys->unloadAllResources();

	delete _stack;
	delete _scriptOpcodes;

	delete _menuKeys;
	delete _gameState;
	delete _videoPlayer;
	delete _menuSystem;
	delete _soundMan;
	delete _updateFunctions;
	delete _threads;
	delete _triggerFunctions;
	delete _talkItems;
	delete _cursor;
	delete _controls;
	delete _camera;
	delete _backgroundInstances;
	delete _actorInstances;
	delete _input;
	delete _screenText;
	delete _screenPalette;
	delete _screen;
	delete _resSys;
	delete _resReader;
	delete _dict;

	debug("Ok");

	return Common::kNoError;
}

bool IllusionsEngine_BBDOU::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

void IllusionsEngine_BBDOU::initInput() {
	_input->setInputEvent(kEventLeftClick, 0x01)
		.addMouseButton(MOUSE_LEFT_BUTTON);
	_input->setInputEvent(kEventRightClick, 0x02)
		.addMouseButton(MOUSE_RIGHT_BUTTON);
	_input->setInputEvent(kEventInventory, 0x04)
		.addMouseButton(MOUSE_RIGHT_BUTTON)
		.addKey(kActionInventory);
	_input->setInputEvent(kEventAbort, 0x08)
		.addKey(kActionAbort);
	_input->setInputEvent(kEventSkip, 0x10)
		.addKey(kActionSkip);
	_input->setInputEvent(kEventF1, 0x20)
		.addKey(kActionCheatMode);
	_input->setInputEvent(kEventUp, 0x40)
		.addKey(kActionCursorUp);
	_input->setInputEvent(kEventDown, 0x80)
		.addMouseButton(MOUSE_RIGHT_BUTTON)
		.addKey(kActionCursorDown);
}

#define UPDATEFUNCTION(priority, sceneId, callback) \
	_updateFunctions->add(priority, sceneId, new Common::Functor1Mem<uint, int, IllusionsEngine_BBDOU> \
		(this, &IllusionsEngine_BBDOU::callback));

void IllusionsEngine_BBDOU::initUpdateFunctions() {
	UPDATEFUNCTION(30, 0, updateScript);
	UPDATEFUNCTION(50, 0, updateActors);
	UPDATEFUNCTION(60, 0, updateMenuKeys);
	UPDATEFUNCTION(60, 0, updateSequences);
	UPDATEFUNCTION(70, 0, updateGraphics);
	UPDATEFUNCTION(70, 0, updateVideoPlayer);
	UPDATEFUNCTION(90, 0, updateSprites);
	UPDATEFUNCTION(120, 0, updateSoundMan);
}

#undef UPDATEFUNCTION

int IllusionsEngine_BBDOU::updateScript(uint flags) {
	_threads->updateThreads();
	return kUFNext;
}

int IllusionsEngine_BBDOU::updateMenuKeys(uint flags) {
	_menuKeys->update();
	return kUFNext;
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
		causeThreadId = startTempScriptThread(_scriptResource->getCode(codeOffs),
			callingThreadId, verbId, objectId2, objectId);
	}
	return causeThreadId;
}

int IllusionsEngine_BBDOU::updateVideoPlayer(uint flags) {
	if (_videoPlayer->isPlaying())
		_videoPlayer->update();
	return kUFNext;
}

void IllusionsEngine_BBDOU::playVideo(uint32 videoId, uint32 objectId, uint32 priority, uint32 callingThreadId) {
	_videoPlayer->start(videoId, objectId, priority, callingThreadId);
}

bool IllusionsEngine_BBDOU::isVideoPlaying() {
	return _videoPlayer->isPlaying();
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
	_specialCode = nullptr;
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
	if (_backgroundInstances->findActiveBackgroundNamedPoint(namedPointId, pt) ||
		_actorInstances->findNamedPoint(namedPointId, pt) ||
		_controls->findNamedPoint(namedPointId, pt))
		return pt;
	// TODO
	switch (namedPointId) {
	case 0x70001:
		return Common::Point(0, 0);
	case 0x70002:
		return Common::Point(640, 0);
	case 0x70023:
		return Common::Point(320, 240);
	default:
		break;
	}
	debug("getNamedPointPosition(%08X) UNKNOWN", namedPointId);
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
	return actorTypeId == 0x50001 && objectId == Illusions::CURSOR_OBJECT_ID;
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
	if (control->_actor->_flags & Illusions::ACTOR_FLAG_IS_VISIBLE) {
		switch (_cursor->_status) {
		case 2:
			// Unused nullsub_1(control);
			break;
		case 3:
			_menuSystem->update(control);
			break;
		default:
			break;
		}
	}
}

void IllusionsEngine_BBDOU::startScriptThreadSimple(uint32 threadId, uint32 callingThreadId) {
	startScriptThread(threadId, callingThreadId, 0, 0, 0);
}

void IllusionsEngine_BBDOU::startScriptThread(uint32 threadId, uint32 callingThreadId,
	uint32 value8, uint32 valueC, uint32 value10) {
	if (threadId == 0x0002041E && ConfMan.hasKey("save_slot")) {
		// Skip intro videos when loading a savegame from the launcher (kludge)
		notifyThreadId(callingThreadId);
		return;
	}
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
		while (scriptThread->_pauseCtr <= 0 && updateResult != kTSTerminate && updateResult != kTSYield) {
			updateResult = scriptThread->update();
		}
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
	SceneInfo *sceneInfo = _scriptResource->getSceneInfo(sceneId & 0xFFFF);
	if (!sceneInfo) {
		dumpActiveScenes(_globalSceneId, threadId);
		sceneId = _theSceneId;
	}
	_activeScenes.push(sceneId);
	if (sceneId == 0x0001007D) {
		// Savegame loading from the ScummVM GUI or command line is only
		// possible after resources have been initialized by the startup script.
		// Once that script is done, it switches to the start menu scene.
		// After that the game is ready and a savegame can finally be loaded.
		_canResumeFromSavegame = true;
	}
	return sceneInfo != nullptr;
}

void IllusionsEngine_BBDOU::exitScene(uint32 threadId) {
	uint32 sceneId = _activeScenes.getCurrentScene();
	_updateFunctions->terminateByScene(sceneId);
	_threads->terminateThreadsBySceneId(sceneId, threadId);
	_controls->destroyControlsBySceneId(sceneId);
	_triggerFunctions->removeBySceneId(sceneId);
	_resSys->unloadResourcesBySceneId(sceneId);
	_activeScenes.pop();
}

void IllusionsEngine_BBDOU::enterPause(uint32 threadId) {
	uint32 sceneId = _activeScenes.getCurrentScene();
	_camera->pushCameraMode();
	_threads->suspendThreadsBySceneId(sceneId, threadId);
	_controls->pauseControlsBySceneId(sceneId);
	_actorInstances->pauseBySceneId(sceneId);
	_backgroundInstances->pauseBySceneId(sceneId);
	_activeScenes.pauseActiveScene();
}

void IllusionsEngine_BBDOU::leavePause(uint32 threadId) {
	uint32 sceneId = _activeScenes.getCurrentScene();
	_backgroundInstances->unpauseBySceneId(sceneId);
	_actorInstances->unpauseBySceneId(sceneId);
	_controls->unpauseControlsBySceneId(sceneId);
	_threads->notifyThreadsBySceneId(sceneId, threadId);
	_camera->popCameraMode();
	_activeScenes.unpauseActiveScene();
}

void IllusionsEngine_BBDOU::dumpActiveScenes(uint32 sceneId, uint32 threadId) {
	uint activeScenesCount = _activeScenes.getActiveScenesCount();
	while (activeScenesCount > 0) {
		uint32 activeSceneId;
		_activeScenes.getActiveSceneInfo(activeScenesCount, &activeSceneId, nullptr);
		if (activeSceneId == sceneId)
			break;
		exitScene(threadId);
		--activeScenesCount;
	}
	_camera->clearCameraModeStack();
}

void IllusionsEngine_BBDOU::pause(uint32 callerThreadId) {
	if (++_pauseCtr == 1) {
		_threads->pauseThreads(callerThreadId);
		_camera->pause();
		pauseFader();
		_controls->pauseActors(0x40004);
	}
}

void IllusionsEngine_BBDOU::unpause(uint32 callerThreadId) {
	if (--_pauseCtr == 0) {
		_controls->unpauseActors(0x40004);
		unpauseFader();
		_camera->unpause();
		_threads->unpauseThreads(callerThreadId);
	}
}

void IllusionsEngine_BBDOU::enterMenuPause() {
	// TODO suspendAudio();
	_screenText->clearText();
}

void IllusionsEngine_BBDOU::leaveMenuPause() {
	_screenText->removeText();
	// TODO unsuspendAudio();
}

void IllusionsEngine_BBDOU::setSceneIdThreadId(uint32 theSceneId, uint32 theThreadId) {
	_theSceneId = theSceneId;
	_theThreadId = theThreadId;
}

bool IllusionsEngine_BBDOU::findTriggerCause(uint32 sceneId, uint32 verbId, uint32 objectId2, uint32 objectId, uint32 &codeOffs) {
	SceneInfo *sceneInfo = _scriptResource->getSceneInfo(sceneId & 0xFFFF);
	if (sceneInfo)
		return sceneInfo->findTriggerCause(verbId, objectId2, objectId, codeOffs);
	return false;
}

void IllusionsEngine_BBDOU::reset() {
	_scriptResource->_blockCounters.clear();
	_scriptResource->_properties.clear();
	setTextDuration(1, 0);
}

void IllusionsEngine_BBDOU::loadSavegameFromScript(int16 slotNum, uint32 callingThreadId) {
	// NOTE Just loads the savegame, doesn't activate it yet
	Common::String fileName = getSavegameFilename(_targetName, _savegameSlotNum);
	_loadGameResult = loadgame(fileName.c_str());
}

void IllusionsEngine_BBDOU::saveSavegameFromScript(int16 slotNum, uint32 callingThreadId) {
	// TODO
	// Common::String fileName = getSavegameFilename(_targetName, slotNum);
	_saveGameResult = false;//savegame(fileName.c_str(), _savegameDescription.c_str());
}

void IllusionsEngine_BBDOU::activateSavegame(uint32 callingThreadId) {
	uint32 sceneId, threadId;
	_prevSceneId = 0x10000;
	_gameState->readState(sceneId, threadId);
	enterScene(sceneId, callingThreadId);
	// TODO Check if value8, valueC, value10 are needed at all
	startAnonScriptThread(threadId, 0, 0, 0, 0);
	_gameState->deleteReadStream();
}

void IllusionsEngine_BBDOU::resumeFromSavegame() {
	// Resetting the game is usually done by the script, when loading from the ScummVM menu or
	// command line this has to be done manually.
	_specialCode->resetBeforeResumeSavegame();
	dumpActiveScenes(0x00010003, 0);
	activateSavegame(0);
}

} // End of namespace Illusions
