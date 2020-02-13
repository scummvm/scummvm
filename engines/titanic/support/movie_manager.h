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

#ifndef TITANIC_MOVIE_MANAGER_H
#define TITANIC_MOVIE_MANAGER_H

#include "titanic/core/list.h"
#include "titanic/core/resource_key.h"
#include "titanic/sound/sound_manager.h"

namespace Titanic {

class CMovie;
class CVideoSurface;

class CMovieManagerBase {
public:
	virtual ~CMovieManagerBase() {}

	/**
	 * Create a new movie and return it
	 */
	virtual CMovie *createMovie(const CResourceKey &key, CVideoSurface *surface) = 0;
};

class CMovieManager : public CMovieManagerBase {
private:
	CSoundManager *_soundManager;
public:
	CMovieManager() : CMovieManagerBase(), _soundManager(nullptr) {}
	~CMovieManager() override {}

	/**
	 * Create a new movie and return it
	 */
	CMovie *createMovie(const CResourceKey &key, CVideoSurface *surface) override;

	/**
	 * Sets the sound manager that will be attached to all created movies
	 */
	void setSoundManager(CSoundManager *soundManager) { _soundManager = soundManager; }
};

} // End of namespace Titanic

#endif /* TITANIC_MOVIE_MANAGER_H */
