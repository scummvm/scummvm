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

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "engines/util.h"
#include "mads/mads.h"
#include "mads/graphics.h"
#include "mads/resources.h"
#include "mads/sound.h"
#include "mads/msurface.h"
#include "mads/msprite.h"

namespace MADS {

MADSEngine::MADSEngine(OSystem *syst, const MADSGameDescription *gameDesc) :
		_gameDescription(gameDesc), Engine(syst), _randomSource("MADS") {
	
	// Initialise fields
	_easyMouse = true;
	_invObjectStill = false;
	_textWindowStill = false;

	_debugger = nullptr;
	_dialogs = nullptr;
	_events = nullptr;
	_font = nullptr;
	_game = nullptr;
	_palette = nullptr;
	_resources = nullptr;
	_screen = nullptr;
	_sound = nullptr;
	_userInterface = nullptr;
}

MADSEngine::~MADSEngine() {
	delete _debugger;
	delete _dialogs;
	delete _events;
	delete _font;
	delete _game;
	delete _palette;
	delete _resources;
	delete _screen;
	delete _sound;
	delete _userInterface;
}

void MADSEngine::initialise() {
	// Set up debug channels
	DebugMan.addDebugChannel(kDebugPath, "Path", "Pathfinding debug level");
	DebugMan.addDebugChannel(kDebugScripts, "scripts", "Game scripts");
	DebugMan.addDebugChannel(kDebugGraphics, "graphics", "Graphics handling");

	// Initial sub-system engine references
	MSurface::setVm(this);
	MSprite::setVm(this);

	Resources::init(this);
	_debugger = new Debugger(this);
	_dialogs = Dialogs::init(this);
	_events = new EventsManager(this);
	_palette = new Palette(this);
	_font = new Font(this);
	_screen = new MSurface(g_system->getWidth(), g_system->getHeight());
	_sound = new SoundManager(this, _mixer);
	_userInterface = UserInterface::init(this);
	_game = Game::init(this);

	_events->loadCursors("*CURSOR.SS");
	_screen->empty();
}

Common::Error MADSEngine::run() {
	initGraphics(MADS_SCREEN_WIDTH, MADS_SCREEN_HEIGHT, false);
	initialise();

	// Run the game
	_game->run();

	// Dummy loop to keep application active
	_events->delay(9999);

	return Common::kNoError;
}

int MADSEngine::getRandomNumber(int maxNumber) {
	return _randomSource.getRandomNumber(maxNumber);
}

} // End of namespace MADS
