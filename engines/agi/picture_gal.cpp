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

#include "agi/agi.h"
#include "agi/graphics.h"
#include "agi/picture.h"

#include "agi/picture_gal.h"

namespace Agi {
	
// PictureMgr_GAL decodes and draws picture resources in early King's Quest 1.
//
// This "Game Adaptation Language" format was used in PC Booters and Apple II.
//
// This format supports lines and flood fills, and visual and priority screens.
//
// As this is the format that evolved into AGI, it is quite similar apart from
// the specific opcodes. The major difference is the line drawing routine;
// it produces different results than AGI. Flood fills implicitly rely on this.
// When KQ1 was ported to AGI, the new lines prevented some fills from working,
// so they just added more. There are still a few white pixels they missed.
//
// As with Troll's Tale, room pictures depend on the game first drawing a frame
// within the entire picture area. When KQ1 was converted to AGI, this frame
// was added to each individual picture resource.

PictureMgr_GAL::PictureMgr_GAL(AgiBase *agi, GfxMgr *gfx) :
	PictureMgr(agi, gfx) {
}

void PictureMgr_GAL::drawPicture() {
	debugC(kDebugLevelPictures, "Drawing picture %d", _resourceNr);

	drawBlackFrame();

	_dataOffset = 0;
	_dataOffsetNibble = false;
	_patCode = 0;
	_patNum = 0;
	_priOn = true; // initially off in AGI
	_scrOn = false;
	_scrColor = 15;
	_priColor = 1;

	// GAL toggles the current screen between visual and priority
	// with opcode F0. This affects opcodes F4-F7, but the rest of
	// the opcodes are explicit about which screen(s) they draw to.
	byte prevScrOn = _scrOn;
	byte prevPriOn = _priOn;

	while (_dataOffset < _dataSize) {
		byte curByte = getNextByte();

		switch (curByte) {
		case 0xf0: // toggle current screen
			draw_SetScreens(!_scrOn, !_priOn);
			break;
		case 0xf1:
			draw_SetColor();
			break;
		case 0xf2:
			draw_SetPriority();
			break;
		case 0xf3:
			draw_SetColor();
			draw_SetPriority();
			break;

		// Line operations drawn to both visual and priority screens
		case 0xf4:
			draw_SetScreens(true, true, prevScrOn, prevPriOn);
			yCorner();
			draw_SetScreens(prevScrOn, prevPriOn);
			break;
		case 0xf5:
			draw_SetScreens(true, true, prevScrOn, prevPriOn);
			xCorner();
			draw_SetScreens(prevScrOn, prevPriOn);
			break;
		case 0xf6:
			draw_SetScreens(true, true, prevScrOn, prevPriOn);
			draw_LineAbsolute();
			draw_SetScreens(prevScrOn, prevPriOn);
			break;
		case 0xf7:
			draw_SetScreens(true, true, prevScrOn, prevPriOn);
			draw_LineShort();
			draw_SetScreens(prevScrOn, prevPriOn);
			break;

		// Line operations drawn to the current screen
		case 0xf8:
			yCorner();
			break;
		case 0xf9:
			xCorner();
			break;
		case 0xfa:
			draw_LineAbsolute();
			break;
		case 0xfb:
			draw_LineShort();
			break;

		// Fill operations drawn to one or both screens
		case 0xfc:
			draw_SetScreens(true, true, prevScrOn, prevPriOn);
			draw_SetColor();
			draw_SetPriority();
			draw_Fill();
			draw_SetScreens(prevScrOn, prevPriOn);
			break;
		case 0xfd:
			draw_SetScreens(false, true, prevScrOn, prevPriOn);
			draw_SetPriority();
			draw_Fill();
			draw_SetScreens(prevScrOn, prevPriOn);
			break;
		case 0xfe:
			draw_SetScreens(true, false, prevScrOn, prevPriOn);
			draw_SetColor();
			draw_Fill();
			draw_SetScreens(prevScrOn, prevPriOn);
			break;

		case 0xff: // end of data
			return;
		default:
			warning("Unknown picture opcode %02x at %04x", curByte, _dataOffset - 1);
			break;
		}
	}
}

/**
 * Sets the status of the visual and priority screens.
 */
void PictureMgr_GAL::draw_SetScreens(byte scrOn, byte priOn) {
	_scrOn = scrOn;
	_priOn = priOn;
}

/**
 * Sets the status of the visual and priority screens,
 * and returns their previous values.
 */
void PictureMgr_GAL::draw_SetScreens(byte scrOn, byte priOn, byte &prevScrOn, byte &prevPriOn) {
	prevScrOn = _scrOn;
	prevPriOn = _priOn;
	_scrOn = scrOn;
	_priOn = priOn;
}

/**
 * Draws a hard-coded black frame in both screens.
 * All room pictures require this to draw correctly.
 *
 * Original data: F3 00 00 F5 00 00 9F A7 00 00 FF
 */
void PictureMgr_GAL::drawBlackFrame() {
	_scrOn = true;
	_scrColor = 0;
	_priOn = true;
	_priColor = 0;
	draw_Line(0, 0, _width - 1, 0);
	draw_Line(_width - 1, 0, _width - 1, _height - 1);
	draw_Line(_width - 1, _height - 1, 0, _height - 1);
	draw_Line(0, _height - 1, 0, 0);
}

/**
 * Draws a horizontal, vertical, or diagonal line using the GAL drawing routine.
 *
 * This routine produces different diagonal lines than the AGI routine.
 */
void PictureMgr_GAL::draw_Line(int16 x1, int16 y1, int16 x2, int16 y2) {
	x1 = CLIP<int16>(x1, 0, _width - 1);
	x2 = CLIP<int16>(x2, 0, _width - 1);
	y1 = CLIP<int16>(y1, 0, _height - 1);
	y2 = CLIP<int16>(y2, 0, _height - 1);

	const byte width  = (x2 > x1) ? (x2 - x1) : (x1 - x2);
	const byte height = (y2 > y1) ? (y2 - y1) : (y1 - y2);

	byte x = 0;
	byte y = 0;
	if (width > height) {
		while (x != width) {
			x++;
			y = (x * height) / width;
			if (((x * height) % width) * 2 > width) {
				y++;
			}

			byte pixelX = (x2 > x1) ? (x1 + x) : (x1 - x);
			byte pixelY = (y2 > y1) ? (y1 + y) : (y1 - y);
			putVirtPixel(pixelX, pixelY);
		}
	} else {
		while (y != height) {
			y++;
			x = (y * width) / height;
			if (((y * width) % height) * 2 > height) {
				x++;
			}

			byte pixelX = (x2 > x1) ? (x1 + x) : (x1 - x);
			byte pixelY = (y2 > y1) ? (y1 + y) : (y1 - y);
			putVirtPixel(pixelX, pixelY);
		}
	}
}

/**
 * Gets the next x coordinate in the current picture instruction,
 * and clip it to the picture width.
 */
bool PictureMgr_GAL::getNextXCoordinate(byte &x) {
	if (!getNextParamByte(x)) {
		return false;
	}

	if (x >= _width) { // 160
		debugC(kDebugLevelPictures, "clipping %c from %d to %d", 'x', x, _width - 1);
		x = _width - 1; // 159
	}

	return true;
}

/**
 * Gets the next y coordinate in the current picture instruction,
 * and clip it to the picture height.
 */
bool PictureMgr_GAL::getNextYCoordinate(byte &y) {
	if (!getNextParamByte(y)) {
		return false;
	}

	if (y >= _height) { // 168
		debugC(kDebugLevelPictures, "clipping %c from %d to %d", 'y', y, _height);
		y = _height - 1; // 167
	}

	return true;
}

} // End of namespace Agi
