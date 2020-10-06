/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/icb/common/px_common.h"
#include "engines/icb/keyboard.h"

namespace ICB {

#define MAX_KEY_BUFFER 32

uint8 keyBacklog = 0;           // The number of key presses waiting to be processed.
uint8 keyPointer = 0;           // Index of the next key to read from the buffer.
char keyBuffer[MAX_KEY_BUFFER]; // The keyboard buffer

void WriteKey(char key) {
	if (keyBacklog < MAX_KEY_BUFFER) {
		keyBuffer[(keyPointer + keyBacklog) % MAX_KEY_BUFFER] = key;
		keyBacklog += 1;
	}
}

void Clear_key_buffer() {
	// clear all the backlog - we need this when we start using other methods (direct input) for reading keys

	keyBacklog = 0;
	keyPointer = 0;
}

bool KeyWaiting() {
	if (keyBacklog)
		return (true);
	else
		return (false);
}

int32 ReadKey(char *key) {
	if (!keyBacklog)
		return (0);

	if (key == NULL)
		return 0;

	*key = keyBuffer[keyPointer++];
	if (keyPointer == MAX_KEY_BUFFER)
		keyPointer = 0;

	keyBacklog -= 1;

	return (1);
}

} // End of namespace ICB
