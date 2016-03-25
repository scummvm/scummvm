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

void CSound::preEnterView(CViewItem *newView, bool isNewRoom) {
	warning("CSound::preEnterView");
}

bool CSound::fn1(int val) {
	if (val == 0 || val == -1) {
		if (!_soundManager.proc14())
			return true;
	}

	return false;
}

void CSound::fn2(int val) {
	warning("TODO: CSound::fn3");
}

void CSound::fn3(int val, int val2, int val3) {
	warning("TODO: CSound::fn3");
}

} // End of namespace Titanic z
