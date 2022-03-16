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

#ifndef CHEWY_MCGA_GRAPHICS_H
#define CHEWY_MCGA_GRAPHICS_H

#include "graphics/screen.h"

namespace Chewy {

class McgaGraphics {
public:
	McgaGraphics();
	~McgaGraphics();

	void init();

	void setClip(int16 x1, int16 y1, int16 x2, int16 y2);
	void setPointer(byte *ptr);

	void setPalette(byte *palette);
	void raster_col(int16 c, int16 r, int16 g, int16 b);
	void einblenden(byte *palette, int16 frames);
	void ausblenden(int16 frames);
	void set_partialpalette(const byte *palette, int16 startCol, int16 nr);

	void cls();
	void drawLine(int16 x1, int16 y1, int16 x2, int16 y2, int16 color);
	void box(int16 x1, int16 y1, int16 x2, int16 y2, int16 color);
	void boxFill(int16 x1, int16 y1, int16 x2, int16 y2, int16 color);
	void pop_box(int16 x, int16 y, int16 x1, int16 y1,
	             int16 col1, int16 col2, int16 back_col);

	void back2screen(byte *ptr);

	void spriteSave(byte *spritePtr, int16 x, int16 y, int16 width,
	                 int16 height, int16 screenWidth);
	void spriteSet(byte *sptr, int16 x, int16 y, int16 scrwidth);
	void scale_set(byte *sptr, int16 x, int16 y, int16 xdiff,
	               int16 ydiff, int16 scrwidth);
	void map_spr2screen(byte *sptr, int16 x, int16 y);

	void plot_scan_cur(int16 x, int16 y, int16 fcol, int16 bcol, int16 scrwidth,
	                   char cursor);
	int16 scanxy(int16 x, int16 y, int16 fcol, int16 bcol, int16 cur_col, int16 scrwidth, const char *string, ...);
	void printxy(int16 x, int16 y, int16 fgCol, int16 bgCol, int16 scrwidth,
	    const char *format);
	void move(int16 x, int16 y);

private:
	int16 devices();

	byte _palTable[PALETTE_SIZE];
	uint8 _einfuegen = 0;
};

} // namespace Chewy

#endif
