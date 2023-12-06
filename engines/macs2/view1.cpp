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

#include "common/system.h"
#include "graphics/palette.h"
#include "macs2/view1.h"
#include "macs2/macs2.h"

namespace Macs2 {

bool View1::msgFocus(const FocusMessage &msg) {
	//Common::fill(&_pal[0], &_pal[256 * 3], 0);
	// _offset = 128;
	return true;
}

bool View1::msgKeypress(const KeypressMessage &msg) {
	// Any keypress to close the view
	close();
	return true;
}

void View1::draw() {
	// Draw a bunch of squares on screen
	Graphics::ManagedSurface s = getSurface();

	/* for (int i = 0; i < 100; i++) {
		s.setPixel(i, i, i);
	} */
	g_system->getPaletteManager()->setPalette(g_engine->_pal, 0, 256);
	s.blitFrom(g_engine->_bgImageShip);

	// Draw the character
	uint16 charX = 50;
	uint16 charY = 50;
	for (int x = 0; x < g_engine->_charWidth; x++) {
		for (int y = 0; y < g_engine->_charHeight; y++) {
			uint8 val = g_engine->_charData[y * g_engine->_charWidth + x];
			if (val != 0) {
				s.setPixel(charX + x, charY + y, val);
			}
		}
	}

	// Draw the border part
	uint16 borderX = 100;
	uint16 borderY = 50;
	for (int x = 0; x < g_engine->_borderWidth; x++) {
		for (int y = 0; y < g_engine->_borderHeight; y++) {
			uint8 val = g_engine->_borderData[y * g_engine->_borderWidth + x];
			if (val != 0) {
				s.setPixel(borderX + x, borderY + y, val);
			}
		}
	}

	// And the highlight part
	borderX = 150;
	borderY = 100;
	for (int x = 0; x < g_engine->_borderHighlightWidth; x++) {
		for (int y = 0; y < g_engine->_borderHighlightHeight; y++) {
			uint8 val = g_engine->_borderHighlightData[y * g_engine->_borderHighlightWidth + x];
			if (val != 0) {
				s.setPixel(borderX + x, borderY + y, val);
			}
		}
	}

	DrawSprite(200, 50, g_engine->_flagWidths[0], g_engine->_flagHeights[0], g_engine->_flagData[0], s);
	DrawSprite(200, 100, g_engine->_flagWidths[1], g_engine->_flagHeights[1], g_engine->_flagData[1], s);
	DrawSprite(200, 150, g_engine->_flagWidths[2], g_engine->_flagHeights[2], g_engine->_flagData[2], s);
	
	
	//for (int i = 0; i < 100; ++i)
	//	s.frameRect(Common::Rect(i, i, 320 - i, 200 - i), i);
}

bool View1::tick() {
	// Cycle the palette
	++_offset;
	//for (int i = 0; i < 256; ++i)
	//	_pal[i * 3 + 1] = (i + _offset) % 256;
	// g_system->getPaletteManager()->setPalette(_pal, 0, 256);

	// Below is redundant since we're only cycling the palette, but it demonstrates
	// how to trigger the view to do further draws after the first time, since views
	// don't automatically keep redrawing unless you tell it to
	//if ((_offset % 256) == 0)
	//	redraw();

	return true;
}

void View1::DrawSprite(uint16 x, uint16 y, uint16 width, uint16 height, byte* data, Graphics::ManagedSurface& s)
{
	for (int currentX = 0; currentX < width; currentX++) {
		for (int currentY = 0; currentY < height; currentY++) {
			uint8 val = data[currentY * width + currentX];
			if (val != 0) {
				s.setPixel(x + currentX, y + currentY, val);
			}
		}
	}
}

} // namespace Macs2
