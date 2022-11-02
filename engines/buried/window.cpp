/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/algorithm.h"
#include "common/textconsole.h"

#include "buried/buried.h"
#include "buried/graphics.h"
#include "buried/message.h"
#include "buried/window.h"

namespace Buried {

const Window *kWindowPosTop = (const Window *)nullptr;
const Window *kWindowPosTopMost = (const Window *)-1;

Window::Window(BuriedEngine *vm, Window *parent, bool visible) : _vm(vm), _parent(parent), _visible(visible) {
	_enabled = true;

	// Add us to the bottom of the parent's window list
	if (_parent)
		_parent->_children.push_front(this);
}

Window::~Window() {
	// Remove us from any of the parent's window lists
	if (_parent) {
		_parent->_children.remove(this);
		_parent->_topMostChildren.remove(this);
	}

	// Remove any of our messages from the queue
	_vm->removeAllMessages(this);

	// ...and any timers
	_vm->removeAllTimers(this);

	// Make sure we're not the focused window
	if (_vm->_focusedWindow == this)
		_vm->_focusedWindow = nullptr;

	// And also not captured
	if (_vm->_captureWindow == this)
		_vm->_captureWindow = nullptr;

	// Invalidate this window's rect as well
	_vm->_gfx->invalidateRect(getAbsoluteRect());
}

void Window::invalidateRect(const Common::Rect &rect, bool erase) {
	_vm->_gfx->invalidateRect(makeAbsoluteRect(rect), erase);
}

Common::Rect Window::getClientRect() const {
	return Common::Rect(_rect.width(), _rect.height());
}

Common::Rect Window::getAbsoluteRect() const {
	return makeAbsoluteRect(_rect);
}

void Window::sendMessage(Message *message) {
	switch (message->getMessageType()) {
	case kMessageTypeKeyUp:
		onKeyUp(((KeyUpMessage *)message)->getKeyState(), ((KeyUpMessage *)message)->getFlags());
		break;
	case kMessageTypeKeyDown:
		onKeyDown(((KeyDownMessage *)message)->getKeyState(), ((KeyDownMessage *)message)->getFlags());
		break;
	case kMessageTypeTimer:
		onTimer(((TimerMessage *)message)->getTimer());
		break;
	case kMessageTypeMouseMove:
		onMouseMove(((MouseMoveMessage *)message)->getPoint(), ((MouseMoveMessage *)message)->getFlags());
		break;
	case kMessageTypeLButtonUp:
		onLButtonUp(((LButtonUpMessage *)message)->getPoint(), ((LButtonUpMessage *)message)->getFlags());
		break;
	case kMessageTypeLButtonDown:
		onLButtonDown(((LButtonDownMessage *)message)->getPoint(), ((LButtonDownMessage *)message)->getFlags());
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
		onSetCursor(((SetCursorMessage *)message)->getMessage());
		break;
	case kMessageTypeEnable:
		onEnable(((EnableMessage *)message)->getEnable());
		break;
	default:
		error("Unknown message type %d", message->getMessageType());
	}

	delete message;
}

void Window::postMessage(Message *message) {
	// Simple wrapper
	_vm->postMessageToWindow(this, message);
}

void Window::updateWindow() {
	// If we're not visible, ignore
	if (!isWindowVisible())
		return;

	// If we're not in the dirty rect, ignore
	if (!_vm->_gfx->getDirtyRect().intersects(getAbsoluteRect()))
		return;

	// If we need to erase, erase first
	if (_vm->_gfx->needsErase())
		onEraseBackground();

	// Always draw this window first
	onPaint();

	// Draw children
	for (WindowList::iterator it = _children.begin(); it != _children.end(); ++it)
		(*it)->updateWindow();

	// Draw top-most children
	for (WindowList::iterator it = _topMostChildren.begin(); it != _topMostChildren.end(); ++it)
		(*it)->updateWindow();
}

void Window::setWindowPos(const Window *insertAfter, int x, int y, int width, int height, uint flags) {
	if (!(flags & kWindowPosNoZOrder)) {
		assert(insertAfter != this); // I don't even want to think about this case

		_parent->_children.remove(this);
		_parent->_topMostChildren.remove(this);

		if (insertAfter == kWindowPosTop) {
			// Reposition the window to the top
			_parent->_children.push_back(this);
		} else if (insertAfter == kWindowPosTopMost) {
			// Reposition the window to the top of the top-most
			_parent->_topMostChildren.push_back(this);
		} else {
			// Reposition the window to after insertAfter
			WindowList::iterator it = Common::find(_parent->_children.begin(), _parent->_children.end(), insertAfter);

			if (it == _parent->_children.end()) {
				it = Common::find(_parent->_topMostChildren.begin(), _parent->_topMostChildren.end(), insertAfter);

				// It has to be in one of the lists
				assert(it != _parent->_topMostChildren.end());

				_parent->_topMostChildren.insert(it, this);
			} else {
				_parent->_children.insert(it, this);
			}
		}
	}

	if (flags & kWindowPosShowWindow) {
		assert(!(flags & kWindowPosHideWindow));
		showWindow(kWindowShow);
	} else if (flags & kWindowPosHideWindow) {
		assert(!(flags & kWindowPosShowWindow));
		showWindow(kWindowHide);
	}

	if (!(flags & kWindowPosNoActivate)) {
		// TODO: Activate the window
	}

	if (!(flags & kWindowPosNoMove))
		_rect.moveTo(x, y);

	if (!(flags & kWindowPosNoSize)) {
		_rect.right = _rect.left + width;
		_rect.bottom = _rect.top + height;
	}
}

void Window::showWindow(WindowShowMode showMode) {
	bool newVisibility = (showMode != kWindowHide);

	if (_visible != newVisibility) {
		invalidateWindow();
		_visible = newVisibility;
	}

	if (showMode == kWindowShowNormal) {
		// TODO: Activate
	}
}

void Window::enableWindow(bool enable) {
	if (_enabled != enable) {
		_enabled = enable;
		postMessage(new EnableMessage(enable));
	}
}

bool Window::isWindowEnabled() const {
	if (_parent && !_parent->isWindowEnabled())
		return false;

	return _enabled && _visible;
}

uint Window::setTimer(uint elapse) {
	return _vm->createTimer(this, elapse);
}

bool Window::killTimer(uint timer) {
	return _vm->killTimer(timer);
}

Common::Rect Window::makeAbsoluteRect(const Common::Rect &rect) const {
	// No parent; it's already absolute
	if (!_parent)
		return rect;

	Common::Rect parentRect = _parent->getAbsoluteRect();
	Common::Rect absoluteRect = rect;
	absoluteRect.translate(parentRect.left, parentRect.top);
	absoluteRect.right = MIN(parentRect.right, absoluteRect.right);
	absoluteRect.bottom = MIN(parentRect.bottom, absoluteRect.bottom);
	return absoluteRect;
}

Window *Window::setFocus() {
	// Don't allow focus to be acquired if the window is disabled
	if (!isWindowEnabled())
		return nullptr;

	Window *oldWindow = nullptr;

	// Notify the old window we just took its focus
	if (_vm->_focusedWindow) {
		_vm->_focusedWindow->onKillFocus(this);
		oldWindow = _vm->_focusedWindow;
	}

	_vm->_focusedWindow = this;
	onSetFocus(oldWindow);
	return oldWindow;
}

Window *Window::childWindowAtPoint(const Common::Point &point) {
	for (WindowList::iterator it = _topMostChildren.reverse_begin(); it != _topMostChildren.end(); --it)
		if ((*it)->getAbsoluteRect().contains(point) && (*it)->isWindowEnabled())
			return (*it)->childWindowAtPoint(point);

	for (WindowList::iterator it = _children.reverse_begin(); it != _children.end(); --it)
		if ((*it)->getAbsoluteRect().contains(point) && (*it)->isWindowEnabled())
			return (*it)->childWindowAtPoint(point);

	return this;
}

Window *Window::setCapture() {
	Window *oldCapturedWindow = _vm->_captureWindow;
	_vm->_captureWindow = this;
	return oldCapturedWindow;
}

Common::Point Window::convertPointToGlobal(const Common::Point &point) {
	Common::Rect absoluteRect = getAbsoluteRect();
	return Common::Point(point.x + absoluteRect.left, point.y + absoluteRect.top);
}

Common::Point Window::convertPointToLocal(const Common::Point &point) {
	Common::Rect absoluteRect = getAbsoluteRect();
	return Common::Point(point.x - absoluteRect.left, point.y - absoluteRect.top);
}

Common::Point Window::convertPointToWindow(const Common::Point &point, Window *dest) {
	return dest->convertPointToLocal(convertPointToGlobal(point));
}

bool Window::onSetCursor(uint message) {
	// The default implementation is to try the parent first
	if (_parent && _parent->onSetCursor(message))
		return true;

	// Then otherwise default to the arrow
	_vm->_gfx->setCursor(kCursorArrow);
	return false;
}

} // End of namespace Buried
