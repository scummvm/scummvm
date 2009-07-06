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

#include "gob/hotspots.h"
#include "gob/global.h"
#include "gob/helper.h"
#include "gob/draw.h"
#include "gob/game.h"
#include "gob/script.h"
#include "gob/inter.h"

namespace Gob {

Hotspots::Hotspot::Hotspot() {
	clear();
}

Hotspots::Hotspot::Hotspot(uint16 i,
		uint16 l, uint16 t, uint16 r, uint16 b, uint16 f, uint16 k,
		uint16 enter, uint16 leave, uint16 pos) {

	id        = i;
	left      = l;
	top       = t;
	right     = r;
	bottom    = b;
	flags     = f;
	key       = k;
	funcEnter = enter;
	funcLeave = leave;
	funcPos   = pos;
	script    = 0;
}

void Hotspots::Hotspot::clear() {
	id        = 0;
	left      = 0xFFFF;
	top       = 0;
	right     = 0;
	bottom    = 0;
	flags     = 0;
	key       = 0;
	funcEnter = 0;
	funcLeave = 0;
	funcPos   = 0;
	script    = 0;
}

Hotspots::Type Hotspots::Hotspot::getType() const {
	return (Type) (flags & 0xF);
}

MouseButtons Hotspots::Hotspot::getButton() const {
	uint8 buttonBits = ((flags & 0x70) >> 4);

	if (buttonBits == 0)
		return kMouseButtonsLeft;
	if (buttonBits == 1)
		return kMouseButtonsRight;
	if (buttonBits == 2)
		return kMouseButtonsAny;

	return kMouseButtonsNone;
}

uint8 Hotspots::Hotspot::getWindow() const {
	return (flags & 0x0F00) >> 8;
}

uint8 Hotspots::Hotspot::getCursor() const {
	return (flags & 0xF000) >> 12;
}

uint8 Hotspots::Hotspot::getState(uint16 id) {
	return (id & 0xF000) >> 12;
}

uint8 Hotspots::Hotspot::getState() const {
	return getState(id);
}

bool Hotspots::Hotspot::isEnd() const {
	return (left == 0xFFFF);
}

bool Hotspots::Hotspot::isInput() const {
	if (getType() < kTypeInput1NoLeave)
		return false;

	if (getType() > kTypeInputFloatLeave)
		return false;

	return true;
}

bool Hotspots::Hotspot::isActiveInput() const {
	if (isEnd())
		return false;

	if (!isFilledEnabled())
		return false;

	if (!isInput())
		return false;

	return true;
}

bool Hotspots::Hotspot::isInputLeave() const {
	if (!isInput())
		return false;

	if (!(getType() & 1))
		return true;

	return false;
}

bool Hotspots::Hotspot::isFilled() const {
	return getState() & kStateFilled;
}

bool Hotspots::Hotspot::isFilledEnabled() const {
	return (getState() & kStateFilledDisabled) == kStateFilled;
}

bool Hotspots::Hotspot::isFilledNew() const {
	return getState() == kStateFilled;
}

bool Hotspots::Hotspot::isDisabled() const {
	return getState() & kStateDisabled;
}

bool Hotspots::Hotspot::isIn(uint16 x, uint16 y) const {
	if (x < left)
		return false;
	if (x > right)
		return false;
	if (y < top)
		return false;
	if (y > bottom)
		return false;

	return true;
}

bool Hotspots::Hotspot::buttonMatch(MouseButtons button) const {
	MouseButtons myButton = getButton();

	if (myButton == kMouseButtonsAny)
		// Any button allowed
		return true;

	if (myButton == kMouseButtonsNone)
		// No button allowed
		return false;

	if (myButton == button)
		// Exact match
		return true;

	return false;
}

void Hotspots::Hotspot::disable() {
	id |= (kStateDisabled << 12);
}

void Hotspots::Hotspot::enable() {
	id &= ~(kStateDisabled << 12);
}


Hotspots::Hotspots(GobEngine *vm) : _vm(vm) {
	_hotspots = new Hotspot[kHotspotCount];

	_shouldPush = false;

	_currentKey   = 0;
	_currentIndex = 0;
	_currentId    = 0;
}

Hotspots::~Hotspots() {
	delete[] _hotspots;

	// Pop the whole stack and free each element's memory
	while (!_stack.empty()) {

		StackEntry backup = _stack.pop();

		delete[] backup.hotspots;
	}
}

void Hotspots::clear() {
	_currentKey = 0;

	for (int i = 0; i < kHotspotCount; i++)
		_hotspots[i].clear();
}

uint16 Hotspots::add(uint16 id,
		uint16 left,  uint16 top, uint16 right, uint16 bottom,
		uint16 flags, uint16 key,
		uint16 funcEnter, uint16 funcLeave, uint16 funcPos) {

	Hotspot hotspot(id, left, top, right, bottom,
			flags, key, funcEnter, funcLeave, funcPos);

	return add(hotspot);
}

uint16 Hotspots::add(const Hotspot &hotspot) {
	for (int i = 0; i < kHotspotCount; i++) {
		Hotspot &spot = _hotspots[i];

		//     free space => add    same id => update
		if (! (spot.isEnd() || (spot.id == hotspot.id)))
			continue;

		// When updating, keep disabled state intact
		uint16 id = hotspot.id;
		if ((spot.id     & ~(kStateDisabled << 12)) ==
		     (hotspot.id & ~(kStateDisabled << 12)))
			id = spot.id;

		// Set
		spot    = hotspot;
		spot.id = id;

		// Remember the current script
		spot.script = _vm->_game->_script;

		debugC(1, kDebugHotspots, "Adding hotspot %03d: %3d+%3d+%3d+%3d - %04X, %04X, %04X - %5d, %5d, %5d",
				i, spot.left, spot.top, spot.right, spot.bottom,
				spot.id, spot.key, spot.flags, spot.funcEnter, spot.funcLeave, spot.funcPos);

		return i;
	}

	error("Hotspots::add(): Hotspot array full");
	return 0xFFFF;
}

void Hotspots::remove(uint16 id) {
	for (int i = 0; i < kHotspotCount; i++) {
		if (_hotspots[i].id == id) {
			debugC(1, kDebugHotspots, "Removing hotspot %d: %X", i, id);
			_hotspots[i].clear();
		}
	}
}

void Hotspots::removeState(uint8 state) {
	for (int i = 0; i < kHotspotCount; i++) {
		Hotspot &spot = _hotspots[i];

		if (spot.getState() == state) {
			debugC(1, kDebugHotspots, "Removing hotspot %d: %X (by state %X)", i, spot.id, state);
			spot.clear();
		}
	}
}

void Hotspots::recalculate(bool force) {
	debugC(5, kDebugHotspots, "Recalculating hotspots");

	for (int i = 0; (i < kHotspotCount) && !_hotspots[i].isEnd(); i++) {
		Hotspot &spot = _hotspots[i];

		if (!force && ((spot.flags & 0x80) != 0))
			// Not forcing a special hotspot
			continue;

		if (spot.funcPos == 0)
			// Simple coordinates don't need update
			continue;

		// Setting the needed script
		Script *curScript = _vm->_game->_script;

		_vm->_game->_script = spot.script;
		if (!_vm->_game->_script)
			_vm->_game->_script = curScript;

		// Calling the function that contains the positions
		_vm->_game->_script->call(spot.funcPos);

		// Calculate positions
		int16 left   = _vm->_game->_script->readValExpr();
		int16 top    = _vm->_game->_script->readValExpr();
		int16 width  = _vm->_game->_script->readValExpr();
		int16 height = _vm->_game->_script->readValExpr();

		// Re-read the flags too, if applicable
		uint16 flags = 0;
		if (spot.getState() == (kStateFilled | kStateType2))
			flags = _vm->_game->_script->readValExpr();

		// Apply backDelta, if needed
		if ((_vm->_draw->_renderFlags & RENDERFLAG_CAPTUREPOP) && (left != -1)) {
			left += _vm->_draw->_backDeltaX;
			top  += _vm->_draw->_backDeltaY;
		}

		// Clamping
		if (left < 0) {
			width += left;
			left   = 0;
		}
		if (top < 0) {
			height += top;
			top     = 0;
		}

		// Set the updated position
		spot.left   = left;
		spot.top    = top;
		spot.right  = left + width  - 1;
		spot.bottom = top  + height - 1;

		if (spot.getState() == (kStateFilled | kStateType2))
			spot.flags = flags;

		// Return
		_vm->_game->_script->pop();

		_vm->_game->_script = curScript;
	}
}

void Hotspots::push(uint8 all, bool force) {
	debugC(1, kDebugHotspots, "Pushing hotspots (%d, %d)", all, force);

	// Should we push at all?
	if (!_shouldPush && !force)
		return;

	// Count the hotspots
	uint32 size = 0;
	for (int i = 0; (i < kHotspotCount) && !_hotspots[i].isEnd(); i++) {
		Hotspot &spot = _hotspots[i];

		     // Save all of them
		if ( (all == 1) ||
		     // Don't save the global ones
		    ((all == 0) && (spot.id >= 20)) ||
		     // Only save disabled ones
		    ((all == 2) && ((spot.getState() == (kStateFilledDisabled | kStateType1)) ||
		                    (spot.getState() == (kStateDisabled)) ||
		                    (spot.getState() == (kStateFilledDisabled | kStateType2))))) {
			size++;
		}

	}

	StackEntry backup;

	backup.shouldPush = _shouldPush;
	backup.size       = size;
	backup.key        = _currentKey;
	backup.id         = _currentId;
	backup.index      = _currentIndex;

	backup.hotspots = new Hotspot[size];

	// Copy the hotspots
	Hotspot *destPtr = backup.hotspots;
	for (int i = 0; (i < kHotspotCount) && !_hotspots[i].isEnd(); i++) {
		Hotspot &spot = _hotspots[i];

		     // Save all of them
		if ( (all == 1) ||
		     // Don't save the global ones
		    ((all == 0) && (spot.id >= 20)) ||
		     // Only save disabled ones
		    ((all == 2) && ((spot.getState() == (kStateFilledDisabled | kStateType1)) ||
		                    (spot.getState() == (kStateDisabled)) ||
		                    (spot.getState() == (kStateFilledDisabled | kStateType2))))) {

			memcpy(destPtr, &spot, sizeof(Hotspot));
			destPtr++;

			spot.clear();
		}

	}

	// Reset current state
	_shouldPush   = false;
	_currentKey   = 0;
	_currentId    = 0;
	_currentIndex = 0;

	_stack.push(backup);
}

void Hotspots::pop() {
	debugC(1, kDebugHotspots, "Popping hotspots");

	assert(!_stack.empty());

	StackEntry backup = _stack.pop();

	// Find the end of the filled hotspot space
	int i;
	Hotspot *destPtr = _hotspots;
	for (i = 0; i < kHotspotCount; i++, destPtr++)
		if (destPtr->isEnd())
			break;

	if (((uint32) (kHotspotCount - i)) < backup.size)
		error("Hotspots::pop(): Not enough free space in the current Hotspot "
		      "array to pop %d elements (got %d)", backup.size, kHotspotCount - i);

	// Copy
	memcpy(destPtr, backup.hotspots, backup.size * sizeof(Hotspot));

	_shouldPush   = backup.shouldPush;
	_currentKey   = backup.key;
	_currentId    = backup.id;
	_currentIndex = backup.index;

	delete[] backup.hotspots;
}

bool Hotspots::isValid(uint16 key, uint16 id, uint16 index) const {
	if (index >= kHotspotCount)
		return false;

	if (key == 0)
		return false;

	if (!(Hotspot::getState(id) & kStateFilled))
		return false;

	return true;
}

void Hotspots::call(uint16 offset) {
	debugC(4, kDebugHotspots, "Calling hotspot function %d", offset);

	_vm->_game->_script->call(offset);

	_shouldPush = true;

	int16 stackSize = _stack.size();

	_vm->_inter->funcBlock(0);

	while (stackSize != _stack.size())
		pop();

	_shouldPush = false;

	_vm->_game->_script->pop();

	recalculate(false);
}

void Hotspots::enter(uint16 index) {
	debugC(2, kDebugHotspots, "Entering hotspot %d", index);

	if (index >= kHotspotCount) {
		warning("Hotspots::enter(): Index %d out of range", index);
		return;
	}

	Hotspot &spot = _hotspots[index];

	// If requested, write the ID into a variable
	if ((spot.getState() == (kStateFilled | kStateType1)) ||
	    (spot.getState() == (kStateFilled | kStateType2)))
		WRITE_VAR(17, -(spot.id & 0x0FFF));

	if (spot.funcEnter != 0)
		call(spot.funcEnter);
}

void Hotspots::leave(uint16 index) {
	debugC(2, kDebugHotspots, "Leaving hotspot %d", index);

	if (index >= kHotspotCount) {
		warning("Hotspots::leave(): Index %d out of range", index);
		return;
	}

	Hotspot &spot = _hotspots[index];

	// If requested, write the ID into a variable
	if ((spot.getState() == (kStateFilled | kStateType1)) ||
	    (spot.getState() == (kStateFilled | kStateType2)))
		WRITE_VAR(17, spot.id & 0x0FFF);

	if (spot.funcLeave != 0)
		call(spot.funcLeave);
}

uint16 Hotspots::checkMouse(Type type, uint16 &id, uint16 &index) const {
	id    = 0;
	index = 0;

	if (type == kTypeMove) {
		// Check where the mouse was moved to

		for (int i = 0; (i < kHotspotCount) && !_hotspots[i].isEnd(); i++) {
			const Hotspot &spot = _hotspots[i];

			if (spot.isDisabled())
				// Only consider enabled hotspots
				continue;

			if (spot.getType() > kTypeMove)
				// Only consider click and move hotspots
				continue;

			if (spot.getWindow() != 0)
				// Only check the main window
				continue;

			if (!spot.isIn(_vm->_global->_inter_mouseX, _vm->_global->_inter_mouseY))
				// If we're not in it, ignore it
				continue;

			id    = spot.id;
			index = i;

			return spot.key;
		}

		return 0;

	} else if (type == kTypeClick) {
		// Check if something was clicked

		for (int i = 0; (i < kHotspotCount) && !_hotspots[i].isEnd(); i++) {
			const Hotspot &spot = _hotspots[i];

			if (spot.isDisabled())
				// Only consider enabled hotspots
				continue;

			if (spot.getWindow() != 0)
				// Only check the main window
				continue;

			if (spot.getType() < kTypeMove)
				// Only consider hotspots that can be clicked
				continue;

			if (!spot.isIn(_vm->_global->_inter_mouseX, _vm->_global->_inter_mouseY))
				// If we're not in it, ignore it
				continue;

			if (!spot.buttonMatch(_vm->_game->_mouseButtons))
				// Don't follow hotspots with button requirements we don't meet
				continue;

			id    = spot.id;
			index = i;

			if ((spot.getType() == kTypeMove) || (spot.getType() == kTypeClick))
				// It's a move or click => return the key
				return spot.key;

			// Otherwise, the key has a different meaning, so ignore it
			return 0;
		}

		if (_vm->_game->_mouseButtons != kMouseButtonsLeft)
			// Let the right mouse button act as an escape key
			return kKeyEscape;

		return 0;

	}

	return 0;
}

bool Hotspots::checkHotspotChanged() {
	uint16 key, id, index;

	// Get the current hotspot
	key = checkMouse(kTypeMove, id, index);

	if (key == _currentKey)
		// Nothing changed => nothing to do
		return false;

	// Leave the old area
	if (isValid(_currentKey, _currentId,_currentIndex))
		leave(_currentIndex);

	_currentKey   = key;
	_currentId    = id;
	_currentIndex = index;

	// Enter the new one
	if (isValid(key, id, index))
		enter(index);

	return true;
}

uint16 Hotspots::check(uint8 handleMouse, int16 delay, uint16 &id, uint16 &index) {
	_vm->_game->_scrollHandleMouse = handleMouse != 0;

	if (delay >= -1) {
		_currentKey   = 0;
		_currentId    = 0;
		_currentIndex = 0;
	}

	id    = 0;
	index = 0;

	if (handleMouse) {
		if ((_vm->_draw->_cursorIndex == -1) && (_currentKey == 0)) {
			// Last know state: No hotspot hit. Look if that changed

			_currentKey = checkMouse(kTypeMove, _currentId, _currentIndex);

			if (isValid(_currentKey, _currentId, _currentIndex))
				enter(_currentIndex);
		}

		_vm->_draw->animateCursor(-1);
	}

	uint32 startTime = _vm->_util->getTimeKey();

	// Update display
	_vm->_draw->blitInvalidated();
	_vm->_video->waitRetrace();

	uint16 key = 0;
	while (key == 0) {

		if (_vm->_inter->_terminate || _vm->shouldQuit()) {
			if (handleMouse)
				_vm->_draw->blitCursor();
			return 0;
		}

		// Anything changed?
		checkHotspotChanged();

		// Update display
		if (!_vm->_draw->_noInvalidated) {
			if (handleMouse)
				_vm->_draw->animateCursor(-1);
			else
				_vm->_draw->blitInvalidated();
			_vm->_video->waitRetrace();
		}

		// Update keyboard and mouse state
		key = _vm->_game->checkKeys(&_vm->_global->_inter_mouseX,
				&_vm->_global->_inter_mouseY, &_vm->_game->_mouseButtons, handleMouse);

		if (!handleMouse && (_vm->_game->_mouseButtons != kMouseButtonsNone)) {
			// We don't want any mouse input but got one => Wait till it went away

			_vm->_util->waitMouseRelease(0);
			key = 3;
		}

		if (key != 0) {
			// Got a key press

			if (handleMouse & 1)
				_vm->_draw->blitCursor();

			id    = 0;
			index = 0;

			// Leave the current hotspot
			if (isValid(_currentKey, _currentId, _currentIndex))
				leave(_currentIndex);

			_currentKey = 0;
			break;
		}

		if (handleMouse) {

			if (_vm->_game->_mouseButtons != kMouseButtonsNone) {
				// Mouse button pressed

				if (delay > 0) {
					// If a delay was requested, wait the specified time

					_vm->_draw->animateCursor(2);
					_vm->_util->delay(delay);
				} else if (handleMouse & 1)
					_vm->_util->waitMouseRelease(1);

				_vm->_draw->animateCursor(-1);

				// Which region was clicked?
				key = checkMouse(kTypeClick, id, index);

				if ((key != 0) || (id != 0)) {
					// Got a valid region

					if ( (handleMouse & 1) &&
						  ((delay <= 0) || (_vm->_game->_mouseButtons == kMouseButtonsNone)))
						_vm->_draw->blitCursor();

					// If the hotspot changed, leave the old one
					if (key != _currentKey)
						leave(_currentIndex);

					_currentKey = 0;
					break;
				}

				if (handleMouse & 4)
					// Nothing further than one simple check was requested => return
					return 0;

				// Leave the current area
				if (_currentKey != 0)
					leave(_currentIndex);

				// No click, but do we have a move event? If so, enter that hotspot
				_currentKey = checkMouse(kTypeMove, _currentId, _currentIndex);
				if (isValid(_currentKey, _currentId, _currentIndex))
					enter(_currentIndex);

			} else
				// No mouse button pressed, check whether the position changed at least
				checkHotspotChanged();
		}

		if ((delay == -2) && (key == 0) &&
		    (_vm->_game->_mouseButtons == kMouseButtonsNone)) {
			// Nothing found and no further handling requested. Return.

			id    = 0;
			index = 0;
			break;
		}

		if (handleMouse)
			_vm->_draw->animateCursor(-1);

		if ((delay < 0) && (key == 0) &&
		    (_vm->_game->_mouseButtons == kMouseButtonsNone)) {

			// Look if we've maybe reached the timeout

			uint32 curTime = _vm->_util->getTimeKey();
			if ((curTime + delay) > startTime) {
				// If so, return

				id    = 0;
				index = 0;
				break;
			}

		}

	// Sleep for a short amount of time
	_vm->_util->delay(10);

	}

	return key;
}

uint16 Hotspots::check(uint8 handleMouse, int16 delay) {
	uint16 id, index;

	// Check and ignore the id and index
	return Hotspots::check(handleMouse, delay, id, index);
}

uint16 Hotspots::updateInput(uint16 xPos, uint16 yPos, uint16 width, uint16 height,
		uint16 backColor, uint16 frontColor, char *str, uint16 fontIndex,
		Type type, int16 &duration, uint16 &id, uint16 &index) {

	if ((fontIndex >= 8) || !_vm->_draw->_fonts[fontIndex]) {
		warning("Hotspots::updateInput(): Invalid font specified: %d", fontIndex);
		return 0;
	}

	// Check if we need to consider mouse events
	bool handleMouse = false;
	if ( (_vm->_game->_handleMouse != 0) &&
	    ((_vm->_global->_useMouse != 0) || (_vm->_game->_forceHandleMouse != 0)))
		handleMouse = true;

	const Video::FontDesc &font = *_vm->_draw->_fonts[fontIndex];

	// The font doesn't specify individual character widths => monospaced
	bool monoSpaced = (font.charWidths == 0);

	// Current position in the string, preset to the end
	uint32 pos      = strlen(str);
	/* Size of input field in characters.
	 * If the font is not monospaced, we can't know that */
	uint32 editSize = monoSpaced ? (width / font.itemWidth) : 0;

	uint16 key = 0;
	char tempStr[256];

	while (1) {
		// If we the edit field has enough space, add a space for the new character
		strncpy0(tempStr, str, 254);
		strcat(tempStr, " ");
		if ((editSize != 0) && strlen(tempStr) > editSize)
			strncpy0(tempStr, str, 255);

		// Clear input area
		fillRect(xPos, yPos,
		         monoSpaced ? (editSize * font.itemWidth) : width, height,
		         backColor);

		// Print the current string, vertically centered
		printText(xPos, yPos + (height - font.itemHeight) / 2,
				tempStr, fontIndex, frontColor);

		// If we've reached the end of the input field, set the cursor to the last character
		if ((editSize != 0) && (pos == editSize))
			pos--;

		// The character under the cursor
		char curSym = tempStr[pos];

		if (_vm->_inter->_variables)
			WRITE_VAR(56, pos);

		bool first = true;
		while (1) {
			tempStr[0] = curSym;
			tempStr[1] = 0;

			// Draw cursor
			uint16 cursorX, cursorY, cursorWidth, cursorHeight;
			getTextCursorPos(font, str, pos, xPos, yPos, width, height,
					cursorX, cursorY, cursorWidth, cursorHeight);
			fillRect(cursorX, cursorY, cursorWidth, cursorHeight, frontColor);

			if (first) {
				// The first time, purge old information too
				key = check(handleMouse, -1, id, index);

				if (key == 0)
					// We didn't catch any input, let's try again with a real timeout
					key = check(handleMouse, -300, id, index);

				first = false;
			} else
				// Try to catch a character
				key = check(handleMouse, -300, id, index);

			tempStr[0] = curSym;
			tempStr[1] = 0;

			// Clear cursor
			getTextCursorPos(font, str, pos, xPos, yPos, width, height,
					cursorX, cursorY, cursorWidth, cursorHeight);
			fillRect(cursorX, cursorY, cursorWidth, cursorHeight, backColor);

			// Print the current string, vertically centered
			printText(cursorX, yPos + (height - font.itemHeight) / 2,
					tempStr, fontIndex, frontColor);

			if ((key != 0) || (id != 0))
				// We did get a key, stop looking
				break;

			// Try again
			key = check(handleMouse, -300, id, index);

			if ((key != 0) || (id != 0) ||
					_vm->_inter->_terminate || _vm->shouldQuit())
				// We did get a key, stop looking
				break;

			if (duration > 0) {
				// Look if we reached the time limit
				duration -= 600;
				if (duration <= 1) {
					// If so, abort
					key = 0;
					id  = 0;
					break;
				}
			}
		}

		if ((key == 0) || (id != 0) ||
				_vm->_inter->_terminate || _vm->shouldQuit())
			// Got no key, or a region ID instead, return
			return 0;

		switch (key) {
		case kKeyRight:
			// If possible, move the cursor right
			if (((editSize != 0) && ((pos > strlen(str)) || (pos > (editSize - 1)))) ||
			    ((editSize == 0) && (pos > strlen(str)))) {
				pos++;
				continue;
			}
			// Continue downwards instead
			return kKeyDown;

		case kKeyLeft:
			// If possible, move the cursor left
			if (pos > 0) {
				pos--;
				continue;
			}
			// Continue upwards instead
			return kKeyUp;

		case kKeyBackspace:
			if (pos > 0) {
				// Delete the character to the left
				_vm->_util->cutFromStr(str, pos - 1, 1);
				pos--;
				continue;
			} else {
				if (pos < strlen(str))
					// Delete the character to the right
					_vm->_util->cutFromStr(str, pos, 1);
			}

		case kKeyDelete:
			if (pos >= strlen(str))
				continue;

			// Delete the character to the right
			_vm->_util->cutFromStr(str, pos, 1);
			continue;

		case kKeyReturn:
		case kKeyF1:
		case kKeyF2:
		case kKeyF3:
		case kKeyF4:
		case kKeyF5:
		case kKeyF6:
		case kKeyF7:
		case kKeyF8:
		case kKeyF9:
		case kKeyF10:
		case kKeyUp:
		case kKeyDown:
			return key;

		case kKeyEscape:
			// If we got an escape event, wait until the mouse buttons have been released
			if (_vm->_global->_useMouse != 0)
				continue;

			_vm->_game->_forceHandleMouse = !_vm->_game->_forceHandleMouse;

			handleMouse = false;
			if ( (_vm->_game->_handleMouse != 0) &&
			    ((_vm->_global->_useMouse != 0) || (_vm->_game->_forceHandleMouse != 0)))
				handleMouse = true;

			while (_vm->_global->_pressedKeys[1] != 0);
			continue;

		default:
			// Got a "normal" key

			uint16 savedKey = key;

			key &= 0xFF;

			if (((type == kTypeInputFloatNoLeave) || (type == kTypeInputFloatLeave)) &&
					 (key >= ' ') && (key <= 0xFF)) {

				// Only allow character found in numerical floating values

				const char *str1 = "0123456789-.,+ ";
				const char *str2 = "0123456789-,,+ ";

				if ((((savedKey >> 8) > 1) && ((savedKey >> 8) < 12)) &&
						((_vm->_global->_pressedKeys[42] != 0) ||
						 (_vm->_global->_pressedKeys[56] != 0)))
					key = ((savedKey >> 8) - 1) % 10 + '0';

				int i;
				for (i = 0; str1[i] != 0; i++) {
					if (key == str1[i]) {
						key = str2[i];
						break;
					}
				}

				if (i == (int16) strlen(str1))
					key = 0;
			}

			if ((key >= ' ') && (key <= 0xFF)) {
				if (editSize == 0) {
					// Length of the string + current character + next one
					int length = _vm->_draw->stringLength(str, fontIndex) +
						font.charWidths[' ' - font.startItem] +
						font.charWidths[key - font.startItem];

					if (length > width)
						// We're above the limit, ignore the key
						continue;

					if (((int32) strlen(str)) >= (_vm->_global->_inter_animDataSize * 4 - 1))
						// Above the limit of character allowed in a string, ignore the key
						continue;

				} else {
					if (strlen(str) > editSize)
						// We're over the upper character limit for this field
						continue;
					else if (editSize == strlen(str))
						// We've reached the upper limit, overwrite the last character
						_vm->_util->cutFromStr(str, strlen(str) - 1, 1);
				}

				// Advance cursor
				pos++;
				tempStr[0] = key;
				tempStr[1] = 0;

				// Add character
				_vm->_util->insertStr(tempStr, str, pos - 1);
			}

		}
	}
}

uint16 Hotspots::handleInputs(int16 time, uint16 inputCount, uint16 &curInput,
		InputDesc *inputs, uint16 &id, uint16 &index) {

	// Redraw all texts in all inputs we currently manage
	updateAllTexts(inputs);

	for (int i = 0; i < 40; i++)
		WRITE_VAR(17 + i, 0);

	while (1) {
		// Find the hotspot index to our current input
		uint16 hotspotIndex = inputToHotspot(curInput);

		assert(hotspotIndex != 0xFFFF);

		Hotspot inputSpot = _hotspots[hotspotIndex];

		// Handle input events from that input field
		uint16 key = updateInput(inputSpot.left, inputSpot.top,
				inputSpot.right - inputSpot.left + 1,
				inputSpot.bottom - inputSpot.top + 1,
				inputs[curInput].backColor, inputs[curInput].frontColor,
				GET_VARO_STR(inputSpot.key), inputs[curInput].fontIndex,
				inputSpot.getType(), time, id, index);

		if (_vm->_inter->_terminate)
			return 0;

		switch (key) {
		case kKeyNone:
			if (id == 0)
				// No key and no hotspot => return
				return 0;

			if (_vm->_game->_mouseButtons != kMouseButtonsNone)
				// Clicked something, get the hotspot index
				index = findClickedInput(index);

			if (!_hotspots[index].isInput())
				// It's no input, return
				return 0;

			// Get the associated input index
			curInput = hotspotToInput(index);
			break;

		case kKeyF1:
		case kKeyF2:
		case kKeyF3:
		case kKeyF4:
		case kKeyF5:
		case kKeyF6:
		case kKeyF7:
		case kKeyF8:
		case kKeyF9:
		case kKeyF10:
			return key;

		case kKeyReturn:
			// Just one input => return
			if (inputCount == 1)
				return kKeyReturn;

			// End of input chain reached => wrap
			if (curInput == (inputCount - 1)) {
				curInput = 0;
				break;
			}

			// Next input
			curInput++;
			break;

		case kKeyDown:
			// Next input
			if ((inputCount - 1) > curInput)
				curInput++;
			break;

		case kKeyUp:
			// Previous input
			if (curInput > 0)
				curInput--;
			break;
		}
	}
}

void Hotspots::evaluateNew(uint16 i, uint16 *ids, InputDesc *inputs,
		uint16 &inputId, bool &hasInput, uint16 &inputCount) {

	ids[i] = 0;

	// Type and window
	byte type = _vm->_game->_script->readByte();
	byte window = 0;

	if ((type & 0x40) != 0) {
		// Got a window ID

		type  -= 0x40;
		window = _vm->_game->_script->readByte();
	}

	// Coordinates
	uint16 left, top, width, height, right, bottom;
	uint32 funcPos = 0;
	if ((type & 0x80) != 0) {
		// Complex coordinate expressions
		funcPos = _vm->_game->_script->pos();
		left    = _vm->_game->_script->readValExpr();
		top     = _vm->_game->_script->readValExpr();
		width   = _vm->_game->_script->readValExpr();
		height  = _vm->_game->_script->readValExpr();
	} else {
		// Immediate values
		funcPos = 0;
		left    = _vm->_game->_script->readUint16();
		top     = _vm->_game->_script->readUint16();
		width   = _vm->_game->_script->readUint16();
		height  = _vm->_game->_script->readUint16();
	}
	type &= 0x7F;

	// Apply global drawing offset
	if ((_vm->_draw->_renderFlags & RENDERFLAG_CAPTUREPOP) && (left != 0xFFFF)) {
		left += _vm->_draw->_backDeltaX;
		top  += _vm->_draw->_backDeltaY;
	}

	right  = left + width  - 1;
	bottom = top  + height - 1;

	// Enabling the hotspots again
	if ((type == kTypeEnable2) || (type == kTypeEnable1)) {
		uint8 wantedState = 0;
		if (type == kTypeEnable2)
			wantedState = kStateFilledDisabled | kStateType2;
		else
			wantedState = kStateFilledDisabled | kStateType1;

		_vm->_game->_script->skip(6);

		for (int j = 0; j < kHotspotCount; j++) {
			Hotspot &spot = _hotspots[j];

			if (spot.getState() == wantedState) {
				spot.enable();
				spot.funcEnter = _vm->_game->_script->pos();
				spot.funcLeave = _vm->_game->_script->pos();
			}
		}

		_vm->_game->_script->skipBlock();

		return;
	}

	int16 key   = 0;
	int16 flags = 0;
	Video::FontDesc *font = 0;
	uint32 funcEnter = 0, funcLeave = 0;

	// Evaluate parameters for the new hotspot
	switch (type) {
	case kTypeNone:
		_vm->_game->_script->skip(6);

		funcEnter = _vm->_game->_script->pos();
		_vm->_game->_script->skipBlock();

		funcLeave = _vm->_game->_script->pos();
		_vm->_game->_script->skipBlock();

		key   = i + ((kStateFilled | kStateType2) << 12);
		flags = type + (window << 8);
		break;

	case kTypeMove:
		key    = _vm->_game->_script->readInt16();
		ids[i] = _vm->_game->_script->readInt16();
		flags  = _vm->_game->_script->readInt16();

		funcEnter = _vm->_game->_script->pos();
		_vm->_game->_script->skipBlock();

		funcLeave = _vm->_game->_script->pos();
		_vm->_game->_script->skipBlock();

		if (key == 0)
			key = i + ((kStateFilled | kStateType2) << 12);

		flags = type + (window << 8) + (flags << 4);
		break;

	case kTypeInput1NoLeave:
	case kTypeInput1Leave:
	case kTypeInput2NoLeave:
	case kTypeInput2Leave:
	case kTypeInput3NoLeave:
	case kTypeInput3Leave:
	case kTypeInputFloatNoLeave:
	case kTypeInputFloatLeave:
		hasInput = true;

		_vm->_util->clearKeyBuf();

		// Input text parameters
		key                           = _vm->_game->_script->readVarIndex();
		inputs[inputCount].fontIndex  = _vm->_game->_script->readInt16();
		inputs[inputCount].backColor  = _vm->_game->_script->readByte();
		inputs[inputCount].frontColor = _vm->_game->_script->readByte();
		inputs[inputCount].length     = 0;
		inputs[inputCount].str        = 0;

		if ((type >= kTypeInput2NoLeave) && (type <= kTypeInput3Leave)) {
			uint16 length = _vm->_game->_script->readUint16();

			inputs[inputCount].str =
				(const char *) (_vm->_game->_script->getData() + _vm->_game->_script->pos());

			_vm->_game->_script->skip(length);
		}

		if (left == 0xFFFF) {
			if (!(type & 1))
				// No coordinates but a leave block => skip it
				_vm->_game->_script->skipBlock();
			break;
		}

		font = _vm->_draw->_fonts[inputs[inputCount].fontIndex];
		if (!font->charWidths)
			// Monospaced font
			right = left + width * font->itemWidth - 1;

		funcEnter = 0;
		funcPos   = 0;
		funcLeave = 0;
		if (!(type & 1)) {
			// Got a leave
			funcLeave = _vm->_game->_script->pos();
			_vm->_game->_script->skipBlock();
		}

		flags = type;

		inputCount++;
		break;

	case 20:
		inputId = i;
		// Fall through to case 2
	case kTypeClick:
		key    = _vm->_game->_script->readInt16();
		ids[i] = _vm->_game->_script->readInt16();
		flags  = _vm->_game->_script->readInt16();

		funcEnter = 0;

		funcLeave = _vm->_game->_script->pos();
		_vm->_game->_script->skipBlock();

		flags = ((uint16) kTypeClick) + (window << 8) + (flags << 4);
		break;

	case kTypeClickEnter:
		key    = _vm->_game->_script->readInt16();
		ids[i] = _vm->_game->_script->readInt16();
		flags  = _vm->_game->_script->readInt16() & 3;

		funcEnter = _vm->_game->_script->pos();
		_vm->_game->_script->skipBlock();

		funcLeave = 0;

		flags = ((uint16) kTypeClick) + (window << 8) + (flags << 4);
		break;
	}

	// Add the new hotspot
	add(i | (kStateFilled << 12), left, top, right, bottom,
			flags, key, funcEnter, funcLeave, funcPos);
}

bool Hotspots::evaluateFind(uint16 key, int16 timeVal, const uint16 *ids,
		uint16 hotspotIndex1, uint16 hotspotIndex2, uint16 endIndex,
		int16 &duration, uint16 &id, uint16 &index, bool &finished) {

	if (id != 0)
		// We already found a hotspot, nothing to do
		return true;

	if (key != 0) {
		// We've got a key

		// Find the hotspot with that key associated
		findKey(key, id, index);
		if (id != 0)
			// Found it
			return true;

		// Try it case insensitively
		findKeyCaseInsensitive(key, id, index);
		if (id != 0)
			// Found it
			return true;

		return false;
	}

	if (duration != 0) {
		// We've got a time duration

		if        (hotspotIndex1 != 0) {
			finished =
				leaveNthPlain(hotspotIndex1, endIndex, timeVal, ids, id, index, duration);
		} else if (hotspotIndex2 != 0) {
			findNthPlain(hotspotIndex2, endIndex, id, index);
		} else {
			findNthPlain(0, 0, id, index);

			// Leave the current hotspot
			if ((_currentKey != 0) && (_hotspots[_currentIndex].funcLeave != 0))
				call(_hotspots[_currentIndex].funcLeave);

			_currentKey = 0;
		}

		if (id != 0)
			return true;

		return false;
	}

	return false;
}

void Hotspots::evaluate() {
	InputDesc inputs[20];
	uint16 ids[kHotspotCount];

	// Push all local hotspots
	push(0);

	// Find the current end of the hotspot block
	uint16 endIndex = 0;
	while (!_hotspots[endIndex].isEnd())
		endIndex++;

	_shouldPush = false;

	_vm->_game->_script->skip(1);

	// Number of new hotspots
	byte count = _vm->_game->_script->readByte();

	// Parameters of this block
	_vm->_game->_handleMouse = _vm->_game->_script->peekByte(0);
	int16 duration           = _vm->_game->_script->peekByte(1);
	byte hotspotIndex1       = _vm->_game->_script->peekByte(3);
	byte hotspotIndex2       = _vm->_game->_script->peekByte(4);
	bool needRecalculation   = _vm->_game->_script->peekByte(5) != 0;

	// Seconds -> Milliseconds
	duration *= 1000;

	if ((hotspotIndex1 != 0) || (hotspotIndex2 != 0)) {
		duration /= 100;
		if (_vm->_game->_script->peekByte(1) == 100)
			duration = 2;
	}

	int16 timeVal = duration;

	_vm->_game->_script->skip(6);

	setCurrentHotspot(0, 0);

	bool finishedDuration = false;

	uint16 id      = 0;
	uint16 inputId = 0xFFFF;
	uint16 index   = 0;

	bool   hasInput   = false;
	uint16 inputCount = 0;

	// Adding new hotspots
	for (uint16 i = 0; i < count; i++)
		evaluateNew(i, ids, inputs, inputId, hasInput, inputCount);

	// Recalculate all hotspots if requested
	if (needRecalculation)
		recalculate(true);

	_vm->_game->_forceHandleMouse = 0;
	_vm->_util->clearKeyBuf();

	while ((id == 0) && !_vm->_inter->_terminate && !_vm->shouldQuit()) {
		uint16 key = 0;
		if (hasInput) {
			// Input

			uint16 curInput = 0;

			key = handleInputs(duration, inputCount, curInput, inputs, id, index);

			// Notify the script of the current input index
			WRITE_VAR(17 + 38, curInput);
			if (key == kKeyReturn) {
				// Return pressed, invoke input leave
				findFirstInputLeave(id, inputId, index);
				break;
			}
		} else
			// Normal move or click check
			key = check(_vm->_game->_handleMouse, -duration, id, index);

		key = convertSpecialKey(key);

		// Try to find a fitting hotspot
		Hotspots::evaluateFind(key, timeVal, ids, hotspotIndex1, hotspotIndex2, endIndex,
				duration, id, index, finishedDuration);

		if (finishedDuration)
			break;

		if ((id == 0) || (_hotspots[index].funcLeave != 0))
			// We don't have a new ID, but haven't yet handled the leave function
			continue;

		_vm->_inter->storeMouse();

		setCurrentHotspot(ids, id);

		// Enter it
		if (_hotspots[index].funcEnter != 0)
			call(_hotspots[index].funcEnter);

		setCurrentHotspot(0, 0);
		id = 0;
	}

	if ((id & 0xFFF) == inputId)
		matchInputStrings(inputs);

	if (_vm->_game->_handleMouse == 1)
		_vm->_draw->blitCursor();

	if (!_vm->_inter->_terminate && (!finishedDuration)) {
		_vm->_game->_script->seek(_hotspots[index].funcLeave);

		_vm->_inter->storeMouse();
		if (getCurrentHotspot() == 0) {
			// No hotspot currently handled, now we'll handle the newly found one

			setCurrentHotspot(ids, id);
		}
	} else
		_vm->_game->_script->setFinished(true);

	for (int i = 0; i < count; i++)
		// Remove all local hotspots
		remove(i + (kStateFilled << 12));

	for (int i = 0; i < kHotspotCount; i++) {
		Hotspot &spot = _hotspots[i];

		// Disable the ones still there
		if ((spot.getState() == (kStateFilled | kStateType1)) ||
				(spot.getState() == (kStateFilled | kStateType2)))
				spot.disable();
	}

}

int16 Hotspots::findCursor(uint16 x, uint16 y) const {
	int16 cursor = 0;

	for (int i = 0; (i < kHotspotCount) && !_hotspots[i].isEnd(); i++) {
		const Hotspot &spot = _hotspots[i];

		if ((spot.getWindow() != 0) || spot.isDisabled())
			// Ignore disabled and non-main-windowed hotspots
			continue;

		if (!spot.isIn(x, y))
			// We're not in that hotspot, ignore it
			continue;

		if (spot.getCursor() == 0) {
			// Hotspot doesn't itself specify a cursor...
			if (spot.getType() >= kTypeInput1NoLeave) {
				// ...but the type has a generic one
				cursor = 3;
				break;
			} else if ((spot.getButton() != kMouseButtonsRight) && (cursor == 0))
				// ...but there's a generic "click" cursor
				cursor = 1;
		} else if (cursor == 0)
			// Hotspot had an attached cursor index
			cursor = spot.getCursor();
	}

	return cursor;
}

uint16 Hotspots::inputToHotspot(uint16 input) const {
	uint16 inputIndex = 0;
	for (int i = 0; i < kHotspotCount; i++) {
		const Hotspot &spot = _hotspots[i];

		if (!spot.isActiveInput())
			// Not an active input
			continue;

		if (inputIndex == input)
			// We've found our input
			return i;

		// Next one
		inputIndex++;
	}

	// None found
	return 0xFFFF;
}

uint16 Hotspots::hotspotToInput(uint16 hotspot) const {
	uint16 input = 0;

	for (int i = 0; i < kHotspotCount; i++) {
		const Hotspot &spot = _hotspots[i];

		if (!spot.isActiveInput())
			// Not an active input
			continue;

		if (i == hotspot)
			// We've found our hotspot
			break;

		// Next one
		input++;
	}

	return input;
}

uint16 Hotspots::findClickedInput(uint16 index) const {
	for (int i = 0; (i < kHotspotCount) && !_hotspots[i].isEnd(); i++) {
		const Hotspot &spot = _hotspots[i];

		if (spot.getWindow() != 0)
			// Ignore other windows
			continue;

		if (spot.isDisabled())
			// Ignore disabled hotspots
			continue;

		if (!spot.isIn(_vm->_global->_inter_mouseX, _vm->_global->_inter_mouseY))
			// This one wasn't it
			continue;

		if (spot.getCursor() != 0)
			// This one specifies a cursor, so we don't want it
			continue;

		if (!spot.isInput())
			// It's no input
			continue;

		index = i;
		break;
	}

	return index;
}

bool Hotspots::findFirstInputLeave(uint16 &id, uint16 &inputId, uint16 &index) const {
	for (int i = 0; (i < kHotspotCount) && !_hotspots[i].isEnd(); i++) {
		const Hotspot &spot = _hotspots[i];

		if (!spot.isFilledEnabled())
			// Not filled or disabled
			continue;

		if (!spot.isInputLeave())
			// Not an input with a leave function
			continue;

		id      = spot.id;
		inputId = spot.id & 0x7FFF;
		index   = i;
		return true;
	}

	return false;
}

bool Hotspots::findKey(uint16 key, uint16 &id, uint16 &index) const {
	id    = 0;
	index = 0;

	for (int i = 0; (i < kHotspotCount) && !_hotspots[i].isEnd(); i++) {
		const Hotspot &spot = _hotspots[i];

		if (!spot.isFilledEnabled())
			// Not filled or disabled
			continue;

		//      Key match              Catch all
		if ((spot.key == key) || (spot.key == 0x7FFF)) {
			id    = spot.id;
			index = i;
			return true;
		}
	}

	return false;
}

bool Hotspots::findKeyCaseInsensitive(uint16 key, uint16 &id, uint16 &index) const {
	id    = 0;
	index = 0;

	for (int i = 0; (i < kHotspotCount) && !_hotspots[i].isEnd(); i++) {
		const Hotspot &spot = _hotspots[i];

		if (!spot.isFilledEnabled())
			// Not filled or disabled, ignore
			continue;

		if ((spot.key & 0xFF00) != 0)
			continue;

		if (spot.key == 0)
			// No associated key, ignore
			continue;

		// Compare
		if (toupper(key & 0xFF) == toupper(spot.key)) {
			id    = spot.id;
			index = i;
			return true;
		}
	}

	return false;
}

bool Hotspots::findNthPlain(uint16 n, uint16 startIndex, uint16 &id, uint16 &index) const {
	id    = 0;
	index = 0;

	uint16 counter = 0;
	for (int i = startIndex; (i < kHotspotCount) && !_hotspots[i].isEnd(); i++) {
		const Hotspot &spot = _hotspots[i];

		if (!spot.isFilledNew())
			// Not filled, ignore
			continue;

		if (++counter != n)
			// Not yet the one we want
			continue;

		id    = spot.id;
		index = i;
		return true;
	}

	return false;
}

bool Hotspots::leaveNthPlain(uint16 n, uint16 startIndex, int16 timeVal, const uint16 *ids,
		uint16 &id, uint16 &index, int16 &duration) {

	id    = 0;
	index = 0;

	if (!findNthPlain(n, startIndex, id, index))
		// Doesn't exist
		return false;

	_vm->_inter->storeMouse();

	if (getCurrentHotspot() != 0)
		// We already handle a hotspot
		return false;

	setCurrentHotspot(ids, id);

	const Hotspot &spot = _hotspots[index];
	if (spot.funcLeave != 0) {
		// It has a leave function

		uint32 startTime, callTime;

		// Call the leave and time it
		startTime = _vm->_util->getTimeKey();
		call(spot.funcLeave);
		callTime = _vm->_util->getTimeKey() - startTime;

		// Remove the time it took from the time we have available
		duration = CLIP<int>(timeVal - callTime, 2, timeVal);
	}

	if (getCurrentHotspot() == 0) {
		id    = 0;
		index = 0;
	}

	return getCurrentHotspot() != 0;
}

void Hotspots::setCurrentHotspot(const uint16 *ids, uint16 id) const {
	if (!ids) {
		WRITE_VAR(16, 0);
		return;
	}

	if (Hotspot::getState(id) == kStateFilled)
		WRITE_VAR(16, ids[id & 0xFFF]);
	else
		WRITE_VAR(16, id & 0xFFF);
}

uint32 Hotspots::getCurrentHotspot() const {
	return VAR(16);
}

void Hotspots::cleanFloatString(const Hotspot &spot) const {
	static char tempStr[256];	//the static keyword is unnecessary, but was added to work around a PSP compiler ICE.

	// Get the string
	strncpy0(tempStr, GET_VARO_STR(spot.key), 255);

	// Remove spaces
	char *ptr;
	while ((ptr = strchr(tempStr, ' ')))
		_vm->_util->cutFromStr(tempStr, (ptr - tempStr), 1);

	// Exchange decimal separator if needed
	if (_vm->_global->_language == kLanguageBritish)
		while ((ptr = strchr(tempStr, '.')))
			*ptr = ',';

	// Write it back
	WRITE_VARO_STR(spot.key, tempStr);
}

void Hotspots::checkStringMatch(const Hotspot &spot, const InputDesc &input,
		uint16 inputPos) const {

	const char *str = input.str;

	char tempStr[256];
	char spotStr[256];

	strncpy0(tempStr, GET_VARO_STR(spot.key), 255);

	if (spot.getType() < kTypeInput3NoLeave)
		_vm->_util->cleanupStr(tempStr);

	uint16 pos = 0;
	do {
		strncpy0(spotStr, str, 255);

		pos += strlen(str) + 1;
		str += strlen(str) + 1;

		if (spot.getType() < kTypeInput3NoLeave)
			_vm->_util->cleanupStr(spotStr);

		// Compare the entered string with the string we wanted
		if (strcmp(tempStr, spotStr) == 0) {
			WRITE_VAR(17, VAR(17) + 1);
			WRITE_VAR(17 + inputPos, 1);
			break;
		}
	} while (input.length > pos);
}

void Hotspots::matchInputStrings(const InputDesc *inputs) const {
	uint16 strInputCount = 0;
	uint16 inputIndex    = 0;
	uint16 inputPos      = 1;

	for (int i = 0; i < kHotspotCount; i++) {
		const Hotspot &spot = _hotspots[i];

		// Looking for all enabled inputs
		if (spot.isEnd())
			continue;
		if (!spot.isFilledEnabled())
			continue;
		if (!spot.isInput())
			continue;

		if (spot.getType() >= kTypeInputFloatNoLeave)
			cleanFloatString(spot);

		if ((spot.getType() >= kTypeInput2NoLeave) && (spot.getType() <= kTypeInput3Leave)) {

			// Look if we find a match between the wanted and the typed string
			checkStringMatch(spot, inputs[inputIndex], inputPos);
			strInputCount++;
		} else
			WRITE_VAR(17 + inputPos, 2);

		inputIndex++;
		inputPos++;
	}

	// Notify the scripts if we reached the requested hotspot
	WRITE_VAR(17, (uint32) (strInputCount == ((uint16) VAR(17))));
}

uint16 Hotspots::convertSpecialKey(uint16 key) const {
	if (((key & 0xFF) >= ' ') && ((key & 0xFF) <= 0xFF) &&
			((key >> 8) > 1) && ((key >> 8) < 12))
		key = '0' + (((key >> 8) - 1) % 10) + (key & 0xFF00);

	return key;
}

void Hotspots::getTextCursorPos(const Video::FontDesc &font, const char *str,
		uint32 pos, uint16 x, uint16 y, uint16 width, uint16 height,
		uint16 &cursorX, uint16 &cursorY, uint16 &cursorWidth, uint16 &cursorHeight) const {

	if (font.charWidths) {
		// Cursor to the right of the current character

		cursorX      = x;
		cursorY      = y;
		cursorWidth  = 1;
		cursorHeight = height;

		// Iterate through the string and add each character's width
		for (uint32 i = 0; i < pos; i++)
			cursorX += font.charWidths[str[i] - font.startItem];

	} else {
		// Cursor underlining the current character

		cursorX      = x + font.itemWidth * pos;
		cursorY      = y + height - 1;
		cursorWidth  = font.itemWidth;
		cursorHeight = 1;
	}
}

void Hotspots::fillRect(uint16 x, uint16 y, uint16 width, uint16 height, uint16 color) const {
	_vm->_draw->_destSurface  = 21;
	_vm->_draw->_destSpriteX  = x;
	_vm->_draw->_destSpriteY  = y;
	_vm->_draw->_spriteRight  = width;
	_vm->_draw->_spriteBottom = height;
	_vm->_draw->_backColor    = color;

	_vm->_draw->spriteOperation(DRAW_FILLRECT | 0x10 );
}

void Hotspots::printText(uint16 x, uint16 y, const char *str, uint16 fontIndex, uint16 color) const {
	_vm->_draw->_destSpriteX  = x;
	_vm->_draw->_destSpriteY  = y;
	_vm->_draw->_frontColor   = color;
	_vm->_draw->_fontIndex    = fontIndex;
	_vm->_draw->_textToPrint  = str;
	_vm->_draw->_transparency = 1;

	_vm->_draw->spriteOperation(DRAW_PRINTTEXT | 0x10);
}

void Hotspots::updateAllTexts(const InputDesc *inputs) const {
	uint16 input = 0;

	for (int i = 0; i < kHotspotCount; i++) {
		const Hotspot &spot = _hotspots[i];

		if (spot.isEnd())
			// It's an end, we don't want it
			continue;

		if (!spot.isFilledEnabled())
			// This one's either not used or disabled
			continue;

		if (!spot.isInput())
			// Not an input
			continue;

		// Get its text
		char tempStr[256];
		strncpy0(tempStr, GET_VARO_STR(spot.key), 255);

		// Coordinates
		uint16 x      = spot.left;
		uint16 y      = spot.top;
		uint16 width  = spot.right  - spot.left + 1;
		uint16 height = spot.bottom - spot.top  + 1;
		// Clear the background
		fillRect(x, y, width, height, inputs[input].backColor);

		// Center the text vertically
		y += (height - _vm->_draw->_fonts[_vm->_draw->_fontIndex]->itemHeight) / 2;

		// Draw it
		printText(x, y, tempStr, inputs[input].fontIndex, inputs[input].frontColor);

		input++;
	}
}

} // End of namespace Gob
