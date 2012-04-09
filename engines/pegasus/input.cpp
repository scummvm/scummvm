/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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

#include "common/events.h"
#include "common/system.h"

#include "pegasus/cursor.h"
#include "pegasus/input.h"
#include "pegasus/pegasus.h"

namespace Common {
DECLARE_SINGLETON(Pegasus::InputDeviceManager);
}

namespace Pegasus {

InputDeviceManager::InputDeviceManager() {
	_lastRawBits = kAllUpBits;
}

void InputDeviceManager::getInput(Input &input, const InputBits filter) {
	InputBits currentBits = 0;
	bool consoleRequested = false;
	bool altDown = false;

	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		// We only care about key down here
		// We're mapping from ScummVM events to pegasus events, which
		// are based on pippin events.
		if (event.type == Common::EVENT_KEYDOWN) {
			switch (event.kbd.keycode) {
			case Common::KEYCODE_UP:
			case Common::KEYCODE_KP8:
				currentBits |= (kRawButtonDown << kUpButtonShift);
				break;
			case Common::KEYCODE_LEFT:
			case Common::KEYCODE_KP4:
				currentBits |= (kRawButtonDown << kLeftButtonShift);
				break;
			case Common::KEYCODE_DOWN:
			case Common::KEYCODE_KP5:
				currentBits |= (kRawButtonDown << kDownButtonShift);
				break;
			case Common::KEYCODE_RIGHT:
			case Common::KEYCODE_KP6:
				currentBits |= (kRawButtonDown << kRightButtonShift);
				break;
			case Common::KEYCODE_RETURN:
			case Common::KEYCODE_SPACE:
				currentBits |= (kRawButtonDown << kTwoButtonShift);
				break;
			case Common::KEYCODE_t:
			case Common::KEYCODE_KP_EQUALS:
				currentBits |= (kRawButtonDown << kThreeButtonShift);
				break;
			case Common::KEYCODE_i:
			case Common::KEYCODE_KP_DIVIDE:
				currentBits |= (kRawButtonDown << kFourButtonShift);
				break;
			case Common::KEYCODE_q:
				currentBits |= (kRawButtonDown << kMod1ButtonShift);
				break;
			case Common::KEYCODE_ESCAPE:
			case Common::KEYCODE_p:
				currentBits |= (kRawButtonDown << kMod3ButtonShift);
				break;
			case Common::KEYCODE_TILDE:
			case Common::KEYCODE_BACKQUOTE:
			case Common::KEYCODE_NUMLOCK: // Yes, the original uses Num Lock/Clear on the Mac...
				currentBits |= (kRawButtonDown << kLeftFireButtonShift);
				break;
			case Common::KEYCODE_BACKSPACE:
			case Common::KEYCODE_KP_MULTIPLY:
				currentBits |= (kRawButtonDown << kRightFireButtonShift);
				break;
			case Common::KEYCODE_d:
				if (event.kbd.flags & Common::KBD_CTRL) // Console!
					consoleRequested = true;
				break;
			case Common::KEYCODE_s:
				// We support meta where available and control elsewhere
				if (event.kbd.flags & (Common::KBD_CTRL|Common::KBD_META))
					((PegasusEngine *)g_engine)->requestSave();
				break;
			case Common::KEYCODE_o: // o for open (original)
			case Common::KEYCODE_l: // l for load (ScummVM terminology)
				// We support meta where available and control elsewhere
				if (event.kbd.flags & (Common::KBD_CTRL|Common::KBD_META))
					((PegasusEngine *)g_engine)->requestLoad();
				break;
			default:
				break;
			}

			// WORKAROUND: The original had a specific key for this, but
			// pressing alt would count as an event (and mess up someone
			// trying to do alt+enter or something). Since it's only used
			// as an easter egg, I'm just going to handle it as a separate
			// bool value.
			if (event.kbd.flags & Common::KBD_ALT)
				altDown = true;
		}
	}

	// Update mouse button state
	// Note that we don't use EVENT_LBUTTONUP/EVENT_LBUTTONDOWN because
	// they do not show if the button is being held down. We're treating
	// both mouse buttons as the same for ease of use.
	if (g_system->getEventManager()->getButtonState() != 0)
		currentBits |= (kRawButtonDown << kTwoButtonShift);

	// Update the mouse position too
	input.setInputLocation(g_system->getEventManager()->getMousePos());

	// Set the outgoing bits
	InputBits filteredBits = currentBits & filter;	
	input.setInputBits((filteredBits & kAllButtonDownBits) | (filteredBits & _lastRawBits & kAllAutoBits));

	// Update the last bits
	_lastRawBits = currentBits;

	// Set the console to be requested or not
	input.setConsoleRequested(consoleRequested);

	// Same for alt
	input.setAltDown(altDown);
}

// Wait until the input device stops returning input allowed by filter...
void InputDeviceManager::waitInput(const InputBits filter) {
	if (filter != 0) {
		for (;;) {
			Input input;
			getInput(input, filter);
			if (!input.anyInput())
				break;
		}
	}
}

int operator==(const Input &arg1, const Input &arg2) {
	return arg1._inputState == arg2._inputState;
}

int operator!=(const Input &arg1, const Input &arg2) {
	return !operator==(arg1, arg2);
}

InputHandler *InputHandler::_inputHandler = 0;
bool InputHandler::_invalHotspots = false;
InputBits InputHandler::_lastFilter = kFilterNoInput;

InputHandler *InputHandler::setInputHandler(InputHandler *currentHandler) {
	InputHandler *result = 0;

	if (_inputHandler != currentHandler && (!_inputHandler || _inputHandler->releaseInputFocus())) {
		result = _inputHandler;
		_inputHandler = currentHandler;
		if (_inputHandler)
			_inputHandler->grabInputFocus();
	}

	return result;
}

void InputHandler::pollForInput() {
	if (_inputHandler) {
		Input input;
		Hotspot *cursorSpot = 0;

		InputHandler::getInput(input, cursorSpot);
		if (_inputHandler->isClickInput(input, cursorSpot))
			_inputHandler->clickInHotspot(input, cursorSpot);
		else
			_inputHandler->handleInput(input, cursorSpot);
	}
}

void InputHandler::getInput(Input &input, Hotspot *&cursorSpot) {
	Cursor *cursor = ((PegasusEngine *)g_engine)->_cursor;
	
	if (_inputHandler)
		_lastFilter = _inputHandler->getInputFilter();
	else
		_lastFilter = kFilterAllInput;

	InputDevice.getInput(input, _lastFilter);

	if (_inputHandler && _inputHandler->wantsCursor() && (_lastFilter & _inputHandler->getClickFilter()) != 0) {
		if (cursor->isVisible()) {
			g_allHotspots.deactivateAllHotspots();
			_inputHandler->activateHotspots();

			Common::Point cursorLocation;
			cursor->getCursorLocation(cursorLocation);
			cursorSpot = g_allHotspots.findHotspot(cursorLocation);

			if (_inputHandler)
				_inputHandler->updateCursor(cursorLocation, cursorSpot);
		} else {
			cursor->hideUntilMoved();
		}
	} else {
		cursor->hide();
	}
}

void InputHandler::readInputDevice(Input &input) {
	InputDevice.getInput(input, kFilterAllInput);
}

InputHandler::InputHandler(InputHandler *nextHandler) {
	_nextHandler = nextHandler;
	allowInput(true);
}

InputHandler::~InputHandler() {
	if (_inputHandler == this)
		setInputHandler(_nextHandler);
}

void InputHandler::handleInput(const Input &input, const Hotspot *cursorSpot) {
	if (_nextHandler)
		_nextHandler->handleInput(input, cursorSpot);
}

void InputHandler::clickInHotspot(const Input &input, const Hotspot *cursorSpot) {
	if (_nextHandler)
		_nextHandler->clickInHotspot(input, cursorSpot);
}

bool InputHandler::isClickInput(const Input &input, const Hotspot *cursorSpot) {
	if (_nextHandler)
		return _nextHandler->isClickInput(input, cursorSpot);

	return false;
}

void InputHandler::activateHotspots() {
	if (_nextHandler)
		_nextHandler->activateHotspots();
}

InputBits InputHandler::getInputFilter() {
	if (_allowInput) {
		if (_nextHandler)
			return _nextHandler->getInputFilter();
		else
			return kFilterAllInput;
	}

	return kFilterNoInput;
}

InputBits InputHandler::getClickFilter() {
	if (_allowInput && _nextHandler)
		return _nextHandler->getClickFilter();

	return kFilterNoInput;
}

void InputHandler::updateCursor(const Common::Point cursorLocation, const Hotspot *cursorSpot) {
	if (_nextHandler)
		_nextHandler->updateCursor(cursorLocation, cursorSpot);
}

bool InputHandler::wantsCursor() {
	if (_allowInput) {
		if (_nextHandler)
			return _nextHandler->wantsCursor();
		else
			return true;
	}

	return false;
}

Tracker *Tracker::_currentTracker = 0;

void Tracker::handleInput(const Input &input, const Hotspot *) {
	if (stopTrackingInput(input))
		stopTracking(input);
	else if (isTracking())
		continueTracking(input);
}

void Tracker::startTracking(const Input &) {
	if (!isTracking()) {
		_savedHandler = InputHandler::setInputHandler(this);
		_currentTracker = this;
	}
}

void Tracker::stopTracking(const Input &) {
	if (isTracking()) {
		_currentTracker = NULL;
		InputHandler::setInputHandler(_savedHandler);
	}
}

bool Tracker::isClickInput(const Input &input, const Hotspot *hotspot) {
	return !isTracking() && InputHandler::isClickInput(input, hotspot);
}

} // End of namespace Pegasus
