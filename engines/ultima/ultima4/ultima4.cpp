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

#include "ultima/ultima4/ultima4.h"
#include "ultima/ultima4/conversation/dialogueloader.h"
#include "ultima/ultima4/core/config.h"
#include "ultima/ultima4/core/debugger.h"
#include "ultima/ultima4/core/error.h"
#include "ultima/ultima4/events/event.h"
#include "ultima/ultima4/filesys/savegame.h"
#include "ultima/ultima4/game/context.h"
#include "ultima/ultima4/game/game.h"
#include "ultima/ultima4/game/intro.h"
#include "ultima/ultima4/sound/music.h"
#include "ultima/ultima4/game/person.h"
#include "ultima/ultima4/gfx/screen.h"
#include "ultima/ultima4/core/settings.h"
#include "ultima/ultima4/sound/sound.h"
#include "ultima/ultima4/core/utils.h"
#include "ultima/ultima4/gfx/imageloader.h"
#include "ultima/ultima4/gfx/imagemgr.h"
#include "ultima/ultima4/map/tileset.h"
#include "common/debug.h"
#include "common/system.h"

namespace Ultima {
namespace Ultima4 {

Ultima4Engine *g_ultima;

Ultima4Engine::Ultima4Engine(OSystem *syst, const Ultima::UltimaGameDescription *gameDesc) :
		Shared::UltimaEngine(syst, gameDesc), _saveSlotToLoad(-1), _config(nullptr),
		_context(nullptr), _dialogueLoaders(nullptr), _game(nullptr), _music(nullptr),
		_imageLoaders(nullptr), _saveGame(nullptr), _screen(nullptr) {
	g_ultima = this;
	g_context = nullptr;
	g_game = nullptr;
	g_screen = nullptr;
}

Ultima4Engine::~Ultima4Engine() {
	delete _config;
	delete _context;
	delete _dialogueLoaders;
	delete _game;
	delete _imageLoaders;
	delete _music;
	delete _saveGame;
	delete _screen;

	Tileset::unloadAll();
	ImageMgr::destroy();

	//delete g_music;
	soundDelete();
}

bool Ultima4Engine::initialize() {
	if (!Shared::UltimaEngine::initialize())
		return false;

	// Initialize the sub-systems
	_config = new Config();
	_context = new Context();
	_dialogueLoaders = new DialogueLoaders();
	_screen = new Screen();
	_game = new GameController();
	_imageLoaders = new ImageLoaders();
	_music = new Music();
	_saveGame = new SaveGame();

	_screen->init();
	setDebugger(new Debugger());

	_saveSlotToLoad = ConfMan.hasKey("save_slot") ? ConfMan.getInt("save_slot") : -1;

	return true;
}

void Ultima4Engine::startup() {
	bool skipInfo = _saveSlotToLoad != -1;

	soundInit();
	Tileset::loadAll();
	creatureMgr->getInstance();

	if (!skipInfo) {
		// do the intro
		intro = new IntroController();
		intro->init();
		intro->preloadMap();

		eventHandler->pushController(intro);
		eventHandler->run();
		eventHandler->popController();
		intro->deleteIntro();
	}
}

Common::Error Ultima4Engine::run() {
	if (initialize()) {
		startup();
		if (!shouldQuit()) {
			g_game->init();

			if (_saveSlotToLoad != -1) {
				if (loadGameState(_saveSlotToLoad).getCode() != Common::kNoError)
					error("Error loading save");
			} else {
				_saveGame->newGame();
			}

			eventHandler->setControllerDone(false);
			eventHandler->pushController(g_game);
			eventHandler->run();
			eventHandler->popController();
		}
	}

	return Common::kNoError;
}

bool Ultima4Engine::isDataRequired(Common::String &folder, int &majorVersion, int &minorVersion) {
	folder = "ultima4";
	majorVersion = 1;
	minorVersion = 0;
	return true;
}

void Ultima4Engine::setToJourneyOnwards() {
	_saveSlotToLoad = ConfMan.hasKey("last_save") ? ConfMan.getInt("last_save") : -1;
	assert(_saveSlotToLoad);
}

bool Ultima4Engine::canSaveGameStateCurrently(bool isAutosave) {
	return g_game != nullptr && g_context != nullptr && eventHandler->getController() == g_game;
}

Common::Error Ultima4Engine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	Common::Error result = Shared::UltimaEngine::saveGameState(slot, desc, isAutosave);
	if (!isAutosave && result.getCode() == Common::kNoError) {
		ConfMan.setInt("last_save", slot);
		ConfMan.flushToDisk();
	}

	return result;
}


Common::Error Ultima4Engine::loadGameStream(Common::SeekableReadStream *stream) {
	g_ultima->_saveGame->load(stream);
	screenUpdate(&g_game->_mapArea, true, false);
	return Common::kNoError;
}

Common::Error Ultima4Engine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	g_ultima->_saveGame->save(stream);
	return Common::kNoError;
}

void Ultima4Engine::quitGame() {
	UltimaEngine::quitGame();

	// Do an event poll to all the quit message to be processed
	Common::Event e;
	g_system->getEventManager()->pollEvent(e);
}

} // End of namespace Ultima4
} // End of namespace Ultima
