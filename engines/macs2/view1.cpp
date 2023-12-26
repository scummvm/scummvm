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
#include <graphics/cursorman.h>

namespace Macs2 {

	View1::View1() : UIElement("View1") {
		_backgroundSurface = g_engine->_bgImageShip;

		CursorMan.replaceCursor(g_engine->_cursorData, g_engine->_cursorWidth, g_engine->_cursorHeight, g_engine->_cursorWidth >> 1, g_engine->_cursorHeight >>1, 0);
		CursorMan.showMouse(true);
	}

	void View1::drawDarkRectangle(uint16 x, uint16 y, uint16 width, uint16 height)
	{
		Graphics::ManagedSurface s = getSurface();
		for (uint16 xOffset = 0; xOffset < width; xOffset++) {
			for (uint16 yOffset = 0; yOffset < height; yOffset++) {
				const uint16 currentX = x + xOffset;
				const uint16 currentY = y + yOffset;
				const uint32 currentValue = s.getPixel(currentX, currentY);
				const uint32 newValue = g_engine->_shadingTable[currentValue];
				s.setPixel(currentX, currentY, newValue);
			}
		}
	}

	void View1::drawGlyphs(Macs2::GlyphData *data, int count, uint16 x, uint16 y, Graphics::ManagedSurface& s) {
		uint16 currentX = x;
		uint16 currentY = y;
		for (int i = 0; i < count; i++) {
			const Macs2::GlyphData &currentData = data[i];
			DrawSprite(currentX, currentY, currentData.Width, currentData.Height, currentData.Data, s);
			currentX += currentData.Width;
			// TODO: Maybe handle end of line
		}
	}

	bool View1::msgFocus(const FocusMessage &msg) {
	//Common::fill(&_pal[0], &_pal[256 * 3], 0);
	// _offset = 128;
	return true;
}

	bool View1::msgMouseDown(const MouseDownMessage& msg)
	{
		uint32 value = getSurface().getPixel(msg._pos.x, msg._pos.y);
		g_system->setWindowCaption(Common::String::format("%u,%u: %u", msg._pos.x, msg._pos.y, value));
		return true;
	}

bool View1::msgKeypress(const KeypressMessage &msg) {
	// Any keypress to close the view
	// close();
	if (msg.ascii == (uint16)'i') {
		_backgroundSurface = g_engine->_map;
		redraw();
	}
	else if (msg.ascii == (uint16)'m') {
		_backgroundSurface = g_engine->_bgImageShip;
		redraw();
	}
	return true;
}

void View1::draw() {
	g_system->getPaletteManager()->setPalette(g_engine->_pal, 0, 256);

	// Draw a bunch of squares on screen
	Graphics::ManagedSurface s = getSurface();

	/* for (int i = 0; i < 100; i++) {
		s.setPixel(i, i, i);
	} */
	
	// s.blitFrom(g_engine->_bgImageShip);
	
	
	// s.blitFrom(_backgroundSurface);
	s.blitFrom(_backgroundSurface);

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

	// Draw the mouse cursor
	DrawSprite(100, 100, g_engine->_cursorWidth, g_engine->_cursorHeight, g_engine->_cursorData, s);

	// Draw the animation frame
	DrawSprite(180, 80, g_engine->_guyWidth, g_engine->_guyHeight, g_engine->_guyData, s);
	
	
	//for (int i = 0; i < 100; ++i)
	//	s.frameRect(Common::Rect(i, i, 320 - i, 200 - i), i);

	// Draw a shaded rectangle
	drawDarkRectangle(50, 50, 100, 50);

	// Draw all glyphs
	drawGlyphs(g_engine->_glyphs, g_engine->numGlyphs, 10, 10, s);
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
