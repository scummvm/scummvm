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
 * $URL$
 * $Id$
 *
 */

#include "asylum/system/savegame.h"

#include "asylum/asylum.h"

namespace Asylum {

Savegame::Savegame(AsylumEngine *engine) : _vm(engine) {
	memset(&_moviesViewed, 0, sizeof(_moviesViewed));
}

Savegame::~Savegame() {
}

bool Savegame::setup() {
	warning("[Savegame::setup] Not implemented!");

	return true;
}

void Savegame::loadList() {
	warning("[Savegame::loadList] Not implemented!");
}


bool Savegame::load() {
	warning("[Savegame::load] Not implemented!");

	return false;
}

bool Savegame::quickLoad() {
	warning("[Savegame::quickLoad] Not implemented!");

	return false;
}

bool Savegame::save() {
	warning("[Savegame::save] Not implemented!");

	return false;
}

bool Savegame::quickSave() {
	warning("[Savegame::quickSave] Not implemented!");

	return false;
}

void Savegame::setMovieViewed(uint32 index) {
	if (index >= ARRAYSIZE(_moviesViewed))
		error("[Savegame::setMovieViewed] Invalid movie index!");

	if (!_moviesViewed[index]) {
		_moviesViewed[index] = 1;

		// TODO Save to disk
		warning("[Savegame::setMovieViewed] Not implemented!");
	}
}

uint32 Savegame::getMoviesViewed(byte *movieList) {
	memset(movieList, -1, 196);

	uint32 count = 0;

	for (uint32 i = 0; i < ARRAYSIZE(_moviesViewed); i++) {
		if (_moviesViewed[i]) {
			movieList[i] = i;
			++count;
		}
	}

	return count;
}

void Savegame::loadViewedMovies() {
	warning("[Savegame::loadViewedMovies] Not implemented!");
}


} // End of namespace Asylum
