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
#include "common/savefile.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/paletteman.h"
#include "ultima/ultima1/ultima1.h"
#include "ultima/ultima1/console.h"
#include "ultima/ultima1/data/tiles.h"
#include "ultima/ultima1/views/views.h"

namespace Ultima {
namespace Ultima1 {

Ultima1Engine *g_engine;

Ultima1Engine::Ultima1Engine(OSystem *syst, const Ultima::UltimaGameDescription *gameDesc) : Engine(syst),
_gameDescription(gameDesc) {
	g_engine = this;
}


Ultima1Engine::~Ultima1Engine() {
}

uint32 Ultima1Engine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::String Ultima1Engine::getGameId() const {
	return _gameDescription->desc.gameId;
}

Common::Error Ultima1Engine::run() {
	// Initialize 320x200 paletted graphics mode
	initGraphics(320, 200);
	g_system->getPaletteManager()->setPalette(Graphics::Palette::createEGAPalette());

	// Set the engine's debugger console
	setDebugger(new Console());

	// Load the overworld tile sheet
	Data::loadTiles(_tiles);

	Views::Views views;
	addView("Startup");
	runGame(views);

	return Common::kNoError;
}

Common::Error Ultima1Engine::syncGame(Common::Serializer &s) {
	_player.synchronize(s);

	if (s.isLoading())
		_game.startGame();

	return Common::kNoError;
}

bool Ultima1Engine::savegamesExist() const {
	Common::String slotName = getSaveStateName(1);
	Common::InSaveFile *saveFile = g_system->getSavefileManager()->openForLoading(slotName);
	bool result = saveFile != nullptr;

	delete saveFile;
	return result;
}

} // namespace Ultima1
} // namespace Ultima
