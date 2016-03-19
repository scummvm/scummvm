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

#include "titanic/sound/sound.h"
#include "titanic/game_manager.h"

namespace Titanic {

CSound::CSound(CGameManager *owner) : _gameManager(owner) {
}

void CSound::save(SimpleFile *file) const {
	_soundManager.save(file);
}

void CSound::load(SimpleFile *file) {
	_soundManager.load(file);
}

void CSound::preLoad() {
	_soundManager.preLoad();

	if (_gameManager)
		_gameManager->_musicRoom.preLoad();
}

} // End of namespace Titanic z
