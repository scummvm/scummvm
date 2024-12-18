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

#include "common/textconsole.h"

namespace Agi {

PictureMgr::PictureMgr(AgiBase *agi, GfxMgr *gfx) {
	_vm = agi;
	_gfx = gfx;

	_resourceNr = 0;
	_data = nullptr;
	_dataSize = 0;
	_dataOffset = 0;
	_dataOffsetNibble = false;

	_patCode = 0;
	_patNum = 0;
	_priOn = 0;
	_scrOn = 0;
	_scrColor = 0;
	_priColor = 0;

	_minCommand = 0xf0;

	_pictureVersion = AGIPIC_V2;
	_width = 0;
	_height = 0;
	_xOffset = 0;
	_yOffset = 0;

	_flags = 0;
	_maxStep = 0;
}

void PictureMgr::putVirtPixel(int x, int y) {
	if (x < 0 || y < 0 || x >= _width || y >= _height)
		return;

	x += _xOffset;
	y += _yOffset;

	// validate coordinate after applying preagi offset.
	// winnie objects go past the bottom of the screen.
	if (x >= SCRIPT_WIDTH || y >= SCRIPT_HEIGHT) {
		return;
	}

	byte drawMask= 0;
	if (_priOn)
		drawMask |= GFX_SCREEN_MASK_PRIORITY;
	if (_scrOn)
		drawMask |= GFX_SCREEN_MASK_VISUAL;

	_gfx->putPixel(x, y, drawMask, _scrColor, _priColor);
}

byte PictureMgr::getNextByte() {
	if (!_dataOffsetNibble) {
		return _data[_dataOffset++];
	} else {
		byte curByte = _data[_dataOffset++] << 4;
		return (_data[_dataOffset] >> 4) | curByte;
	}
}

bool PictureMgr::getNextParamByte(byte &b) {
	byte value = getNextByte();
	if (value >= _minCommand) {
		_dataOffset--;
		return false;
	}
	b = value;
	return true;
}

byte PictureMgr::getNextNibble() {
	if (!_dataOffsetNibble) {
		_dataOffsetNibble = true;
		return _data[_dataOffset] >> 4;
	} else {
		_dataOffsetNibble = false;
		return _data[_dataOffset++] & 0x0F;
	}
}

bool PictureMgr::getNextXCoordinate(byte &x) {
	if (!(getNextParamByte(x))) {
		return false;
	}

	if (_pictureVersion == AGIPIC_PREAGI) {
		if (x >= _width) {
			debugC(kDebugLevelPictures, "preagi: clipping x from %d to %d", x, _width - 1);
			x = _width - 1; // 139
		}
	}
	return true;
}

bool PictureMgr::getNextYCoordinate(byte &y) {
	if (!(getNextParamByte(y))) {
		return false;
	}

	if (_pictureVersion == AGIPIC_PREAGI) {
		if (y > _height) {
			debugC(kDebugLevelPictures, "preagi: clipping y from %d to %d", y, _height);
			y = _height; // 159
		}
	}
	return true;
}

bool PictureMgr::getNextCoordinates(byte &x, byte &y) {
	return getNextXCoordinate(x) && getNextYCoordinate(y);
}

/**************************************************************************
** xCorner
**
** Draws an xCorner  (drawing action 0xF5)
**************************************************************************/
void PictureMgr::xCorner(bool skipOtherCoords) {
	byte x1, x2, y1, y2, dummy;

	if (!getNextCoordinates(x1, y1))
		return;

	putVirtPixel(x1, y1);

	for (;;) {
		if (!getNextXCoordinate(x2))
			break;

		if (skipOtherCoords)
			if (!getNextParamByte(dummy))
				break;

		draw_Line(x1, y1, x2, y1);
		x1 = x2;

		if (skipOtherCoords)
			if (!getNextParamByte(dummy))
				break;

		if (!getNextYCoordinate(y2))
			break;

		draw_Line(x1, y1, x1, y2);
		y1 = y2;
	}
}

/**************************************************************************
** yCorner
**
** Draws an yCorner  (drawing action 0xF4)
**************************************************************************/
void PictureMgr::yCorner(bool skipOtherCoords) {
	byte x1, x2, y1, y2, dummy;

	if (!getNextCoordinates(x1, y1))
		return;

	putVirtPixel(x1, y1);

	for (;;) {
		if (skipOtherCoords)
			if (!getNextParamByte(dummy))
				break;

		if (!getNextYCoordinate(y2))
			break;

		draw_Line(x1, y1, x1, y2);
		y1 = y2;
		if (!getNextXCoordinate(x2))
			break;

		if (skipOtherCoords)
			if (!getNextParamByte(dummy))
				break;

		draw_Line(x1, y1, x2, y1);
		x1 = x2;
	}
}

/**************************************************************************
** plotPattern
**
** Draws pixels, circles, squares, or splatter brush patterns depending
** on the pattern code.
**************************************************************************/
void PictureMgr::plotPattern(int x, int y) {
	static const uint16 binary_list[] = {
		0x8000, 0x4000, 0x2000, 0x1000, 0x800, 0x400, 0x200, 0x100,
		0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1
	};

	static const uint8 circle_list[] = {
		0, 1, 4, 9, 16, 25, 37, 50
	};

	static const uint16 circle_data[] = {
		0x8000,
		0x0000, 0xE000, 0x0000,
		0x7000, 0xF800, 0x0F800, 0x0F800, 0x7000,
		0x3800, 0x7C00, 0x0FE00, 0x0FE00, 0x0FE00, 0x7C00, 0x3800,
		0x1C00, 0x7F00, 0x0FF80, 0x0FF80, 0x0FF80, 0x0FF80, 0x0FF80, 0x7F00, 0x1C00,
		0x0E00, 0x3F80, 0x7FC0, 0x7FC0, 0x0FFE0, 0x0FFE0, 0x0FFE0, 0x7FC0, 0x7FC0, 0x3F80, 0x1F00, 0x0E00,
		0x0F80, 0x3FE0, 0x7FF0, 0x7FF0, 0x0FFF8, 0x0FFF8, 0x0FFF8, 0x0FFF8, 0x0FFF8, 0x7FF0, 0x7FF0, 0x3FE0, 0x0F80,
		0x07C0, 0x1FF0, 0x3FF8, 0x7FFC, 0x7FFC, 0x0FFFE, 0x0FFFE, 0x0FFFE, 0x0FFFE, 0x0FFFE, 0x7FFC, 0x7FFC, 0x3FF8, 0x1FF0, 0x07C0
	};

	uint16 circle_word;
	const uint16 *circle_ptr;
	uint16 counter;
	uint16 pen_width = 0;
	int pen_final_x = 0;
	int pen_final_y = 0;

	uint8 t = 0;
	uint8 temp8;
	uint16 temp16;

	int pen_x = x;
	int pen_y = y;
	uint16 pen_size = (_patCode & 0x07);

	circle_ptr = &circle_data[circle_list[pen_size]];

	// setup the X position
	// = pen_x - pen.size/2

	pen_x = (pen_x * 2) - pen_size;
	if (pen_x < 0) pen_x = 0;

	temp16 = (_width * 2) - (2 * pen_size);
	if (pen_x >= temp16)
		pen_x = temp16;

	pen_x /= 2;
	pen_final_x = pen_x;    // original starting point?? -> used in plotrelated

	// Setup the Y Position
	// = pen_y - pen.size
	pen_y = pen_y - pen_size;
	if (pen_y < 0) pen_y = 0;

	temp16 = 167 - (2 * pen_size);
	if (pen_y >= temp16)
		pen_y = temp16;

	pen_final_y = pen_y;    // used in plotrelated

	t = (uint8)(_patNum | 0x01);     // even

	// new purpose for temp16

	temp16 = (pen_size << 1) + 1;   // pen size
	pen_final_y += temp16;                  // the last row of this shape
	temp16 = temp16 << 1;
	pen_width = temp16;                 // width of shape?

	bool circleCond = ((_patCode & 0x10) != 0);
	int counterStep = 4;
	int ditherCond = 0x02;

	for (; pen_y < pen_final_y; pen_y++) {
		circle_word = *circle_ptr++;

		for (counter = 0; counter <= pen_width; counter += counterStep) {
			if (circleCond || ((binary_list[counter >> 1] & circle_word) != 0)) {
				if ((_patCode & 0x20) != 0) {
					temp8 = t % 2;
					t = t >> 1;
					if (temp8 != 0)
						t = t ^ 0xB8;
				}

				// == box plot, != circle plot
				if ((_patCode & 0x20) == 0 || (t & 0x03) == ditherCond)
					putVirtPixel(pen_x, pen_y);
			}
			pen_x++;
		}

		pen_x = pen_final_x;
	}
}

/**************************************************************************
** plotBrush
**
** Plots points and various brush patterns.
**************************************************************************/
void PictureMgr::plotBrush() {
	for (;;) {
		if (_patCode & 0x20) {
			if (!getNextParamByte(_patNum))
				break;
		}

		byte x1, y1;
		if (!getNextCoordinates(x1, y1))
			break;

		plotPattern(x1, y1);
	}
}

void PictureMgr::plotBrush_PreAGI() {
	_patCode = getNextByte();
	if (_patCode > 12) {
		_patCode = 12;
	}

	for (;;) {
		byte x, y;
		if (!getNextCoordinates(x, y))
			break;

		plotPattern_PreAGI(x, y);
	}
}

void PictureMgr::plotPattern_PreAGI(byte x, byte y) {
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

/**************************************************************************
** Draw AGI picture
**************************************************************************/
void PictureMgr::drawPicture() {
	_dataOffset = 0;
	_dataOffsetNibble = false;
	_patCode = 0;
	_patNum = 0;
	_priOn = false;
	_scrOn = false;
	_scrColor = 15;
	_priColor = 4;

	switch (_pictureVersion) {
	case AGIPIC_C64:
		drawPictureC64();
		break;
	case AGIPIC_V1:
		drawPictureV1();
		break;
	case AGIPIC_V15:
		drawPictureV15();
		break;
	case AGIPIC_PREAGI:
		drawPicturePreAGI();
		break;
	case AGIPIC_V2:
		drawPictureV2();
		break;
	default:
		break;
	}
}

void PictureMgr::drawPictureC64() {
	debugC(kDebugLevelPictures, "Drawing Apple II / C64 / CoCo picture");

	_scrColor = 0;

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
			draw_Fill();
			break;
		case 0xe5:  // enable screen drawing
			_scrOn = true;
			break;
		case 0xe6:  // plot brush
			plotBrush_PreAGI();
			break;
		case 0xff: // end of data
			return;
		default:
			warning("Unknown picture opcode (%x) at (%x)", curByte, _dataOffset - 1);
			break;
		}
	}
}

void PictureMgr::drawPictureV1() {
	debugC(kDebugLevelPictures, "Drawing V1 picture");

	while (_dataOffset < _dataSize) {
		byte curByte = getNextByte();

		switch (curByte) {
		case 0xf1:
			draw_SetColor();
			_scrOn = true;
			_priOn = false;
			break;
		case 0xf3:
			draw_SetColor();
			_scrOn = true;
			draw_SetPriority();
			_priOn = true;
			break;
		case 0xfa:
			_scrOn = false;
			_priOn = true;
			draw_LineAbsolute();
			_scrOn = true;
			_priOn = false;
			break;
		case 0xfb:
			draw_LineShort();
			break;
		case 0xfc:
			draw_SetColor();
			draw_SetPriority();
			draw_Fill();
			break;
		case 0xff: // end of data
			return;
		default:
			warning("Unknown picture opcode (%x) at (%x)", curByte, _dataOffset - 1);
			break;
		}
	}
}

void PictureMgr::drawPictureV15() {
	debugC(kDebugLevelPictures, "Drawing V1.5 picture");

	while (_dataOffset < _dataSize) {
		byte curByte = getNextByte();

		switch (curByte) {
		case 0xf0:
			// happens in all Troll's Tale pictures
			// TODO: figure out what it was meant for
			break;
		case 0xf1:
			draw_SetColor();
			_scrOn = true;
			break;
		case 0xf3:
			if (_flags & kPicFf3Stop)
				return;
			break;
		case 0xf8:
			yCorner(true);
			break;
		case 0xf9:
			xCorner(true);
			break;
		case 0xfa:
			// TODO: is this really correct?
			draw_LineAbsolute();
			break;
		case 0xfb:
			// TODO: is this really correct?
			draw_LineAbsolute();
			break;
		case 0xfe:
			draw_SetColor();
			_scrOn = true;
			draw_Fill();
			break;
		case 0xff: // end of data
			return;
		default:
			warning("Unknown picture opcode (%x) at (%x)", curByte, _dataOffset - 1);
			break;
		}
	}
}

void PictureMgr::drawPicturePreAGI() {
	debugC(kDebugLevelPictures, "Drawing PreAGI picture");

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
			draw_Fill();
			_scrOn = prevScrOn;
			break;
		}
		case 0xf9:
			plotBrush_PreAGI();
			break;
		case 0xff: // end of data
			return;
		default:
			warning("Unknown picture opcode (%x) at (%x)", curByte, _dataOffset - 1);
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

void PictureMgr::drawPictureV2() {
	debugC(kDebugLevelPictures, "Drawing V2/V3 picture");

	// AGIv3 nibble parameters are indicated by a flag in the picture's directory entry
	bool nibbleMode = (_vm->_game.dirPic[_resourceNr].flags & RES_PICTURE_V3_NIBBLE_PARM) != 0;

	while (_dataOffset < _dataSize) {
		byte curByte = getNextByte();

		switch (curByte) {
		case 0xf0:
			if (!nibbleMode) {
				draw_SetColor();
			} else {
				draw_SetNibbleColor();
			}
			_scrOn = true;
			break;
		case 0xf1:
			_scrOn = false;
			break;
		case 0xf2:
			if (!nibbleMode) {
				draw_SetPriority();
			} else {
				draw_SetNibblePriority();
			}
			_priOn = true;
			break;
		case 0xf3:
			_priOn = false;
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
		case 0xf8:
			draw_Fill();
			break;
		case 0xf9:
			_patCode = getNextByte();
			break;
		case 0xfa:
			plotBrush();
			break;
		case 0xff: // end of data
			return;
		default:
			warning("Unknown picture opcode (%x) at (%x)", curByte, _dataOffset - 1);
			break;
		}
	}
}

void PictureMgr::drawPictureAGI256() {
	const uint32 maxFlen = _width * _height;
	int16 x = 0;
	int16 y = 0;
	byte *dataPtr = _data;
	byte *dataEndPtr = _data + _dataSize;

	debugC(kDebugLevelPictures, "Drawing AGI256 picture");

	while (dataPtr < dataEndPtr) {
		byte color = *dataPtr++;
		_gfx->putPixel(x, y, GFX_SCREEN_MASK_VISUAL, color, 0);

		x++;
		if (x >= _width) {
			x = 0;
			y++;
			if (y >= _height) {
				break;
			}
		}
	}

	if (_dataSize < maxFlen) {
		warning("Undersized AGI256 picture resource %d, using it anyway. Filling rest with white", _resourceNr);
		while (_dataSize < maxFlen) {
			x++;
			if (x >= _width) {
				x = 0;
				y++;
				if (y >= _height)
					break;
			}
			_gfx->putPixel(x, y, GFX_SCREEN_MASK_VISUAL, 15, 0);
		}
	} else if (_dataSize > maxFlen)
		warning("Oversized AGI256 picture resource %d, decoding only %ux%u part of it", _resourceNr, _width, _height);
}

void PictureMgr::draw_SetColor() {
	_scrColor = getNextByte();

	// For CGA, replace the color with its mixture color
	if (_vm->_renderMode == Common::kRenderCGA) {
		_scrColor = _gfx->getCGAMixtureColor(_scrColor);
	}
}

void PictureMgr::draw_SetPriority() {
	_priColor = getNextByte();
}

// this gets a nibble instead of a full byte
// used by some V3 games, special resource flag RES_PICTURE_V3_NIBBLE_PARM is set
void PictureMgr::draw_SetNibbleColor() {
	_scrColor = getNextNibble();

	// For CGA, replace the color with its mixture color
	if (_vm->_renderMode == Common::kRenderCGA) {
		_scrColor = _gfx->getCGAMixtureColor(_scrColor);
	}
}

void PictureMgr::draw_SetNibblePriority() {
	_priColor = getNextNibble();
}

/**
 * Draw an AGI line.
 * A line drawing routine sent by Joshua Neal, modified by Stuart George
 * (fixed >>2 to >>1 and some other bugs like x1 instead of y1, etc.)
 * @param x1  x coordinate of start point
 * @param y1  y coordinate of start point
 * @param x2  x coordinate of end point
 * @param y2  y coordinate of end point
 */
void PictureMgr::draw_Line(int16 x1, int16 y1, int16 x2, int16 y2) {
	x1 = CLIP<int16>(x1, 0, _width - 1);
	x2 = CLIP<int16>(x2, 0, _width - 1);
	y1 = CLIP<int16>(y1, 0, _height - 1);
	y2 = CLIP<int16>(y2, 0, _height - 1);

	// Vertical line

	if (x1 == x2) {
		if (y1 > y2) {
			SWAP(y1, y2);
		}

		for (; y1 <= y2; y1++)
			putVirtPixel(x1, y1);

		return;
	}

	// Horizontal line

	if (y1 == y2) {
		if (x1 > x2) {
			SWAP(x1, x2);
		}
		for (; x1 <= x2; x1++)
			putVirtPixel(x1, y1);
		return;
	}

	int stepX = 1;
	int deltaX = x2 - x1;
	if (deltaX < 0) {
		stepX = -1;
		deltaX = -deltaX;
	}

	int stepY = 1;
	int deltaY = y2 - y1;
	if (deltaY < 0) {
		stepY = -1;
		deltaY = -deltaY;
	}

	int i, detdelta, errorX, errorY;
	if (deltaY > deltaX) {
		i = deltaY;
		detdelta = deltaY;
		errorX = deltaY / 2;
		errorY = 0;
	} else {
		i = deltaX;
		detdelta = deltaX;
		errorX = 0;
		errorY = deltaX / 2;
	}

	int x = x1;
	int y = y1;
	putVirtPixel(x, y);

	do {
		errorY += deltaY;
		if (errorY >= detdelta) {
			errorY -= detdelta;
			y += stepY;
		}

		errorX += deltaX;
		if (errorX >= detdelta) {
			errorX -= detdelta;
			x += stepX;
		}

		putVirtPixel(x, y);
		i--;
	} while (i > 0);
}

/**
 * Draw a relative AGI line.
 * Draws short lines relative to last position. (drawing action 0xF7)
 */
void PictureMgr::draw_LineShort() {
	byte x1, y1, disp;

	if (!getNextCoordinates(x1, y1))
		return;

	putVirtPixel(x1, y1);

	for (;;) {
		if (!getNextParamByte(disp))
			break;

		int dx = ((disp & 0xf0) >> 4) & 0x0f;
		int dy = (disp & 0x0f);

		if (dx & 0x08)
			dx = -(dx & 0x07);
		if (dy & 0x08)
			dy = -(dy & 0x07);

		draw_Line(x1, y1, x1 + dx, y1 + dy);
		x1 += dx;
		y1 += dy;
	}
}

/**************************************************************************
** absoluteLine
**
** Draws long lines to actual locations (cf. relative) (drawing action 0xF6)
**************************************************************************/
void PictureMgr::draw_LineAbsolute() {
	byte x1, y1, x2, y2;

	if (!getNextCoordinates(x1, y1))
		return;

	putVirtPixel(x1, y1);

	for (;;) {
		if (!getNextCoordinates(x2, y2))
			break;

		draw_Line(x1, y1, x2, y2);
		x1 = x2;
		y1 = y2;
	}
}

// flood fill
void PictureMgr::draw_Fill() {
	byte x, y;

	while (getNextCoordinates(x, y)) {
		// PreAGI: getNextCoordinates clips to (139, 159), and then
		// flood fill checks if y >= 159 and decrements to 158.
		// The flood fill check is not in in Apple II/C64/CoCo
		// versions of Winnie, as can be seen by the table edge
		// being a different color than Winnie's shirt in the first
		// room, but the same color in DOS/Amiga (picture 28).
		if (_pictureVersion == AGIPIC_PREAGI) {
			if (y >= _height) { // 159
				debugC(kDebugLevelPictures, "preagi: fill clipping y from %d to %d", y, _height - 1);
				y = _height - 1; // 158
			}
		}

		draw_Fill(x, y);
	}
}

void PictureMgr::draw_Fill(int16 x, int16 y) {
	if (!_scrOn && !_priOn)
		return;

	// Push initial pixel on the stack
	Common::Stack<Common::Point> stack;
	stack.push(Common::Point(x, y));

	// Exit if stack is empty
	while (!stack.empty()) {
		Common::Point p = stack.pop();

		if (!draw_FillCheck(p.x, p.y))
			continue;

		// Scan for left border
		uint c;
		for (c = p.x - 1; draw_FillCheck(c, p.y); c--)
			;

		bool newspanUp = true;
		bool newspanDown = true;
		for (c++; draw_FillCheck(c, p.y); c++) {
			putVirtPixel(c, p.y);
			if (draw_FillCheck(c, p.y - 1)) {
				if (newspanUp) {
					stack.push(Common::Point(c, p.y - 1));
					newspanUp = false;
				}
			} else {
				newspanUp = true;
			}

			if (draw_FillCheck(c, p.y + 1)) {
				if (newspanDown) {
					stack.push(Common::Point(c, p.y + 1));
					newspanDown = false;
				}
			} else {
				newspanDown = true;
			}
		}
	}
}

bool PictureMgr::draw_FillCheck(int16 x, int16 y) {
	if (x < 0 || x >= _width || y < 0 || y >= _height)
		return false;

	x += _xOffset;
	y += _yOffset;

	// validate coordinate after applying preagi offset.
	// winnie objects go past the bottom of the screen.
	if (x >= SCRIPT_WIDTH || y >= SCRIPT_HEIGHT) {
		return false;
	}

	byte screenColor = _gfx->getColor(x, y);
	byte screenPriority = _gfx->getPriority(x, y);

	if (_flags & kPicFTrollMode)
		return ((screenColor != 11) && (screenColor != _scrColor));

	if (!_priOn && _scrOn && _scrColor != 15)
		return (screenColor == 15);

	if (_priOn && !_scrOn && _priColor != 4)
		return (screenPriority == 4);

	return (_scrOn && screenColor == 15 && _scrColor != 15);
}

/**
 * Decode an AGI picture resource. Used by regular AGI games.
 * This function decodes an AGI picture resource into the correct slot
 * and draws it on the AGI screen, optionally clearing the screen before
 * drawing.
 * @param n      AGI picture resource number
 * @param clear  clear AGI screen before drawing
 * @param agi256 load an AGI256 picture resource
 */
void PictureMgr::decodePicture(int16 resourceNr, bool clearScreen, bool agi256, int16 width, int16 height) {
	_resourceNr = resourceNr;
	_data = _vm->_game.pictures[resourceNr].rdata;
	_dataSize = _vm->_game.dirPic[resourceNr].len;
	_width = width;
	_height = height;

	if (clearScreen) {
		_gfx->clear(15, 4); // white, priority 4
	}

	if (!agi256) {
		drawPicture();
	} else {
		drawPictureAGI256();
	}

	if (clearScreen) {
		_vm->clearImageStack();
	}
	_vm->recordImageStackCall(ADD_PIC, resourceNr, clearScreen, agi256, 0, 0, 0, 0);
}

/**
 * Decode an AGI picture resource. Used by preAGI.
 * This function decodes an AGI picture resource into the correct slot
 * and draws it on the AGI screen, optionally clearing the screen before
 * drawing.
 * @param data   the AGI Picture data
 * @param length the size of the picture data buffer
 * @param clear  clear AGI screen before drawing
 */
void PictureMgr::decodePictureFromBuffer(byte *data, uint32 length, bool clearScreen, int16 width, int16 height) {
	_data = data;
	_dataSize = length;
	_width = width;
	_height = height;

	if (clearScreen) {
		_gfx->clear(15, 4); // white, priority 4
	}

	drawPicture();
}

void PictureMgr::showPicture(int16 x, int16 y, int16 width, int16 height) {
	debugC(kDebugLevelPictures, "Show picture");

	_gfx->render_Block(x, y, width, height);
}

void PictureMgr::showPictureWithTransition() {
	_width = SCRIPT_WIDTH;
	_height = SCRIPT_HEIGHT;

	debugC(kDebugLevelPictures, "Show picture");

	if (!_vm->_game.automaticRestoreGame) {
		// only do transitions when we are not restoring a saved game

		if (!_vm->_game.gfxMode) {
			// if we are not yet in graphics mode, set graphics mode palette now
			// TODO: maybe change text mode to use different colors for drawing
			// so that we don't have to change palettes at all
			_gfx->setPalette(true);
		}

		switch (_vm->_renderMode) {
		case Common::kRenderAmiga:
		case Common::kRenderApple2GS:
			// Platform Amiga/Apple II GS -> render and do Amiga transition
			_gfx->render_Block(0, 0, SCRIPT_WIDTH, SCRIPT_HEIGHT, false);
			_gfx->transition_Amiga();
			return;
		case Common::kRenderAtariST:
			// Platform Atari ST used a different transition, looks "high-res" (full 320x168)
			_gfx->render_Block(0, 0, SCRIPT_WIDTH, SCRIPT_HEIGHT, false);
			_gfx->transition_AtariSt();
			return;
		default:
			// Platform PC -> render directly
			// Macintosh AGI also doesn't seem to have any transitions
			break;
		}
	}

	_gfx->render_Block(0, 0, SCRIPT_WIDTH, SCRIPT_HEIGHT);
}

void PictureMgr::setPictureVersion(AgiPictureVersion version) {
	_pictureVersion = version;

	if (version == AGIPIC_C64)
		_minCommand = 0xe0;
	else
		_minCommand = 0xf0;
}

} // End of namespace Agi
