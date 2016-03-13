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

#include "titanic/input_handler.h"
#include "titanic/screen_manager.h"

namespace Titanic {

CInputHandler::CInputHandler(CGameManager *owner) :
		_gameManager(owner), _inputTranslator(nullptr),
		_field4(0), _field8(0), _fieldC(0), _field10(0), _field14(0),
		_lockCount(0), _field24(0) {
	CScreenManager::_screenManagerPtr->_inputHandler = this;
}

void CInputHandler::setTranslator(CInputTranslator *translator) {
	_inputTranslator = translator;
}

void CInputHandler::incLockCount() {
	++_lockCount;
}

void CInputHandler::decLockCount() {
	if (--_lockCount == 0 && _inputTranslator) {
		warning("TODO");
	}
}

} // End of namespace Titanic z
