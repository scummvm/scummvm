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

#include "agi/agi.h"
#include "agi/graphics.h"

#include "common/textconsole.h"

namespace Agi {

PictureMgr::PictureMgr(AgiBase *agi, GfxMgr *gfx) {
	_vm = agi;
	_gfx = gfx;

	_resourceNr = 0;
	_data = NULL;
	_dataSize = 0;
	_dataOffset = 0;
	_dataOffsetNibble = false;

	_patCode = _patNum = _priOn = _scrOn = _scrColor = _priColor = 0;
	_xOffset = _yOffset = 0;

	_pictureVersion = AGIPIC_V2;
	_minCommand = 0xf0;
	_flags = 0;
	_currentStep = 0;

	_width = _height = 0;
}

void PictureMgr::putVirtPixel(int x, int y) {
	byte drawMask = 0;

	if (x < 0 || y < 0 || x >= _width || y >= _height)
		return;

	x += _xOffset;
	y += _yOffset;

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

byte PictureMgr::getNextNibble() {
	if (!_dataOffsetNibble) {
		_dataOffsetNibble = true;
		return _data[_dataOffset] >> 4;
	} else {
		_dataOffsetNibble = false;
		return _data[_dataOffset++] & 0x0F;
	}
}

/**************************************************************************
** xCorner
**
** Draws an xCorner  (drawing action 0xF5)
**************************************************************************/
void PictureMgr::draw_xCorner(bool skipOtherCoords) {
	int x1, x2, y1, y2;

	if ((x1 = getNextByte()) >= _minCommand ||
	        (y1 = getNextByte()) >= _minCommand) {
		_dataOffset--;
		return;
	}

	putVirtPixel(x1, y1);

	for (;;) {
		x2 = getNextByte();

		if (x2 >= _minCommand)
			break;

		if (skipOtherCoords)
			if (getNextByte() >= _minCommand)
				break;

		draw_Line(x1, y1, x2, y1);
		x1 = x2;

		if (skipOtherCoords)
			if (getNextByte() >= _minCommand)
				break;

		y2 = getNextByte();

		if (y2 >= _minCommand)
			break;

		draw_Line(x1, y1, x1, y2);
		y1 = y2;
	}
	_dataOffset--;
}

/**************************************************************************
** yCorner
**
** Draws an yCorner  (drawing action 0xF4)
**************************************************************************/
void PictureMgr::yCorner(bool skipOtherCoords) {
	int x1, x2, y1, y2;

	if ((x1 = getNextByte()) >= _minCommand ||
	        (y1 = getNextByte()) >= _minCommand) {
		_dataOffset--;
		return;
	}

	putVirtPixel(x1, y1);

	for (;;) {
		if (skipOtherCoords)
			if (getNextByte() >= _minCommand)
				break;

		y2 = getNextByte();

		if (y2 >= _minCommand)
			break;

		draw_Line(x1, y1, x1, y2);
		y1 = y2;
		x2 = getNextByte();

		if (x2 >= _minCommand)
			break;

		if (skipOtherCoords)
			if (getNextByte() >= _minCommand)
				break;

		draw_Line(x1, y1, x2, y1);
		x1 = x2;
	}

	_dataOffset--;
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

	static uint16 circle_data[] = {
		0x8000,
		0xE000, 0xE000, 0xE000,
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
	uint16 texture_num = 0;
	uint16 pen_size = (_patCode & 0x07);

	circle_ptr = &circle_data[circle_list[pen_size]];

	// SGEORGE : Fix v3 picture data for drawing circles. Manifests in goldrush
	if (_pictureVersion == 3) {
		circle_data[1] = 0;
		circle_data[3] = 0;
	}

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

	t = (uint8)(texture_num | 0x01);        // even

	// new purpose for temp16

	temp16 = (pen_size << 1) + 1;   // pen size
	pen_final_y += temp16;                  // the last row of this shape
	temp16 = temp16 << 1;
	pen_width = temp16;                 // width of shape?

	bool circleCond;
	int counterStep;
	int ditherCond;

	if (_flags & kPicFCircle)
		_patCode |= 0x10;

	if (_vm->getGameType() == GType_PreAGI) {
		circleCond = ((_patCode & 0x10) == 0);
		counterStep = 3;
		ditherCond = 0x03;
	} else {
		circleCond = ((_patCode & 0x10) != 0);
		counterStep = 4;
		ditherCond = 0x01;
	}

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

	return;
}

/**************************************************************************
** plotBrush
**
** Plots points and various brush patterns.
**************************************************************************/
void PictureMgr::plotBrush() {
	int x1, y1;

	for (;;) {
		if (_patCode & 0x20) {
			if ((_patNum = getNextByte()) >= _minCommand)
				break;
			_patNum = (_patNum >> 1) & 0x7f;
		}

		if ((x1 = getNextByte()) >= _minCommand)
			break;

		if ((y1 = getNextByte()) >= _minCommand)
			break;

		plotPattern(x1, y1);
	}

	_dataOffset--;
}

/**************************************************************************
** Draw AGI picture
**************************************************************************/
void PictureMgr::drawPicture() {
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
	case AGIPIC_V2:
		drawPictureV2();
		break;
	case AGIPIC_256:
		drawPictureAGI256();
		break;
	default:
		break;
	}
}

void PictureMgr::drawPictureC64() {
	byte curByte;

	debugC(8, kDebugLevelMain, "Drawing C64 picture");

	_scrColor = 0x0;

	while (_dataOffset < _dataSize) {
		curByte = getNextByte();

		if ((curByte >= 0xF0) && (curByte <= 0xFE)) {
			_scrColor = curByte & 0x0F;
			continue;
		}

		switch (curByte) {
		case 0xe0:  // x-corner
			draw_xCorner();
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
			_patCode = getNextByte();
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

void PictureMgr::drawPictureV1() {
	byte curByte;

	debugC(8, kDebugLevelMain, "Drawing V1 picture");

	while (_dataOffset < _dataSize) {
		curByte = getNextByte();

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
		case 0xff: // end of data
			return;
		default:
			warning("Unknown picture opcode (%x) at (%x)", curByte, _dataOffset - 1);
			break;
		}
	}
}

void PictureMgr::drawPictureV15() {
	byte curByte;

	debugC(8, kDebugLevelMain, "Drawing V1.5 picture");

	while (_dataOffset < _dataSize) {
		curByte = getNextByte();

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
			draw_xCorner(true);
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

void PictureMgr::drawPictureV2() {
	byte curByte;
	bool nibbleMode = false;
	bool mickeyCrystalAnimation = false;
	int  mickeyIteration = 0;

	debugC(8, kDebugLevelMain, "Drawing V2/V3 picture");

	if (_vm->_game.dirPic[_resourceNr].flags & RES_PICTURE_V3_NIBBLE_PARM) {
		// check, if this resource uses nibble mode (0xF0 + 0xF2 commands take nibbles instead of bytes)
		nibbleMode = true;
	}

	if ((_flags & kPicFStep) && _vm->getGameType() == GType_PreAGI) {
		mickeyCrystalAnimation = true;
	}

	while (_dataOffset < _dataSize) {
		curByte = getNextByte();

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
			draw_xCorner();
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

			if (_vm->getGameType() == GType_PreAGI)
				plotBrush();
			break;
		case 0xfa:
			plotBrush();
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

		// This is used by Mickey for the crystal animation
		// One frame of the crystal animation is shown on each iteration, based on _currentStep
		if (mickeyCrystalAnimation) {
			if (_currentStep == mickeyIteration) {
				int storedXOffset = _xOffset;
				int storedYOffset = _yOffset;
				// Note that picture coordinates are correct for Mickey only
				showPic(10, 0, _width, _height);
				_xOffset = storedXOffset;
				_yOffset = storedYOffset;
				_currentStep++;
				if (_currentStep > 14)  // crystal animation is 15 frames
					_currentStep = 0;
				// reset the picture step flag - it will be set when the next frame of the crystal animation is drawn
				_flags &= ~kPicFStep;
				return;     // return back to the game loop
			}
			mickeyIteration++;
		}
	}
}

void PictureMgr::drawPictureAGI256() {
	const uint32 maxFlen = _width * _height;
	int16 x = 0;
	int16 y = 0;
	byte *dataPtr = _data;
	byte *dataEndPtr = _data + _dataSize;
	byte color = 0;

	debugC(8, kDebugLevelMain, "Drawing AGI256 picture");

	while (dataPtr < dataEndPtr) {
		color = *dataPtr++;
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
		warning("Undersized AGI256 picture resource %d, using it anyway. Filling rest with white.", _resourceNr);
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
	switch (_vm->_renderMode) {
	case Common::kRenderCGA:
		_scrColor = _gfx->getCGAMixtureColor(_scrColor);
		break;
	default:
		break;
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
	switch (_vm->_renderMode) {
	case Common::kRenderCGA:
		_scrColor = _gfx->getCGAMixtureColor(_scrColor);
		break;
	default:
		break;
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

	int i, x, y, deltaX, deltaY, stepX, stepY, errorX, errorY, detdelta;

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

	y = y1;
	x = x1;

	stepY = 1;
	deltaY = y2 - y1;
	if (deltaY < 0) {
		stepY = -1;
		deltaY = -deltaY;
	}

	stepX = 1;
	deltaX = x2 - x1;
	if (deltaX < 0) {
		stepX = -1;
		deltaX = -deltaX;
	}

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
	int x1, y1, disp, dx, dy;

	if ((x1 = getNextByte()) >= _minCommand ||
	        (y1 = getNextByte()) >= _minCommand) {
		_dataOffset--;
		return;
	}

	putVirtPixel(x1, y1);

	for (;;) {
		if ((disp = getNextByte()) >= _minCommand)
			break;

		dx = ((disp & 0xf0) >> 4) & 0x0f;
		dy = (disp & 0x0f);

		if (dx & 0x08)
			dx = -(dx & 0x07);
		if (dy & 0x08)
			dy = -(dy & 0x07);

		draw_Line(x1, y1, x1 + dx, y1 + dy);
		x1 += dx;
		y1 += dy;
	}
	_dataOffset--;
}

/**************************************************************************
** absoluteLine
**
** Draws long lines to actual locations (cf. relative) (drawing action 0xF6)
**************************************************************************/
void PictureMgr::draw_LineAbsolute() {
	int16 x1, y1, x2, y2;

	if ((x1 = getNextByte()) >= _minCommand ||
	        (y1 = getNextByte()) >= _minCommand) {
		_dataOffset--;
		return;
	}

	putVirtPixel(x1, y1);

	for (;;) {
		if ((x2 = getNextByte()) >= _minCommand)
			break;

		if ((y2 = getNextByte()) >= _minCommand)
			break;

		draw_Line(x1, y1, x2, y2);
		x1 = x2;
		y1 = y2;
	}
	_dataOffset--;
}

// flood fill
void PictureMgr::draw_Fill() {
	int16 x1, y1;

	while ((x1 = getNextByte()) < _minCommand && (y1 = getNextByte()) < _minCommand)
		draw_Fill(x1, y1);

	_dataOffset--;
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
		unsigned int c;
		bool newspanUp, newspanDown;

		if (!draw_FillCheck(p.x, p.y))
			continue;

		// Scan for left border
		for (c = p.x - 1; draw_FillCheck(c, p.y); c--)
			;

		newspanUp = newspanDown = true;
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

int PictureMgr::draw_FillCheck(int16 x, int16 y) {
	byte screenColor;
	byte screenPriority;

	if (x < 0 || x >= _width || y < 0 || y >= _height)
		return false;

	x += _xOffset;
	y += _yOffset;

	screenColor = _gfx->getColor(x, y);
	screenPriority = _gfx->getPriority(x, y);

	if (_flags & kPicFTrollMode)
		return ((screenColor != 11) && (screenColor != _scrColor));

	if (!_priOn && _scrOn && _scrColor != 15)
		return (screenColor == 15);

	if (_priOn && !_scrOn && _priColor != 4)
		return screenPriority == 4;

	return (_scrOn && screenColor == 15 && _scrColor != 15);
}

/**
 * Decode an AGI picture resource.
 * This function decodes an AGI picture resource into the correct slot
 * and draws it on the AGI screen, optionally clearing the screen before
 * drawing.
 * @param n      AGI picture resource number
 * @param clear  clear AGI screen before drawing
 * @param agi256 load an AGI256 picture resource
 */
int PictureMgr::decodePicture(int16 resourceNr, bool clearScreen, bool agi256, int16 pic_width, int16 pic_height) {
	debugC(8, kDebugLevelResources, "(%d)", resourceNr);

	_patCode = 0;
	_patNum = 0;
	_priOn = _scrOn = false;
	_scrColor = 0xF;
	_priColor = 0x4;

	_resourceNr = resourceNr;
	_data = _vm->_game.pictures[resourceNr].rdata;
	_dataSize = _vm->_game.dirPic[resourceNr].len;
	_dataOffset = 0;
	_dataOffsetNibble = false;

	_width = pic_width;
	_height = pic_height;

	if (clearScreen && !agi256) { // 256 color pictures should always fill the whole screen, so no clearing for them.
		_gfx->clear(15, 4); // Clear 16 color AGI screen (Priority 4, color white).
	}

	if (!agi256) {
		drawPicture(); // Draw 16 color picture.
	} else {
		drawPictureAGI256();
	}

	if (clearScreen)
		_vm->clearImageStack();
	_vm->recordImageStackCall(ADD_PIC, resourceNr, clearScreen, agi256, 0, 0, 0, 0);

	return errOK;
}

/**
 * Decode an AGI picture resource.
 * This function decodes an AGI picture resource into the correct slot
 * and draws it on the AGI screen, optionally clearing the screen before
 * drawing.
 * @param data   the AGI Picture data
 * @param length the size of the picture data buffer
 * @param clear  clear AGI screen before drawing
 */
int PictureMgr::decodePicture(byte *data, uint32 length, int clr, int pic_width, int pic_height) {
	_patCode = 0;
	_patNum = 0;
	_priOn = _scrOn = false;
	_scrColor = 0xF;
	_priColor = 0x4;

	_data = data;
	_dataSize = length;
	_dataOffset = 0;
	_dataOffsetNibble = false;

	_width = pic_width;
	_height = pic_height;

	if (clr) // 256 color pictures should always fill the whole screen, so no clearing for them.
		clear();

	drawPicture(); // Draw 16 color picture.

	return errOK;
}

/**
 * Unload an AGI picture resource.
 * This function unloads an AGI picture resource and deallocates
 * resource data.
 * @param n AGI picture resource number
 */
int PictureMgr::unloadPicture(int n) {
	// remove visual buffer & priority buffer if they exist
	if (_vm->_game.dirPic[n].flags & RES_LOADED) {
		free(_vm->_game.pictures[n].rdata);
		_vm->_game.dirPic[n].flags &= ~RES_LOADED;
	}

	return errOK;
}

void PictureMgr::clear() {
	_gfx->clear(15, 4); // Clear 16 color AGI screen (Priority 4, color white).
}

void PictureMgr::showPic() {
	debugC(8, kDebugLevelMain, "Show picture!");

	_gfx->render_Block(0, 0, SCRIPT_WIDTH, SCRIPT_HEIGHT);
}

/**
 * Show AGI picture.
 * This function copies a ``hidden'' AGI picture to the output device.
 */
void PictureMgr::showPic(int16 x, int16 y, int16 pic_width, int16 pic_height) {
	_width = pic_width;
	_height = pic_height;

	debugC(8, kDebugLevelMain, "Show picture!");

	_gfx->render_Block(x, y, pic_width, pic_height);
}

void PictureMgr::showPicWithTransition() {
	_width = SCRIPT_WIDTH;
	_height = SCRIPT_HEIGHT;

	debugC(8, kDebugLevelMain, "Show picture!");

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
			break;
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

// preagi needed functions (for plotPattern)
void PictureMgr::setPattern(uint8 code, uint8 num) {
	_patCode = code;
	_patNum = num;
}

void PictureMgr::setPictureVersion(AgiPictureVersion version) {
	_pictureVersion = version;

	if (version == AGIPIC_C64)
		_minCommand = 0xe0;
	else
		_minCommand = 0xf0;
}

void PictureMgr::setPictureData(uint8 *data, int len) {
	_data = data;
	_dataSize = len;
	_dataOffset = 0;
	_dataOffsetNibble = false;
	_flags = 0;
}

} // End of namespace Agi
