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

#include "sherlock/tattoo/tattoo.h"
#include "engines/util.h"

namespace Sherlock {

namespace Tattoo {

TattooEngine::TattooEngine(OSystem *syst, const SherlockGameDescription *gameDesc) :
		SherlockEngine(syst, gameDesc) {
	_creditsActive = false;
}

void TattooEngine::showOpening() {
	// TODO
}

void TattooEngine::initialize() {
	initGraphics(640, 480, true);

	// Initialize the base engine
	SherlockEngine::initialize();

	_flags.resize(100 * 8);

	// Add some more files to the cache
	_res->addToCache("walk.lib");

	// Starting scene
	_scene->_goToScene = 91;

	// Load an initial palette
	loadInitialPalette();
}

void TattooEngine::startScene() {
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

} // End of namespace Tattoo

} // End of namespace Sherlock
