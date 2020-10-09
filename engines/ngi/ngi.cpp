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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "base/plugins.h"

#include "common/archive.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "audio/mixer.h"

#include "engines/util.h"
#include "graphics/surface.h"

#include "ngi/ngi.h"
#include "ngi/detection.h"
#include "ngi/gameloader.h"
#include "ngi/messages.h"
#include "ngi/behavior.h"
#include "ngi/modal.h"
#include "ngi/input.h"
#include "ngi/motion.h"
#include "ngi/statics.h"
#include "ngi/scenes.h"
#include "ngi/floaters.h"
#include "ngi/console.h"
#include "ngi/constants.h"

namespace NGI {

NGIEngine *g_nmi = nullptr;
Vars *g_vars = nullptr;

NGIEngine::NGIEngine(OSystem *syst, const NGIGameDescription *gameDesc) :
	Engine(syst),
	_gameDescription(gameDesc),
	_rnd("ngi"),
	_gameProject(nullptr),
	_modalObject(nullptr),
	_currSoundList1(),
	_mapTable() {
	DebugMan.addDebugChannel(kDebugPathfinding, "path", "Pathfinding");
	DebugMan.addDebugChannel(kDebugDrawing, "drawing", "Drawing");
	DebugMan.addDebugChannel(kDebugLoading, "loading", "Scene loading");
	DebugMan.addDebugChannel(kDebugAnimation, "animation", "Animation");
	DebugMan.addDebugChannel(kDebugBehavior, "behavior", "Behavior");
	DebugMan.addDebugChannel(kDebugMemory, "memory", "Memory management");
	DebugMan.addDebugChannel(kDebugEvents, "events", "Event handling");
	DebugMan.addDebugChannel(kDebugInventory, "inventory", "Inventory");
	DebugMan.addDebugChannel(kDebugSceneLogic, "scenelogic", "Scene Logic");
	DebugMan.addDebugChannel(kDebugInteractions, "interactions", "Interactions");
	DebugMan.addDebugChannel(kDebugXML, "xml", "XML");

	// Setup mixer
	if (!_mixer->isReady()) {
		warning("Sound initialization failed.");
	}

	syncSoundSettings();
	_sfxVolume = ConfMan.getInt("sfx_volume") * 39 - 10000;
	_musicVolume = ConfMan.getInt("music_volume");

	setDebugger(new Console(this));

	_gameProjectVersion = 0;
	_pictureScale = 8;
	_scrollSpeed = 0;
	_currSoundListCount = 0;

	_updateTicks = 0;
	_lastInputTicks = 0;
	_lastButtonUpTicks = 0;

	_currArchive = 0;

	_soundEnabled = true;
	_flgSoundList = true;

	_inputController = 0;
	_inputDisabled = false;

	_normalSpeed = true;

	_currentCheat = -1;
	_currentCheatPos = 0;

	_liftEnterMQ = 0;
	_liftExitMQ = 0;
	_lift = 0;
	_lastLiftButton = nullptr;
	_liftX = 0;
	_liftY = 0;

	_gameContinue = true;
	_needRestart = false;
	_flgPlayIntro = true;
	_gamePaused = false;
	_inputArFlag = false;
	_recordEvents = false;
	_mainMenu_debugEnabled = false;

	_flgGameIsRunning = true;

	_isProcessingMessages = false;

	_musicAllowed = -1;
	_musicGameVar = 0;
	_musicMinDelay = 0;
	_musicMaxDelay = 0;
	_musicLocal = 0;
	_trackStartDelay = 0;

	_stream2playing = false;

	_numSceneTracks = 0;
	_sceneTrackHasSequence = false;
	_sceneTrackIsPlaying = false;

	_aniMan = nullptr;
	_aniMan2 = nullptr;
	_currentScene = nullptr;
	_loaderScene = nullptr;
	_scene2 = nullptr;
	_scene3 = nullptr;
	_messageHandlers = nullptr;

	_updateScreenCallback = nullptr;
	_updateCursorCallback = nullptr;

	_msgX = 0;
	_msgY = 0;
	_msgObjectId2 = 0;
	_msgId = 0;
	_mouseVirtX = 0;
	_mouseVirtY = 0;

	_currSelectedInventoryItemId = 0;

	_cursorId = 0;

	_keyState = Common::KEYCODE_INVALID;
	_buttonState = 0;

	_updateFlag = true;
	_flgCanOpenMap = true;

	_sceneWidth = 1;
	_sceneHeight = 1;

	_inventoryScene = nullptr;
	_inventory = nullptr;

	_minCursorId = 0xffff;
	_maxCursorId = 0;
	_objectAtCursor = 0;
	_objectIdAtCursor = 0;

	_arcadeOverlay = nullptr;
	_arcadeOverlayHelper = nullptr;
	_arcadeOverlayX = 0;
	_arcadeOverlayY = 0;
	_arcadeOverlayMidX = 0;
	_arcadeOverlayMidY = 0;

	_isSaveAllowed = true;

	g_nmi = this;
	g_vars = new Vars;
}

NGIEngine::~NGIEngine() {
	delete g_vars;
	g_vars = nullptr;
}

void NGIEngine::restartGame() {
	_floaters->stopAll();

	clearGlobalMessageQueueList();
	clearMessages();

	initObjectStates();

	if (_scene2) {
		_scene2->getAniMan();
		_scene2 = nullptr;
	}

	if (_currentScene) {
		_gameLoader->unloadScene(_currentScene->_sceneId);

		_currentScene = nullptr;
	}

	_gameLoader->restoreDefPicAniInfos();

	getGameLoaderInventory()->clear();
	getGameLoaderInventory()->addItem(ANI_INV_MAP, 1);
	getGameLoaderInventory()->rebuildItemRects();

	initMap();

	if (_flgPlayIntro) {
		_gameLoader->loadScene(SC_INTRO1);
		_gameLoader->gotoScene(SC_INTRO1, TrubaUp);
	} else {
		_gameLoader->loadScene(SC_1);
		_gameLoader->gotoScene(SC_1, TrubaLeft);
	}
}

bool NGIEngine::shouldQuit() {
	return !_gameContinue || Engine::shouldQuit();
}

Common::Error NGIEngine::loadGameState(int slot) {
	deleteModalObject();

	if (_gameLoader->readSavegame(getSavegameFile(slot)))
		return Common::kNoError;
	else
		return Common::kUnknownError;
}

Common::Error NGIEngine::saveGameState(int slot, const Common::String &description, bool isAutosave) {
	if (_gameLoader->writeSavegame(_currentScene, getSavegameFile(slot), description))
		return Common::kNoError;
	else
		return Common::kUnknownError;
}

Common::String NGIEngine::getSaveStateName(int slot) const {
	return Common::String::format("%s.s%02d", getGameId(), slot);
}

Common::Error NGIEngine::run() {
	const Graphics::PixelFormat format(4, 8, 8, 8, 8, 24, 16, 8, 0);
	// Initialize backend
	initGraphics(800, 600, &format);

	_backgroundSurface.create(800, 600, format);
	_origFormat = Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);

	_globalMessageQueueList.reset(new GlobalMessageQueueList);
	_behaviorManager.reset(new BehaviorManager);

	_sceneRect.left = 0;
	_sceneRect.top = 0;
	_sceneRect.right = 799;
	_sceneRect.bottom = 599;

	_floaters.reset(new Floaters);
	_aniHandler.reset(new AniHandler);
	_globalPalette = &_defaultPalette;

	_isSaveAllowed = false;

	if (debugChannelSet(-1, kDebugXML))
		loadGameObjH();

	int scene = 0;
	if (ConfMan.hasKey("boot_param"))
		scene = convertScene(ConfMan.getInt("boot_param"));

	if (ConfMan.hasKey("save_slot"))
		scene = -1;

	switch (getGameGID()) {
	case GID_FULLPIPE:
		if (!loadGam("fullpipe.gam", scene))
			return Common::kNoGameDataFoundError;
		break;
	case GID_MDREAM:
		if (!loadGam("new.gam", scene))
			return Common::kNoGameDataFoundError;
		break;
	default:
		warning("Unknown GID");
		return Common::kUnsupportedGameidError;
	}

	if (ConfMan.hasKey("save_slot")) {
		loadGameState(ConfMan.getInt("save_slot"));
	}

#if 0
	loadAllScenes();
#endif

	int time1 = g_nmi->_system->getMillis();

	// Center mouse
	_system->warpMouse(400, 300);

	for (;;) {
		updateEvents();
		if (shouldQuit()) {
			break;
		}

		int time2 = g_nmi->_system->getMillis();

		// 30fps
		if (time2 - time1 >= 33 || !_normalSpeed) {
			time1 = time2;
			updateScreen();
		}

		if (_needRestart) {
			delete _modalObject;
			freeGameLoader();
			_currentScene = nullptr;
			_updateTicks = 0;
			_globalPalette = &_defaultPalette;

			loadGam("fullpipe.gam");
			_needRestart = false;
		}

		_system->delayMillis(5);
		_system->updateScreen();
	}

	delete _modalObject;
	freeGameLoader();

	cleanup();
	_backgroundSurface.free();

	return Common::kNoError;
}

void NGIEngine::updateEvents() {
	Common::Event event;
	Common::EventManager *eventMan = _system->getEventManager();
	ExCommand *ex;

	while (eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			_keyState = event.kbd.keycode;

			switch (event.kbd.keycode) {
			case Common::KEYCODE_SPACE:
				if (_gamePaused) {
					if (_modalObject) {
						if (_modalObject->init(42)) {
							_modalObject->update();
						} else {
							deleteModalObject();
						}
					} else {
						_gameLoader->updateSystems(42);
					}
					return;
				}

				ex = new ExCommand(0, 17, 36, 0, 0, 0, 1, 0, 0, 0);
				ex->_param = 32;
				ex->_excFlags |= 3;
				ex->handle();
				break;
			case Common::KEYCODE_s:
				if (_gamePaused) {
					_gamePaused = 0;
					_flgGameIsRunning = true;
					return;
				}

				ex = new ExCommand(0, 17, 36, 0, 0, 0, 1, 0, 0, 0);
				ex->_param = event.kbd.keycode;
				ex->_excFlags |= 3;
				ex->handle();
				break;
			case Common::KEYCODE_q:
				return;
				break;
			default:
				ex = new ExCommand(0, 17, 36, 0, 0, 0, 1, 0, 0, 0);
				ex->_param = event.kbd.keycode;
				ex->_excFlags |= 3;
				ex->handle();
				break;
			}
			break;
		case Common::EVENT_KEYUP:
			if (!_inputArFlag) {
				ex = new ExCommand(0, 17, 37, 0, 0, 0, 1, 0, 0, 0);
				ex->_excFlags |= 3;
				ex->handle();
			}
			_keyState = Common::KEYCODE_INVALID;
			break;
		case Common::EVENT_MOUSEMOVE:
			if (_recordEvents) {
				ex = new ExCommand(0, 17, 31, event.mouse.x, event.mouse.y, 0, 1, 0, 0, 0);
				ex->_excFlags |= 3;
				ex->handle();
			}

			_mouseScreenPos = event.mouse;
			break;
		case Common::EVENT_QUIT:
			return;
		case Common::EVENT_RBUTTONDOWN:
			if (!_inputArFlag && (_updateTicks - _lastInputTicks) >= 2) {
				ex = new ExCommand(0, 17, 107, event.mouse.x, event.mouse.y, 0, 1, 0, 0, 0);
				ex->_excFlags |= 3;
				_lastInputTicks = _updateTicks;
				ex->handle();
			}
			_mouseScreenPos = event.mouse;
			break;
		case Common::EVENT_LBUTTONDOWN:
			if (!_inputArFlag && (_updateTicks - _lastInputTicks) >= 2) {
				ex = new ExCommand(0, 17, 29, event.mouse.x, event.mouse.y, 0, 1, 0, 0, 0);

				ex->_sceneClickX = _sceneRect.left + ex->_x;
				ex->_sceneClickY = _sceneRect.top + ex->_y;
				ex->_param = getGameLoaderInventory()->getSelectedItemId();
				ex->_excFlags |= 3;
				_lastInputTicks = _updateTicks;
				ex->handle();
			}
			_mouseScreenPos = event.mouse;
			break;
		case Common::EVENT_LBUTTONUP:
			if (!_inputArFlag && (_updateTicks - _lastButtonUpTicks) >= 2) {
				ex = new ExCommand(0, 17, 30, 0, 0, 0, 1, 0, 0, 0);
				ex->_excFlags |= 3;
				_lastButtonUpTicks = _updateTicks;
				ex->handle();
			}
			_mouseScreenPos = event.mouse;
			break;
		default:
			break;
		}
	}

	// pollEvent() is implemented only for video player. So skip it.
	//if (event.kbd.keycode == MSG_SC11_SHOWSWING && _modalObject) {
	//	_modalObject->pollEvent();
	//}
}

void NGIEngine::freeGameLoader() {
	setCursor(0);
	_floaters->stopAll();
	_gameLoader.reset();
	_currentScene = 0;
	_scene2 = 0;
	_loaderScene = 0;
}

void NGIEngine::cleanup() {
	//cleanRecorder();
	clearMessageHandlers();
	clearMessages();
	_globalMessageQueueList->compact();

	for (uint i = 0; i < _globalMessageQueueList->size(); i++)
		delete (*_globalMessageQueueList)[i];

	stopAllSoundStreams();
}

void NGIEngine::deleteModalObject() {
	if (!_modalObject)
		return;

	_modalObject->saveload();
	BaseModalObject *tmp = _modalObject->_parentObj;

	delete _modalObject;

	_modalObject = tmp;
}

void NGIEngine::updateScreen() {
	debugC(4, kDebugDrawing, "NGIEngine::updateScreen()");

	_mouseVirtX = _mouseScreenPos.x + _sceneRect.left;
	_mouseVirtY = _mouseScreenPos.y + _sceneRect.top;

	//if (inputArFlag)
	//	updateGame_inputArFlag();

	if (_modalObject || (_flgGameIsRunning && (_gameLoader->updateSystems(42), _modalObject != 0))) {
		if (_flgGameIsRunning) {
			if (_modalObject->init(42)) {
				_modalObject->update();
			} else {
				deleteModalObject();
			}
		}
	} else if (_currentScene) {
		_currentScene->draw();

		if (_inventoryScene)
			_inventory->draw();

		if (_updateScreenCallback)
			_updateScreenCallback();

		//if (inputArFlag && _currentScene) {
		//	vrtTextOut(*(_DWORD *)g_vrtHandle, smallNftData, "DEMO", 4, 380, 580);
		//	vrtTextOut(*(_DWORD *)g_vrtHandle, smallNftData, "Alt+F4 - exit", 14, 695, 580);
		//}
	} else {
		//vrtRectangle(*(_DWORD *)g_vrtHandle, 0, 0, 0, 800, 600);
	}
	_inputController->drawCursor(_mouseScreenPos.x, _mouseScreenPos.y);

	++_updateTicks;
}

int NGIEngine::getObjectEnumState(const Common::String &name, const char *state) {
	GameVar *var = _gameLoader->_gameVar->getSubVarByName("OBJSTATES");

	if (!var) {
		var = _gameLoader->_gameVar->addSubVarAsInt("OBJSTATES", 0);
	}

	var = var->getSubVarByName(name);
	if (var) {
		var = var->getSubVarByName("ENUMSTATES");
		if (var)
			return var->getSubVarAsInt(state);
	}

	return 0;
}

int NGIEngine::getObjectState(const Common::String &objname) {
	GameVar *var = _gameLoader->_gameVar->getSubVarByName("OBJSTATES");

	if (var)
		return var->getSubVarAsInt(objname);

  return 0;
}

void NGIEngine::setObjectState(const Common::String &name, int state) {
	GameVar *var = _gameLoader->_gameVar->getSubVarByName("OBJSTATES");

	if (!var) {
		var = _gameLoader->_gameVar->addSubVarAsInt("OBJSTATES", 0);
	}

	var->setSubVarAsInt(name, state);
}

void NGIEngine::disableSaves(ExCommand *ex) {
	if (_isSaveAllowed) {
		_isSaveAllowed = false;

		if (_globalMessageQueueList->size() && (*_globalMessageQueueList)[0] != 0) {
			for (uint i = 0; i < _globalMessageQueueList->size(); i++) {
				if ((*_globalMessageQueueList)[i]->_flags & 1)
					if ((*_globalMessageQueueList)[i]->_id != ex->_parId && !(*_globalMessageQueueList)[i]->_isFinished)
						return;
			}
		}

		// Original was makeing a save on every room entering
		if (_currentScene) {
			_gameLoader->saveScenePicAniInfos(_currentScene->_sceneId);
			//	_gameLoader->writeSavegame(_currentScene, "savetmp.sav");
		}
	}
}

bool NGIEngine::isSaveAllowed() {
	if (!g_nmi->_isSaveAllowed)
		return false;

	bool allowed = true;

	for (Common::Array<MessageQueue *>::iterator s = g_nmi->_globalMessageQueueList->begin(); s != g_nmi->_globalMessageQueueList->end(); ++s) {
		if (!(*s)->_isFinished && ((*s)->getFlags() & 1))
			allowed = false;
	}

	return allowed;
}


} // End of namespace NGI
