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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/textconsole.h"

#include "buried/buried.h"
#include "buried/graphics.h"
#include "buried/message.h"
#include "buried/window.h"

namespace Buried {

Window::Window(BuriedEngine *vm) : _vm(vm) {
	_parent = 0;
	_enabled = true;
}

Window::~Window() {
	// Make sure the queue is cleaned out
	while (!_queue.empty())
		delete _queue.pop();
}

void Window::invalidateRect(const Common::Rect &rect, bool erase) {
	_vm->_gfx->invalidateRect(rect, erase);
}

void Window::createChild(const Common::Rect &rect, Window *parent) {
	_rect = rect;
	_parent = parent;
}

Common::Rect Window::getClientRect() const {
	return Common::Rect(_rect.width(), _rect.height());
}

void Window::dispatchAllMessages() {
	while (!_queue.empty() && !_vm->shouldQuit()) {
		Message *message = _queue.pop();

		switch (message->getMessageType()) {
		case kMessageTypeEraseBackground:
			onEraseBackground();
			break;
		case kMessageTypeKeyUp:
			onKeyUp(((KeyUpMessage *)message)->getKeyState(), ((KeyUpMessage *)message)->getFlags());
			break;
		case kMessageTypeKeyDown:
			onKeyDown(((KeyDownMessage *)message)->getKeyState(), ((KeyDownMessage *)message)->getFlags());
			break;
		case kMessageTypeTimer:
			onTimer(((TimerMessage *)message)->getTimer());
			break;
		case kMessageTypeSetFocus:
			onSetFocus(((SetFocusMessage *)message)->getWindow());
			break;
		case kMessageTypeKillFocus:
			onKillFocus(((KillFocusMessage *)message)->getWindow());
			break;
		case kMessageTypeQueryNewPalette:
			onQueryNewPalette();
			break;
		case kMessageTypeLButtonUp:
			onLButtonUp(((LButtonUpMessage *)message)->getPoint(), ((LButtonUpMessage *)message)->getFlags());
			break;
		case kMessageTypeLButtonDown:
			onLButtonDown(((LButtonDownMessage *)message)->getPoint(), ((LButtonDownMessage *)message)->getFlags());
			break;
		case kMessageTypeLButtonDoubleClick:
			onLButtonDoubleClick(((LButtonDoubleClickMessage *)message)->getPoint(), ((LButtonDoubleClickMessage *)message)->getFlags());
			break;
		case kMessageTypeMButtonUp:
			onMButtonUp(((MButtonUpMessage *)message)->getPoint(), ((MButtonUpMessage *)message)->getFlags());
			break;
		case kMessageTypeRButtonUp:
			onRButtonUp(((RButtonUpMessage *)message)->getPoint(), ((RButtonUpMessage *)message)->getFlags());
			break;
		case kMessageTypeRButtonDown:
			onRButtonDown(((RButtonDownMessage *)message)->getPoint(), ((RButtonDownMessage *)message)->getFlags());
			break;
		case kMessageTypeSetCursor:
			onSetCursor(((SetCursorMessage *)message)->getWindow(), ((SetCursorMessage *)message)->getCursor());
			break;
		case kMessageTypeEnable:
			onEnable(((EnableMessage *)message)->getEnable());
			break;
		default:
			error("Unknown message type %d", message->getMessageType());
		}

		delete message;
	}
}

void Window::enableWindow(bool enable) {
	if (_enabled != enable) {
		_enabled = enable;
		sendMessage(new EnableMessage(enable));
	}
}

bool Window::isWindowEnabled() const {
	if (_parent && !_parent->isWindowEnabled())
		return false;

	return _enabled;
}

} // End of namespace Buried
