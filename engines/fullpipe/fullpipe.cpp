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

#include "base/plugins.h"

#include "common/archive.h"
#include "common/config-manager.h"

#include "engines/util.h"

#include "fullpipe/fullpipe.h"
#include "fullpipe/gameloader.h"
#include "fullpipe/messages.h"
#include "fullpipe/behavior.h"
#include "fullpipe/modal.h"
#include "fullpipe/input.h"
#include "fullpipe/scenes.h"

namespace Fullpipe {

FullpipeEngine *g_fullpipe = 0;
Vars *g_vars = 0;

FullpipeEngine::FullpipeEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	// Setup mixer
	if (!_mixer->isReady()) {
		warning("Sound initialization failed.");
	}

	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	_rnd = new Common::RandomSource("fullpipe");

	_gameProjectVersion = 0;
	_pictureScale = 8;
	_scrollSpeed = 0;
	_currSoundListCount = 0;
	_globalPalette = 0;

	_updateTicks = 0;
	_lastInputTicks = 0;
	_lastButtonUpTicks = 0;

	_currArchive = 0;

	_soundEnabled = true;
	_flgSoundList = true;

	_sfxVolume = 0;

	_inputController = 0;
	_inputDisabled = false;

	_modalObject = 0;

	_gameContinue = true;
	_needRestart = false;
	_flgPlayIntro = false;
	_gamePaused = false;
	_inputArFlag = false;
	_recordEvents = false;

	_flgGameIsRunning = true;

	_isProcessingMessages = false;

	_musicAllowed = -1;

	_aniMan = 0;
	_aniMan2 = 0;
	_currentScene = 0;
	_scene2 = 0;
	_movTable = 0;

	_globalMessageQueueList = 0;
	_messageHandlers = 0;

	_updateScreenCallback = 0;
	_updateCursorCallback = 0;

	_msgX = 0;
	_msgY = 0;
	_msgObjectId2 = 0;
	_msgId = 0;
	_mouseVirtX = 0;
	_mouseVirtY = 0;

	_currSelectedInventoryItemId = 0;

	_behaviorManager = 0;

	_cursorId = 0;

	g_fullpipe = this;
	g_vars = new Vars;
}

FullpipeEngine::~FullpipeEngine() {
	delete _rnd;
	delete _globalMessageQueueList;
}

void FullpipeEngine::initialize() {
	_globalMessageQueueList = new GlobalMessageQueueList;
	_behaviorManager = new BehaviorManager;

	_sceneRect.left = 0;
	_sceneRect.top = 0;
	_sceneRect.right = 799;
	_sceneRect.bottom = 599;
}

Common::Error FullpipeEngine::run() {
	const Graphics::PixelFormat format(2, 5, 6, 5, 0, 11, 5, 0, 0);
	// Initialize backend
	initGraphics(800, 600, true, &format);

	_backgroundSurface.create(800, 600, format);

	initialize();

	_isSaveAllowed = false;

	int scene = 0;
	if (ConfMan.hasKey("boot_param"))
		scene = ConfMan.getInt("boot_param");

	if (!loadGam("fullpipe.gam", scene))
		return Common::kNoGameDataFoundError;

#if 0
	loadAllScenes();
#endif

	_gameContinue = true;

	while (_gameContinue) {
		updateEvents();

		updateScreen();

		if (_needRestart) {
			if (_modalObject) {
				delete _modalObject;
				_modalObject = 0;
			}

			freeGameLoader();
			_currentScene = 0;
			_updateTicks = 0;
			
			loadGam("fullpipe.gam");
			_needRestart = false;
		}

		_system->delayMillis(10);
		_system->updateScreen();

	}

	freeGameLoader();

	cleanup();

	return Common::kNoError;
}

void FullpipeEngine::updateEvents() {
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
							_modalObject->saveload();
							CBaseModalObject *obj = _modalObject->_parentObj;
							if (obj)
								delete _modalObject;
							_modalObject = obj;
						}
					} else {
						_gameLoader->updateSystems(42);
					}
					return;
				}

				ex = new ExCommand(0, 17, 36, 0, 0, 0, 1, 0, 0, 0);
				ex->_keyCode = 32;
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
				ex->_keyCode = 83;
				ex->_excFlags |= 3;
				ex->handle();
				break;
			case Common::KEYCODE_q:
				return;
				break;
			default:
				ex = new ExCommand(0, 17, 36, 0, 0, 0, 1, 0, 0, 0);
				ex->_keyCode = event.kbd.keycode;
				ex->_excFlags |= 3;
				ex->handle();
				break;
			}
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
				ex = new ExCommand(0, 17, 31, event.mouse.x, event.mouse.x, 0, 1, 0, 0, 0);
				ex->_excFlags |= 3;
				ex->handle();
			}

			_mouseScreenPos = event.mouse;
			break;
		case Common::EVENT_QUIT:
			_gameContinue = false;
			break;
			case Common::EVENT_RBUTTONDOWN:
			if (!_inputArFlag && (_updateTicks - _lastInputTicks) >= 2) {
				ex = new ExCommand(0, 17, 107, event.mouse.x, event.mouse.x, 0, 1, 0, 0, 0);
				ex->_excFlags |= 3;
				_lastInputTicks = _updateTicks;
				ex->handle();
			}
			break;
		case Common::EVENT_LBUTTONDOWN:
			if (!_inputArFlag && (_updateTicks - _lastInputTicks) >= 2) {
				ex = new ExCommand(0, 17, 29, event.mouse.x, event.mouse.x, 0, 1, 0, 0, 0);

				ex->_sceneClickX = _sceneRect.left + ex->_x;
				ex->_sceneClickY = _sceneRect.top + ex->_y;
				ex->_keyCode = getGameLoaderInventory()->getSelectedItemId();
				ex->_excFlags |= 3;
				_lastInputTicks = _updateTicks;
				ex->handle();
			}
			break;
		case Common::EVENT_LBUTTONUP:
			if (!_inputArFlag && (_updateTicks - _lastButtonUpTicks) >= 2) {
				ex = new ExCommand(0, 17, 30, 0, 0, 0, 1, 0, 0, 0);
				ex->_excFlags |= 3;
				_lastButtonUpTicks = _updateTicks;
				ex->handle();
			}
			break;
		default:
			break;
		}
	}

		
#if 0
	warning("STUB: FullpipeEngine::updateEvents() <mainWindowProc>");
	if (Msg == MSG_SC11_SHOWSWING && _modalObject) {
		_modalObject->method14();
	}
#endif
}

void FullpipeEngine::freeGameLoader() {
	warning("STUB: FullpipeEngine::freeGameLoader()");
}

void FullpipeEngine::cleanup() {
	warning("STUB: FullpipeEngine::cleanup()");
}

void FullpipeEngine::updateScreen() {
	_mouseVirtX = _mouseScreenPos.x + _sceneRect.left;
	_mouseVirtY = _mouseScreenPos.y + _sceneRect.top;

	//if (inputArFlag)
	//	updateGame_inputArFlag();

	if (_modalObject || _flgGameIsRunning && (_gameLoader->updateSystems(42), _modalObject != 0)) {
		if (_flgGameIsRunning) {
			if (_modalObject->init(42)) {
				_modalObject->update();
			} else {
				_modalObject->saveload();
				CBaseModalObject *tmp = _modalObject->_parentObj;

				if (_modalObject)
					delete _modalObject;

				_modalObject = tmp;
			}
		}
	} else if (_currentScene) {
		//_currentScene->update(42);  // HACK. FIXME
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

int FullpipeEngine::getObjectEnumState(const char *name, const char *state) {
	CGameVar *var = _gameLoader->_gameVar->getSubVarByName("OBJSTATES");

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

int FullpipeEngine::getObjectState(const char *objname) {
	CGameVar *var = _gameLoader->_gameVar->getSubVarByName("OBJSTATES");

	if (var)
		return var->getSubVarAsInt(objname);

  return 0;
}

void FullpipeEngine::setObjectState(const char *name, int state) {
	CGameVar *var = _gameLoader->_gameVar->getSubVarByName("OBJSTATES");

	if (!var) {
		var = _gameLoader->_gameVar->addSubVarAsInt("OBJSTATES", 0);
	}

	var->setSubVarAsInt(name, state);
}

void FullpipeEngine::updateMapPiece(int mapId, int update) {
	for (int i = 0; i < 200; i++) {
		int hiWord = (_mapTable[i] >> 16) & 0xffff;

		if (hiWord == mapId) {
			_mapTable[i] |= update;
			return;
		}
		if (!hiWord) {
			_mapTable[i] = (mapId << 16) | update;
			return;
		}
	}
}

void FullpipeEngine::disableSaves(ExCommand *ex) {
	warning("STUB: FullpipeEngine::disableSaves()");
}


} // End of namespace Fullpipe
