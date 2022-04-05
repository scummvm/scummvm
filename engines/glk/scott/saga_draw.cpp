#include "saga_draw.h"
#include "globals.h"
#include "scott.h"
#include "definitions.h"

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
		uint8_t data = *dataptr++;
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
		if (dataptr - _G(_entireFile) > _G(_fileLength))
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
				for (int i = 0; i < 8; i++)
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
		error("Invalid image number % d !Last image: % d\n ", pictureNumber, numgraphics - 1);
		return;
	}

	Image img = _G(_images)[pictureNumber];

	if (img._imageData == nullptr)
		return;

	drawSagaPictureFromData(img._imageData, img._width, img._height, img._xOff, img._yOff);
}

void putPixel(glsi32 x, glsi32 y, int32_t color) {
	int yOffset = 0;

	glui32 glk_color = ((_G(_pal)[color][0] << 16)) | ((_G(_pal)[color][1] << 8)) | (_G(_pal)[color][2]);

	g_vm->glk_window_fill_rect(_G(_graphics), glk_color, x * _G(_pixelSize) + _G(_xOffset),
								 y * _G(_pixelSize) + yOffset, _G(_pixelSize), _G(_pixelSize));
}

void rectFill(int32_t x, int32_t y, int32_t width, int32_t height, int32_t color) {
	int yOffset = 0;

	int bufferpos = (y / 8) * 32 + (x / 8);
	if (bufferpos >= 0xD80)
		return;
	_G(_buffer)[bufferpos][8] = _G(_buffer)[bufferpos][8] | (color << 3);

	glui32 glk_color = ((_G(_pal)[color][0] << 16)) | ((_G(_pal)[color][1] << 8)) | (_G(_pal)[color][2]);

	g_vm->glk_window_fill_rect(_G(_graphics), glk_color, x * _G(_pixelSize) + _G(_xOffset),
							   y * _G(_pixelSize) + yOffset, width * _G(_pixelSize), height * _G(_pixelSize));
}

int32_t remap(int32_t color) {
	int32_t mapcol;

	if ((_G(_palChosen) == ZX) || (_G(_palChosen) == ZXOPT)) {
		/* nothing to remap here; shows that the gfx were created on a ZX */
		mapcol = (((color >= 0) && (color <= 15)) ? color : INVALID_COLOR);
	} else if (_G(_palChosen) == C64A) {
		/* remap A determined from Golden Baton, applies to S1/S3/S13 too (8col) */
		int32_t c64remap[] = {0, 6, 2, 4, 5, 3, 7, 1, 8, 1, 1, 1, 7, 12, 8, 7 };
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
