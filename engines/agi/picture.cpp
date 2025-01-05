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
	
// PictureMgr decodes and draws AGI picture resources.
//
// AGI pictures are vector-based, and contain the visual and priority screens.
// Drawing instructions begin with an opcode byte within the range F0-FF.
// Opcode parameters are each one byte, with the exception of AGIv3 nibble
// compression. Some opcodes take a variable number of parameters. The end of
// an instruction is detected by the next byte with a value in the opcode range.
// If an instruction has extra bytes, or a picture contains an unknown opcode
// byte, then these bytes ignored. Pictures end with opcode FF.
//
// AGIv3 introduced a compression scheme where two opcode parameters were
// each reduced to one nibble; this is indicated by a flag in the picture's
// resource directory entry.
//
// AGI's picture format evolved from variants used in earlier Sierra games.
// We implement support for these formats as subclasses of PictureMgr.
// In this way, we treat AGI as the baseline to be overridden as needed.

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

	_width = 0;
	_height = 0;
}

/**
 * Draws a pixel to the visual and/or control screen.
 */
void PictureMgr::putVirtPixel(int16 x, int16 y) {
	if (!getGraphicsCoordinates(x, y)) {
		return;
	}

	byte drawMask= 0;
	if (_priOn)
		drawMask |= GFX_SCREEN_MASK_PRIORITY;
	if (_scrOn)
		drawMask |= GFX_SCREEN_MASK_VISUAL;

	_gfx->putPixel(x, y, drawMask, _scrColor, _priColor);
}

/**
 * Gets the next byte in the picture data.
 */
byte PictureMgr::getNextByte() {
	if (!_dataOffsetNibble) {
		return _data[_dataOffset++];
	} else {
		byte curByte = _data[_dataOffset++] << 4;
		return (_data[_dataOffset] >> 4) | curByte;
	}
}

/**
 * Gets the next byte in the current picture instruction.
 * If the next byte in the picture data is an opcode, then this
 * function returns false and the data offset is not advanced.
 */
bool PictureMgr::getNextParamByte(byte &b) {
	byte value = getNextByte();
	if (value >= _minCommand) {
		_dataOffset--;
		return false;
	}
	b = value;
	return true;
}

/**
 * Gets the next nibble in the picture data.
 */
byte PictureMgr::getNextNibble() {
	if (!_dataOffsetNibble) {
		_dataOffsetNibble = true;
		return _data[_dataOffset] >> 4;
	} else {
		_dataOffsetNibble = false;
		return _data[_dataOffset++] & 0x0F;
	}
}

/**
 * Gets the next x coordinate in the current picture instruction.
 *
 * Subclasses override this to implement coordinate clipping.
 */
bool PictureMgr::getNextXCoordinate(byte &x) {
	return getNextParamByte(x);
}

/**
 * Gets the next y coordinate in the current picture instruction.
 *
 * Subclasses override this to implement coordinate clipping.
 */
bool PictureMgr::getNextYCoordinate(byte &y) {
	return getNextParamByte(y);
}

/**
 * Gets the next x and y coordinates in the current picture instruction.
 *
 * Returns false if both coordinates are not present. If only an x coordinate is
 * present, then the data offset is only advanced by one, and the x coordinate
 * will be ignored.
 */
bool PictureMgr::getNextCoordinates(byte &x, byte &y) {
	return getNextXCoordinate(x) && getNextYCoordinate(y);
}

/**
 * Validates picture coordinates and translates them to GfxMgr coordinates.
 *
 * Subclasses can override this to implement the PreAGI offset feature that
 * allowed a picture to be drawn at an arbitrary point on the screen.
 * Returns false if picture coordinates are out of bounds, or for subclasses,
 * if the PreAGI offset would place the coordinate outside of GfxMgr's screen.
 */
bool PictureMgr::getGraphicsCoordinates(int16 &x, int16 &y) {
	return (0 <= x && x < _width && 0 <= y && y < _height);
}

/**
 * xCorner
 *
 * Draws a series of lines with right angles between them.
 * The first two bytes are the start point, followed by alternating
 * x and y coordinates for subsequent points.
 *
 * Set skipOtherCoords to ignore extra coordinates in Troll's pictures.
 * Troll includes both the x and y coordinate of each point.
 */
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

/**
 * yCorner
 *
 * Draws a series of lines with right angles between them.
 * The first two bytes are the start point, followed by alternating
 * y and x coordinates for subsequent points.
 *
 * Set skipOtherCoords to ignore extra coordinates in Troll's pictures.
 * Troll includes both the x and y coordinate of each point.
 */
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

/**
 * plotPattern
 *
 * Draws a circle or square. Size and optional splatter brush pattern
 * are determined by the current pattern code.
 *
 * This routine is originally from NAGI.
 */
void PictureMgr::plotPattern(byte x, byte y) {
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

/**
 * plotBrush
 *
 * Plots the current brush pattern.
 */
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

/**
 * Draws the current picture to the visual and priority screens.
 */
void PictureMgr::drawPicture() {
	debugC(kDebugLevelPictures, "Drawing picture %d", _resourceNr);

	_dataOffset = 0;
	_dataOffsetNibble = false;
	_patCode = 0;
	_patNum = 0;
	_priOn = false;
	_scrOn = false;
	_scrColor = 15;
	_priColor = 4;

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
			warning("Unknown picture opcode %02x at %04x", curByte, _dataOffset - 1);
			break;
		}
	}
}

/**
 * Draws the current AGI256 picture to the visual screen.
 */
void PictureMgr::drawPicture_AGI256() {
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

/**
 * Sets the visual screen color to the next byte in the picture data.
 */
void PictureMgr::draw_SetColor() {
	_scrColor = getNextByte();

	// For CGA, replace the color with its mixture color
	if (_vm->_renderMode == Common::kRenderCGA) {
		_scrColor = _gfx->getCGAMixtureColor(_scrColor);
	}
}

/**
 * Sets the priority screen color to the next byte in the picture data.
 */
void PictureMgr::draw_SetPriority() {
	_priColor = getNextByte();
}

/**
 * Sets the visual screen color to the next nibble in the picture data.
 * Used in AGIv3 to compress the set-color instructions when the flag
 * RES_PICTURE_V3_NIBBLE_PARM is set in the picture's directory entry.
 */
void PictureMgr::draw_SetNibbleColor() {
	_scrColor = getNextNibble();

	// For CGA, replace the color with its mixture color
	if (_vm->_renderMode == Common::kRenderCGA) {
		_scrColor = _gfx->getCGAMixtureColor(_scrColor);
	}
}

/**
 * Sets the priority screen color to the next nibble in the picture data.
 * Used in AGIv3 to compress the set-color instructions when the flag
 * RES_PICTURE_V3_NIBBLE_PARM is set in the picture's directory entry.
 */
void PictureMgr::draw_SetNibblePriority() {
	_priColor = getNextNibble();
}

/**
 * Draws a horizontal, vertical, or diagonal line.
 *
 * This routine is originally from Sarien. Original comment:
 *
 *   A line drawing routine sent by Joshua Neal, modified by Stuart George
 *   (fixed >>2 to >>1 and some other bugs like x1 instead of y1, etc.)
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
 * draw_LineShort
 *
 * Draws short lines between positions in relative coordinates.
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

/**
 * draw_LineAbsolute
 *
 * Draws lines between positions in absolute coordinates.
 */
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

/**
 * Flood fills from a series of start positions.
 */
void PictureMgr::draw_Fill() {
	byte x, y;

	while (getNextCoordinates(x, y)) {
		draw_Fill(x, y);
	}
}

/**
 * Flood fills from a start position.
 */
void PictureMgr::draw_Fill(int16 x, int16 y) {
	if (!_scrOn && !_priOn)
		return;

	// Push initial pixel on the stack
	Common::Stack<Common::Point> stack;
	stack.push(Common::Point(x, y));

	// Exit if stack is empty
	while (!stack.empty()) {
		Common::Point p = stack.pop();

		if (!draw_FillCheck(p.x, p.y, false))
			continue;

		// Scan for left border
		uint c;
		for (c = p.x - 1; draw_FillCheck(c, p.y, true); c--)
			;

		bool newspanUp = true;
		bool newspanDown = true;
		for (c++; draw_FillCheck(c, p.y, true); c++) {
			putVirtPixel(c, p.y);
			if (draw_FillCheck(c, p.y - 1, false)) {
				if (newspanUp) {
					stack.push(Common::Point(c, p.y - 1));
					newspanUp = false;
				}
			} else {
				newspanUp = true;
			}

			if (draw_FillCheck(c, p.y + 1, false)) {
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

/**
 * Checks if flood fill is allowed at a position.
 *
 * horizontalCheck indicates if the flood fill algorithm is scanning the current
 * line horizontally for a boundary. This is used by PictureMgr_Troll to handle
 * Troll's Tale custom flood fill behavior when drawing the Troll over pictures.
 */
bool PictureMgr::draw_FillCheck(int16 x, int16 y, bool horizontalCheck) {
	if (!getGraphicsCoordinates(x, y)) {
		return false;
	}

	byte screenColor = _gfx->getColor(x, y);
	byte screenPriority = _gfx->getPriority(x, y);

	if (!_priOn && _scrOn && _scrColor != 15)
		return (screenColor == 15);

	if (_priOn && !_scrOn && _priColor != 4)
		return (screenPriority == 4);

	return (_scrOn && screenColor == 15 && _scrColor != 15);
}

/**
 * Draws a picture by resource number to the visual and control screens.
 * This interface is used by AGI games and GAL (KQ1 early).
 *
 * The picture resource must already be loaded. This function sets the current
 * picture and optionally clears the screens before drawing.
 */
void PictureMgr::decodePicture(int16 resourceNr, bool clearScreen, bool agi256, int16 width, int16 height) {
	_resourceNr = resourceNr;
	_data = _vm->_game.pictures[resourceNr].rdata;
	_dataSize = _vm->_game.dirPic[resourceNr].len;
	_width = width;
	_height = height;

	if (clearScreen) {
		_gfx->clear(15, getInitialPriorityColor()); // white, priority 4 or 1
	}

	if (!agi256) {
		drawPicture();
	} else {
		drawPicture_AGI256();
	}

	if (clearScreen) {
		_vm->clearImageStack();
	}
	_vm->recordImageStackCall(ADD_PIC, resourceNr, clearScreen, agi256, 0, 0, 0, 0);
}

/**
 * Draws a picture from a buffer to the visual and control screens.
 * This interface is used by PreAGI games.
 *
 * This function sets the current picture and optionally clears the screens
 * before drawing.
 */
void PictureMgr::decodePictureFromBuffer(byte *data, uint32 length, bool clearScreen, int16 width, int16 height) {
	_data = data;
	_dataSize = length;
	_width = width;
	_height = height;

	if (clearScreen) {
		_gfx->clear(15, getInitialPriorityColor()); // white, priority 4 or 1
	}

	drawPicture();
}

/**
 * Renders a drawn picture from the active screen to the display screen.
 *
 * The active screen is usually the visual screen, but this can be toggled
 * to the priority screen in debug modes.
 */
void PictureMgr::showPicture(int16 x, int16 y, int16 width, int16 height) {
	debugC(kDebugLevelPictures, "Show picture");

	_gfx->render_Block(x, y, width, height);
}

/**
 * Renders a drawn picture from the active screen to the display screen
 * with transition effects. The effect is determined by the render mode.
 *
 * The active screen is usually the visual screen, but this can be toggled
 * to the priority screen in debug modes.
 */
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

} // End of namespace Agi
