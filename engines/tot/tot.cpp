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

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "engines/tot/util.h"
#include "engines/util.h"
#include "graphics/framelimiter.h"
#include "graphics/paletteman.h"

#include "tot/chrono.h"
#include "tot/console.h"
#include "tot/detection.h"
#include "tot/font/bgifont.h"
#include "tot/graphman.h"
#include "tot/mouse.h"
#include "tot/playanim.h"
#include "tot/routines.h"
#include "tot/routines2.h"
#include "tot/soundman.h"
#include "tot/tot.h"

namespace Tot {

TotEngine *g_engine;

TotEngine::TotEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
																		 _gameDescription(gameDesc), _randomSource("Tot") {
	g_engine = this;

	_lang = _gameDescription->language;
}

TotEngine::~TotEngine() {
	clearGame();
	delete _screen;
	delete _graphics;
	delete _sound;
	delete _chrono;
}

uint32 TotEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String TotEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Error TotEngine::run() {
	// Initialize 320x200 paletted graphics mode
	initGraphics(320, 200);

	ConfMan.registerDefault("introSeen", false);
	//Static initializations

	_screen = new Graphics::Screen();
	_graphics = new GraphicsManager();
	_sound = new SoundManager(_mixer);
	_chrono = new ChronoManager();
	// _room = new RoomManager();
	_mouseManager = new MouseManager();

	initSound();
	initGraph();
	initPlayAnim();

	introSeen = ConfMan.getBool("introSeen");
	// _inventoryManager = new InventoryManager();
	// _totGame = new TotGame();
	// Set the engine's debugger console
	setDebugger(new TotConsole(this));

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);

	engine_start();

	return Common::kNoError;
}

} // End of namespace Tot
