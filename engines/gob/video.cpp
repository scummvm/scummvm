/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */
#include "common/stdafx.h"
#include "common/endian.h"

#include "gob/gob.h"
#include "gob/global.h"
#include "gob/video.h"
#include "gob/dataio.h"
#include "gob/draw.h"

#include "gob/driver_vga.h"

namespace Gob {

/* NOT IMPLEMENTED */

Video::Video(GobEngine *vm) : _vm(vm) {
	_extraMode = false;
}

char Video::initDriver(int16 vidMode) {
	warning("STUB: Video::initDriver");

	// FIXME: Finish all this stuff :)
	_videoDriver = new VGAVideoDriver();

	return 1;
}

void Video::freeDriver() {
	delete _videoDriver;
	warning("STUB: Video::freeDriver");
}

int32 Video::getRectSize(int16 width, int16 height, int16 flag, int16 mode) {
	int32 size;

	if ((mode & 0x7f) != 0x13)
		warning
		    ("Video::getRectSize: Video mode %d is not fully supported!",
		    mode & 0x7f);
	switch (mode & 0x7f) {
	case 5:
	case 7:
		size = ((int32)((width + 3) / 4)) * height * (flag + 1);
		break;
	case 0x13:
		size = (int32)width *height;
		break;
	case 0x14:
	case 0x15:
	case 0x16:
		size = ((int32)((width + 3) / 4)) * height * 4;
		break;
	default:
		size = ((int32)((width + 7) / 8)) * height * (flag + 4);
		break;
	}
	return size;
}

void Video::freeSurfDesc(SurfaceDesc * surfDesc) {
	delete[] surfDesc->vidPtr;
	// GOB2: surfDesc != _vm->_draw->_frontSurface (since _frontSurface is set
	// to _pPrimarySurfDesc in Game::prepareStart(), is there a difference?)
	if (surfDesc != _vm->_global->_pPrimarySurfDesc) {
		_vm->_global->_sprAllocated--;
		delete surfDesc;
	}
}

int16 Video::clampValue(int16 val, int16 max) {
	if (val >= max)
		val = max - 1;

	if (val < 0)
		val = 0;

	return val;
}

void Video::drawSprite(SurfaceDesc *source, SurfaceDesc *dest,
	    int16 left, int16 top, int16 right, int16 bottom, int16 x, int16 y, int16 transp) {
	int16 temp;
	int16 destRight;
	int16 destBottom;

	if (_vm->_global->_doRangeClamp) {
		if (left > right) {
			temp = left;
			left = right;
			right = temp;
		}
		if (top > bottom) {
			temp = top;
			top = bottom;
			bottom = temp;
		}
		if (right < 0)
			return;
		if (bottom < 0)
			return;
		if (left >= source->width)
			return;
		if (top >= source->height)
			return;

		if (left < 0) {
			x -= left;
			left = 0;
		}
		if (top < 0) {
			y -= top;
			top = 0;
		}
		right = Video::clampValue(right, source->width);
		bottom = Video::clampValue(bottom, source->height);
		if (right - left >= source->width)
			right = left + source->width - 1;
		if (bottom - top >= source->height)
			bottom = top + source->height - 1;

		if (x < 0) {
			left -= x;
			x = 0;
		}
		if (y < 0) {
			top -= y;
			y = 0;
		}
		if (left > right)
			return;
		if (top > bottom)
			return;

		if (x >= dest->width)
			return;

		if (y >= dest->height)
			return;

		destRight = x + right - left;
		destBottom = y + bottom - top;
		if (destRight >= dest->width)
			right -= destRight - dest->width + 1;

		if (destBottom >= dest->height)
			bottom -= destBottom - dest->height + 1;
	}

	_videoDriver->drawSprite(source, dest, left, top, right, bottom, x, y, transp);
}

void Video::fillRect(SurfaceDesc *dest, int16 left, int16 top, int16 right, int16 bottom,
	    int16 color) {
	int16 temp;

	if (_vm->_global->_doRangeClamp) {
		if (left > right) {
			temp = left;
			left = right;
			right = temp;
		}
		if (top > bottom) {
			temp = top;
			top = bottom;
			bottom = temp;
		}
		if (right < 0)
			return;
		if (bottom < 0)
			return;
		if (left >= dest->width)
			return;
		if (top >= dest->height)
			return;

		left = Video::clampValue(left, dest->width);
		top = Video::clampValue(top, dest->height);
		right = Video::clampValue(right, dest->width);
		bottom = Video::clampValue(bottom, dest->height);
	}

	_videoDriver->fillRect(dest, left, top, right, bottom, color);
}

void Video::drawLine(SurfaceDesc *dest, int16 x0, int16 y0, int16 x1, int16 y1, int16 color) {
	if (x0 == x1 || y0 == y1) {
		Video::fillRect(dest, x0, y0, x1, y1, color);
		return;
	}

	_videoDriver->drawLine(dest, x0, y0, x1, y1, color);
}

void Video::putPixel(int16 x, int16 y, int16 color, SurfaceDesc *dest) {
	if (x < 0 || y < 0 || x >= dest->width || y >= dest->height)
		return;

	_videoDriver->putPixel(x, y, color, dest);
}

void Video::clearSurf(SurfaceDesc *dest) {
	Video::fillRect(dest, 0, 0, dest->width - 1, dest->height - 1, 0);
}

void Video::drawCircle(Video::SurfaceDesc *dest, int16 x, int16 y, int16 radius, int16 color) {
	int16 si;
	int16 var_18;
	int16 var_16;
	int16 y4;
	int16 y3;
	int16 x4;
	int16 x3;
	int16 x2;
	int16 y2;
	int16 x1;
	int16 y1;
	int16 var_4;
	int16 var_2;
	
	var_2 = radius;
	var_4 = 0;
	si = -radius;
	y1 = y;
	x1 = x + radius;
	y2 = y + radius;
	x2 = x;
	x3 = x - radius;
	x4 = x;
	y3 = y;
	y4 = y - radius;
	var_16 = 0;
	var_18 = radius * 2;

	while (var_2 >= var_4) {
		putPixel(x1, y1, color, dest);
		putPixel(x2, y2, color, dest);
		putPixel(x3, y1, color, dest);
		putPixel(x4, y2, color, dest);
		putPixel(x1, y3, color, dest);
		putPixel(x2, y4, color, dest);
		putPixel(x3, y3, color, dest);
		putPixel(x4, y4, color, dest);
		y1++;
		x2++;
		x4--;
		y3--;
		var_16 += 2;
		var_4++;
		si += var_16 + 1;
		if (si > 0) {
			x1--;
			y2--;
			x3++;
			y4++;
			var_18 -= 2;
			var_2--;
			si -= var_18 + 1;
		}
	}
}

void Video::drawPackedSprite(byte *sprBuf, int16 width, int16 height, int16 x, int16 y,
	    int16 transp, SurfaceDesc *dest) {

	if (spriteUncompressor(sprBuf, width, height, x, y, transp, dest))
		return;

	if ((dest->vidMode & 0x7f) != 0x13)
		error("Video::drawPackedSprite: Video mode 0x%x is not fully supported!",
		    dest->vidMode & 0x7f);

	_videoDriver->drawPackedSprite(sprBuf, width, height, x, y, transp, dest);
}

void Video::setPalElem(int16 index, char red, char green, char blue, int16 unused,
	    int16 vidMode) {
	byte pal[4];

	_vm->_global->_redPalette[index] = red;
	_vm->_global->_greenPalette[index] = green;
	_vm->_global->_bluePalette[index] = blue;

	if (vidMode != 0x13)
		error("Video::setPalElem: Video mode 0x%x is not supported!",
		    vidMode);

	pal[0] = (red << 2) | (red >> 4);
	pal[1] = (green << 2) | (green >> 4);
	pal[2] = (blue << 2) | (blue >> 4);
	pal[3] = 0;
	g_system->setPalette(pal, index, 1);
}

void Video::setPalette(PalDesc *palDesc) {
	int16 i;
	byte pal[1024];
	int16 numcolors;

	if (_vm->_global->_videoMode != 0x13)
		error("Video::setPalette: Video mode 0x%x is not supported!",
		    _vm->_global->_videoMode);

	if (_vm->_global->_setAllPalette)
		numcolors = 256;
	else
		numcolors = 16;

	for (i = 0; i < numcolors; i++) {
		pal[i * 4 + 0] = (palDesc->vgaPal[i].red << 2) | (palDesc->vgaPal[i].red >> 4);
		pal[i * 4 + 1] = (palDesc->vgaPal[i].green << 2) | (palDesc->vgaPal[i].green >> 4);
		pal[i * 4 + 2] = (palDesc->vgaPal[i].blue << 2) | (palDesc->vgaPal[i].blue >> 4);
		pal[i * 4 + 3] = 0;
	}

	g_system->setPalette(pal, 0, numcolors);
}

void Video::setFullPalette(PalDesc *palDesc) {
	Color *colors;
	int16 i;
	byte pal[1024];

	if (_vm->_global->_setAllPalette) {
		colors = palDesc->vgaPal;
		for (i = 0; i < 256; i++) {
			_vm->_global->_redPalette[i] = colors[i].red;
			_vm->_global->_greenPalette[i] = colors[i].green;
			_vm->_global->_bluePalette[i] = colors[i].blue;
		}

		for (i = 0; i < 256; i++) {
			pal[i * 4 + 0] = (colors[i].red << 2) | (colors[i].red >> 4);
			pal[i * 4 + 1] = (colors[i].green << 2) | (colors[i].green >> 4);
			pal[i * 4 + 2] = (colors[i].blue << 2) | (colors[i].blue >> 4);
			pal[i * 4 + 3] = 0;
		}
		g_system->setPalette(pal, 0, 256);
	} else {
		Video::setPalette(palDesc);
	}
}

void Video::initPrimary(int16 mode) {
	int16 old;
	if (mode != 0x13 && mode != 3 && mode != -1)
		error("Video::initPrimary: Video mode 0x%x is not supported!",
		    mode);

	if (_vm->_global->_videoMode != 0x13)
		error("Video::initPrimary: Video mode 0x%x is not supported!",
		    mode);

	old = _vm->_global->_oldMode;
	if (mode == -1)
		mode = 3;

	_vm->_global->_oldMode = mode;
	if (mode != 3)
		Video::initDriver(mode);

	if (mode != 3) {
		initSurfDesc(mode, 320, 200, PRIMARY_SURFACE);

		if (_vm->_global->_dontSetPalette)
			return;

		Video::setFullPalette(_vm->_global->_pPaletteDesc);
	}
}

void Video::setHandlers() { _vm->_global->_setAllPalette = 1; }

}				// End of namespace Gob
