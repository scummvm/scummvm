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
#include "ultima/ultima4/core/config.h"
#include "ultima/ultima4/core/debug.h"
#include "ultima/ultima4/core/error.h"
#include "ultima/ultima4/events/event.h"
#include "ultima/ultima4/game/context.h"
#include "ultima/ultima4/game/game.h"
#include "ultima/ultima4/game/intro.h"
#include "ultima/ultima4/sound/music.h"
#include "ultima/ultima4/game/person.h"
#include "ultima/ultima4/game/progress_bar.h"
#include "ultima/ultima4/gfx/screen.h"
#include "ultima/ultima4/core/settings.h"
#include "ultima/ultima4/sound/sound.h"
#include "ultima/ultima4/core/utils.h"
#include "ultima/ultima4/gfx/imageloader.h"
#include "ultima/ultima4/gfx/imagemgr.h"
#include "ultima/ultima4/map/tileset.h"
#include "common/debug.h"

namespace Ultima {
namespace Ultima4 {

bool quit = false, verbose = false;
Ultima4Engine *g_ultima;

Ultima4Engine::Ultima4Engine(OSystem *syst, const Ultima::UltimaGameDescription *gameDesc) :
	Shared::UltimaEngine(syst, gameDesc), _config(nullptr), _game(nullptr),
	_imageLoaders(nullptr), _screen(nullptr) {
	g_ultima = this;
	g_context = nullptr;
	g_game = nullptr;
	g_screen = nullptr;
}

Ultima4Engine::~Ultima4Engine() {
	delete _config;
	delete _game;
	delete _imageLoaders;
	delete _screen;

	Tileset::unloadAll();
	ImageMgr::destroy();

	//delete musicMgr;
	soundDelete();
}

bool Ultima4Engine::initialize() {
	if (!Shared::UltimaEngine::initialize())
		return false;

	// Initialize the sub-systems
	_config = new Config();
	_screen = new Screen();
	_game = new GameController();
	_imageLoaders = new ImageLoaders();
	_screen->init();

	return true;
}

void Ultima4Engine::startup() {
	int saveSlot = ConfMan.hasKey("save_slot") ? ConfMan.getInt("save_slot") : -1;
	bool skipInfo = saveSlot != -1;

	ProgressBar pb((320 / 2) - (200 / 2), (200 / 2), 200, 10, 0, (skipInfo ? 4 : 7));
	pb.setBorderColor(240, 240, 240);
	pb.setColor(0, 0, 128);
	pb.setBorderWidth(1);

	screenTextAt(15, 11, "Loading...");
	g_screen->update();
	++pb;

	soundInit();
	++pb;

	Tileset::loadAll();
	++pb;

	creatureMgr->getInstance();
	++pb;

	if (!skipInfo) {
		/* do the intro */
		intro = new IntroController();
		intro->init();
		++pb;

		intro->preloadMap();
		++pb;

		musicMgr->init();
		++pb;

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

			int saveSlot = ConfMan.hasKey("save_slot") ? ConfMan.getInt("save_slot") : -1;
			if (saveSlot != -1) {
				if (loadGameState(saveSlot).getCode() != Common::kNoError)
					error("Error loading save");
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

bool Ultima4Engine::canSaveGameStateCurrently(bool isAutosave) {
	return g_game != nullptr && g_context != nullptr && eventHandler->getController() == g_game;
}

Common::Error Ultima4Engine::loadGameStream(Common::SeekableReadStream *stream) {
	Common::Serializer ser(stream, nullptr);
	g_context->_saveGame->synchronize(ser);

	return Common::kNoError;
}

Common::Error Ultima4Engine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	Common::Serializer ser(nullptr, stream);
	g_context->_saveGame->synchronize(ser);
	return Common::kNoError;
}

} // End of namespace Ultima4
} // End of namespace Ultima
