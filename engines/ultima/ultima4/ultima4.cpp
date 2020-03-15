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
#include "ultima/ultima4/config.h"
#include "ultima/ultima4/debug.h"
#include "ultima/ultima4/error.h"
#include "ultima/ultima4/event.h"
#include "ultima/ultima4/game.h"
#include "ultima/ultima4/intro.h"
#include "ultima/ultima4/music.h"
#include "ultima/ultima4/person.h"
#include "ultima/ultima4/progress_bar.h"
#include "ultima/ultima4/screen.h"
#include "ultima/ultima4/settings.h"
#include "ultima/ultima4/sound.h"
#include "ultima/ultima4/tileset.h"
#include "ultima/ultima4/utils.h"
#include "common/debug.h"

namespace Ultima {
namespace Ultima4 {

bool quit = false, verbose = false;

Ultima4Engine::Ultima4Engine(OSystem *syst, const Ultima::UltimaGameDescription *gameDesc) :
	Shared::UltimaEngine(syst, gameDesc), _config(nullptr), _game(nullptr), _screen(nullptr) {
}

Ultima4Engine::~Ultima4Engine() {
	delete _config;
	delete _game;
	delete _screen;

	Tileset::unloadAll();

	delete musicMgr;
	soundDelete();
}

bool Ultima4Engine::initialize() {
	if (!Shared::UltimaEngine::initialize())
		return false;

	// Initialize the sub-systems
	_config = new Config();
	_game = new GameController();
	_screen = new Screen();
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
	screenRedrawScreen();
	++pb;

	soundInit();
	++pb;

	Tileset::loadAll();
	++pb;

	creatureMgr->getInstance();
	++pb;

	intro = new IntroController();
	if (!skipInfo) {
		/* do the intro */
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

} // End of namespace Ultima4
} // End of namespace Ultima
