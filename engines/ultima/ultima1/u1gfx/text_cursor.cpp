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

#include "ultima/ultima1/u1gfx/text_cursor.h"
#include "ultima/shared/early/ultima_early.h"
#include "ultima/shared/gfx/screen.h"
#include "common/system.h"
#include "graphics/managed_surface.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Gfx {

#define CURSOR_ANIM_FRAME_TIME 100
#define CURSOR_W 8
#define CURSOR_H 8

static const byte TEXT_CURSOR_FRAMES[4][8] = {
	{ 0x66, 0x3C, 0x18, 0x66, 0x66, 0x18, 0x3C, 0x66 },
	{ 0x3C, 0x18, 0x66, 0x24, 0x24, 0x66, 0x18, 0x3C },
	{ 0x18, 0x66, 0x24, 0x3C, 0x3C, 0x24, 0x66, 0x18 },
	{ 0x66, 0x24, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x66 }
};

U1TextCursor::U1TextCursor(const byte &fgColor, const byte &bgColor) : _fgColor(fgColor),
		_bgColor(bgColor), _frameNum(0), _lastFrameFrame(0) {
	_bounds = Common::Rect(0, 0, 8, 8);
}

void U1TextCursor::update() {
	uint32 time = getTime();
	if (!_visible || !_fgColor || (time - _lastFrameFrame) < CURSOR_ANIM_FRAME_TIME)
		return;

	// Increment to next frame
	_lastFrameFrame = time;
	_frameNum = (_frameNum + 1) % 3;

	markAsDirty();
}

void U1TextCursor::draw() {
	if (!_visible)
		return;

	// Get the surface area to draw the cursor on
	Graphics::ManagedSurface s(8, 8);
		
	// Loop to draw the cursor
	for (int y = 0; y < CURSOR_H; ++y) {
		byte *lineP = (byte *)s.getBasePtr(0, y);
		byte bits = TEXT_CURSOR_FRAMES[_frameNum][y];

		for (int x = 0; x < CURSOR_W; ++x, ++lineP, bits >>= 1) {
			*lineP = (bits & 1) ? _fgColor : _bgColor;
		}
	}

	g_system->copyRectToScreen(s.getPixels(), s.pitch, _bounds.left, _bounds.top, _bounds.width(), _bounds.height());
}

uint32 U1TextCursor::getTime() {
	return g_system->getMillis();
}

} // End of namespace U1Gfx
} // End of namespace Ultima1
} // End of namespace Ultima
