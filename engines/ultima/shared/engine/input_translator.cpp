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

#include "ultima/shared/engine/input_handler.h"
#include "ultima/shared/engine/input_translator.h"
#include "ultima/shared/engine/events.h"
#include "ultima/shared/engine/messages.h"
#include "ultima/shared/early/ultima_early.h"

namespace Ultima {
namespace Shared {

InputTranslator::InputTranslator(InputHandler *inputHandler) :
	_inputHandler(inputHandler) {
	inputHandler->setTranslator(this);
}

int InputTranslator::getButtons(int special) const {
	int buttons = 0;
	if (special & MK_LBUTTON)
		buttons |= MB_LEFT;
	if (special & MK_MBUTTON)
		buttons |= MB_MIDDLE;
	if (special & MK_RBUTTON)
		buttons |= MB_RIGHT;

	return buttons;
}

void InputTranslator::mouseMove(int special, const Point &pt) {
	CMouseMoveMsg msg(pt, getButtons(special));
	_inputHandler->handleMessage(msg);
}

void InputTranslator::mouseDrag(int special, const Point &pt) {
	CMouseDragMsg msg(pt, getButtons(special));
	_inputHandler->handleMessage(msg);
}

void InputTranslator::leftButtonDown(int special, const Point &pt) {
	CMouseButtonDownMsg msg(pt, MB_LEFT);
	_inputHandler->handleMessage(msg);
}

void InputTranslator::leftButtonUp(int special, const Point &pt) {
	CMouseButtonUpMsg msg(pt, MB_LEFT);
	_inputHandler->handleMessage(msg);
}

void InputTranslator::leftButtonDoubleClick(int special, const Point &pt) {
	CMouseDoubleClickMsg msg(pt, MB_LEFT);
	_inputHandler->handleMessage(msg);
}

void InputTranslator::middleButtonDown(int special, const Point &pt) {
	CMouseButtonDownMsg msg(pt, MB_MIDDLE);
	_inputHandler->handleMessage(msg);
}

void InputTranslator::middleButtonUp(int special, const Point &pt) {
	CMouseButtonUpMsg msg(pt, MB_MIDDLE);
	_inputHandler->handleMessage(msg);
}

void InputTranslator::middleButtonDoubleClick(int special, const Point &pt) {
	CMouseDoubleClickMsg msg(pt, MB_MIDDLE);
	_inputHandler->handleMessage(msg);
}

void InputTranslator::rightButtonDown(int special, const Point &pt) {
	CMouseButtonDownMsg msg(pt, MB_RIGHT);
	_inputHandler->handleMessage(msg);
}

void InputTranslator::rightButtonUp(int special, const Point &pt) {
	CMouseButtonUpMsg msg(pt, MB_RIGHT);
	_inputHandler->handleMessage(msg);
}

void InputTranslator::mouseWheel(bool wheelUp, const Point &pt) {
	CMouseWheelMsg msg(pt, wheelUp);
	_inputHandler->handleMessage(msg);
}

void InputTranslator::rightButtonDoubleClick(int special, const Point &pt) {
	CMouseDoubleClickMsg msg(pt, MB_RIGHT);
	_inputHandler->handleMessage(msg);
}

void InputTranslator::keyDown(const Common::KeyState &keyState) {
	CKeypressMsg pressMsg(keyState);
	_inputHandler->handleMessage(pressMsg);

	if (keyState.ascii >= 32 && keyState.ascii <= 127) {
		CKeyCharMsg charMsg(keyState.ascii);
		_inputHandler->handleMessage(charMsg);
	}
}

bool InputTranslator::isMousePressed() const {
	return g_vm->_events->getSpecialButtons() & (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON);
}

} // End of namespace Shared
} // End of namespace Ultima
