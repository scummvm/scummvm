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
uint16 Input::readInput() {

	Common::Event e;
	uint16 KeyDown = 0;

	_mouseButtons = kMouseNone;

	Common::EventManager *eventMan = _vm->_system->getEventManager();
	while (eventMan->pollEvent(e)) {

		switch (e.type) {
		case Common::EVENT_KEYDOWN:
			if (e.kbd.flags == Common::KBD_CTRL && e.kbd.keycode == 'd')
				_vm->_debugger->attach();
			if (_vm->getFeatures() & GF_DEMO) break;
			if (e.kbd.keycode == Common::KEYCODE_l) KeyDown = kEvLoadGame;
			if (e.kbd.keycode == Common::KEYCODE_s) KeyDown = kEvSaveGame;
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
		case Common::EVENT_RTL:
		case Common::EVENT_QUIT:
			_vm->_quit = true;
			return KeyDown;

		default:
			break;

		}

	}

	if (_vm->_debugger->isAttached())
		_vm->_debugger->onFrame();

	return KeyDown;

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

// FIXME: see comment for readInput()
void Input::waitUntilLeftClick() {

	do {
		readInput();
		_vm->_gfx->updateScreen();
		_vm->_system->delayMillis(30);
	} while (_mouseButtons != kMouseLeftUp);

	return;
}


void Input::updateGameInput() {

	int16 keyDown = readInput();

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

	if (keyDown == kEvQuitGame) {
		_inputData._event = kEvQuitGame;
	} else
	if (keyDown == kEvSaveGame) {
		_inputData._event = kEvSaveGame;
	} else
	if (keyDown == kEvLoadGame) {
		_inputData._event = kEvLoadGame;
	} else {
		_inputData._mousePos = _mousePos;
		_inputData._event = kEvNone;
		if (!translateGameInput()) {
			translateInventoryInput();
		}
	}

}

void Input::updateCommentInput() {
	waitUntilLeftClick();

	_vm->hideDialogueStuff();
	_vm->_gfx->setHalfbriteMode(false);

	_inputMode = kInputModeGame;
}

InputData* Input::updateInput() {

	_inputData._event = kEvNone;

	switch (_inputMode) {
	case kInputModeComment:
		updateCommentInput();
		break;

	case kInputModeGame:
		updateGameInput();
		break;
	}

	return &_inputData;
}

bool Input::translateGameInput() {

	if ((_engineFlags & kEnginePauseJobs) || (_engineFlags & kEngineInventory)) {
		return false;
	}

	if (_actionAfterWalk) {
		// if walking is over, then take programmed action
		_inputData._event = kEvAction;
		_actionAfterWalk = false;
		return true;
	}

	if (_mouseButtons == kMouseRightDown) {
		// right button down shows inventory

		if (_vm->hitZone(kZoneYou, _mousePos.x, _mousePos.y) && (_activeItem._id != 0)) {
			_activeItem._index = (_activeItem._id >> 16) & 0xFFFF;
			_engineFlags |= kEngineDragging;
		}

		_inputData._event = kEvOpenInventory;
		_transCurrentHoverItem = -1;
		return true;
	}

	// test if mouse is hovering on an interactive zone for the currently selected inventory item
	ZonePtr z = _vm->hitZone(_activeItem._id, _mousePos.x, _mousePos.y);

	if (((_mouseButtons == kMouseLeftUp) && (_activeItem._id == 0) && ((_engineFlags & kEngineWalking) == 0)) && ((!z) || ((z->_type & 0xFFFF) != kZoneCommand))) {
		_inputData._event = kEvWalk;
		return true;
	}

	if ((z != _hoverZone) && (_hoverZone)) {
		_hoverZone = nullZonePtr;
		_inputData._event = kEvExitZone;
		return true;
	}

	if (!z) {
		_inputData._event = kEvNone;
		return true;
	}

	if ((!_hoverZone) && ((z->_flags & kFlagsNoName) == 0)) {
		_hoverZone = z;
		_inputData._event = kEvEnterZone;
		_inputData._label = z->_label;
		return true;
	}

	if ((_mouseButtons == kMouseLeftUp) && ((_activeItem._id != 0) || ((z->_type & 0xFFFF) == kZoneCommand))) {

		_inputData._zone = z;
		if (z->_flags & kFlagsNoWalk) {
			// character doesn't need to walk to take specified action
			_inputData._event = kEvAction;

		} else {
			// action delayed: if Zone defined a moveto position the character is programmed to move there,
			// else it will move to the mouse position
			_inputData._event = kEvWalk;
			_actionAfterWalk = true;
			if (z->_moveTo.y != 0) {
				_inputData._mousePos = z->_moveTo;
			}
		}

		_vm->beep();
		_vm->setArrowCursor();
		return true;
	}

	return true;

}

bool Input::translateInventoryInput() {

	if ((_engineFlags & kEngineInventory) == 0) {
		return false;
	}

	// in inventory
	int16 _si = _vm->getHoverInventoryItem(_mousePos.x, _mousePos.y);

	if (_mouseButtons == kMouseRightUp) {
		// right up hides inventory

		_inputData._event = kEvCloseInventory;
		_inputData._inventoryIndex = _vm->getHoverInventoryItem(_mousePos.x, _mousePos.y);
		_vm->highlightInventoryItem(-1);			// disable

		if ((_engineFlags & kEngineDragging) == 0) {
			return true;
		}

		_engineFlags &= ~kEngineDragging;
		ZonePtr z = _vm->hitZone(kZoneMerge, _activeItem._index, _vm->getInventoryItemIndex(_inputData._inventoryIndex));

		if (z) {
			_vm->dropItem(z->u.merge->_obj1);
			_vm->dropItem(z->u.merge->_obj2);
			_vm->addInventoryItem(z->u.merge->_obj3);
			_vm->_cmdExec->run(z->_commands);
		}

		return true;
	}

	if (_si == _transCurrentHoverItem) {
		_inputData._event = kEvNone;
		return true;
	}

	_transCurrentHoverItem = _si;
	_inputData._event = kEvHoverInventory;
	_inputData._inventoryIndex = _si;
	return true;

}

void Input::showCursor(bool visible) {
	_mouseHidden = !visible;
	_vm->_system->showMouse(visible);
}


} // namespace Parallaction
