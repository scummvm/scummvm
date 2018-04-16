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

#include "common/scummsys.h"
#include "tsage/screen.h"

namespace TsAGE {

Screen::Screen(): GfxSurface() {
	create(SCREEN_WIDTH, SCREEN_HEIGHT);
}

Screen::~Screen() {
	// Delete the screen's surface
	free();
}

void Screen::update() {
	// When dialogs are active, the screen surface may be remapped to
	// sub-sections of the screen. But for drawing we'll need to temporarily
	// remove any such remappings and use the entirety of the screen
	Rect clipBounds = getBounds();
	setBounds(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));

	// Update the screen
	Graphics::Screen::update();

	// Reset the clipping
	setBounds(clipBounds);
}

} // End of namespace TsAGE
