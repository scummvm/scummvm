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
#include "titanic/titanic.h"
#include "titanic/game_manager.h"
#include "titanic/screen_manager.h"

namespace Titanic {

CGameState::CGameState(CGameManager *gameManager) :
		_gameManager(gameManager), _gameLocation(this),
		_field8(0), _fieldC(0), _mode(GSMODE_0), _field14(0), _field18(0),
		_field1C(0), _field20(0), _field24(0), _nodeChangeCtr(0),
		_nodeEnterTicks(0), _field38(0) {
}

void CGameState::save(SimpleFile *file) const {
	file->writeNumber(_field18);
	file->writeNumber(_field8);
	file->writeNumber(_fieldC);
	file->writeNumber(_field14);
	file->writeNumber(_field24);
	file->writeNumber(_field38);
	_gameLocation.save(file);
	file->writeNumber(_field1C);
}

void CGameState::load(SimpleFile *file) {
	_field18 = file->readNumber();
	_field8 = file->readNumber();
	_fieldC = file->readNumber();
	_field14 = file->readNumber();
	_field24 = file->readNumber();
	_field38 = file->readNumber();
	_gameLocation.load(file);

	_field1C = file->readNumber();
	_nodeChangeCtr = 0;
	_nodeEnterTicks = 0;
}

void CGameState::setMode(GameStateMode newMode) {
	CScreenManager *sm = CScreenManager::_screenManagerPtr;

	if (newMode == GSMODE_2 && newMode != _mode) {
		if (_gameManager)
			_gameManager->lockInputHandler();

		if (sm && sm->_mouseCursor)
			sm->_mouseCursor->hide();

	} else if (newMode != GSMODE_2 && newMode != _mode) {
		if (sm && sm->_mouseCursor)
			sm->_mouseCursor->show();
	
		if (_gameManager)
			_gameManager->unlockInputHandler();
	}

	_mode = newMode;
}

void CGameState::setMousePos(const Common::Point &pt) {
	_mousePos = pt;
}

void CGameState::enterNode() {
	++_nodeChangeCtr;
	_nodeEnterTicks = g_vm->_events->getTicksCount();
}

} // End of namespace Titanic z
