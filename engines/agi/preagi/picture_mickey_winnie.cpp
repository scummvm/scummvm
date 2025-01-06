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

#include "agi/preagi/picture_mickey_winnie.h"

namespace Agi {

// PictureMgr_Mickey_Winnie decodes and draws picture resources in Mickey's
// Space Adventure (DOS) and Winnie the Pooh (DOS/Amiga/A2/C64/CoCo).
//
// Mickey and Winnie DOS/Amiga use the same format. The picture code in
// their executables appears to be the same.
//
// The A2/C64/CoCo versions of Winnie use a completely different format, but
// they do support the same features. These games start in ScummVM but they
// don't really work yet. TODO: display the right colors, figure out the line
// differences, support these versions.
//
// Both formats support lines, flood fills, and patterns. No priority screen.
//
// Unique features to these formats:
//
// 1. Pictures can be drawn on top of others at arbitrary locations. Used to
//    draw items in rooms, and to draw room pictures with a buffer on each side
//    in DOS/Amiga. The pictures don't fill the screen width because they were
//    designed for the Apple II.
//
// 2. Mickey's crystals animate. Most of the work is done in MickeyEngine;
//    this class just allows the engine to set a maximum number of picture
//    instructions to execute. Unclear if this is same effect as the original.
//
// 3. The pattern opcode draws solid circles in up to 17 sizes.
//
// Mickey features animating spaceship lights, but the engine handles that.
// The lights are a picture whose instructions are modified before drawing.
//
// TODO: There are extremely minor inaccuracies in several Winnie pictures.
// The F1 opcode's effects are not fully understood, and it creates subtle
// discrepancies. It may be related to dithering. However, so few pictures
// contain F3, and even fewer are affected by ignoring it or not, and only
// by a few pixels, that it doesn't matter except for completeness.
// See: picture 34 door handles (Rabbit's kitchen)

PictureMgr_Mickey_Winnie::PictureMgr_Mickey_Winnie(AgiBase *agi, GfxMgr *gfx) :
	PictureMgr(agi, gfx) {

	switch (agi->getPlatform()) {
	case Common::kPlatformAmiga:
	case Common::kPlatformDOS:
		_isDosOrAmiga = true;
		break;
	default:
		_isDosOrAmiga = false;
		_minCommand = 0xe0;
		break;
	}

	_xOffset = 0;
	_yOffset = 0;
	_maxStep = 0;
}

void PictureMgr_Mickey_Winnie::drawPicture() {
	debugC(kDebugLevelPictures, "Drawing picture");

	_dataOffset = 0;
	_dataOffsetNibble = false;
	_patCode = 0;
	_patNum = 0;
	_priOn = false;
	_scrOn = false;
	_priColor = 4;

	if (_isDosOrAmiga) {
		_scrColor = 15;
		drawPicture_DOS_Amiga();
	} else {
		_scrColor = 0;
		drawPicture_A2_C64_CoCo();
	}
}

void PictureMgr_Mickey_Winnie::drawPicture_DOS_Amiga() {
	int step = 0;
	while (_dataOffset < _dataSize) {
		byte curByte = getNextByte();

		switch (curByte) {
		case 0xf0:
			draw_SetColor();
			_scrOn = true;
			break;
		case 0xf1:
			_scrOn = false;
			break;
		case 0xf4:
			yCorner();
			break;
		case 0xf5:
			xCorner();
			break;
		case 0xf6:
			draw_LineAbsolute();
			break;
		case 0xf7:
			draw_LineShort();
			break;
		case 0xf8: {
			// The screen-on flag does not prevent PreAGI flood fills.
			// Winnie picture 7 (Roo) contains F1 before several fills.
			byte prevScrOn = _scrOn;
			_scrOn = true;
			PictureMgr::draw_Fill();
			_scrOn = prevScrOn;
			break;
		}
		case 0xf9:
			plotBrush();
			break;
		case 0xff: // end of data
			return;
		default:
			warning("Unknown picture opcode %02x at %04x", curByte, _dataOffset - 1);
			break;
		}

		// Limit drawing to the optional maximum number of opcodes.
		// Used by Mickey for crystal animation.
		step++;
		if (step == _maxStep) {
			return;
		}
	}
}

void PictureMgr_Mickey_Winnie::drawPicture_A2_C64_CoCo() {
	while (_dataOffset < _dataSize) {
		byte curByte = getNextByte();

		if ((curByte >= 0xF0) && (curByte <= 0xFE)) {
			_scrColor = curByte & 0x0F;
			continue;
		}

		switch (curByte) {
		case 0xe0:  // x-corner
			xCorner();
			break;
		case 0xe1:  // y-corner
			yCorner();
			break;
		case 0xe2:  // dynamic draw lines
			draw_LineShort();
			break;
		case 0xe3:  // absolute draw lines
			draw_LineAbsolute();
			break;
		case 0xe4:  // fill
			draw_SetColor();
			PictureMgr::draw_Fill();
			break;
		case 0xe5:  // enable screen drawing
			_scrOn = true;
			break;
		case 0xe6:  // plot brush
			plotBrush();
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
 * plotBrush (PreAGI)
 *
 * Plots the given brush pattern. All brushes are solid circles.
 */
void PictureMgr_Mickey_Winnie::plotBrush() {
	_patCode = getNextByte();
	if (_patCode > 12) {
		_patCode = 12;
	}

	for (;;) {
		byte x, y;
		if (!getNextCoordinates(x, y))
			break;

		plotPattern(x, y);
	}
}

/**
 * plotPattern
 *
 * Draws a solid circle. Size is determined by the pattern code.
 */
void PictureMgr_Mickey_Winnie::plotPattern(byte x, byte y) {
	// PreAGI patterns are 13 solid circles
	static const byte circleData[] = {
		0x00,
		0x01, 0x01,
		0x01, 0x02, 0x02,
		0x01, 0x02, 0x03, 0x03,
		0x02, 0x03, 0x04, 0x04, 0x04,
		0x02, 0x03, 0x04, 0x05, 0x05, 0x05,
		0x02, 0x04, 0x05, 0x05, 0x06, 0x06, 0x06,
		0x02, 0x04, 0x05, 0x06, 0x06, 0x07, 0x07, 0x07,
		0x02, 0x04, 0x06, 0x06, 0x07, 0x07, 0x08, 0x08, 0x08,
		0x03, 0x05, 0x06, 0x07, 0x08, 0x08, 0x08, 0x09, 0x09, 0x09,
		0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x09, 0x0a, 0x0a, 0x0a, 0x0a,
		0x03, 0x05, 0x07, 0x08, 0x09, 0x09, 0x0a, 0x0a, 0x0b, 0x0b, 0x0b, 0x0b,
		0x03, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0a, 0x0b, 0x0b, 0x0c, 0x0c, 0x0c, 0x0c
	};

	int circleDataIndex = (_patCode * (_patCode + 1)) / 2;

	// draw the circle by drawing its vertical lines two at a time, starting at the
	// left and right edges and working inwards. circles have odd widths, so the
	// final iteration draws the middle line twice.
	for (int i = _patCode; i >= 0; i--) {
		const byte height = circleData[circleDataIndex++];
		int16 x1, y1, x2, y2;

		// left vertical line
		x1 = x - i;
		x2 = x1;
		y1 = y - height;
		y2 = y + height;
		draw_Line(x1, y1, x2, y2);

		// right vertical line
		x1 = x + i;
		x2 = x1;
		draw_Line(x1, y1, x2, y2);
	}
}

/**
 * Flood fills from a start position, with a clipped height.
 */
void PictureMgr_Mickey_Winnie::draw_Fill(int16 x, int16 y) {
	// Flood fill does extra height clipping, and pictures rely on this.
	// The get-coordinates routine clips to (139, 159) and then the
	// flood fill routine checks if y >= 159 and decrements to 158.
	// The flood fill clip is not in in Apple II/C64/CoCo versions
	// of Winnie, as can be seen by the table edge being a different
	// color than Winnie's shirt in the first room, but the same
	// color as the shirt in DOS/Amiga. (Picture 28)
	if (_isDosOrAmiga) {
		if (y >= _height) { // 159
			debugC(kDebugLevelPictures, "clipping %c from %d to %d", 'y', y, _height - 1);
			y = _height - 1; // 158
		}
	}

	PictureMgr::draw_Fill(x, y);
}

/**
 * Gets the next x coordinate in the current picture instruction,
 * and clip it to the picture width. Many Winnie pictures contain
 * out of bounds coordinates and rely on this clipping.
 */
bool PictureMgr_Mickey_Winnie::getNextXCoordinate(byte &x) {
	if (!getNextParamByte(x)) {
		return false;
	}

	if (_isDosOrAmiga) {
		if (x >= _width) { // 140
			debugC(kDebugLevelPictures, "clipping %c from %d to %d", 'x', x, _width - 1);
			x = _width - 1; // 139
		}
	}

	return true;
}

/**
 * Gets the next y coordinate in the current picture instruction,
 * and clip it to the picture height. Many Winnie pictures contain
 * out of bounds coordinates and rely on this clipping.
 */
bool PictureMgr_Mickey_Winnie::getNextYCoordinate(byte &y) {
	if (!getNextParamByte(y)) {
		return false;
	}

	if (_isDosOrAmiga) {
		// note that this is a different clip than for the x coordinate
		if (y > _height) { // 159
			debugC(kDebugLevelPictures, "clipping %c from %d to %d", 'y', y, _height);
			y = _height; // 159
		}
	}

	return true;
}

/**
 * Validates picture coordinates and translates them to GfxMgr coordinates.
 *
 * This function applies the current picture object and validates that the
 * graphics coordinates are within GfxMgr's boundaries. Validation is necessary
 * because Winnie places tall objects at the bottom of the screen in several
 * rooms, and GfxMgr does not validate coordinates.
 */
bool PictureMgr_Mickey_Winnie::getGraphicsCoordinates(int16 &x, int16 &y) {
	// validate that the coordinates are within the picture's boundaries
	if (!PictureMgr::getGraphicsCoordinates(x, y)) {
		return false;
	}

	x += _xOffset;
	y += _yOffset;

	// validate that the offset coordinates are within the screen's boundaries
	return (x < SCRIPT_WIDTH && y < SCRIPT_HEIGHT);
}

} // End of namespace Agi
