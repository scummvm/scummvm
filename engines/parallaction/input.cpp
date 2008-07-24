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
 * $URL$
 * $Id$
 *
 */

#include "common/events.h"

#include "parallaction/input.h"
#include "parallaction/parallaction.h"
#include "parallaction/debug.h"

namespace Parallaction {

// FIXME: the engine has 3 event loops. The following routine hosts the main one,
// and it's called from 8 different places in the code. There exist 2 more specialised
// loops which could possibly be merged into this one with some effort in changing
// caller code, i.e. adding condition checks.
//
void Input::readInput() {

	Common::Event e;

	_mouseButtons = kMouseNone;
	_hasKeyPressEvent = false;

	Common::EventManager *eventMan = _vm->_system->getEventManager();
	while (eventMan->pollEvent(e)) {

		switch (e.type) {
		case Common::EVENT_KEYDOWN:
			_hasKeyPressEvent = true;
			_keyPressed = e.kbd;

			if (e.kbd.flags == Common::KBD_CTRL && e.kbd.keycode == 'd')
				_vm->_debugger->attach();
			break;

		case Common::EVENT_LBUTTONDOWN:
			_mouseButtons = kMouseLeftDown;
			_mousePos = e.mouse;
			break;

		case Common::EVENT_LBUTTONUP:
			_mouseButtons = kMouseLeftUp;
			_mousePos = e.mouse;
			break;

		case Common::EVENT_RBUTTONDOWN:
			_mouseButtons = kMouseRightDown;
			_mousePos = e.mouse;
			break;

		case Common::EVENT_RBUTTONUP:
			_mouseButtons = kMouseRightUp;
			_mousePos = e.mouse;
			break;

		case Common::EVENT_MOUSEMOVE:
			_mousePos = e.mouse;
			break;

		case Common::EVENT_QUIT:
			_engineFlags |= kEngineQuit;
			return;

		default:
			break;

		}

	}

	if (_vm->_debugger->isAttached())
		_vm->_debugger->onFrame();

	return;

}

bool Input::getLastKeyDown(uint16 &ascii) {
	ascii = _keyPressed.ascii;
	return (_hasKeyPressEvent);
}

// FIXME: see comment for readInput()
void Input::waitForButtonEvent(uint32 buttonEventMask, int32 timeout) {

	if (buttonEventMask == kMouseNone) {
		_mouseButtons = kMouseNone;	// don't wait on nothing
		return;
	}

	const int32 LOOP_RESOLUTION = 30;
	if (timeout <= 0) {
		do {
			readInput();
			_vm->_system->delayMillis(LOOP_RESOLUTION);
		} while ((_mouseButtons & buttonEventMask) == 0);
	} else {
		do {
			readInput();
			_vm->_system->delayMillis(LOOP_RESOLUTION);
			timeout -= LOOP_RESOLUTION;
		} while ((timeout > 0) && (_mouseButtons & buttonEventMask) == 0);
	}

}


void Input::updateGameInput() {

	readInput();

	debugC(3, kDebugInput, "translateInput: input flags (%i, %i, %i, %i)",
		!_mouseHidden,
		(_engineFlags & kEngineBlockInput) == 0,
		(_engineFlags & kEngineWalking) == 0,
		(_engineFlags & kEngineChangeLocation) == 0
	);

	if ((_mouseHidden) ||
		(_engineFlags & kEngineBlockInput) ||
		(_engineFlags & kEngineWalking) ||
		(_engineFlags & kEngineChangeLocation)) {

		return;
	}

	if (_hasKeyPressEvent && (_vm->getFeatures() & GF_DEMO) == 0) {
		if (_keyPressed.keycode == Common::KEYCODE_l) _inputData._event = kEvLoadGame;
		if (_keyPressed.keycode == Common::KEYCODE_s) _inputData._event = kEvSaveGame;
	}

	if (_inputData._event == kEvNone) {
		_inputData._mousePos = _mousePos;
		translateGameInput();
	}

}


InputData* Input::updateInput() {

	_inputData._event = kEvNone;

	switch (_inputMode) {
	case kInputModeComment:
	case kInputModeDialogue:
	case kInputModeMenu:
		readInput();
		break;

	case kInputModeGame:
		updateGameInput();
		break;

	case kInputModeInventory:
		readInput();
		updateInventoryInput();
		break;
	}

	return &_inputData;
}

void Input::trackMouse(ZonePtr z) {
	if ((z != _hoverZone) && (_hoverZone)) {
		stopHovering();
		return;
	}

	if (z == nullZonePtr) {
		return;
	}

	if ((!_hoverZone) && ((z->_flags & kFlagsNoName) == 0)) {
		_hoverZone = z;
		_vm->_gfx->showFloatingLabel(_hoverZone->_label);
		return;
	}
}

void Input::stopHovering() {
	_hoverZone = nullZonePtr;
	_vm->_gfx->hideFloatingLabel();
}

void Input::takeAction(ZonePtr z) {
	stopHovering();
	_vm->pauseJobs();
	_vm->runZone(z);
	_vm->resumeJobs();
}

void Input::walkTo(const Common::Point &dest) {
	stopHovering();
	_vm->setArrowCursor();
	_vm->_char.scheduleWalk(dest.x, dest.y);
}

bool Input::translateGameInput() {

	if (_engineFlags & kEnginePauseJobs) {
		return false;
	}

	if (_hasDelayedAction) {
		// if walking is over, then take programmed action
		takeAction(_delayedActionZone);
		_hasDelayedAction = false;
		_delayedActionZone = nullZonePtr;
		return true;
	}

	if (_mouseButtons == kMouseRightDown) {
		// right button down shows inventory
		enterInventoryMode();
		return true;
	}

	// test if mouse is hovering on an interactive zone for the currently selected inventory item
	ZonePtr z = _vm->hitZone(_activeItem._id, _mousePos.x, _mousePos.y);
	Common::Point dest(_mousePos);

	if (((_mouseButtons == kMouseLeftUp) && (_activeItem._id == 0) && ((_engineFlags & kEngineWalking) == 0)) && ((!z) || ((z->_type & 0xFFFF) != kZoneCommand))) {
		walkTo(dest);
		return true;
	}

	trackMouse(z);
 	if (!z) {
 		return true;
 	}

	if ((_mouseButtons == kMouseLeftUp) && ((_activeItem._id != 0) || ((z->_type & 0xFFFF) == kZoneCommand))) {

		_inputData._zone = z;
		if (z->_flags & kFlagsNoWalk) {
			// character doesn't need to walk to take specified action
			takeAction(z);
		} else {
			// action delayed: if Zone defined a moveto position the character is programmed to move there,
			// else it will move to the mouse position
			_delayedActionZone = z;
			_hasDelayedAction = true;
			if (z->_moveTo.y != 0) {
				dest = z->_moveTo;
			}

			walkTo(dest);
		}

		_vm->beep();
		_vm->setArrowCursor();
		return true;
	}

	return true;
}


void Input::enterInventoryMode() {
	bool hitCharacter = _vm->hitZone(kZoneYou, _mousePos.x, _mousePos.y);

	if (hitCharacter) {
		if (_activeItem._id != 0) {
			_activeItem._index = (_activeItem._id >> 16) & 0xFFFF;
			_engineFlags |= kEngineDragging;
		} else {
			_vm->setArrowCursor();
		}
	}

	stopHovering();
	_vm->pauseJobs();
	_vm->openInventory();

	_transCurrentHoverItem = -1;

	_inputMode = kInputModeInventory;
}

void Input::exitInventoryMode() {
	// right up hides inventory

	int item = _vm->getHoverInventoryItem(_mousePos.x, _mousePos.y);
	_vm->highlightInventoryItem(-1);			// disable

	if ((_engineFlags & kEngineDragging)) {

		_engineFlags &= ~kEngineDragging;
		ZonePtr z = _vm->hitZone(kZoneMerge, _activeItem._index, _vm->getInventoryItemIndex(item));

		if (z) {
			_vm->dropItem(z->u.merge->_obj1);
			_vm->dropItem(z->u.merge->_obj2);
			_vm->addInventoryItem(z->u.merge->_obj3);
			_vm->_cmdExec->run(z->_commands);
		}

	}

	_vm->closeInventory();
	if (item == -1) {
		_vm->setArrowCursor();
	} else {
		_vm->setInventoryCursor(item);
	}
	_vm->resumeJobs();

	_inputMode = kInputModeGame;
}

bool Input::updateInventoryInput() {
	if (_mouseButtons == kMouseRightUp) {
		exitInventoryMode();
		return true;
	}

	int16 _si = _vm->getHoverInventoryItem(_mousePos.x, _mousePos.y);
	if (_si != _transCurrentHoverItem) {
		_transCurrentHoverItem = _si;
		_vm->highlightInventoryItem(_si);						// enable
	}

	return true;

}

void Input::showCursor(bool visible) {
	_mouseHidden = !visible;
	_vm->_system->showMouse(visible);
}


} // namespace Parallaction
