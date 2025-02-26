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

#include "graphics/cursorman.h"
#include "bagel/spacebar/boflib/gfx/cursor.h"
#include "bagel/boflib/stdinc.h"

namespace Bagel {
namespace SpaceBar {

void CBofCursor::initialize() {
	show();
}

CBofCursor::~CBofCursor() {
	assert(isValidObject(this));

	unLoad();
}

ErrorCode CBofCursor::load() {
	assert(isValidObject(this));

	// kill any previous cursor
	unLoad();

	return _errCode;
}

ErrorCode CBofCursor::unLoad() {
	assert(isValidObject(this));

	return _errCode;
}

ErrorCode CBofCursor::set() {
	assert(isValidObject(this));

	return _errCode;
}

// TODO: This controlled the "Windows cursor" in the original game
// ScummVM doesn't have one, so just show the arrow cursor or
// use CursorMan accordingly
void CBofCursor::hide() {
	//CursorMan.showMouse(false);
}

void CBofCursor::show() {
	CursorMan.showMouse(true);
}

} // namespace SpaceBar
} // namespace Bagel
