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
#include "bs2/driver/driver96.h"

namespace Sword2 {

uint8 keyBacklog = 0;	// The number of key presses waiting to be processed.
uint8 keyPointer = 0;	// Index of the next key to read from the buffer.

_keyboardEvent keyBuffer[MAX_KEY_BUFFER];		// The keyboard buffer

void WriteKey(uint16 ascii, int keycode, int modifiers) {
	if (keyBuffer && keyBacklog < MAX_KEY_BUFFER) {
		_keyboardEvent *slot = &keyBuffer[(keyPointer + keyBacklog) % MAX_KEY_BUFFER];

		slot->ascii = ascii;
		slot->keycode = keycode;
		slot->modifiers = modifiers;
		keyBacklog++;
	}
}

/**
 * @return true if there is an unprocessed key waiting in the queue
 */

bool KeyWaiting(void) {
	return keyBacklog != 0;
}

/**
 * Sets the value of the keyboard event passed in to the current waiting key.
 * @return RD_OK, or an error code to indicate there is no key waiting.
 */

int32 ReadKey(_keyboardEvent *ev) {
	if (!keyBacklog)
		return RDERR_NOKEYWAITING;

	if (ev == NULL)
		return RDERR_INVALIDPOINTER;

	ev->ascii = keyBuffer[keyPointer].ascii;
	ev->keycode = keyBuffer[keyPointer].keycode;
	ev->modifiers = keyBuffer[keyPointer].modifiers;

	keyPointer++;

	if (keyPointer == MAX_KEY_BUFFER)
		keyPointer = 0;

	keyBacklog--;

	return RD_OK;
}

} // End of namespace Sword2
