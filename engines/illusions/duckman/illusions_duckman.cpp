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

#include "illusions/duckman/illusions_duckman.h"
#include "illusions/duckman/duckman_dialog.h"
#include "illusions/duckman/duckman_specialcode.h"
#include "illusions/duckman/gamestate_duckman.h"
#include "illusions/duckman/menusystem_duckman.h"
#include "illusions/duckman/scriptopcodes_duckman.h"
#include "illusions/duckman/duckman_videoplayer.h"
#include "illusions/actor.h"
#include "illusions/camera.h"
#include "illusions/console.h"
#include "illusions/cursor.h"
#include "illusions/dictionary.h"
#include "illusions/gamresourcereader.h"
#include "illusions/graphics.h"
#include "illusions/input.h"
#include "illusions/resources/actorresource.h"
#include "illusions/resources/backgroundresource.h"
#include "illusions/resources/fontresource.h"
#include "illusions/resources/genericresource.h"
#include "illusions/resources/midiresource.h"
#include "illusions/resources/scriptresource.h"
#include "illusions/resources/soundresource.h"
#include "illusions/resources/talkresource.h"
#include "illusions/resourcesystem.h"
#include "illusions/screen.h"
#include "illusions/screentext.h"
#include "illusions/scriptstack.h"
#include "illusions/sound.h"
#include "illusions/specialcode.h"
#include "illusions/textdrawer.h"
#include "illusions/thread.h"
#include "illusions/time.h"
#include "illusions/updatefunctions.h"

#include "illusions/threads/abortablethread.h"
#include "illusions/threads/causethread_duckman.h"
#include "illusions/threads/scriptthread.h"
#include "illusions/threads/talkthread_duckman.h"
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

// IllusionsEngine_Duckman

IllusionsEngine_Duckman::IllusionsEngine_Duckman(OSystem *syst, const IllusionsGameDescription *gd)
	: IllusionsEngine(syst, gd) {
}

Common::Error IllusionsEngine_Duckman::run() {

	// Init search paths
	const Common::FSNode gameDataDir(ConfMan.getPath("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "music");
	SearchMan.addSubDirectoryMatching(gameDataDir, "sfx", 0, 2);
	SearchMan.addSubDirectoryMatching(gameDataDir, "video");
	SearchMan.addSubDirectoryMatching(gameDataDir, "voice");

	_dict = new Dictionary();

	_resReader = new ResourceReaderGamArchive("duckman.gam");

	_resSys = new ResourceSystem(this);
	_resSys->addResourceLoader(0x00060000, new ActorResourceLoader(this));
	_resSys->addResourceLoader(0x00080000, new SoundGroupResourceLoader(this));
	_resSys->addResourceLoader(0x000A0000, new MidiGroupResourceLoader(this));
	_resSys->addResourceLoader(0x000D0000, new ScriptResourceLoader(this));
	_resSys->addResourceLoader(0x000F0000, new TalkResourceLoader(this));
	_resSys->addResourceLoader(0x00100000, new ActorResourceLoader(this));
	_resSys->addResourceLoader(0x00110000, new BackgroundResourceLoader(this));
	_resSys->addResourceLoader(0x00120000, new FontResourceLoader(this));
	_resSys->addResourceLoader(0x00190000, new GenericResourceLoader(this));

	setDebugger(new Console(this));

	_screen = new Screen8Bit(this, 320, 200);
	_screenPalette = new ScreenPalette(this);
	_screenText = new ScreenText(this);
	_input = new Input();
	_actorInstances = new ActorInstanceList(this);
	_backgroundInstances = new BackgroundInstanceList(this);
	_camera = new Camera(this);
	_controls = new Controls(this);
	_talkItems = new TalkInstanceList(this);
	_threads = new ThreadList(this);
	_updateFunctions = new UpdateFunctions();
	_soundMan = new SoundMan(this);
	_menuSystem = new DuckmanMenuSystem(this);
	_videoPlayer = new DuckmanVideoPlayer(this);
	_gameState = new Duckman_GameState(this);

	_fader = new Fader();

	_dialogSys = new DuckmanDialogSystem(this);

	_screen->setColorKey1(0);

	initInput();

	initUpdateFunctions();

	_scriptOpcodes = new ScriptOpcodes_Duckman(this);
	_stack = new ScriptStack();

	// TODO Move to own class
	_resGetCtr = 0;
	_unpauseControlActorFlag = false;
	_lastUpdateTime = 0;

	_currWalkOverlappedControl = nullptr;

	_pauseCtr = 0;
	_doScriptThreadInit = false;
	_field8 = 1;
	_fieldA = 0;

	ConfMan.registerDefault("talkspeed", 240);
	_subtitleDuration = (uint16)ConfMan.getInt("talkspeed");
	debug(0, "talkspeed: %d", _subtitleDuration);

	_globalSceneId = 0x00010003;

	_savedInventoryActorIndex = 0;

	loadSpecialCode(0);
	setDefaultTextCoords();
	initCursor();
	initActiveScenes();

	_resSys->loadResource(0x120001, 0x00010001, 0);
	_resSys->loadResource(0x120002, 0x00010001, 0);
	_resSys->loadResource(0x120003, 0x00010001, 0);
	_resSys->loadResource(0x000D0001, 0x00010001, 0);

#if 0
	//DEBUG
	_scriptResource->_properties.set(0x000E003A, true);
	_scriptResource->_properties.set(0x000E0020, true);
	_scriptResource->_properties.set(0x000E003A, true);
	_scriptResource->_properties.set(0x000E003B, true);
	_scriptResource->_properties.set(0x000E0009, true);
	_scriptResource->_properties.set(0x000E003D, true);
	_scriptResource->_properties.set(0x000E0024, true);
#endif

#if 0
	// DEBUG Enterprise
	_scriptResource->_blockCounters.set(238, 1);
#endif

#if 0
	// DEBUG Map / special code 0016001A
	_scriptResource->_properties.set(0x000E0017, true);
	_scriptResource->_properties.set(0x000E0022, false);
#endif

	if (ConfMan.hasKey("save_slot")) {
		_doScriptThreadInit = true;
		// Load global resources manually, usually done by the game script
		enterScene(0x00010003, 0);
		loadGameState(ConfMan.getInt("save_slot"));
	} else {
		startScriptThread(0x00020004, 0);
		_doScriptThreadInit = true;
	}

	while (!shouldQuit()) {
		if (_resumeFromSavegameRequested) {
			activateSavegame(0);
			resumeFromSavegame(0);
			_resumeFromSavegameRequested = false;
		}
		runUpdateFunctions();
		_system->updateScreen();
		updateEvents();
	}

	unloadSpecialCode(0);

	_resSys->unloadResourceById(0x120001);
	_resSys->unloadResourceById(0x120002);
	_resSys->unloadResourceById(0x120003);
	_resSys->unloadResourceById(0x000D0001);
	_resSys->unloadAllResources();

	delete _stack;
	delete _scriptOpcodes;

	delete _dialogSys;

	delete _fader;

	delete _gameState;
	delete _menuSystem;
	delete _videoPlayer;
	delete _soundMan;
	delete _updateFunctions;
	delete _threads;
	delete _talkItems;
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

bool IllusionsEngine_Duckman::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

void IllusionsEngine_Duckman::initInput() {
	_input->setInputEvent(kEventLeftClick, 0x01)
		.addMouseButton(MOUSE_LEFT_BUTTON);
	_input->setInputEvent(kEventRightClick, 0x02)
		.addMouseButton(MOUSE_RIGHT_BUTTON);
	// 0x04 is unused
	_input->setInputEvent(kEventInventory, 0x08)
		.addKey(kActionInventory);
	_input->setInputEvent(kEventAbort, 0x10)
		.addKey(kActionAbort);
	_input->setInputEvent(kEventSkip, 0x20)
		.addMouseButton(MOUSE_LEFT_BUTTON)
		.addKey(kActionSkip);
	_input->setInputEvent(kEventUp, 0x40)
		.addKey(kActionCursorUp);
	_input->setInputEvent(kEventDown, 0x80)
		.addMouseButton(MOUSE_RIGHT_BUTTON)
		.addKey(kActionCursorDown);
	_input->setInputEvent(kEventF1, 0x100)
		.addKey(kActionCheatMode);
}

#define UPDATEFUNCTION(priority, sceneId, callback) \
	_updateFunctions->add(priority, sceneId, new Common::Functor1Mem<uint, int, IllusionsEngine_Duckman> \
		(this, &IllusionsEngine_Duckman::callback));

void IllusionsEngine_Duckman::initUpdateFunctions() {
	UPDATEFUNCTION(25, 0, updateVideoPlayer);
	UPDATEFUNCTION(30, 0, updateScript);
	UPDATEFUNCTION(50, 0, updateActors);
	UPDATEFUNCTION(60, 0, updateSequences);
	UPDATEFUNCTION(70, 0, updateGraphics);
	UPDATEFUNCTION(90, 0, updateSprites);
	UPDATEFUNCTION(120, 0, updateSoundMan);
}

#undef UPDATEFUNCTION

int IllusionsEngine_Duckman::updateScript(uint flags) {
	if (_screen->isDisplayOn() && !_screenPalette->isFaderActive() && _pauseCtr == 0) {
		if (_input->pollEvent(kEventAbort)) {
			startScriptThread(0x00020342, 0);
		} else if (_input->isCheatModeActive() && _input->pollEvent(kEventF1)) {
			startScriptThread(0x0002033F, 0);
		}
	}
	_threads->updateThreads();

	//TODO need to call startScriptThread2(0x10002, 0x20001, 0);
	return kUFNext;
}

void IllusionsEngine_Duckman::startScreenShaker(uint pointsCount, uint32 duration, const ScreenShakerPoint *points, uint32 threadId) {
	_screenShaker = new ScreenShaker();
	_screenShaker->_pointsIndex = 0;
	_screenShaker->_pointsCount = pointsCount;
	_screenShaker->_finished = false;
	_screenShaker->_duration = duration;
	_screenShaker->_nextTime = duration + getCurrentTime();
	_screenShaker->_points = points;
	_screenShaker->_notifyThreadId = threadId;
	_updateFunctions->add(71, getCurrentScene(), new Common::Functor1Mem<uint, int, IllusionsEngine_Duckman>
		(this, &IllusionsEngine_Duckman::updateScreenShaker));
}

void IllusionsEngine_Duckman::stopScreenShaker() {
	if (_screenShaker)
		_screenShaker->_finished = true;
}

int IllusionsEngine_Duckman::updateScreenShaker(uint flags) {
	if (_pauseCtr > 0 || getCurrentScene() == 0x10038) {
		_screenShaker->_nextTime = getCurrentTime();
		return 1;
	}

	if (flags & 1)
		_screenShaker->_finished = true;

	if (!_screenShaker->_finished) {
		if (getCurrentTime() >= _screenShaker->_nextTime) {
			++_screenShaker->_pointsIndex;
			if (_screenShaker->_pointsIndex <= _screenShaker->_pointsCount) {
				ScreenShakerPoint shakePt = _screenShaker->_points[_screenShaker->_pointsIndex - 1];
				if (shakePt.x == (int16)0x8000) {
					// Loop
					_screenShaker->_pointsIndex = 1;
					shakePt = _screenShaker->_points[_screenShaker->_pointsIndex - 1];
				}
				_screenShaker->_nextTime = getCurrentTime() + _screenShaker->_duration;
				_screen->setScreenOffset(Common::Point(shakePt.x, shakePt.y));
			} else
				_screenShaker->_finished = true;
		}
	}

	if (_screenShaker->_finished) {
		notifyThreadId(_screenShaker->_notifyThreadId);
		delete _screenShaker;
		_screenShaker = nullptr;
		_screen->setScreenOffset(Common::Point(0, 0));
		return 2;
	}

	return 1;
}

void IllusionsEngine_Duckman::startFader(int duration, int minValue, int maxValue, int firstIndex, int lastIndex, uint32 threadId) {
	_fader->_active = true;
	_fader->_currValue = minValue;
	_fader->_minValue = minValue;
	_fader->_maxValue = maxValue;
	_fader->_firstIndex = firstIndex;
	_fader->_lastIndex = lastIndex;
	_fader->_startTime = getCurrentTime();
	_fader->_duration = duration;
	_fader->_notifyThreadId = threadId;
}

void IllusionsEngine_Duckman::updateFader() {
	if (_fader && !_fader->_paused && _fader->_active) {
		int32 currTime = getCurrentTime();
		int32 currDuration = currTime - _fader->_startTime;
		if (currDuration) {
			int newValue;
			if (currDuration >= _fader->_duration) {
				newValue = _fader->_maxValue;
			} else {
				newValue = (currDuration * (_fader->_maxValue - _fader->_minValue) / _fader->_duration) + _fader->_minValue;
			}
			if (_fader->_currValue != newValue) {
				_fader->_currValue = newValue;
				_screenPalette->setFader(newValue, _fader->_firstIndex, _fader->_lastIndex);
			}
			if (_fader->_currValue == _fader->_maxValue) {
				_fader->_active = false;
				notifyThreadId(_fader->_notifyThreadId);
			}
		}
	}
}

void IllusionsEngine_Duckman::clearFader() {
	_fader->_active = false;
	_fader->_currValue = 255;
	_fader->_minValue = 255;
	_fader->_maxValue = 255;
	_fader->_firstIndex = 1;
	_fader->_lastIndex = 256;
	_fader->_startTime = 0;
	_fader->_duration = 0;
	_fader->_notifyThreadId = 0;
}

void IllusionsEngine_Duckman::pauseFader() {
	_fader->_paused = true;
	_fader->_startTime = getCurrentTime() - _fader->_startTime;
}

void IllusionsEngine_Duckman::unpauseFader() {
	_fader->_startTime = getCurrentTime() - _fader->_startTime;
	_fader->_paused = false;
}

int IllusionsEngine_Duckman::updateVideoPlayer(uint flags) {
	if (_videoPlayer->isPlaying())
		_videoPlayer->update();
	return kUFNext;
}

void IllusionsEngine_Duckman::playVideo(uint32 videoId, uint32 callingThreadId) {
	_videoPlayer->start(videoId, callingThreadId);
}

bool IllusionsEngine_Duckman::isVideoPlaying() {
	return _videoPlayer->isPlaying();
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
	_specialCode = new DuckmanSpecialCode(this);
	_specialCode->init();
}

void IllusionsEngine_Duckman::unloadSpecialCode(uint32 resId) {
	delete _specialCode;
	_specialCode = nullptr;
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
		_input->pollEvent(kEventSkip);
}

bool IllusionsEngine_Duckman::testMainActorCollision(Control *control) {
	bool result = false;
	Control *overlappedControl;
	if (_controls->getOverlappedWalkObject(control, control->_actor->_position, &overlappedControl)) {
		if (_currWalkOverlappedControl != overlappedControl) {
			_currWalkOverlappedControl = overlappedControl;
			if (runTriggerCause(9, 0, overlappedControl->_objectId)) {
				delete control->_actor->_pathNode;
				control->_actor->_flags &= ~Illusions::ACTOR_FLAG_400;
				control->_actor->_pathNode = nullptr;
				control->_actor->_pathPoints = 0;
				control->_actor->_pathPointsCount = 0;
				_threads->terminateThreadChain(control->_actor->_walkCallerThreadId1);
				if (control->_actor->_notifyId3C) {
					notifyThreadId(control->_actor->_notifyId3C);
					control->_actor->_walkCallerThreadId1 = 0;
				}
				result = true;
			}
		}
	} else {
		_currWalkOverlappedControl = nullptr;
	}
	return result;
}

Control *IllusionsEngine_Duckman::getObjectControl(uint32 objectId) {
	return _dict->getObjectControl(objectId);
}

static const uint8 kPointConversionTable[] = {
		0, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,
		35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,
		35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,
		35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,
		35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,
		35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,
		35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,
		35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 1, 2, 3,
		4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 35,
		35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,
		35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,
		35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,
		35, 35, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
		33, 34
};
Common::Point IllusionsEngine_Duckman::getNamedPointPosition(uint32 namedPointId) {
	Common::Point pt;
	Common::Point currPan = _camera->getCurrentPan();
	if (_backgroundInstances->findActiveBackgroundNamedPoint(namedPointId, pt)) {
		return pt;
	} else if (namedPointId - 0x00070001 > 209) {
		if (_controls->findNamedPoint(namedPointId, pt)) {
			return pt;
		} else {
			return currPan;
		}
	} else {
		debug(1, "getNamedPointPosition(%02d)", kPointConversionTable[namedPointId - 0x00070001]);
		switch(kPointConversionTable[namedPointId - 0x00070001]) {
			case 0 : return Common::Point(160, 100);
			case 1 : return currPan;
			case 2 : return Common::Point(currPan.x - 160, currPan.y);
			case 3 : return Common::Point(currPan.x + 160, currPan.y);
			case 4 : return Common::Point(currPan.x, currPan.y - 100);
			case 5 : return Common::Point(currPan.x, currPan.y + 100);
			case 6 : return Common::Point(currPan.x - 160, currPan.y - 100);
			case 7 : return Common::Point((uint16)(currPan.x + 160), currPan.y - 100);
			case 8 : return Common::Point(currPan.x - 160, currPan.y + 100);
			case 9 : return Common::Point(currPan.x + 160, currPan.y + 100);
			/* TODO implement these
			case 10 : return Common::Point(0, 0);
			case 11 : return Common::Point(0, 0);
			case 12 : return Common::Point(0, 0);
			case 13 : return Common::Point(0, 0);
			case 14 : return Common::Point(0, 0);
			*/
			case 15 : return Common::Point(0, 0);
			/* TODO implement these
			case 16 : return Common::Point(0, 0);
			case 17 : return Common::Point(0, 0);
			case 18 : return Common::Point(0, 0);
			 */
			case 19 : return Common::Point(0, 0);
			case 20 : return Common::Point(320, 0);
			case 21 : return Common::Point(640, 0);
			case 22 : return Common::Point(960, 0);
			case 23 : return Common::Point(0, 200);
			case 24 : return Common::Point(320, 200);
			case 25 : return Common::Point(640, 200);
			case 26 : return Common::Point(960, 200);
			case 27 : return Common::Point(0, 400);
			case 28 : return Common::Point(320, 400);
			case 29 : return Common::Point(640, 400);
			case 30 : return Common::Point(960, 400);
			case 31 : return Common::Point(0, 600);
			case 32 : return Common::Point(320, 0);
			case 33 : return Common::Point(640, 0);
			case 34 : return Common::Point(960, 0);
			default : break;
		}
		error("getNamedPointPosition(%02d) UNKNOWN", kPointConversionTable[namedPointId - 0x00070001]);
		return Common::Point(0, 0);
	}
}

uint32 IllusionsEngine_Duckman::getPriorityFromBase(int16 priority) {
	return priority << 16;
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
	_cursor._currOverlappedControl = nullptr;
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
	_cursor._op113_choiceOfsPtr = nullptr;
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
	_cursor._control = nullptr;
	_cursor._position.x = 160;
	_cursor._position.y = 100;
	_cursor._objectId = 0;
	_cursor._actorIndex = 1;
	_cursor._savedActorIndex = 1;
	_cursor._currOverlappedControl = nullptr;
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
	_cursor._op113_choiceOfsPtr = nullptr;
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
		_cursor._currOverlappedControl = nullptr;
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
	_cursor._field14[6] = true;
	_cursor._control->startSequenceActor(sequenceId, 2, 0);
	setCursorActorIndex(_cursor._actorIndex, 1, 0);
	_cursor._currOverlappedControl = nullptr;
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
	if (control->_actor->_flags & Illusions::ACTOR_FLAG_IS_VISIBLE) {
		switch (_cursor._gameState) {
		case 2:
			updateGameState2();
			break;
		case 3:
			_dialogSys->updateDialogState();
			break;
		case 4:
			_menuSystem->update(_cursor._control);
			break;
		default:
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

void IllusionsEngine_Duckman::startScriptThread2(uint32 sceneId, uint32 threadId, uint32 callingThreadId) {
	debug(2, "Starting script thread2");
	_savegameSceneId = sceneId;
	_savegameThreadId = threadId;
	startScriptThread(0x20002, callingThreadId);
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

void IllusionsEngine_Duckman::resumeFromSavegame(uint32 callingThreadId) {
	_input->discardAllEvents();
	if (changeScene(_savegameSceneId, _savegameThreadId, callingThreadId)) {
		_savegameSceneId = 0;
		_savegameThreadId = 0;
	}
}

void IllusionsEngine_Duckman::newScriptThread(uint32 threadId, uint32 callingThreadId, uint notifyFlags,
	byte *scriptCodeIp) {
	ScriptThread *scriptThread = new ScriptThread(this, threadId, callingThreadId, notifyFlags,
		scriptCodeIp, 0, 0, 0);
	_threads->startThread(scriptThread);
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
	SceneInfo *sceneInfo = _scriptResource->getSceneInfo(sceneId & 0xFFFF);
	if (!sceneInfo)
		return false;
	pushActiveScene(sceneId);
	uint resourcesCount;
	uint32 *resources;
	sceneInfo->getResources(resourcesCount, resources);
	for (uint i = 0; i < resourcesCount; ++i) {
		_resSys->loadResource(resources[i], sceneId, 0);
	}
	return true;
}

bool IllusionsEngine_Duckman::enterScene(uint32 sceneId, uint32 threadId) {
	if (loadScene(sceneId)) {
		if (threadId)
			startScriptThread(threadId, 0);
		return true;
	}
	startScriptThread2(0x10002, 0x20001, 0);
	return false;
}

void IllusionsEngine_Duckman::exitScene() {
	popActiveScene();
}

bool IllusionsEngine_Duckman::changeScene(uint32 sceneId, uint32 threadId, uint32 callerThreadId) {
	uint32 currSceneId = getCurrentScene();
	if (currSceneId != 0x10003)
		dumpCurrSceneFiles(currSceneId, callerThreadId);
	_soundMan->stopLoopingSounds(); //Fix for global looping sound not stopping in falling scene.
	_threads->terminateThreads(callerThreadId);
	_controls->destroyControls();
	_resSys->unloadSceneResources(0x10003, 0x10001);
	if (enterScene(sceneId, threadId)) {
		_gameState->writeState(sceneId, threadId);
		return true;
	}
	return false;
}

void IllusionsEngine_Duckman::enterPause(uint32 sceneId, uint32 threadId) {
	_threads->suspendThreads(threadId);
	_controls->pauseControls();
	_actorInstances->pauseBySceneId(sceneId);
	_backgroundInstances->pauseBySceneId(sceneId);
}

void IllusionsEngine_Duckman::leavePause(uint32 sceneId, uint32 threadId) {
	_backgroundInstances->unpauseBySceneId(sceneId);
	_actorInstances->unpauseBySceneId(sceneId);
	_controls->unpauseControls();
	_threads->notifyThreads(threadId);
}

void IllusionsEngine_Duckman::dumpActiveScenes(uint32 sceneId, uint32 threadId) {
	// TODO?
}

void IllusionsEngine_Duckman::dumpCurrSceneFiles(uint32 sceneId, uint32 threadId) {
	_updateFunctions->terminateByScene(sceneId);
	_threads->terminateActiveThreads(threadId);
	_threads->terminateThreadsBySceneId(sceneId, threadId);
	_controls->destroyActiveControls();
	_resSys->unloadResourcesBySceneId(sceneId);
}

void IllusionsEngine_Duckman::pause(uint32 callerThreadId) {
	if (++_pauseCtr == 1) {
		_threads->pauseThreads(callerThreadId);
		_camera->pause();
		pauseFader();
		_controls->pauseActors(Illusions::CURSOR_OBJECT_ID);
	}
}

void IllusionsEngine_Duckman::unpause(uint32 callerThreadId) {
	if (--_pauseCtr == 0) {
		_controls->unpauseActors(Illusions::CURSOR_OBJECT_ID);
		unpauseFader();
		_camera->unpause();
		_threads->unpauseThreads(callerThreadId);
	}
}

void IllusionsEngine_Duckman::setSceneIdThreadId(uint32 theSceneId, uint32 theThreadId) {
	_theSceneId = theSceneId;
	_theThreadId = theThreadId;
}

bool IllusionsEngine_Duckman::findTriggerCause(uint32 sceneId, uint32 verbId, uint32 objectId2, uint32 objectId, uint32 &codeOffs) {
	SceneInfo *sceneInfo = _scriptResource->getSceneInfo(sceneId & 0xFFFF);
	if (sceneInfo)
		return sceneInfo->findTriggerCause(verbId, objectId2, objectId, codeOffs);
	return false;
}

void IllusionsEngine_Duckman::reset() {
	_scriptResource->_blockCounters.clear();
	_scriptResource->_properties.clear();
	setTextDuration(1, 0);
	// TODO resetCursor();
}

uint32 IllusionsEngine_Duckman::getObjectActorTypeId(uint32 objectId) {
	return _scriptResource->getObjectActorTypeId(objectId);
}

Common::Point IllusionsEngine_Duckman::convertMousePos(Common::Point mousePos) {
	return mousePos + _camera->getScreenOffset();
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
		_cursor._currOverlappedControl = nullptr;
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
		_cursor._currOverlappedControl = nullptr;
	}

	if (_input->pollEvent(kEventLeftClick)) {
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
	} else if (_input->pollEvent(kEventRightClick)) {
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
	} else if (_input->pollEvent(kEventInventory)) {
		if (_cursor._field14[0] == 1) {
			runTriggerCause(1, 0, _scriptResource->getMainActorObjectId());
		} else if (_cursor._field14[1] == 1) {
			runTriggerCause(2, 0, _scriptResource->getMainActorObjectId());
		}
	}

}

void IllusionsEngine_Duckman::playSoundEffect(int index) {
	uint32 soundEffectId = 0;
	uint32 *soundIds = _scriptResource->_soundIds;
	switch (index) {
	case 1:
		soundEffectId = soundIds[0];
		break;
	case 2:
		soundEffectId = soundIds[1];
		break;
	case 3:
		soundEffectId = soundIds[2];
		break;
	case 4:
		soundEffectId = soundIds[3];
		break;
	case 5:
		soundEffectId = soundIds[4];
		break;
	case 6:
		soundEffectId = soundIds[getRandom(4) + 5];
		break;
	case 7:
		soundEffectId = soundIds[getRandom(4) + 9];
		break;
	case 8:
		soundEffectId = soundIds[13];
		break;
	case 9:
		soundEffectId = soundIds[14];
		break;
	case 10:
		soundEffectId = soundIds[15];
		break;
	case 11:
		soundEffectId = soundIds[16];
		break;
	case 12:
		soundEffectId = soundIds[getRandom(4) + 17];
		break;
	case 13:
		soundEffectId = soundIds[21];
		break;
	case 14:
		soundEffectId = soundIds[22];
		break;
	case 15:
		soundEffectId = soundIds[23];
		break;
	case 16:
		soundEffectId = soundIds[24];
		break;
	case 17:
		soundEffectId = soundIds[25];
		break;
	case 18:
		soundEffectId = soundIds[26];
		break;
	default:
		break;
	}

	if (soundEffectId) {
		_soundMan->playSound(soundEffectId, 255, 0);
	}
}

bool IllusionsEngine_Duckman::getTriggerCause(uint32 verbId, uint32 objectId2, uint32 objectId, uint32 &outThreadId) {
	SceneInfo *sceneInfo = _scriptResource->getSceneInfo(getCurrentScene() & 0xFFFF);
	bool found =
		sceneInfo->findTriggerCause(verbId, objectId2, objectId, outThreadId) ||
		sceneInfo->findTriggerCause(verbId, objectId2, 0x40001, outThreadId);
	if (!found) {
		sceneInfo = _scriptResource->getSceneInfo(3);
		found =
			sceneInfo->findTriggerCause(verbId, objectId2, objectId, outThreadId) ||
			sceneInfo->findTriggerCause(verbId, objectId2, 0x40001, outThreadId);
	}
	return found;
}

uint32 IllusionsEngine_Duckman::runTriggerCause(uint32 verbId, uint32 objectId2, uint32 objectId) {
	debug(1, "runTriggerCause(%08X, %08X, %08X)", verbId, objectId2, objectId);
	uint32 triggerThreadId;

	if (!getTriggerCause(verbId, objectId2, objectId, triggerThreadId))
		return 0;

	playTriggerCauseSound(verbId, objectId2, objectId);

	uint32 tempThreadId = newTempThreadId();
	debug(1, "Starting cause thread %08X with triggerThreadId %08X", tempThreadId, triggerThreadId);
	CauseThread_Duckman *causeThread = new CauseThread_Duckman(this, tempThreadId, 0, 0,
		triggerThreadId);
	_threads->startThread(causeThread);

	return tempThreadId;
}

void IllusionsEngine_Duckman::playTriggerCauseSound(uint32 verbId, uint32 objectId2, uint32 objectId) {
	bool soundWasPlayed = false;
	if (_scriptResource->_properties.get(0x000E003C)) {
		if (verbId == 7 && objectId == 0x40003) {
			playSoundEffect(7);
			soundWasPlayed = true;
		} else if (objectId == 0x40003) {
			playSoundEffect(14);
			soundWasPlayed = true;
		} else if (verbId == 3) {
			playSoundEffect(16);
			soundWasPlayed = true;
		} else if (verbId == 2) {
			soundWasPlayed = true;
		}
	}
	if (!soundWasPlayed) {
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
}

bool IllusionsEngine_Duckman::loadSavegameFromScript(int16 slotNum, uint32 callingThreadId) {
	if (_savegameSlotNum < 0) {
		return false; // TODO need to handle reset from new game (without exising savegame).
	}

	Common::String fileName = getSavegameFilename(_targetName, _savegameSlotNum);
	bool success = loadgame(fileName.c_str());
	if (success)
		activateSavegame(callingThreadId);
	_gameState->deleteReadStream();
	return success;
}

bool IllusionsEngine_Duckman::saveSavegameFromScript(int16 slotNum, uint32 callingThreadId) {
	Common::String fileName = getSavegameFilename(_targetName, _savegameSlotNum);
	return savegame(fileName.c_str(), _savegameDescription.c_str());
}

void IllusionsEngine_Duckman::activateSavegame(uint32 callingThreadId) {
	// TODO _screen->setDisplayOn(false);
	uint32 currSceneId = getCurrentScene();
	if (currSceneId != 0x10003)
		dumpCurrSceneFiles(currSceneId, callingThreadId);
	reset();
	// TODO stopMidi();
	// TODO clearMidiPlayList();
	_gameState->readState(_savegameSceneId, _savegameThreadId);
	pushActiveScene(0x10000);
	_gameState->deleteReadStream();
}

} // End of namespace Illusions
