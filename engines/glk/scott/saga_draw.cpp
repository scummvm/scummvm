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

/*
 * Based on ScottFree interpreter version 1.14 developed by Swansea
 * University Computer Society without disassembly of any other game
 * drivers, only of game databases as permitted by EEC law (for purposes
 * of compatibility).
 *
 * Licensed under GPLv2
 *
 * https://github.com/angstsmurf/spatterlight/tree/master/terps/scott
 */

#include "glk/scott/scott.h"
#include "glk/scott/definitions.h"
#include "glk/scott/resource.h"
#include "glk/scott/globals.h"
#include "glk/scott/saga_draw.h"

namespace Glk {
namespace Scott {

#define INVALID_COLOR 16

void rot90(uint8_t character[]) {
	int32_t i, j;
	uint8_t work2[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	for (i = 0; i < 8; i++)
		for (j = 0; j < 8; j++)
			if ((character[j] & (1 << i)) != 0)
				work2[7 - i] += 1 << j;

	for (i = 0; i < 8; i++)
		character[i] = work2[i];
}

void rot180(uint8_t character[]) {
	int32_t i, j;
	uint8_t work2[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	for (i = 0; i < 8; i++)
		for (j = 0; j < 8; j++)
			if ((character[i] & (1 << j)) != 0)
				work2[7 - i] += 1 << (7 - j);

	for (i = 0; i < 8; i++)
		character[i] = work2[i];
}

void rot270(uint8_t character[]) {
	int32_t i, j;
	uint8_t work2[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	for (i = 0; i < 8; i++)
		for (j = 0; j < 8; j++)
			if ((character[j] & (1 << i)) != 0)
				work2[i] += 1 << (7 - j);

	for (i = 0; i < 8; i++)
		character[i] = work2[i];
}

void flip(uint8_t character[]) {
	int32_t i, j;
	uint8_t work2[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	for (i = 0; i < 8; i++)
		for (j = 0; j < 8; j++)
			if ((character[i] & (1 << j)) != 0)
				work2[i] += 1 << (7 - j);
	for (i = 0; i < 8; i++)
		character[i] = work2[i];
}

void background(int32_t x, int32_t y, int32_t color) {
	/* Draw the background */
	rectFill(x * 8, y * 8, 8, 8, color);
}

void plotsprite(int32_t character, int32_t x, int32_t y, int32_t fg, int32_t bg) {
	int32_t i, j;
	background(x, y, bg);
	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++)
			if ((_G(_screenchars)[character][i] & (1 << j)) != 0)
				putPixel(x * 8 + j, y * 8 + i, fg);
	}
}

void transform(int32_t character, int32_t flipMode, int32_t ptr) {
	if (character > 255)
		return;
	uint8_t work[8];
	int32_t i;

#ifdef DRAWDEBUG
	debug("Plotting char: %d with flip: %02x (%s) at %d: %d,%d\n",
		  character, flip_mode, flipdescription[(flip_mode & 48) >> 4],
		  ptr, ptr % 0x20, ptr / 0x20);
#endif

	// first copy the character into work
	for (i = 0; i < 8; i++)
		work[i] = _G(_sprite)[character][i];

	// Now flip it
	if ((flipMode & 0x30) == 0x10) {
		rot90(work);
	}
	if ((flipMode & 0x30) == 0x20) {
		rot180(work);
	}
	if ((flipMode & 0x30) == 0x30) {
		rot270(work);
	}
	if ((flipMode & 0x40) != 0) {
		flip(work);
	}
	flip(work);

	// Now mask it onto the previous character
	for (i = 0; i < 8; i++) {
		if ((flipMode & 0x0c) == 12)
			_G(_screenchars)[ptr][i] ^= work[i];
		else if ((flipMode & 0x0c) == 8)
			_G(_screenchars)[ptr][i] &= work[i];
		else if ((flipMode & 0x0c) == 4)
			_G(_screenchars)[ptr][i] |= work[i];
		else
			_G(_screenchars)[ptr][i] = work[i];
	}
}

uint8_t *drawSagaPictureFromData(uint8_t *dataptr, int xSize, int ySize, int xOff, int yOff) {
	int32_t offset = 0, cont = 0;
	int32_t i, x, y, mask_mode;
	uint8_t data, data2, old = 0;
	int32_t ink[0x22][14], paper[0x22][14];

	//   uint8_t *origptr = dataptr;
	int version = _G(_game)->_pictureFormatVersion;

	offset = 0;
	int32_t character = 0;
	int32_t count;
	do {
		count = 1;

		/* first handle mode */
		data = *dataptr++;
		if (data < 0x80) {
			if (character > 127 && version > 2) {
				data += 128;
			}
			character = data;
#ifdef DRAWDEBUG
			debug("******* SOLO CHARACTER: %04x\n", character);
#endif
			transform(character, 0, offset);
			offset++;
			if (offset > 767)
				break;
		} else {
			// first check for a count
			if ((data & 2) == 2) {
				count = (*dataptr++) + 1;
			}

			// Next get character and plot (count) times
			character = *dataptr++;

			// Plot the initial character
			if ((data & 1) == 1 && character < 128)
				character += 128;

			for (i = 0; i < count; i++)
				transform(character, (data & 0x0c) ? (data & 0xf3) : data, offset + i);

			// Now check for overlays
			if ((data & 0xc) != 0) {
				// We have overlays so grab each member of the stream and work out what
				// to do with it

				mask_mode = (data & 0xc);
				data2 = *dataptr++;
				old = data;
				do {
					cont = 0;
					if (data2 < 0x80) {
						if (version == 4 && (old & 1) == 1)
							data2 += 128;
#ifdef DRAWDEBUG
						debug("Plotting %d directly (overlay) at %d\n", data2, offset);
#endif
						for (i = 0; i < count; i++)
							transform(data2, old & 0x0c, offset + i);
					} else {
						character = *dataptr++;
						if ((data2 & 1) == 1)
							character += 128;
#ifdef DRAWDEBUG
						debug("Plotting %d with flip %02x (%s) at %d %d\n", character, (data2 | mask_mode), flipdescription[((data2 | mask_mode) & 48) >> 4],
							  offset, count);
#endif
						for (i = 0; i < count; i++)
							transform(character, (data2 & 0xf3) | mask_mode, offset + i);
						if ((data2 & 0x0c) != 0) {
							mask_mode = (data2 & 0x0c);
							old = data2;
							cont = 1;
							data2 = *dataptr++;
						}
					}
				} while (cont != 0);
			}
			offset += count;
		}
	} while (offset < xSize * ySize);

	y = 0;
	x = 0;

	uint8_t colour = 0;
	// Note version 3 count is inverse it is repeat previous colour
	// Whilst version0-2 count is repeat next character
	while (y < ySize) {
		if (dataptr > _G(_entireFile) && static_cast<size_t>(dataptr - _G(_entireFile)) > _G(_fileLength))
			return dataptr - 1;
		data = *dataptr++;
		if ((data & 0x80)) {
			count = (data & 0x7f) + 1;
			if (version >= 3) {
				count--;
			} else {
				colour = *dataptr++;
			}
		} else {
			count = 1;
			colour = data;
		}
		while (count > 0) {
			// Now split up depending on which version we're using

			// For version 3+

			if (_G(_drawToBuffer))
				_G(_buffer)[(yOff + y) * 32 + (xOff + x)][8] = colour;
			else {
				if (version > 2) {
					if (x > 33)
						return nullptr;
					// ink is 0-2, screen is 3-5, 6 in bright flag
					ink[x][y] = colour & 0x07;
					paper[x][y] = (colour & 0x38) >> 3;

					if ((colour & 0x40) == 0x40) {
						paper[x][y] += 8;
						ink[x][y] += 8;
					}
				} else {
					if (x > 33)
						return nullptr;
					paper[x][y] = colour & 0x07;
					ink[x][y] = ((colour & 0x70) >> 4);

					if ((colour & 0x08) == 0x08 || version < 2) {
						paper[x][y] += 8;
						ink[x][y] += 8;
					}
				}
			}

			x++;
			if (x == xSize) {
				x = 0;
				y++;
			}
			count--;
		}
	}
	offset = 0;
	int32_t xoff2;
	for (y = 0; y < ySize; y++)
		for (x = 0; x < xSize; x++) {
			xoff2 = xOff;
			if (version > 0 && version < 3)
				xoff2 = xOff - 4;

			if (_G(_drawToBuffer)) {
				for (i = 0; i < 8; i++)
					_G(_buffer)[(y + yOff) * 32 + x + xoff2][i] = _G(_screenchars)[offset][i];
			} else {
				plotsprite(offset, x + xoff2, y + yOff, remap(ink[x][y]), remap(paper[x][y]));
			}

#ifdef DRAWDEBUG
			debug("(gfx#:plotting %d,%d:paper=%s,ink=%s)\n", x + xoff2, y + yoff,
				  colortext(remap(paper[x][y])), colortext(remap(ink[x][y])));
#endif
			offset++;
			if (offset > 766)
				break;
		}
	return dataptr;
}

void drawSagaPictureNumber(int pictureNumber) {
	int numgraphics = _G(_game)->_numberOfPictures;
	if (pictureNumber >= numgraphics) {

		error("drawSagaPictureNumber: Invalid image number % d !Last image: % d", pictureNumber, numgraphics - 1);
		return;
	}

	Image img = _G(_images)[pictureNumber];

	if (img._imageData == nullptr)
		return;

	drawSagaPictureFromData(img._imageData, img._width, img._height, img._xOff, img._yOff);
}

void drawSagaPictureAtPos(int pictureNumber, int x, int y) {
	Image img = _G(_images)[pictureNumber];

	drawSagaPictureFromData(img._imageData, img._width, img._height, x, y);
}

void sagaSetup(size_t imgOffset) {
	int32_t i, y;

	Common::Array<uint16_t> imageOffsets(_G(_game)->_numberOfPictures);

	if (_G(_palChosen) == NO_PALETTE) {
		_G(_palChosen) = _G(_game)->_palette;
	}

	if (_G(_palChosen) == NO_PALETTE) {
		error("sagaSetup: unknown palette");
	}

	definePalette();

	int version = _G(_game)->_pictureFormatVersion;

	int32_t CHAR_START = _G(_game)->_startOfCharacters + _G(_fileBaselineOffset);
	int32_t OFFSET_TABLE_START = _G(_game)->_startOfImageData + _G(_fileBaselineOffset);

	if (_G(_game)->_startOfImageData == FOLLOWS) {
		OFFSET_TABLE_START = CHAR_START + 0x800;
	}

	int32_t DATA_OFFSET = _G(_game)->_imageAddressOffset + _G(_fileBaselineOffset);
	if (imgOffset)
		DATA_OFFSET = imgOffset;
	uint8_t *pos;
	int numgraphics = _G(_game)->_numberOfPictures;

	pos = seekToPos(_G(_entireFile), CHAR_START);

#ifdef DRAWDEBUG
	debug("Grabbing Character details\n");
	debug("Character Offset: %04x\n", CHAR_START - _G(_fileBaselineOffset));
#endif
	for (i = 0; i < 256; i++) {
		for (y = 0; y < 8; y++) {
			_G(_sprite)[i][y] = *(pos++);
		}
	}

	_G(_images).resize(numgraphics);
	Image *img = &_G(_images)[0];

	pos = seekToPos(_G(_entireFile), OFFSET_TABLE_START);
	pos = seekToPos(_G(_entireFile), OFFSET_TABLE_START);

	for (i = 0; i < numgraphics; i++) {
		if (_G(_game)->_pictureFormatVersion == 0) {
			uint16_t address;

			if (i < 11) {
				address = _G(_game)->_startOfImageData + (i * 2);
			} else if (i < 28) {
				address = _G(_hulkItemImageOffsets) + (i - 10) * 2;
			} else if (i < 34) {
				address = _G(_hulkLookImageOffsets) + (i - 28) * 2;
			} else {
				address = _G(_hulkSpecialImageOffsets) + (i - 34) * 2;
			}

			address += _G(_fileBaselineOffset);
			address = _G(_entireFile)[address] + _G(_entireFile)[address + 1] * 0x100;

			imageOffsets[i] = address + _G(_hulkImageOffset);
		} else {
			imageOffsets[i] = *(pos++);
			imageOffsets[i] += *(pos++) * 0x100;
		}
	}

	for (int picture_number = 0; picture_number < numgraphics; picture_number++) {
		pos = seekToPos(_G(_entireFile), imageOffsets[picture_number] + DATA_OFFSET);
		if (pos == 0)
			return;

		img->_width = *(pos++);
		if (img->_width > 32)
			img->_width = 32;

		img->_height = *(pos++);
		if (img->_height > 12)
			img->_height = 12;

		if (version > 0) {
			img->_xOff = *(pos++);
			if (img->_xOff > 32)
				img->_xOff = 4;
			img->_yOff = *(pos++);
			if (img->_yOff > 12)
				img->_yOff = 0;
		} else {
			if (picture_number > 9 && picture_number < 28) {
				img->_xOff = _G(_entireFile)[_G(_hulkCoordinates) + picture_number - 10 + _G(_fileBaselineOffset)];
				img->_yOff = _G(_entireFile)[_G(_hulkCoordinates) + 18 + picture_number - 10 + _G(_fileBaselineOffset)];
			} else {
				img->_xOff = img->_yOff = 0;
			}
		}

		img->_imageData = pos;

		img++;
	}
}

void putPixel(glsi32 x, glsi32 y, int32_t color) {
	int yOffset = 0;

	glui32 glk_color = ((_G(_pal)[color][0] << 16)) | ((_G(_pal)[color][1] << 8)) | (_G(_pal)[color][2]);

	g_scott->glk_window_fill_rect(_G(_graphics), glk_color, x * _G(_pixelSize) + _G(_xOffset),
							   y * _G(_pixelSize) + yOffset, _G(_pixelSize), _G(_pixelSize));
}

void rectFill(int32_t x, int32_t y, int32_t width, int32_t height, int32_t color) {
	int yOffset = 0;

	int bufferpos = (y / 8) * 32 + (x / 8);
	if (bufferpos >= 0xD80)
		return;
	_G(_buffer)[bufferpos][8] = _G(_buffer)[bufferpos][8] | (color << 3);

	glui32 glk_color = ((_G(_pal)[color][0] << 16)) | ((_G(_pal)[color][1] << 8)) | (_G(_pal)[color][2]);

	g_scott->glk_window_fill_rect(_G(_graphics), glk_color, x * _G(_pixelSize) + _G(_xOffset),
							   y * _G(_pixelSize) + yOffset, width * _G(_pixelSize), height * _G(_pixelSize));
}

void switchPalettes(int pal1, int pal2) {
	uint8_t temp[3];

	temp[0] = _G(_pal)[pal1][0];
	temp[1] = _G(_pal)[pal1][1];
	temp[2] = _G(_pal)[pal1][2];

	_G(_pal)[pal1][0] = _G(_pal)[pal2][0];
	_G(_pal)[pal1][1] = _G(_pal)[pal2][1];
	_G(_pal)[pal1][2] = _G(_pal)[pal2][2];

	_G(_pal)[pal2][0] = temp[0];
	_G(_pal)[pal2][1] = temp[1];
	_G(_pal)[pal2][2] = temp[2];
}

void setColor(int32_t index, RGB *colour) {
	_G(_pal)[index][0] = (*colour)[0];
	_G(_pal)[index][1] = (*colour)[1];
	_G(_pal)[index][2] = (*colour)[2];
}

void definePalette() {
	/* set up the palette */
	if (_G(_palChosen) == VGA) {
		RGB black = {0, 0, 0};
		RGB blue = {0, 0, 255};
		RGB red = {255, 0, 0};
		RGB magenta = {255, 0, 255};
		RGB green = {0, 255, 0};
		RGB cyan = {0, 255, 255};
		RGB yellow = {255, 255, 0};
		RGB white = {255, 255, 255};
		RGB brblack = {0, 0, 0};
		RGB brblue = {0, 0, 255};
		RGB brred = {255, 0, 0};
		RGB brmagenta = {255, 0, 255};
		RGB brgreen = {0, 255, 0};
		RGB brcyan = {0, 255, 255};
		RGB bryellow = {255, 255, 0};
		RGB brwhite = {255, 255, 255};

		setColor(0, &black);
		setColor(1, &blue);
		setColor(2, &red);
		setColor(3, &magenta);
		setColor(4, &green);
		setColor(5, &cyan);
		setColor(6, &yellow);
		setColor(7, &white);
		setColor(8, &brblack);
		setColor(9, &brblue);
		setColor(10, &brred);
		setColor(11, &brmagenta);
		setColor(12, &brgreen);
		setColor(13, &brcyan);
		setColor(14, &bryellow);
		setColor(15, &brwhite);
	} else if (_G(_palChosen) == ZX) {
		/* corrected Sinclair ZX palette (pretty dull though) */
		RGB black = {0, 0, 0};
		RGB blue = {0, 0, 154};
		RGB red = {154, 0, 0};
		RGB magenta = {154, 0, 154};
		RGB green = {0, 154, 0};
		RGB cyan = {0, 154, 154};
		RGB yellow = {154, 154, 0};
		RGB white = {154, 154, 154};
		RGB brblack = {0, 0, 0};
		RGB brblue = {0, 0, 170};
		RGB brred = {186, 0, 0};
		RGB brmagenta = {206, 0, 206};
		RGB brgreen = {0, 206, 0};
		RGB brcyan = {0, 223, 223};
		RGB bryellow = {239, 239, 0};
		RGB brwhite = {255, 255, 255};

		setColor(0, &black);
		setColor(1, &blue);
		setColor(2, &red);
		setColor(3, &magenta);
		setColor(4, &green);
		setColor(5, &cyan);
		setColor(6, &yellow);
		setColor(7, &white);
		setColor(8, &brblack);
		setColor(9, &brblue);
		setColor(10, &brred);
		setColor(11, &brmagenta);
		setColor(12, &brgreen);
		setColor(13, &brcyan);
		setColor(14, &bryellow);
		setColor(15, &brwhite);

		_G(_whiteColour) = 15;
		_G(_blueColour) = 9;
		_G(_diceColour) = 0xff0000;
	} else if (_G(_palChosen) == ZXOPT) {
		/* optimized but not realistic Sinclair ZX palette (SPIN emu) */
		RGB black = {0, 0, 0};
		RGB blue = {0, 0, 202};
		RGB red = {202, 0, 0};
		RGB magenta = {202, 0, 202};
		RGB green = {0, 202, 0};
		RGB cyan = {0, 202, 202};
		RGB yellow = {202, 202, 0};
		RGB white = {202, 202, 202};
		/*
	 old David Lodge palette:

	 RGB black = { 0, 0, 0 };
	 RGB blue = { 0, 0, 214 };
	 RGB red = { 214, 0, 0 };
	 RGB magenta = { 214, 0, 214 };
	 RGB green = { 0, 214, 0 };
	 RGB cyan = { 0, 214, 214 };
	 RGB yellow = { 214, 214, 0 };
	 RGB white = { 214, 214, 214 };
	 */
		RGB brblack = {0, 0, 0};
		RGB brblue = {0, 0, 255};
		RGB brred = {255, 0, 20};
		RGB brmagenta = {255, 0, 255};
		RGB brgreen = {0, 255, 0};
		RGB brcyan = {0, 255, 255};
		RGB bryellow = {255, 255, 0};
		RGB brwhite = {255, 255, 255};

		setColor(0, &black);
		setColor(1, &blue);
		setColor(2, &red);
		setColor(3, &magenta);
		setColor(4, &green);
		setColor(5, &cyan);
		setColor(6, &yellow);
		setColor(7, &white);
		setColor(8, &brblack);
		setColor(9, &brblue);
		setColor(10, &brred);
		setColor(11, &brmagenta);
		setColor(12, &brgreen);
		setColor(13, &brcyan);
		setColor(14, &bryellow);
		setColor(15, &brwhite);

		_G(_whiteColour) = 15;
		_G(_blueColour) = 9;
		_G(_diceColour) = 0xff0000;

	} else if ((_G(_palChosen) == C64A) || (_G(_palChosen) == C64B)) {
		/* and now: C64 palette (pepto/VICE) */
		RGB black = {0, 0, 0};
		RGB white = {255, 255, 255};
		RGB red = {191, 97, 72};
		RGB cyan = {153, 230, 249};
		RGB purple = {177, 89, 185};
		RGB green = {121, 213, 112};
		RGB blue = {95, 72, 233};
		RGB yellow = {247, 255, 108};
		RGB orange = {186, 134, 32};
		RGB brown = {116, 105, 0};
		RGB lred = {180, 105, 164};
		RGB dgrey = {69, 69, 69};
		RGB grey = {167, 167, 167};
		RGB lgreen = {154, 210, 134};
		RGB lblue = {162, 143, 255};
		RGB lgrey = {150, 150, 150};

		setColor(0, &black);
		setColor(1, &white);
		setColor(2, &red);
		setColor(3, &cyan);
		setColor(4, &purple);
		setColor(5, &green);
		setColor(6, &blue);
		setColor(7, &yellow);
		setColor(8, &orange);
		setColor(9, &brown);
		setColor(10, &lred);
		setColor(11, &dgrey);
		setColor(12, &grey);
		setColor(13, &lgreen);
		setColor(14, &lblue);
		setColor(15, &lgrey);

		_G(_whiteColour) = 1;
		_G(_blueColour) = 6;
		_G(_diceColour) = 0x5f48e9;
	}
}

int32_t remap(int32_t color) {
	int32_t mapcol;

	if ((_G(_palChosen) == ZX) || (_G(_palChosen) == ZXOPT)) {
		/* nothing to remap here; shows that the gfx were created on a ZX */
		mapcol = (((color >= 0) && (color <= 15)) ? color : INVALID_COLOR);
	} else if (_G(_palChosen) == C64A) {
		/* remap A determined from Golden Baton, applies to S1/S3/S13 too (8col) */
		int32_t c64remap[] = {0, 6, 2, 4, 5, 3, 7, 1, 8, 1, 1, 1, 7, 12, 8, 7};
		mapcol = (((color >= 0) && (color <= 15)) ? c64remap[color] : INVALID_COLOR);
	} else if (_G(_palChosen) == C64B) {
		/* remap B determined from Spiderman (16col) */
		int32_t c64remap[] = {0, 6, 9, 4, 5, 14, 8, 12, 0, 6, 2, 4, 5, 3, 7, 1};
		mapcol = (((color >= 0) && (color <= 15)) ? c64remap[color] : INVALID_COLOR);
	} else
		mapcol = (((color >= 0) && (color <= 15)) ? color : INVALID_COLOR);

	return (mapcol);
}

} // End of namespace Scott
} // End of namespace Glk
