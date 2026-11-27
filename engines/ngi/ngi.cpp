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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "base/plugins.h"
#include "backends/keymapper/keymapper.h"

#include "common/archive.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "audio/mixer.h"

#include "engines/metaengine.h"
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

	_currArchive = nullptr;

	_soundEnabled = true;
	_flgSoundList = true;

	_inputController = nullptr;
	_inputDisabled = false;

	_normalSpeed = true;

	_currentCheat = -1;
	_currentCheatPos = 0;

	_liftEnterMQ = nullptr;
	_liftExitMQ = nullptr;
	_lift = nullptr;
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
	_musicGameVar = nullptr;
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
	_objectAtCursor = nullptr;
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
	clearMessageHandlers();
	delete g_vars;
	g_vars = nullptr;
}

void NGIEngine::restartGame() {
	_floaters->deleteAllFlies();

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

	getGameLoaderInventory()->removeAllItems();
	getGameLoaderInventory()->addItem(ANI_INV_MAP, 1);
	getGameLoaderInventory()->recalcItemPos();

	initMap();

	if (_flgPlayIntro) {
		_gameLoader->loadScene(SC_INTRO1);
		_gameLoader->setScene(SC_INTRO1, TrubaUp);
	} else {
		_gameLoader->loadScene(SC_1);
		_gameLoader->setScene(SC_1, TrubaLeft);
	}
}

bool NGIEngine::shouldQuit() {
	return !_gameContinue || Engine::shouldQuit();
}

Common::Error NGIEngine::loadGameState(int slot) {
	deleteModalObject();

	if (_gameLoader->readSavegame(g_nmi->getMetaEngine()->getSavegameFile(slot,"fullpipe").c_str()))
		return Common::kNoError;
	else
		return Common::kUnknownError;
}

Common::Error NGIEngine::saveGameState(int slot, const Common::String &description, bool isAutosave) {
	if (_gameLoader->writeSavegame(_currentScene, g_nmi->getMetaEngine()->getSavegameFile(slot,"fullpipe").c_str(), description))
		return Common::kNoError;
	else
		return Common::kUnknownError;
}

Common::String NGIEngine::getSaveStateName(int slot) const {
	return Common::String::format("%s.s%02d", getGameId(), slot);
}

Common::Error NGIEngine::initGame() {
	_behaviorManager.reset(new BehaviorController);

	_sceneRect.left = 0;
	_sceneRect.top = 0;
	_sceneRect.right = 799;
	_sceneRect.bottom = 599;

	_floaters.reset(new FlyPack);
	_globalPalette = &_defaultPalette;

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

	_system->warpMouse(400, 300);
	return Common::kNoError;
}

bool NGIEngine::handleGuiEvent(const Common::Event &event) {
	if (event.type == Common::EVENT_SCREEN_CHANGED) {
		return true;
	} else if (_modalObject) {
		const bool back = (event.type == Common::EVENT_KEYUP &&
				(event.kbd.keycode == Common::KEYCODE_ESCAPE || event.kbd.keycode == Common::KEYCODE_AC_BACK)) ||
			(event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_END && event.customType == kActionCloseDialog);
		if (event.type == Common::EVENT_LBUTTONUP || back) {
			Command cmd;
			cmd._cmd = 17;
			cmd._messageNum = back ? 37 : 30;
			cmd._param = back ? kActionCloseDialog : 0;
			_modalObject->handleMessage(&cmd);
		}
	} else if (_currentScene->_guiManager) {
		GuiManager &gui = *_currentScene->_guiManager;
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			gui.onKeyDown(event.kbd.keycode);
			if (event.kbd.ascii)
				gui.onChar(event.kbd.ascii);
			break;
		case Common::EVENT_KEYUP:
			gui.onKeyUp(event.kbd.keycode);
			break;
		case Common::EVENT_MOUSEMOVE:
		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_LBUTTONUP:
		case Common::EVENT_RBUTTONDOWN:
		case Common::EVENT_RBUTTONUP:
			gui.onMouseMove(event.mouse);
			if (event.type == Common::EVENT_LBUTTONDOWN)
				gui.onMouseLKeyDown(event.mouse);
			else if (event.type == Common::EVENT_LBUTTONUP)
				gui.onMouseLKeyUp(event.mouse);
			else if (event.type == Common::EVENT_RBUTTONDOWN)
				gui.onMouseRKeyDown(event.mouse);
			else if (event.type == Common::EVENT_RBUTTONUP)
				gui.onMouseRKeyUp(event.mouse);
			return true;
		default:
			break;
		}
	}
	return false;
}

Common::Error NGIEngine::run() {
	_globalMessageQueueList.reset(new GlobalMessageQueueList);
	_aniHandler.reset(new AniHandler);
	const bool android = getPlatform() == Common::kPlatformAndroid;
	const SceneTag *start = nullptr;
	int width = 800, height = 600;
	Common::Error result = Common::kNoError;
	if (android) {
		_gameLoader.reset(new GameLoader());
		result = _gameLoader->loadXML(start, width, height);
		if (result.getCode() != Common::kNoError)
			return result;
		_gameProject = _gameLoader->_gameProject.get();
	}

	initGraphics(width, height, nullptr);
	const Graphics::PixelFormat format = _system->getScreenFormat();
	if (format.isCLUT8())
		return Common::kUnsupportedColorMode;
	_backgroundSurface.create(width, height, format);
	_isSaveAllowed = false;

	if (android) {
		Common::Keymapper *mapper = _system->getEventManager()->getKeymapper();
		mapper->disableAllGameKeymaps();
		mapper->setGameKeymapState("ngi-default", true);
		if (!_gameLoader->setScene(start->_sceneId, 0))
			return Common::Error(Common::kNoGameDataFoundError, "Could not load the Full Pipe Android start scene");
	} else {
		result = initGame();
		if (result.getCode() != Common::kNoError)
			return result;
	}

	const bool intro = android && start->_sceneId == SC_INTRO1;
	byte brightness = intro ? 0 : 255;
	if (android) {
		if (brightness)
			_currentScene->render();
		else
			_backgroundSurface.clear(0);
		_system->copyRectToScreen(_backgroundSurface.getPixels(), _backgroundSurface.pitch, 0, 0, width, height);
		_system->updateScreen();
		debugC(1, kDebugDrawing, "Presented first Full Pipe Android frame for %s", start->_tag.c_str());
	}

	uint32 lastUpdate = _system->getMillis();
	while (!shouldQuit()) {
		const bool redraw = updateEvents();
		if (shouldQuit())
			break;
		const uint32 now = _system->getMillis();
		const uint32 elapsed = now - lastUpdate;
		if (android || elapsed >= 33 || !_normalSpeed) {
			lastUpdate = now;
			// CGame::Update discounts long stalls on Android.
			uint32 step = 42;
			if (android)
				step = elapsed > 350 ? 15 : elapsed;
			result = updateScreen(step, brightness, redraw);
			if (result.getCode() != Common::kNoError)
				break;
		}
		if (!android && _needRestart) {
			delete _modalObject;
			freeGameLoader();
			_currentScene = nullptr;
			_updateTicks = 0;
			_globalPalette = &_defaultPalette;

			loadGam("fullpipe.gam");
			_needRestart = false;
		}

		_system->updateScreen();
		_system->delayMillis(android ? 10 : 5);
	}

	if (android) {
		deleteModalObject();
		clearMessages();
		clearGlobalMessageQueueList();
		clearMessageHandlers();
		while (_currSoundListCount)
			_currSoundList1[--_currSoundListCount] = nullptr;
		g_vars->sceneIntro_aniin1man = nullptr;
		g_vars->scene01_picSc01Osk = nullptr;
		g_vars->scene01_picSc01Osk2 = nullptr;
		_aniMan = _aniMan2 = nullptr;
		_currentScene = nullptr;
		_gameProject = nullptr;
		_gameLoader.reset();
	} else {
		delete _modalObject;
		freeGameLoader();
		cleanup();
	}
	_backgroundSurface.free();
	return result;
}

bool NGIEngine::updateEvents() {
	Common::Event event;
	Common::EventManager *eventMan = _system->getEventManager();
	Command *ex;
	bool redraw = false;

	while (eventMan->pollEvent(event)) {
		if (getPlatform() == Common::kPlatformAndroid) {
			redraw |= handleGuiEvent(event);
			continue;
		}
		switch (event.type) {
		case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
			ex = new Command(0, 17, 36, 0, 0, 0, 1, 0, 0, 0);
			ex->_param = event.customType;
			ex->_excFlags |= 3;
			ex->handle();
			break;
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
						_gameLoader->updateContent(42);
					}
					return redraw;
				}

				ex = new Command(0, 17, 36, 0, 0, 0, 1, 0, 0, 0);
				ex->_param = 32;
				ex->_excFlags |= 3;
				ex->handle();
				break;
			case Common::KEYCODE_s:
				if (_gamePaused) {
					_gamePaused = 0;
					_flgGameIsRunning = true;
					return redraw;
				}

				ex = new Command(0, 17, 36, 0, 0, 0, 1, 0, 0, 0);
				ex->_param = event.kbd.keycode;
				ex->_excFlags |= 3;
				ex->handle();
				break;
			case Common::KEYCODE_q:
				return redraw;
				break;
			default:
				ex = new Command(0, 17, 36, 0, 0, 0, 1, 0, 0, 0);
				ex->_param = event.kbd.keycode;
				ex->_excFlags |= 3;
				ex->handle();
				break;
			}
			break;
		case Common::EVENT_KEYUP:
			if (!_inputArFlag) {
				ex = new Command(0, 17, 37, 0, 0, 0, 1, 0, 0, 0);
				ex->_excFlags |= 3;
				ex->handle();
			}
			_keyState = Common::KEYCODE_INVALID;
			break;
		case Common::EVENT_MOUSEMOVE:
			if (_recordEvents) {
				ex = new Command(0, 17, 31, event.mouse.x, event.mouse.y, 0, 1, 0, 0, 0);
				ex->_excFlags |= 3;
				ex->handle();
			}

			_mouseScreenPos = event.mouse;
			break;
		case Common::EVENT_QUIT:
			return redraw;
		case Common::EVENT_RBUTTONDOWN:
			if (!_inputArFlag && (_updateTicks - _lastInputTicks) >= 2) {
				ex = new Command(0, 17, 107, event.mouse.x, event.mouse.y, 0, 1, 0, 0, 0);
				ex->_excFlags |= 3;
				_lastInputTicks = _updateTicks;
				ex->handle();
			}
			_mouseScreenPos = event.mouse;
			break;
		case Common::EVENT_LBUTTONDOWN:
			if (!_inputArFlag && (_updateTicks - _lastInputTicks) >= 2) {
				ex = new Command(0, 17, 29, event.mouse.x, event.mouse.y, 0, 1, 0, 0, 0);

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
				ex = new Command(0, 17, 30, 0, 0, 0, 1, 0, 0, 0);
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
	return redraw;
}

void NGIEngine::freeGameLoader() {
	setCursor(0);
	_floaters->deleteAllFlies();
	_gameLoader.reset();
	_currentScene = nullptr;
	_scene2 = nullptr;
	_loaderScene = nullptr;
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

Common::Error NGIEngine::updateScreen(uint32 elapsed, byte &brightness, bool redraw) {
	const bool android = getPlatform() == Common::kPlatformAndroid;
	_mouseVirtX = _mouseScreenPos.x + _sceneRect.left;
	_mouseVirtY = _mouseScreenPos.y + _sceneRect.top;

	Scene *scene = _currentScene;
	const bool hadModal = _modalObject != nullptr;
	if (!hadModal && (android || _flgGameIsRunning)) {
		redraw |= _gameLoader->updateContent(elapsed);
		if (android && !_modalObject && scene == _currentScene && scene->_sceneId == SC_INTRO1)
			redraw |= sceneIntro_updateAndroid(elapsed, brightness);
		if (android)
			processMessages();
	}
	const bool renderScene = !_modalObject;
	if (_modalObject && (android || _flgGameIsRunning)) {
		// Android begins updating a newly opened dialog on the following tick.
		if ((!android || hadModal) && !_modalObject->init(elapsed))
			deleteModalObject();
		redraw = true;
	}

	if (android && scene != _currentScene) {
		if (scene->_guiManager)
			scene->_guiManager->onHide();
		stopAllSounds(Sound::kSpeech);
		stopAllSounds(Sound::kEffect);
		if (_currentScene->_behaviorController)
			_currentScene->_behaviorController->onShow(*_currentScene);
		debugC(2, kDebugDrawing, "Switched Android scene from %d to %d", scene->_sceneId, _currentScene->_sceneId);
		redraw = true;
	}
	if (android && !_modalObject && _gameLoader->_preloadSceneId) {
		if (!_gameLoader->setScene(_gameLoader->_preloadSceneId, _gameLoader->_preloadEntranceId))
			return Common::Error(Common::kNoGameDataFoundError, "Could not enter the requested Android scene");
		_gameLoader->_preloadSceneId = _gameLoader->_preloadEntranceId = 0;
		brightness = 255;
		redraw = true;
	}

	if (_modalObject) {
		if (android || _flgGameIsRunning)
			_modalObject->update();
	} else if (_currentScene && (android ? redraw : renderScene)) {
		_currentScene->render();
		if (_inventoryScene)
			_inventory->render();
		if (_updateScreenCallback)
			_updateScreenCallback();
		if (brightness < 255)
			drawAlphaRectangle(0, 0, _backgroundSurface.w, _backgroundSurface.h, brightness);
	}

	if (android) {
		if (redraw)
			_system->copyRectToScreen(_backgroundSurface.getPixels(), _backgroundSurface.pitch, 0, 0, _backgroundSurface.w, _backgroundSurface.h);
	} else {
		_inputController->render(_mouseScreenPos.x, _mouseScreenPos.y);
	}
	++_updateTicks;
	return Common::kNoError;
}

int NGIEngine::getStateValue(const Common::String &name, const char *state) {
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

int NGIEngine::getStateId(const Common::String &objname) {
	GameVar *var = _gameLoader->_gameVar->getSubVarByName("OBJSTATES");

	if (var)
		return var->getSubVarAsInt(objname);

  return 0;
}

void NGIEngine::setState(const Common::String &name, int state) {
	GameVar *var = _gameLoader->_gameVar->getSubVarByName("OBJSTATES");

	if (!var) {
		var = _gameLoader->_gameVar->addSubVarAsInt("OBJSTATES", 0);
	}

	var->setSubVarAsInt(name, state);
}

void NGIEngine::disableSaves(Command *ex) {
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
