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

#ifndef TITANIC_SOUND_H
#define TITANIC_SOUND_H

#include "titanic/simple_file.h"
#include "titanic/sound/sound_manager.h"

namespace Titanic {

class CGameManager;

class CSound {
private:
	CGameManager *_gameManager;
	QSoundManager _soundManager;
public:
	CSound(CGameManager *owner);

	/**
	 * Save the data for the class to file
	 */
	void save(SimpleFile *file) const;

	/**
	 * Load the data for the class from file
	 */
	void load(SimpleFile *file);

	/**
	 * Called when a game is about to be loaded
	 */
	void preLoad();

	/**
	 * Called when loading a game is complete
	 */
	void postLoad() { _soundManager.postLoad(); }

	/**
	 * Called when a game is about to be saved
	 */
	void preSave() { _soundManager.preSave(); }

	/**
	 * Called when a game has finished being saved
	 */
	void postSave() { _soundManager.postSave(); }
};

} // End of namespace Titanic

#endif /* TITANIC_SOUND_H */
