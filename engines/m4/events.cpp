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

// TODO: There is a 'please_hyperwalk' variable that gets accessed that is meant to be global, but
// at the moment it's implemented as a local variable

#include "graphics/cursorman.h"

#include "m4/events.h"
#include "m4/graphics.h"
#include "m4/scene.h"
#include "m4/viewmgr.h"
#include "m4/m4.h"

namespace M4 {

bool please_hyperwalk = false;

/*--------------------------------------------------------------------------*
 * Events																*
 *																			*
 * Implements an interface to the event system								*
 *--------------------------------------------------------------------------*/

Events::Events(MadsM4Engine *vm) : _vm(vm) {
	_mouseState = MSTATE_NO_EVENT;
	quitFlag = false;
	_keyCode = 0;
	_mouseButtons = 0;
	_ctrlFlag = false;

	if (_vm->isM4())
		_console = new M4Console(_m4Vm);
	else
		_console = new MadsConsole(_madsVm);
}

Events::~Events() {
	delete _console;
}

M4EventType Events::handleEvents() {
	static int oldX = -1, oldY = -1;
	static uint32 dclickTime = 0;

	// Handle event types
	while (g_system->getEventManager()->pollEvent(_event)) {
		switch (_event.type) {
		case Common::EVENT_QUIT:
			quitFlag = true;
			break;
		case Common::EVENT_KEYDOWN:
			// Note: The Ctrl-D ScummVM shortcut has been specialised so it will only activate the debugger
			// if it's the first key pressed after the Ctrl key is held down
			if ((_event.kbd.keycode == Common::KEYCODE_LCTRL) || (_event.kbd.keycode == Common::KEYCODE_RCTRL))
				_ctrlFlag = true;

			else if (_event.kbd.hasFlags(Common::KBD_CTRL)) {
				if ((_event.kbd.keycode == Common::KEYCODE_d) && _ctrlFlag) {
					_console->attach();
					_console->onFrame();
				}
				_ctrlFlag = false;
			}
			_keyCode = (int32)_event.kbd.keycode | ((_event.kbd.flags & (Common::KBD_CTRL | Common::KBD_ALT | Common::KBD_SHIFT)) << 24);


			break;
		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_LBUTTONUP:
		case Common::EVENT_RBUTTONDOWN:
		case Common::EVENT_RBUTTONUP:
		case Common::EVENT_MBUTTONDOWN:
		case Common::EVENT_MBUTTONUP:
		case Common::EVENT_MOUSEMOVE:
		case Common::EVENT_WHEELUP:
		case Common::EVENT_WHEELDOWN:
			_vm->_mouse->handleEvent(_event);
			break;
		default:
			break;
		}
	}

	_mouseButtons = g_system->getEventManager()->getButtonState();

	// State machine for moving between states
	switch (_mouseState) {
	case MSTATE_NO_EVENT:
		if (_mouseButtons & LEFT_BUTTON_DOWN) {
			if ((dclickTime != 0) && (g_system->getMillis() < dclickTime)) {
				_mouseState = MSTATE_DOUBLECLICK_DOWN;
				dclickTime = 0;
				return MEVENT_DOUBLECLICK;
			}
			dclickTime = 0;
			_mouseState = MSTATE_LEFT_CLICK_DOWN;
			return MEVENT_LEFT_CLICK;
		}
		if (_mouseButtons & RIGHT_BUTTON_DOWN) {
			_mouseState = MSTATE_RIGHT_CLICK_DOWN;
			return MEVENT_RIGHT_CLICK;
		}
		if ((_event.mouse.x != oldX) || (_event.mouse.y != oldY)) {
			oldX = _event.mouse.x; oldY = _event.mouse.y;
			return MEVENT_MOVE;
		}
		return MEVENT_NO_EVENT;

	case MSTATE_LEFT_CLICK_DOWN:
		if (!(_mouseButtons & LEFT_BUTTON_DOWN)) {
			dclickTime = g_system->getMillis() + 1000 * 15 / 60;
			_mouseState = MSTATE_NO_EVENT;
			return MEVENT_LEFT_RELEASE;
		}
		if ((_event.mouse.x != oldX) || (_event.mouse.y != oldY)) {
			oldX = _event.mouse.x; oldY = _event.mouse.y;
			return MEVENT_LEFT_DRAG;
		}
		return MEVENT_LEFT_HOLD;

	case MSTATE_RIGHT_CLICK_DOWN:
		if (!(_mouseButtons & RIGHT_BUTTON_DOWN)) {
			_mouseState = MSTATE_NO_EVENT;
			please_hyperwalk = true;
			return MEVENT_RIGHT_RELEASE;
		}
		if ((_event.mouse.x != oldX) || (_event.mouse.y != oldY)) {
			oldX = _event.mouse.x; oldY = _event.mouse.y;
			return MEVENT_RIGHT_DRAG;
		}
		return MEVENT_RIGHT_HOLD;

	case MSTATE_DOUBLECLICK_DOWN:
		if (!(_mouseButtons & LEFT_BUTTON_DOWN)) {
			_mouseState = MSTATE_NO_EVENT;
			return MEVENT_DOUBLECLICK_RELEASE;
		}
		if ((_event.mouse.x != oldX) || (_event.mouse.y != oldY)) {
			oldX = _event.mouse.x; oldY = _event.mouse.y;
			return MEVENT_DOUBLECLICK_DRAG;
		}
		return MEVENT_DOUBLECLICK_HOLD;

	default:
		return MEVENT_NO_EVENT;
	}
}

bool Events::kbdCheck(uint32 &keyCode) {
	if (_keyCode == 0)
		return false;

	keyCode = _keyCode;
	_keyCode = 0;
	return true;
}


/*--------------------------------------------------------------------------*
 * Mouse																*
 *																			*
 * Implements an interface to the mouse										*
 *--------------------------------------------------------------------------*/

Mouse::Mouse(MadsM4Engine *vm) : _vm(vm) {
	_locked = false;
	_cursorOn = false;
	_cursor = NULL;
	_cursorSprites = NULL;
	resetMouse();
}

Mouse::~Mouse() {
	delete _cursorSprites;
}

bool Mouse::init(const char *seriesName, RGB8 *palette) {
	Common::SeekableReadStream *stream = _vm->res()->get(seriesName);
	int colorCount = 0;
	RGB8* cursorPalette;

	_cursorSprites = new SpriteAsset(_vm, stream, stream->size(), seriesName);

	// Remove cursor special pixels and set the mouse cursor hotspot in MADS games
	if (!_vm->isM4()) {
		byte *data = NULL;
		for (int i = 0; i < _cursorSprites->getCount(); i++) {
			bool hotSpotSet = false;

			for (int x = 0; x < _cursorSprites->getFrame(i)->width(); x++) {
				for (int y = 0; y < _cursorSprites->getFrame(i)->height(); y++) {
					data = _cursorSprites->getFrame(i)->getBasePtr(x, y);
					if (*data == 1) {
						// It seems that some cursors have more than one hotspot
						// In such a case, the first hotspot seems to set the x and
						// the second one the y hotspot offset
						if (!hotSpotSet) {
							_cursorSprites->getFrame(i)->xOffset = x;
							_cursorSprites->getFrame(i)->yOffset = y;
							hotSpotSet = true;
						} else {
							_cursorSprites->getFrame(i)->yOffset = y;
						}
						*data = 0;
					}
				}	// for y
			}	// for x
		}	// for i
	}

	colorCount = _cursorSprites->getColorCount();
	cursorPalette = _cursorSprites->getPalette();
	_vm->_palette->setPalette(cursorPalette, 0, colorCount);

	//debugCN(kDebugCore, "Cursor count: %d\n", _cursorSprites->getCount());

	_vm->res()->toss(seriesName);

	_currentCursor = -1;
	return true;
}

bool Mouse::setCursorNum(int cursorIndex) {
	if ((cursorIndex < 0) || (cursorIndex >= (int)_cursorSprites->getCount()))
		return false;

	_lockedCursor = cursorIndex;
	if (_locked)
		// Cursor is locked, so don't go ahead with changing cursor
		return true;

	_currentCursor = _lockedCursor;
	_cursor = _cursorSprites->getFrame(cursorIndex);

	// Set the cursor to the sprite
	CursorMan.replaceCursor((const byte *)_cursor->getBasePtr(), _cursor->width(), _cursor->height(),
		_cursor->xOffset, _cursor->yOffset, TRANSPARENT_COLOR_INDEX);

	return true;
}

int Mouse::cursorCount() {
	return _cursorSprites->getCount();
}

void Mouse::cursorOn() {
	_cursorOn = true;
	CursorMan.showMouse(!inHideArea());
}

void Mouse::cursorOff() {
	_cursorOn = false;
	CursorMan.showMouse(false);
}

void Mouse::lockCursor(int cursorIndex) {
	_locked = false;
	setCursorNum(cursorIndex);
	_locked = true;
}

void Mouse::unlockCursor() {
	_locked = false;
	setCursorNum(_lockedCursor);
}

const char *Mouse::getVerb() {
	switch (_vm->_mouse->getCursorNum()) {
	case CURSOR_LOOK:
		return "LOOK AT";
	case CURSOR_TAKE:
		return "TAKE";
	case CURSOR_USE:
		return "GEAR";
	default:
		return NULL;
	}
}

void Mouse::resetMouse() {
	_hideRect.left = -1;
	_hideRect.top = -1;
	_hideRect.right = -1;
	_hideRect.bottom = -1;
	_showRect.left = -1;
	_showRect.top = -1;
	_showRect.right = -1;
	_showRect.bottom = -1;
}

void Mouse::setHideRect(Common::Rect &r) {
	_hideRect = r;
}

void Mouse::setShowRect(Common::Rect &r) {
	_showRect = r;
}

const Common::Rect *Mouse::getHideRect() {
	return &_hideRect;
}

const Common::Rect *Mouse::getShowRect() {
	if ((_showRect.top == -1) || (_showRect.left == -1)) {
		// Show rectangle uninitialised - set it to current screen dimensions
		_showRect.top = 0;
		_showRect.left = 0;
		_showRect.right = _vm->_screen->width() - 1;
		_showRect.bottom = _vm->_screen->height() -1;
	}

	return &_showRect;
}

void Mouse::handleEvent(Common::Event &event) {
	_currentPos.x = event.mouse.x;
	_currentPos.y = event.mouse.y;

	// If mouse is turned on, check to see if the position is in the hide rect, or outside the show rect.
	// If so, handle toggling the visibility of the mouse
	bool showFlag = !inHideArea();
	if (_cursorOn && (CursorMan.isVisible() != showFlag)) {
		CursorMan.showMouse(showFlag);
	}
}

bool Mouse::inHideArea() {
	// Returns true if the mouse is inside a specified hide rect, or if a show rect is specified and
	// the mouse is currently outside it
	if ((_currentPos.x >= _hideRect.left) && (_currentPos.x <= _hideRect.right) &&
		(_currentPos.y >= _hideRect.top) && (_currentPos.y <= _hideRect.bottom))
		// Inside a hide area
		return true;


	if ((_showRect.top == -1) && (_showRect.left == -1))
		// No show rect defined
		return false;

	// Return true if the mouse is outside the show area
	return (_currentPos.x < _showRect.left) || (_currentPos.x > _showRect.right) ||
		(_currentPos.y < _showRect.top) || (_currentPos.y > _showRect.bottom);
}

} // End of namespace M4
