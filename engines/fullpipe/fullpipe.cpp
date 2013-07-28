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

namespace Fullpipe {

FullpipeEngine *g_fullpipe = 0;

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

	_currArchive = 0;

	_soundEnabled = true;
	_flgSoundList = true;

	_inputController = 0;
	_inputDisabled = false;

	_needQuit = false;
	_flgPlayIntro = true;

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

	_behaviorManager = 0;

	_cursorId = 0;

	g_fullpipe = this;
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

	loadGam("fullpipe.gam");

	EntranceInfo ent;

	if (ConfMan.hasKey("boot_param"))
		ent._sceneId = ConfMan.getInt("boot_param");
	else 
		ent._sceneId = 3896;

	sceneSwitcher(&ent);

#if 1
	loadAllScenes();
#endif

	_currentScene->draw();

	while (!g_fullpipe->_needQuit) {
		updateEvents();
		_system->delayMillis(10);
		_system->updateScreen();

		switch (g_fullpipe->_keyState) {
		case Common::KEYCODE_q:
			g_fullpipe->_needQuit = true;
			break;
		case Common::KEYCODE_UP:
			_sceneRect.moveTo(_sceneRect.left, _sceneRect.top + 10);
			_currentScene->draw();
			g_fullpipe->_keyState = Common::KEYCODE_INVALID;
			break;
		case Common::KEYCODE_DOWN:
			_sceneRect.moveTo(_sceneRect.left, _sceneRect.top - 10);
			_currentScene->draw();
			g_fullpipe->_keyState = Common::KEYCODE_INVALID;
			break;
		case Common::KEYCODE_LEFT:
			_sceneRect.moveTo(_sceneRect.left + 10, _sceneRect.top);
			_currentScene->draw();
			g_fullpipe->_keyState = Common::KEYCODE_INVALID;
			break;
		case Common::KEYCODE_RIGHT:
			_sceneRect.moveTo(_sceneRect.left - 10, _sceneRect.top);
			_currentScene->draw();
			g_fullpipe->_keyState = Common::KEYCODE_INVALID;
			break;
		case Common::KEYCODE_z:
			_sceneRect.moveTo(0, 0);
			_currentScene->draw();
			g_fullpipe->_keyState = Common::KEYCODE_INVALID;
			break;
		default:
			break;
		}
	}


	return Common::kNoError;
}

void FullpipeEngine::updateEvents() {
	Common::Event event;
	Common::EventManager *eventMan = _system->getEventManager();

	while (eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			_keyState = event.kbd.keycode;
			break;
		case Common::EVENT_KEYUP:
			_keyState = Common::KEYCODE_INVALID;
			break;
		case Common::EVENT_MOUSEMOVE:
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y;
			break;
		case Common::EVENT_QUIT:
			_needQuit = true;
			break;
		default:
			break;
		}
	}
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

} // End of namespace Fullpipe
