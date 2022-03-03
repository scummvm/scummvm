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
#include "chewy/ngstypes.h"

namespace Chewy {

class McgaGraphics {
public:
	McgaGraphics();
	~McgaGraphics();

	void init();
	void setWriteMode(char wm);
	void initMouseMode(MouseInfo *mInfo);

	void setClip(int16 x1, int16 y1, int16 x2, int16 y2);
	void setPointer(byte *ptr);
	byte *getPointer();

	void setPalette(byte *palette);
	void savePalette(byte *palette = nullptr);
	void rest_palette();
	void raster_col(int16 c, int16 r, int16 g, int16 b);
	void einblenden(byte *palette, int16 frames);
	void ausblenden(int16 frames);
	void set_teilpalette(const byte *palette, int16 startcol, int16 anz);

	void cls();
	uint8 get_pixel(int16 xpos, int16 ypos);
	void linie(int16 x1, int16 y1, int16 x2, int16 y2, int16 farbe);
	void box(int16 x1, int16 y1, int16 x2, int16 y2, int16 farbe);
	void boxFill(int16 x1, int16 y1, int16 x2, int16 y2, int16 farbe);
	void pop_box(int16 x, int16 y, int16 x1, int16 y1,
	             int16 col1, int16 col2, int16 back_col);

	void back2screen(byte *ptr);
	void back2back(byte *ptr1, byte *ptr2);

	void sprite_save(byte *sptr, int16 x, int16 y, int16 breite,
	                 int16 hoehe, int16 scrwidth);
	void blockcopy(byte *sptr, int16 x, int16 y, int16 scrwidth);
	void spriteSet(byte *sptr, int16 x, int16 y, int16 scrwidth);
	void scale_set(byte *sptr, int16 x, int16 y, int16 xdiff,
	               int16 ydiff, int16 scrwidth);
	void map_spr2screen(byte *sptr, int16 x, int16 y);

	void plot_scan_cur(int16 x, int16 y, int16 fcol, int16 bcol, int16 scrwidth,
	                   char cursor);
	int16 scanxy(int16 x, int16 y, int16 fcol, int16 bcol, int16 cur_col, int16 scrwidth, const char *string, ...);
	void printxy(int16 x, int16 y, int16 fgCol, int16 bgCol, int16 scrwidth,
	    const char *format);
	void vorschub();
	void move(int16 x, int16 y);

private:
	int16 devices();

	float _sines[360];
	float _cosines[360];
	byte _palTable[PALETTE_SIZE];
	uint8 _writeMode = 0;
	int16 _crlfx = 0, _crlfy = 0;
	int16 _fontBr = 0, _fontH = 0;
	uint8 _svga = 0;
	uint8 _einfuegen = 0;
	MouseInfo *_mInfo = nullptr;

	void ltoa(long N, char *str, int base);
	void ultoa(uint32 N, char *str, int base);
};

} // namespace Chewy

#endif
