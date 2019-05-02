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
#include "titanic/input_translator.h"
#include "titanic/events.h"
#include "titanic/messages/mouse_messages.h"
#include "titanic/titanic.h"

namespace Titanic {

CInputTranslator::CInputTranslator(CInputHandler *inputHandler) :
		_inputHandler(inputHandler) {
	inputHandler->setTranslator(this);
}

int CInputTranslator::getButtons(int special) const {
	int buttons = 0;
	if (special & MK_LBUTTON)
		buttons |= MB_LEFT;
	if (special & MK_MBUTTON)
		buttons |= MB_MIDDLE;
	if (special & MK_RBUTTON)
		buttons |= MB_RIGHT;

	return buttons;
}

void CInputTranslator::mouseMove(int special, const Point &pt) {
	CMouseMoveMsg msg(pt, getButtons(special));
	_inputHandler->handleMessage(msg);
}

void CInputTranslator::leftButtonDown(int special, const Point &pt) {
	CMouseButtonDownMsg msg(pt, MB_LEFT);
	_inputHandler->handleMessage(msg);
}

void CInputTranslator::leftButtonUp(int special, const Point &pt) {
	CMouseButtonUpMsg msg(pt, MB_LEFT);
	_inputHandler->handleMessage(msg);
}

void CInputTranslator::leftButtonDoubleClick(int special, const Point &pt) {
	CMouseDoubleClickMsg msg(pt, MB_LEFT);
	_inputHandler->handleMessage(msg);
}

void CInputTranslator::middleButtonDown(int special, const Point &pt) {
	CMouseButtonDownMsg msg(pt, MB_MIDDLE);
	_inputHandler->handleMessage(msg);
}

void CInputTranslator::middleButtonUp(int special, const Point &pt) {
	CMouseButtonUpMsg msg(pt, MB_MIDDLE);
	_inputHandler->handleMessage(msg);
}

void CInputTranslator::middleButtonDoubleClick(int special, const Point &pt) {
	CMouseDoubleClickMsg msg(pt, MB_MIDDLE);
	_inputHandler->handleMessage(msg);
}

void CInputTranslator::mouseWheel(bool wheelUp, const Point &pt) {
	CMouseWheelMsg msg(pt, wheelUp);
	_inputHandler->handleMessage(msg);
}

void CInputTranslator::keyDown(const Common::KeyState &keyState) {
	if (keyState.ascii > 0 && keyState.ascii <= 127) {
		CKeyCharMsg msg(keyState.ascii);
		if (_inputHandler->handleMessage(msg))
			return;
	}

	if (CMovementMsg::getMovement(keyState.keycode) != MOVE_NONE) {
		CMovementMsg msg(keyState.keycode);
		if (_inputHandler->handleMessage(msg))
			return;
	}

	if (isSpecialKey(keyState.keycode)) {
		CVirtualKeyCharMsg msg(keyState);
		msg._keyState.ascii = 0;
		_inputHandler->handleMessage(msg);
	}
}

bool CInputTranslator::isMousePressed() const {
	return g_vm->_events->getSpecialButtons() & (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON);
}

bool CInputTranslator::isSpecialKey(Common::KeyCode key) {
	if ((key >= Common::KEYCODE_F1 && key <= Common::KEYCODE_F8) ||
		(key >= Common::KEYCODE_KP1 && key <= Common::KEYCODE_KP9))
		return true;

	if (key == Common::KEYCODE_PAGEUP || key == Common::KEYCODE_PAGEDOWN ||
		key == Common::KEYCODE_HOME || key == Common::KEYCODE_END ||
		key == Common::KEYCODE_LEFT || key == Common::KEYCODE_RIGHT ||
		key == Common::KEYCODE_UP || key == Common::KEYCODE_DOWN)
		return true;

	return false;
}

} // End of namespace Titanic
