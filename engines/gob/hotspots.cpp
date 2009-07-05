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

bool Hotspots::Hotspot::isEnd() const {
	return (left == 0xFFFF);
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
		return true;

	if (myButton == kMouseButtonsNone)
		return false;

	if (myButton == button)
		return true;

	return false;
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

		// When updating, keep bit 0x4000 intact
		uint16 id = hotspot.id;
		if ((spot.id & 0xBFFF) == (hotspot.id & 0xBFFF))
			id = spot.id;

		// Set
		spot    = hotspot;
		spot.id = id;

		// Remember the current script
		spot.script = _vm->_game->_script;

		return i;
	}

	error("Hotspots::add(): Hotspot array full");
	return 0xFFFF;
}

void Hotspots::remove(uint16 id) {
	for (int i = 0; i < kHotspotCount; i++) {
		if (_hotspots[i].id == id)
			_hotspots[i].clear();
	}
}

void Hotspots::removeState(uint16 state) {
	for (int i = 0; i < kHotspotCount; i++) {
		Hotspot &spot = _hotspots[i];

		if ((spot.id & 0xF000) == state)
			spot.clear();
	}
}

void Hotspots::recalculate(bool force) {
	for (int i = 0; (i < kHotspotCount) && !_hotspots[i].isEnd(); i++) {
		Hotspot &spot = _hotspots[i];

		if (!force && ((spot.flags & 0x80) != 0))
			continue;

		if (spot.funcPos == 0)
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
		if ((spot.id & 0xF000) == 0xA000)
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

		if ((spot.id & 0xF000) == 0xA000)
			spot.flags = flags;

		// Return
		_vm->_game->_script->pop();

		_vm->_game->_script = curScript;
	}
}

void Hotspots::push(uint8 all, bool force) {
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
		     // Only save the ones with the correct state
		    ((all == 2) && (((spot.id & 0xF000) == 0xD000) ||
		                    ((spot.id & 0xF000) == 0x4000) ||
		                    ((spot.id & 0xF000) == 0xE000)))) {
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
		     // Only save the ones with the correct state
		    ((all == 2) && (((spot.id & 0xF000) == 0xD000) ||
		                    ((spot.id & 0xF000) == 0x4000) ||
		                    ((spot.id & 0xF000) == 0xE000)))) {

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

	if (!(id & 0x8000))
			return false;

	return true;
}

void Hotspots::call(uint16 offset) {
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
	if (index >= kHotspotCount) {
		warning("Hotspots::enter(): Index %d out of range", index);
		return;
	}

	Hotspot &spot = _hotspots[index];

	if (((spot.id & 0xF000) == 0xA000) || ((spot.id & 0xF000) == 0x9000))
		WRITE_VAR(17, -(spot.id & 0x0FFF));

	if (spot.funcEnter != 0)
		call(spot.funcEnter);
}

void Hotspots::leave(uint16 index) {
	if (index >= kHotspotCount) {
		warning("Hotspots::leave(): Index %d out of range", index);
		return;
	}

	Hotspot &spot = _hotspots[index];

	if (((spot.id & 0xF000) == 0xA000) || ((spot.id & 0xF000) == 0x9000))
		WRITE_VAR(17, spot.id & 0x0FFF);

	if (spot.funcLeave != 0)
		call(spot.funcLeave);
}

uint16 Hotspots::checkMouse(Type type, uint16 &id, uint16 &index) const {
	id    = 0;
	index = 0;

	if        (type == kTypeMove) {

		for (int i = 0; (i < kHotspotCount) && !_hotspots[i].isEnd(); i++) {
			Hotspot &spot = _hotspots[i];

			if (spot.id & 0x4000)
				continue;

			if (spot.getType() > kTypeMove)
				continue;

			if (spot.getWindow() != 0)
				continue;

			if (!spot.isIn(_vm->_global->_inter_mouseX, _vm->_global->_inter_mouseY))
				continue;

			id    = spot.id;
			index = i;

			return spot.key;
		}

		return 0;

	} else if (type == kTypeClick) {

		for (int i = 0; (i < kHotspotCount) && !_hotspots[i].isEnd(); i++) {
			Hotspot &spot = _hotspots[i];

			if (spot.id & 0x4000)
				continue;

			if (spot.getWindow() != 0)
				continue;

			if (spot.getType() < kTypeMove)
				continue;

			if (!spot.isIn(_vm->_global->_inter_mouseX, _vm->_global->_inter_mouseY))
				continue;

			if (!spot.buttonMatch(_vm->_game->_mouseButtons))
				continue;

			id    = spot.id;
			index = i;

			if ((spot.getType() == kTypeMove) || (spot.getType() == kTypeClick))
				return spot.key;

			return 0;
		}

		if (_vm->_game->_mouseButtons != kMouseButtonsLeft)
			return kKeyEscape;

		return 0;

	}

	return 0;
}

void Hotspots::checkHotspotChanged() {
	uint16 key, id, index;

	key = checkMouse(kTypeMove, id, index);

	if (key == _currentKey)
		return;

	if (isValid(_currentKey, _currentId, _currentIndex))
		leave(_currentIndex);

	_currentKey   = key;
	_currentId    = id;
	_currentIndex = index;

	if (isValid(key, id, index))
		enter(index);
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
			_currentKey = checkMouse(kTypeMove, _currentId, _currentIndex);

			if (isValid(_currentKey, _currentId, _currentIndex))
				enter(_currentIndex);
		}

		_vm->_draw->animateCursor(-1);
	}

	uint32 startTime = _vm->_util->getTimeKey();

	_vm->_draw->blitInvalidated();
	_vm->_video->retrace();

	uint16 key = 0;
	while (key == 0) {

		if (_vm->_inter->_terminate || _vm->shouldQuit()) {
			if (handleMouse)
				_vm->_draw->blitCursor();
			return 0;
		}

		checkHotspotChanged();

		if (!_vm->_draw->_noInvalidated) {
			if (handleMouse)
				_vm->_draw->animateCursor(-1);
			else
				_vm->_draw->blitInvalidated();
			_vm->_video->waitRetrace();
		}

		key = _vm->_game->checkKeys(&_vm->_global->_inter_mouseX,
				&_vm->_global->_inter_mouseY, &_vm->_game->_mouseButtons, handleMouse);

		if (!handleMouse && (_vm->_game->_mouseButtons != kMouseButtonsNone)) {
			_vm->_util->waitMouseRelease(0);
			key = 3;
		}

		if (key != 0) {
			if (handleMouse & 1)
				_vm->_draw->blitCursor();

			id    = 0;
			index = 0;

			if (isValid(_currentKey, _currentId, _currentIndex))
				leave(_currentIndex);

			_currentKey = 0;
			break;
		}

		if (handleMouse) {

			if (_vm->_game->_mouseButtons != kMouseButtonsNone) {

				if (delay > 0) {
					_vm->_draw->animateCursor(2);
					_vm->_util->delay(delay);
				} else if (handleMouse & 1)
					_vm->_util->waitMouseRelease(1);

				_vm->_draw->animateCursor(-1);

				key = checkMouse(kTypeClick, id, index);

				if ((key != 0) || (id != 0)) {
					if ( (handleMouse & 1) &&
						  ((delay <= 0) || (_vm->_game->_mouseButtons == kMouseButtonsNone)))
						_vm->_draw->blitCursor();

					if (key != _currentKey)
						leave(_currentIndex);

					_currentKey = 0;
					break;
				}

				if (handleMouse & 4)
					return 0;

				if (_currentKey != 0)
					leave(_currentIndex);

				_currentKey = checkMouse(kTypeMove, _currentId, _currentIndex);
				if (isValid(_currentKey, _currentId, _currentIndex))
					enter(_currentIndex);

			} else
				checkHotspotChanged();

		}

		if ((delay == -2) && (key == 0) &&
		    (_vm->_game->_mouseButtons == kMouseButtonsNone)) {

			id    = 0;
			index = 0;
			break;
		}

		if (handleMouse)
			_vm->_draw->animateCursor(-1);

		if ((delay < 0) && (key == 0) &&
		    (_vm->_game->_mouseButtons == kMouseButtonsNone)) {

			uint32 curTime = _vm->_util->getTimeKey();
			// Timeout reached?
			if ((curTime + delay) > startTime) {
				id    = 0;
				index = 0;
				break;
			}

		}

	_vm->_util->delay(10);

	}

	return key;
}

uint16 Hotspots::check(uint8 handleMouse, int16 delay) {
	uint16 id, index;

	return Hotspots::check(handleMouse, delay, id, index);
}

uint16 Hotspots::readString(uint16 xPos, uint16 yPos, uint16 width, uint16 height,
		uint16 backColor, uint16 frontColor, char *str, uint16 fontIndex,
		Type type, int16 &duration, uint16 &id, uint16 index) {

	if ((fontIndex >= 8) || !_vm->_draw->_fonts[fontIndex])
		return 0;

	bool handleMouse = false;
	if ( (_vm->_game->_handleMouse != 0) &&
	    ((_vm->_global->_useMouse != 0) || (_vm->_game->_forceHandleMouse != 0)))
		handleMouse = true;

	Video::FontDesc &font = *_vm->_draw->_fonts[fontIndex];

	bool monoSpaced = (font.extraData == 0);

	uint32 pos            = strlen(str);
	uint32 editSize       = monoSpaced ? 0 : (width / font.itemWidth);

	uint16 key = 0;
	char tempStr[256];

	while (1) {
		strncpy0(tempStr, str, 254);
		strcat(tempStr, " ");
		if ((editSize != 0) && strlen(tempStr) > editSize)
			strncpy0(tempStr, str, 255);

		_vm->_draw->_destSpriteX  = xPos;
		_vm->_draw->_destSpriteY  = yPos;
		_vm->_draw->_spriteRight  = monoSpaced ? (editSize * font.itemWidth) : width;
		_vm->_draw->_spriteBottom = height;

		_vm->_draw->_destSurface  = 21;
		_vm->_draw->_backColor    = backColor;
		_vm->_draw->_frontColor   = frontColor;
		_vm->_draw->_textToPrint  = tempStr;
		_vm->_draw->_transparency = 1;
		_vm->_draw->_fontIndex    = fontIndex;
		_vm->_draw->spriteOperation(DRAW_FILLRECT | 0x10 );

		_vm->_draw->_destSpriteY  = yPos + (height - font.itemHeight) / 2;
		_vm->_draw->spriteOperation(DRAW_PRINTTEXT | 0x10);

		if ((editSize != 0) && (pos == editSize))
			pos--;

		char curSym = tempStr[pos];

		if (_vm->_inter->_variables)
			WRITE_VAR(56, pos);

		bool first = true;
		while (1) {
			tempStr[0] = curSym;
			tempStr[1] = 0;

			if (font.extraData) {
				_vm->_draw->_destSpriteY  = yPos;
				_vm->_draw->_spriteBottom = height;
				_vm->_draw->_spriteRight  = 1;

				_vm->_draw->_destSpriteX = xPos;
				for (uint32 j = 0; j < pos; j++)
					_vm->_draw->_destSpriteX += font.extraData[str[j] - font.startItem];

			} else {
				_vm->_draw->_destSpriteX  = xPos + font.itemWidth * pos;
				_vm->_draw->_destSpriteY  = yPos + height - 1;
				_vm->_draw->_spriteRight  = font.itemWidth;
				_vm->_draw->_spriteBottom = 1;
			}

			_vm->_draw->_destSurface = 21;
			_vm->_draw->_backColor   = frontColor;
			_vm->_draw->spriteOperation(DRAW_FILLRECT | 0x10);

			if (first) {
				key = check(handleMouse, -1, id, index);

				if (key == 0)
					key = check(handleMouse, -300, id, index);

				first = false;
			} else
				key = check(handleMouse, -300, id, index);

			tempStr[0] = curSym;
			tempStr[1] = 0;

			if (font.extraData) {
				_vm->_draw->_destSpriteY  = yPos;
				_vm->_draw->_spriteBottom = height;
				_vm->_draw->_spriteRight  = 1;

				_vm->_draw->_destSpriteX = xPos;
				for (uint32 j = 0; j < pos; j++)
					_vm->_draw->_destSpriteX += font.extraData[str[j] - font.startItem];

			} else {
				_vm->_draw->_destSpriteX  = xPos + font.itemWidth * pos;
				_vm->_draw->_destSpriteY  = yPos + height - 1;
				_vm->_draw->_spriteRight  = font.itemWidth;
				_vm->_draw->_spriteBottom = 1;
			}

			_vm->_draw->_destSurface  = 21;
			_vm->_draw->_backColor    = backColor;
			_vm->_draw->_frontColor   = frontColor;
			_vm->_draw->_textToPrint  = tempStr;
			_vm->_draw->_transparency = 1;
			_vm->_draw->_fontIndex    = fontIndex;
			_vm->_draw->spriteOperation(DRAW_FILLRECT | 0x10);

			_vm->_draw->_destSpriteY = yPos + (height - font.itemHeight) / 2;
			_vm->_draw->spriteOperation(DRAW_PRINTTEXT | 0x10);

			if ((key != 0) || (id != 0))
				break;

			key = check(handleMouse, -300, id, index);

			if ((key != 0) || (id != 0) ||
					_vm->_inter->_terminate || _vm->shouldQuit())
				break;

			if (duration > 0) {
				duration -= 600;
				if (duration <= 1) {
					key = 0;
					id  = 0;
					break;
				}
			}
		}

		if ((key == 0) || (id != 0) ||
				_vm->_inter->_terminate || _vm->shouldQuit())
			return 0;

		switch (key) {
		case kKeyRight:
			if ((pos > strlen(str)) || (pos > (editSize - 1)) || (editSize == 0)) {
				pos++;
				continue;
			}
			return kKeyDown;

		case kKeyLeft:
			if (pos > 0) {
				pos--;
				continue;
			}
			return kKeyUp;

		case kKeyBackspace:
			if (pos > 0) {
				_vm->_util->cutFromStr(str, pos - 1, 1);
				pos--;
				continue;
			} else {
				if (pos < strlen(str))
					_vm->_util->cutFromStr(str, pos, 1);
			}

		case kKeyDelete:
			if (pos >= strlen(str))
				continue;

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
			if (_vm->_global->_useMouse != 0)
				continue;

			_vm->_game->_forceHandleMouse = !_vm->_game->_forceHandleMouse;

			handleMouse = false;
			if ( (_vm->_game->_handleMouse != 0) &&
			    ((_vm->_global->_useMouse != 0) || (_vm->_game->_forceHandleMouse != 0)))
				handleMouse = true;

			while (_vm->_global->_pressedKeys[1] != 0)
				;
			continue;

		default:
			uint16 savedKey = key;

			key &= 0xFF;

			if (((type == kTypeInputFloatNoLeave) || (type == kTypeInputFloatLeave)) &&
					 (key >= ' ') && (key <= 0xFF)) {
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
					int length = _vm->_draw->stringLength(str, fontIndex) +
						font.extraData[' ' - font.startItem] +
						font.extraData[key - font.startItem];

					if (length > width)
						continue;

					if (((int32) strlen(str)) >= (_vm->_global->_inter_animDataSize * 4 - 1))
						continue;

				} else {
					if (strlen(str) > editSize)
						continue;
					else if (editSize == strlen(str))
						_vm->_util->cutFromStr(str, strlen(str) - 1, 1);
				}

				pos++;
				tempStr[0] = key;
				tempStr[1] = 0;

				_vm->_util->insertStr(tempStr, str, pos - 1);
			}

		}
	}
}

uint16 Hotspots::handleInput(int16 time, uint16 maxPos, uint16 &curPos,
		InputDesc *inpDesc, uint16 &id, uint16 &index) {

	uint16 descInd = 0;
	uint16 key     = 0;
	uint16 found   = 0xFFFF;

	for (int i = 0; i < kHotspotCount; i++) {
		Hotspot &spot = _hotspots[i];

		if (spot.isEnd())
			continue;

		if ((spot.id & 0xC000) != 0x8000)
			continue;

		if (spot.getType() < kTypeInput1NoLeave)
			continue;

		if (spot.getType() > kTypeInputFloatLeave)
			continue;

		char tempStr[256];
		strncpy0(tempStr, GET_VARO_STR(spot.key), 255);

		_vm->_draw->_destSpriteX  = spot.left;
		_vm->_draw->_destSpriteY  = spot.top;
		_vm->_draw->_spriteRight  = spot.right  - spot.left + 1;
		_vm->_draw->_spriteBottom = spot.bottom - spot.top  + 1;

		_vm->_draw->_destSurface = 21;

		_vm->_draw->_backColor    = inpDesc[descInd].backColor;
		_vm->_draw->_frontColor   = inpDesc[descInd].frontColor;
		_vm->_draw->_textToPrint  = tempStr;
		_vm->_draw->_transparency = 1;
		_vm->_draw->_fontIndex    = inpDesc[descInd].fontIndex;

		_vm->_draw->spriteOperation(DRAW_FILLRECT | 0x10);

		_vm->_draw->_destSpriteY += ((spot.bottom - spot.top + 1) -
				_vm->_draw->_fonts[_vm->_draw->_fontIndex]->itemHeight) / 2;
		_vm->_draw->spriteOperation(DRAW_PRINTTEXT | 0x10);

		descInd++;
	}

	for (int i = 0; i < 40; i++)
		WRITE_VAR_OFFSET(i * 4 + 0x44, 0);

	while (1) {
		descInd = 0;

		for (int i = 0; i < kHotspotCount; i++) {
			Hotspot &spot = _hotspots[i];

			if (spot.isEnd())
				continue;

			if ((spot.id & 0xC000) != 0x8000)
				continue;

			if (spot.getType() < kTypeInput1NoLeave)
				continue;

			if (spot.getType() > kTypeInputFloatLeave)
				continue;

			if (descInd == curPos) {
				found = i;
				break;
			}

			descInd++;
		}

		assert(found != 0xFFFF);

		Hotspot inputSpot = _hotspots[found];

		key = readString(inputSpot.left, inputSpot.top,
		    inputSpot.right - inputSpot.left + 1,
		    inputSpot.bottom - inputSpot.top + 1,
		    inpDesc[curPos].backColor, inpDesc[curPos].frontColor,
		    GET_VARO_STR(inputSpot.key), inpDesc[curPos].fontIndex,
				inputSpot.getType(), time, id, index);

		if (_vm->_inter->_terminate)
			return 0;

		switch (key) {
		case kKeyNone:
			if (id == 0)
				return 0;

			if (_vm->_game->_mouseButtons != kMouseButtonsNone) {
				for (int i = 0; (i < kHotspotCount) && !_hotspots[i].isEnd(); i++) {
					Hotspot &spot = _hotspots[i];

					if (spot.getWindow() != 0)
						continue;

					if ((spot.id & 0x4000))
						continue;

					if (!spot.isIn(_vm->_global->_inter_mouseX, _vm->_global->_inter_mouseY))
						continue;

					if ((spot.id & 0xF000))
						continue;

					if (spot.getType() < kTypeInput1NoLeave)
						continue;

					if (spot.getType() > kTypeInputFloatLeave)
						continue;

					index = i;
					break;
				}
			}

			if (_hotspots[index].getType() < kTypeInput1NoLeave)
				return 0;

			if (_hotspots[index].getType() > kTypeInputFloatLeave)
				return 0;

			curPos = 0;
			for (int i = 0; i < kHotspotCount; i++) {
				Hotspot &spot = _hotspots[i];

				if (spot.isEnd())
					continue;

				if ((spot.id & 0xC000) != 0x8000)
					continue;

				if (spot.getType() < kTypeInput1NoLeave)
					continue;

				if (spot.getType() > kTypeInputFloatLeave)
					continue;

				if (i == index)
					break;

				curPos++;
			}
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

			if (maxPos == 1)
				return key;

			if (curPos == (maxPos - 1)) {
				curPos = 0;
				break;
			}

			curPos++;
			break;

		case kKeyDown:
			if ((maxPos - 1) > curPos)
				curPos++;
			break;

		case kKeyUp:
			if (curPos > 0)
				curPos--;
			break;
		}
	}
}

void Hotspots::evaluate() {
	InputDesc descArray[20];
	int16 array[300];
	char *str;
	int16 counter;
	int16 var_24;
	int16 var_26;
	int16 collStackPos;

	push(0);

	uint16 endIndex = 0;
	while (!_hotspots[endIndex].isEnd())
		endIndex++;

	_shouldPush = false;

	_vm->_game->_script->skip(1);

	byte count = _vm->_game->_script->readByte();

	_vm->_game->_handleMouse           = _vm->_game->_script->peekByte(0);
	int16 duration         = _vm->_game->_script->peekByte(1);
	byte stackPos2         = _vm->_game->_script->peekByte(3);
	byte descIndex         = _vm->_game->_script->peekByte(4);
	bool needRecalculation = _vm->_game->_script->peekByte(5) != 0;

	duration *= 1000;
	if ((stackPos2 != 0) || (descIndex != 0)) {
		duration /= 100;
		if (_vm->_game->_script->peekByte(1) == 100)
			duration = 2;
	}

	int16 timeVal = duration;

	_vm->_game->_script->skip(6);

	WRITE_VAR(16, 0);

	byte var_41 = 0;
	int16 var_46 = 0;

	uint16 id      = 0;
	uint16 validId = 0xFFFF;
	uint16 index   = 0;

	bool   hasInput   = false;
	uint16 inputIndex = 0;

	for (uint16 i = 0; i < count; i++) {
		array[i] = 0;

		byte type = _vm->_game->_script->readByte();
		byte window = 0;

		if ((type & 0x40) != 0) {
			type  -= 0x40;
			window = _vm->_game->_script->readByte();
		}

		uint16 left, top, width, height, right, bottom;
		uint32 funcEnter = 0, funcLeave = 0, funcPos = 0;
		if ((type & 0x80) != 0) {
			funcPos = _vm->_game->_script->pos();
			left    = _vm->_game->_script->readValExpr();
			top     = _vm->_game->_script->readValExpr();
			width   = _vm->_game->_script->readValExpr();
			height  = _vm->_game->_script->readValExpr();
		} else {
			funcPos = 0;
			left    = _vm->_game->_script->readUint16();
			top     = _vm->_game->_script->readUint16();
			width   = _vm->_game->_script->readUint16();
			height  = _vm->_game->_script->readUint16();
		}

		if ((_vm->_draw->_renderFlags & RENDERFLAG_CAPTUREPOP) && (left != 0xFFFF)) {
			left += _vm->_draw->_backDeltaX;
			top  += _vm->_draw->_backDeltaY;
		}

		right  = left + width  - 1;
		bottom = top  + height - 1;

		int16 key   = 0;
		int16 flags = 0;
		Video::FontDesc *font = 0;

		type &= 0x7F;
		switch (type) {
		case kTypeNone:
			_vm->_game->_script->skip(6);

			funcEnter = _vm->_game->_script->pos();
			_vm->_game->_script->skipBlock();

			funcLeave = _vm->_game->_script->pos();
			_vm->_game->_script->skipBlock();

			key   = i + 0xA000;
			flags = type + (window << 8);

			add(i + 0x8000, left, top, right, bottom,
					flags, key, funcEnter, funcLeave, funcPos);
			break;

		case kTypeMove:
			key      = _vm->_game->_script->readInt16();
			array[i] = _vm->_game->_script->readInt16();
			flags    = _vm->_game->_script->readInt16();

			funcEnter = _vm->_game->_script->pos();
			_vm->_game->_script->skipBlock();

			funcLeave = _vm->_game->_script->pos();
			_vm->_game->_script->skipBlock();

			if (key == 0)
				key = i + 0xA000;
			flags = type + (window << 8) + (flags << 4);

			add(i + 0x8000, left, top, right, bottom,
					flags, key, funcEnter, funcLeave, funcPos);
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

			key                              = _vm->_game->_script->readVarIndex();
			descArray[inputIndex].fontIndex  = _vm->_game->_script->readInt16();
			descArray[inputIndex].backColor  = _vm->_game->_script->readByte();
			descArray[inputIndex].frontColor = _vm->_game->_script->readByte();
			descArray[inputIndex].ptr        = 0;

			if ((type >= kTypeInput2NoLeave) && (type <= kTypeInput3Leave)) {
				descArray[inputIndex].ptr = _vm->_game->_script->getData() + _vm->_game->_script->pos() + 2;
				_vm->_game->_script->skip(_vm->_game->_script->peekUint16() + 2);
			}

			if (left == 0xFFFF) {
				if ((type & 1) == 0)
					_vm->_game->_script->skipBlock();
				break;
			}

			font = _vm->_draw->_fonts[descArray[inputIndex].fontIndex];
			if (!font->extraData)
				right = left + width * font->itemWidth - 1;

			funcEnter = 0;
			funcPos   = 0;
			funcLeave = 0;
			if (!(type & 1)) {
				funcLeave = _vm->_game->_script->pos();
				_vm->_game->_script->skipBlock();
			}

			flags = type;

			inputIndex++;

			add(i + 0x8000, left, top, right, bottom,
					flags, key, funcEnter, funcLeave, funcPos);

			break;

		case 11:
			_vm->_game->_script->skip(6);

			for (int j = 0; j < kHotspotCount; j++) {
				Hotspot &spot = _hotspots[j];

				if ((spot.id & 0xF000) == 0xE000) {
					spot.id       &= 0xBFFF;
					spot.funcEnter = _vm->_game->_script->pos();
					spot.funcLeave = _vm->_game->_script->pos();
				}
			}

			_vm->_game->_script->skipBlock();
			break;

		case 12:
			_vm->_game->_script->skip(6);

			for (int j = 0; j < kHotspotCount; j++) {
				Hotspot &spot = _hotspots[j];

				if ((spot.id & 0xF000) == 0xD000) {
					spot.id       &= 0xBFFF;
					spot.funcEnter = _vm->_game->_script->pos();
					spot.funcLeave = _vm->_game->_script->pos();
				}
			}

			_vm->_game->_script->skipBlock();
			break;

		case 20:
			validId = i;
			// Fall through to case 2
		case kTypeClick:
			key      = _vm->_game->_script->readInt16();
			array[i] = _vm->_game->_script->readInt16();
			flags    = _vm->_game->_script->readInt16();

			funcEnter = 0;

			funcLeave = _vm->_game->_script->pos();
			_vm->_game->_script->skipBlock();

			flags = 2 + (window << 8) + (flags << 4);

			add(i + 0x8000, left, top, right, bottom,
					flags, key, funcEnter, funcLeave, funcPos);
			break;

		case 21:
			key      = _vm->_game->_script->readInt16();
			array[i] = _vm->_game->_script->readInt16();
			flags    = _vm->_game->_script->readInt16() & 3;

			funcEnter = _vm->_game->_script->pos();
			_vm->_game->_script->skipBlock();

			funcLeave = 0;

			flags = 2 + (window << 8) + (flags << 4);

			add(i + 0x8000, left, top, right, bottom,
					flags, key, funcEnter, funcLeave, funcPos);
			break;
		}
	}

	if (needRecalculation)
		recalculate(true);

	_vm->_game->_forceHandleMouse = 0;
	_vm->_util->clearKeyBuf();

	do {
		uint16 key = 0;
		if (hasInput) {
			uint16 curEditIndex = 0;

			key = handleInput(duration, inputIndex, curEditIndex, descArray, id, index);

			WRITE_VAR(55, curEditIndex);
			if (key == kKeyReturn) {
				for (int i = 0; (i < kHotspotCount) && !_hotspots[i].isEnd(); i++) {
					Hotspot &spot = _hotspots[i];

					if ((spot.id & 0xC000) != 0x8000)
						continue;

					if ((spot.getType() & 1) != 0)
						continue;

					if (spot.getType() <= kTypeClick)
						continue;

					id      = spot.id;
					validId = spot.id & 0x7FFF;
					index   = i;
					break;
				}
				break;
			}
		} else
			key = check(_vm->_game->_handleMouse, -duration, id, index);

		if (((key & 0xFF) >= ' ') && ((key & 0xFF) <= 0xFF) &&
		    ((key >> 8) > 1) && ((key >> 8) < 12))
			key = '0' + (((key >> 8) - 1) % 10) + (key & 0xFF00);

		if (id == 0) {
			if (key != 0) {
				for (int i = 0; (i < kHotspotCount) && !_hotspots[i].isEnd(); i++) {
					Hotspot &spot = _hotspots[i];

					if ((spot.id & 0xC000) != 0x8000)
						continue;

					if ((spot.key == key) || (spot.key == 0x7FFF)) {
						id    = spot.id;
						index = i;
						break;
					}
				}

				if (id == 0) {
					for (int i = 0; (i < kHotspotCount) && !_hotspots[i].isEnd(); i++) {
						Hotspot &spot = _hotspots[i];

						if ((spot.id & 0xC000) != 0x8000)
							continue;

						if ((spot.key & 0xFF00) != 0)
							continue;

						if (spot.key == 0)
							continue;

						if (toupper(key & 0xFF) == toupper(spot.key)) {
							id    = spot.id;
							index = i;
							break;
						}
					}
				}
			} else if (duration != 0) {
				if (stackPos2 != 0) {
					collStackPos = 0;

					for (int i = endIndex; (i < kHotspotCount) && !_hotspots[i].isEnd(); i++) {
						Hotspot &spot = _hotspots[i];

						if ((spot.id & 0xF000) != 0x8000)
							continue;

						collStackPos++;
						if (collStackPos != stackPos2)
							continue;

						id    = spot.id;
						index = i;
						_vm->_inter->storeMouse();
						if (VAR(16) != 0)
							break;

						if ((id & 0xF000) == 0x8000)
							WRITE_VAR(16, array[id & 0xFFF]);
						else
							WRITE_VAR(16, id & 0xFFF);

						if (spot.funcLeave != 0) {
							uint32 timeKey = _vm->_util->getTimeKey();
							call(spot.funcLeave);

							if (timeVal != 2) {
								duration = timeVal - (_vm->_util->getTimeKey() - timeKey);

								if ((duration - var_46) < 3) {
									var_46 -= (duration - 3);
									duration = 3;
								} else if (var_46 != 0) {
									duration -= var_46;
									var_46 = 0;
								}

								if (duration > timeVal)
									duration = timeVal;

							} else
								duration = 2;

						}

						if (VAR(16) == 0)
							id = 0;
						else
							var_41 = 1;

						break;
					}

				} else {
					if (descIndex != 0) {

						counter = 0;
						for (int i = endIndex; (i < kHotspotCount) && !_hotspots[i].isEnd(); i++) {
							Hotspot &spot = _hotspots[i];

							if ((spot.id & 0xF000) == 0x8000) {
								if (++counter == descIndex) {
									id    = spot.id;
									index = i;
									break;
								}
							}

						}

					} else {

						for (int i = 0; (i < kHotspotCount) && !_hotspots[i].isEnd(); i++) {
							Hotspot &spot = _hotspots[i];

							if ((spot.id & 0xF000) == 0x8000) {
								id    = spot.id;
								index = i;
								break;
							}
						}

						if ((_currentKey != 0) && (_hotspots[_currentIndex].funcLeave != 0))
							call(_hotspots[_currentIndex].funcLeave);

						_currentKey = 0;
					}

				}
			}
		}

		if (var_41 != 0)
			break;

		if ((id == 0) || (_hotspots[index].funcLeave != 0))
			continue;

		_vm->_inter->storeMouse();

		if ((id & 0xF000) == 0x8000)
			WRITE_VAR(16, array[id & 0xFFF]);
		else
			WRITE_VAR(16, id & 0xFFF);

		if (_hotspots[index].funcEnter != 0)
			call(_hotspots[index].funcEnter);

		WRITE_VAR(16, 0);
		id = 0;
	}
	while ((id == 0) && !_vm->_inter->_terminate && !_vm->shouldQuit());

	char tempStr[256];
	if ((id & 0xFFF) == validId) {
		collStackPos = 0;
		var_24 = 0;
		var_26 = 1;
		for (int i = 0; i < kHotspotCount; i++) {
			Hotspot &spot = _hotspots[i];

			if (spot.isEnd())
				continue;

			if ((spot.id & 0xC000) != 0x8000)
				continue;

			if (spot.getType() < kTypeInput1NoLeave)
				continue;

			if (spot.getType() > kTypeInputFloatLeave)
				continue;

			if (spot.getType() > kTypeInput3Leave) {
				char *ptr;
				strncpy0(tempStr, GET_VARO_STR(spot.key), 255);
				while ((ptr = strchr(tempStr, ' ')))
					_vm->_util->cutFromStr(tempStr, (ptr - tempStr), 1);
				if (_vm->_global->_language == kLanguageBritish)
					while ((ptr = strchr(tempStr, '.')))
						*ptr = ',';
				WRITE_VARO_STR(spot.key, tempStr);
			}

			if ((spot.getType() >= kTypeInput2NoLeave) && (spot.getType() <= kTypeInput3Leave)) {
				str = (char *) descArray[var_24].ptr;

				strncpy0(tempStr, GET_VARO_STR(spot.key), 255);

				if (spot.getType() < kTypeInput3NoLeave)
					_vm->_util->cleanupStr(tempStr);

				int16 pos = 0;
				do {
					char spotStr[256];

					strncpy0(spotStr, str, 255);
					pos += strlen(str) + 1;

					str += strlen(str) + 1;

					if (spot.getType() < kTypeInput3NoLeave)
						_vm->_util->cleanupStr(spotStr);

					if (strcmp(tempStr, spotStr) == 0) {
						WRITE_VAR(17, VAR(17) + 1);
						WRITE_VAR(17 + var_26, 1);
						break;
					}
				} while (READ_LE_UINT16(descArray[var_24].ptr - 2) > pos);
				collStackPos++;
			} else {
				WRITE_VAR(17 + var_26, 2);
			}
			var_24++;
			var_26++;
		}

		if (collStackPos != (int16) VAR(17))
			WRITE_VAR(17, 0);
		else
			WRITE_VAR(17, 1);
	}

	if (_vm->_game->_handleMouse == 1)
		_vm->_draw->blitCursor();

	if (!_vm->_inter->_terminate && (var_41 == 0)) {
		_vm->_game->_script->seek(_hotspots[index].funcLeave);

		_vm->_inter->storeMouse();
		if (VAR(16) == 0) {
			if ((id & 0xF000) == 0x8000)
				WRITE_VAR(16, array[id & 0xFFF]);
			else
				WRITE_VAR(16, id & 0xFFF);
		}
	} else
		_vm->_game->_script->setFinished(true);

	for (int i = 0; i < count; i++)
		remove(i + 0x8000);

	for (int i = 0; i < kHotspotCount; i++) {
		Hotspot &spot = _hotspots[i];

		if (((spot.id & 0xF000) == 0xA000) || ((spot.id & 0xF000) == 0x9000))
			spot.id |= 0x4000;
	}

}

int16 Hotspots::findCursor(uint16 x, uint16 y) const {
	int16 cursor = 0;

	for (int i = 0; (i < kHotspotCount) && !_hotspots[i].isEnd(); i++) {
		Hotspot &spot = _hotspots[i];

		if ((spot.getWindow() != 0) || (spot.id & 0x4000))
			continue;

		if (!spot.isIn(x, y))
			continue;

		if (spot.getCursor() == 0) {
			if (spot.getType() >= kTypeInput1NoLeave) {
				cursor = 3;
				break;
			} else if ((spot.getButton() != kMouseButtonsRight) && (cursor == 0))
				cursor = 1;
		} else if (cursor == 0)
			cursor = spot.getCursor();
	}

	return cursor;
}

} // End of namespace Gob
