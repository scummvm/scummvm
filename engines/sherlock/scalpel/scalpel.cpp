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

#include "sherlock/scalpel/scalpel.h"
#include "sherlock/sherlock.h"

namespace Sherlock {

namespace Scalpel {

/**
 * Game initialization
 */
void ScalpelEngine::initialize() {
	SherlockEngine::initialize();

	_flags.resize(100 * 8);
	_flags[3] = true;		// Turn on Alley
	_flags[39] = true;		// Turn on Baker Street

	// Starting room
	_rooms->_goToRoom = 4;
}

/**
 * Show the opening sequence
 */
void ScalpelEngine::showOpening() {
	if (!showCityCutscene())
		return;
	if (!showAlleyCutscene())
		return;
	if (!showStreetCutscene())
		return;
	if (!showOfficeCutscene())
		return;

	_events->clearEvents();
	_sound->stopMusic();
}

bool ScalpelEngine::showCityCutscene() {
	byte palette[PALETTE_SIZE];
	
	_sound->playMusic("prolog1.mus");
	_titleOverride = "title.lib";
	_soundOverride = "title.snd";
	bool finished = _animation->playPrologue("26open1", 1, 255, true, 2);

	if (finished) {
		Sprite titleSprites("title2.vgs", true);
		_screen->_backBuffer.blitFrom(*_screen);
		_screen->_backBuffer2.blitFrom(*_screen);

		// London, England
		_screen->_backBuffer.transBlitFrom(titleSprites[0], Common::Point(10, 11));
		_screen->randomTransition();
		finished = _events->delay(1000, true);

		// November, 1888
		if (finished) {
			_screen->_backBuffer.transBlitFrom(titleSprites[1], Common::Point(101, 102));
			_screen->randomTransition();
			finished = _events->delay(5000, true);
		}

		// Transition out the title
		_screen->_backBuffer.blitFrom(_screen->_backBuffer2);
		_screen->randomTransition();
	}

	if (finished)
		finished = _animation->playPrologue("26open2", 1, 0, false, 2);

	if (finished) {
		Sprite titleSprites("title.vgs", true);
		_screen->_backBuffer.blitFrom(*_screen);
		_screen->_backBuffer2.blitFrom(*_screen);
		
		// The Lost Files of
		_screen->_backBuffer.transBlitFrom(titleSprites[0], Common::Point(75, 6));
		// Sherlock Holmes
		_screen->_backBuffer.transBlitFrom(titleSprites[1], Common::Point(34, 21));
		// copyright
		_screen->_backBuffer.transBlitFrom(titleSprites[2], Common::Point(4, 190));

		_screen->verticalTransition();
		finished = _events->delay(4000, true);

		if (finished) {
			_screen->_backBuffer.blitFrom(_screen->_backBuffer2);
			_screen->randomTransition();
			finished = _events->delay(2000);
		}

		if (finished) {
			_screen->getPalette(palette);
			_screen->fadeToBlack(2);
		}

		if (finished) {
			// In the alley...
			_screen->transBlitFrom(titleSprites[3], Common::Point(72, 51));
			_screen->fadeIn(palette, 3);
			finished = _events->delay(3000, true);
		}
	}

	_titleOverride = "";
	_soundOverride = "";
	return finished;
}

bool ScalpelEngine::showAlleyCutscene() {
	return true;
}

bool ScalpelEngine::showStreetCutscene() {
	return true;
}

bool ScalpelEngine::showOfficeCutscene() {
	return true;
}

} // End of namespace Scalpel

} // End of namespace Scalpel
