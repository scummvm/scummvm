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
#include "mads/game.h"
#include "mads/screen.h"
#include "mads/msurface.h"
#include "mads/resources.h"
#include "mads/sound.h"
#include "mads/sprites.h"

namespace MADS {

MADSEngine::MADSEngine(OSystem *syst, const MADSGameDescription *gameDesc) :
		_gameDescription(gameDesc), Engine(syst), _randomSource("MADS") {

	// Initialize fields
	_easyMouse = true;
	_invObjectsAnimated = true;
	_textWindowStill = false;
	_screenFade = SCREEN_FADE_SMOOTH;
	_musicFlag = true;
	_dithering = false;

	_debugger = nullptr;
	_dialogs = nullptr;
	_events = nullptr;
	_font = nullptr;
	_game = nullptr;
	_palette = nullptr;
	_resources = nullptr;
	_sound = nullptr;
	_audio = nullptr;
}

MADSEngine::~MADSEngine() {
	delete _debugger;
	delete _dialogs;
	delete _events;
	delete _font;
	Font::deinit();
	delete _game;
	delete _palette;
	delete _resources;
	delete _sound;
	delete _audio;
}

void MADSEngine::initialize() {
	// Set up debug channels
	DebugMan.addDebugChannel(kDebugPath, "Path", "Pathfinding debug level");
	DebugMan.addDebugChannel(kDebugScripts, "scripts", "Game scripts");
	DebugMan.addDebugChannel(kDebugGraphics, "graphics", "Graphics handling");

	// Initial sub-system engine references
	MSurface::setVm(this);
	MSprite::setVm(this);

	Resources::init(this);
	Conversation::init(this);
	_debugger = new Debugger(this);
	_dialogs = Dialogs::init(this);
	_events = new EventsManager(this);
	_palette = new Palette(this);
	Font::init(this);
	_font = new Font();
	_screen.init();
	_sound = new SoundManager(this, _mixer);
	_audio = new AudioPlayer(_mixer, getGameID());
	_game = Game::init(this);

	_screen.empty();
}

Common::Error MADSEngine::run() {
	initGraphics(MADS_SCREEN_WIDTH, MADS_SCREEN_HEIGHT, false);
	initialize();

	// Run the game
	_game->run();

	// Dummy loop to keep application active
	_events->delay(9999);

	return Common::kNoError;
}

int MADSEngine::getRandomNumber(int maxNumber) {
	return _randomSource.getRandomNumber(maxNumber);
}

int MADSEngine::getRandomNumber(int minNumber, int maxNumber) {
	int range = maxNumber - minNumber;

	return minNumber + _randomSource.getRandomNumber(range);
}

int MADSEngine::hypotenuse(int xv, int yv) {
	return (int)sqrt((double)(xv * xv + yv * yv));
}

bool MADSEngine::canLoadGameStateCurrently() {
	return !_game->_winStatus && !_game->globals()[5]
		&& _dialogs->_pendingDialog == DIALOG_NONE
		&& _events->_cursorId != CURSOR_WAIT;
}

bool MADSEngine::canSaveGameStateCurrently() {
	return !_game->_winStatus && !_game->globals()[5]
		&& _dialogs->_pendingDialog == DIALOG_NONE
		&& _events->_cursorId != CURSOR_WAIT;
}

/**
* Support method that generates a savegame name
* @param slot		Slot number
*/
Common::String MADSEngine::generateSaveName(int slot) {
	return Common::String::format("%s.%03d", _targetName.c_str(), slot);
}

Common::Error MADSEngine::loadGameState(int slot) {
	_game->_loadGameSlot = slot;
	_game->_scene._currentSceneId = -1;
	_game->_currentSectionNumber = -1;
	return Common::kNoError;
}

Common::Error MADSEngine::saveGameState(int slot, const Common::String &desc) {
	_game->saveGame(slot, desc);
	return Common::kNoError;
}

} // End of namespace MADS
