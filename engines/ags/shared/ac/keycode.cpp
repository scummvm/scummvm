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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "ags/shared/ac/keycode.h"

namespace AGS3 {

int AGSKeyToScriptKey(int keycode) {
	// Script API requires strictly capital letters, if this is a small letter - capitalize it
	return (keycode >= 'a' && keycode <= 'z') ? keycode - 'a' + 'A' : keycode;
}

char AGSKeyToText(int keycode) {
	// support only printable characters (128-255 are chars from extended fonts)
	if (keycode >= 32 && keycode < 256)
		return static_cast<char>(keycode);
	return 0;
}

} // namespace AGS3
