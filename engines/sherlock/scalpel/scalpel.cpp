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
	if (!_events->isKeyPressed())
		showCityCutscene();
	if (!_events->isKeyPressed())
		showAlleyCutscene();
	if (!_events->isKeyPressed())
		showStreetCutscene();
	if (!_events->isKeyPressed())
		showOfficeCutscene();

	_events->clearEvents();
	_sound->stopMusic();
}

void ScalpelEngine::showCityCutscene() {
	byte palette[PALETTE_SIZE];
	
	_sound->playMusic("prolog1.mus");
	_animation->_titleOverride = "title.lib";
	_animation->_soundOverride = "title.snd";
	_animation->playPrologue("26open1", 1, 255, true, 2);

	// TODO
}

void ScalpelEngine::showAlleyCutscene() {

}

void ScalpelEngine::showStreetCutscene() {

}

void ScalpelEngine::showOfficeCutscene() {

}


} // End of namespace Scalpel

} // End of namespace Scalpel
