/* Copyright (C) 1994-2003 Revolution Software Ltd
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "stdafx.h"
#include "sword2/driver/driver96.h"

namespace Sword2 {

void Input::writeKey(uint16 ascii, int keycode, int modifiers) {
	if (_keyBuffer && _keyBacklog < MAX_KEY_BUFFER) {
		_keyboardEvent *slot = &_keyBuffer[(_keyLogPos + _keyBacklog) % MAX_KEY_BUFFER];

		slot->ascii = ascii;
		slot->keycode = keycode;
		slot->modifiers = modifiers;
		_keyBacklog++;
	}
}

/**
 * @return true if there is an unprocessed key waiting in the queue
 */

bool Input::keyWaiting(void) {
	return _keyBacklog != 0;
}

/**
 * Sets the value of the keyboard event passed in to the current waiting key.
 * @return RD_OK, or an error code to indicate there is no key waiting.
 */

int32 Input::readKey(_keyboardEvent *ev) {
	if (!_keyBacklog)
		return RDERR_NOKEYWAITING;

	if (ev == NULL)
		return RDERR_INVALIDPOINTER;

	ev->ascii = _keyBuffer[_keyLogPos].ascii;
	ev->keycode = _keyBuffer[_keyLogPos].keycode;
	ev->modifiers = _keyBuffer[_keyLogPos].modifiers;

	_keyLogPos++;

	if (_keyLogPos == MAX_KEY_BUFFER)
		_keyLogPos = 0;

	_keyBacklog--;
	return RD_OK;
}

} // End of namespace Sword2
