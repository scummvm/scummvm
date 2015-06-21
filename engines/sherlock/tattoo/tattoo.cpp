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

#include "engines/util.h"
#include "sherlock/tattoo/tattoo.h"
#include "sherlock/tattoo/tattoo_resources.h"
#include "sherlock/tattoo/tattoo_scene.h"
#include "sherlock/tattoo/widget_base.h"
#include "sherlock/people.h"

namespace Sherlock {

namespace Tattoo {

TattooEngine::TattooEngine(OSystem *syst, const SherlockGameDescription *gameDesc) :
		SherlockEngine(syst, gameDesc) {
	_creditsActive = false;
	_runningProlog = false;
	_fastMode = false;
	_allowFastMode = true;
	_transparentMenus = true;
}

TattooEngine::~TattooEngine() {
}

void TattooEngine::showOpening() {
	// TODO
}

void TattooEngine::initialize() {
	initGraphics(640, 480, true);

	// Initialize the base engine
	SherlockEngine::initialize();

	// Initialise the global flags
	_flags.resize(3200);
	_flags[1] = _flags[4] = _flags[76] = true;
	_runningProlog = true;

	// Add some more files to the cache
	_res->addToCache("walk.lib");
	
	// Set up list of people
	for (int idx = 0; idx < TATTOO_MAX_PEOPLE; ++idx) {
		_people->_characters.push_back(PersonData(
			getLanguage() == Common::FR_FRA ? FRENCH_NAMES[idx] : ENGLISH_NAMES[idx],
			PORTRAITS[idx], nullptr, nullptr));
	}

	// Starting scene
	_scene->_goToScene = STARTING_INTRO_SCENE;

	// Load an initial palette
	loadInitialPalette();
}

void TattooEngine::startScene() {
	if (_scene->_goToScene == OVERHEAD_MAP || _scene->_goToScene == OVERHEAD_MAP2) {
		// Show the map
		_scene->_currentScene = OVERHEAD_MAP;
		_scene->_goToScene = _map->show();

		_people->_hSavedPos = Common::Point(-1, -1);
		_people->_hSavedFacing = -1;
	}

	// TODO
}

void TattooEngine::loadInitialPalette() {
	byte palette[768];
	Common::SeekableReadStream *stream = _res->load("room.pal");
	stream->read(palette, PALETTE_SIZE);
	_screen->translatePalette(palette);
	_screen->setPalette(palette);

	delete stream;
}

void TattooEngine::drawCredits() {
	// TODO
}

void TattooEngine::blitCredits() {
	// TODO
}

void TattooEngine::eraseCredits() {
	// TODO
}

void TattooEngine::doHangManPuzzle() {
	// TODO
}

} // End of namespace Tattoo

} // End of namespace Sherlock
