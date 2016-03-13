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

#include "titanic/game_state.h"
#include "titanic/game_manager.h"
#include "titanic/screen_manager.h"

namespace Titanic {

CGameState::CGameState(CGameManager *gameManager) :
		_gameManager(gameManager), _sub(this),
		_field8(0), _fieldC(0), _mode(10), _field14(0), _field18(0),
		_field1C(0), _field20(0), _field24(0), _field28(0), _field2C(0),
		_field30(0), _field34(0), _field38(0) {
}

void CGameState::save(SimpleFile *file) const {
	file->writeNumber(_field18);
	file->writeNumber(_field8);
	file->writeNumber(_fieldC);
	file->writeNumber(_field14);
	file->writeNumber(_field24);
	file->writeNumber(_field38);
	_sub.save(file);
	file->writeNumber(_field1C);
}

void CGameState::load(SimpleFile *file) {
	_field18 = file->readNumber();
	_field8 = file->readNumber();
	_fieldC = file->readNumber();
	_field14 = file->readNumber();
	_field24 = file->readNumber();
	_field38 = file->readNumber();
	_sub.load(file);

	_field1C = file->readNumber();
	_field28 = _field2C = 0;
}

void CGameState::setMode(int newMode) {
	CScreenManager *sm = CScreenManager::_screenManagerPtr;

	if (newMode == 2 && newMode != _mode) {
		if (_gameManager)
			_gameManager->lockInputHandler();

		if (sm && sm->_mouseCursor)
			sm->_mouseCursor->hide();

	} else if (newMode != 2 && newMode != _mode) {
		if (sm && sm->_mouseCursor)
			sm->_mouseCursor->show();
	
		if (_gameManager)
			_gameManager->unlockInputHandler();
	}

	_mode = newMode;
}

} // End of namespace Titanic z
