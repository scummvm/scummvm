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

#include "titanic/support/movie_manager.h"
#include "titanic/support/movie.h"
#include "titanic/support/video_surface.h"

namespace Titanic {

CMovie *CMovieManager::createMovie(const CResourceKey &key, CVideoSurface *surface) {
	CMovie *movie = new OSMovie(key, surface);
	movie->setSoundManager(_soundManager);
	return movie;
}

} // End of namespace Titanic
