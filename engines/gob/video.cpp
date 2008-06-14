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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/endian.h"
#include "graphics/cursorman.h"

#include "gob/gob.h"
#include "gob/video.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/dataio.h"
#include "gob/draw.h"

#include "gob/driver_vga.h"

namespace Gob {

SurfaceDesc::SurfaceDesc(int16 vidMode, int16 width, int16 height,
		byte *vidMem) : _width(width), _height(height) {

	if (vidMem) {
		_vidMode = vidMode;
		_ownVidMem = false;
		_vidMem = vidMem;
	} else {
		_vidMode = vidMode;
		_ownVidMem = true;
		_vidMem = new byte[width * height];
		assert(_vidMem);
		memset(_vidMem, 0, width * height);
	}
}

void SurfaceDesc::setVidMem(byte *vidMem) {
	assert(vidMem);

	if (hasOwnVidMem())
		delete[] _vidMem;

	_ownVidMem = false;
	_vidMem = vidMem;
}

void SurfaceDesc::resize(int16 width, int16 height) {
	if (hasOwnVidMem())
		delete[] _vidMem;

	_width = width;
	_height = height;
	_ownVidMem = true;
	_vidMem = new byte[width * height];
	assert(_vidMem);
	memset(_vidMem, 0, width * height);
}

void SurfaceDesc::swap(SurfaceDesc &surf) {
	SWAP(_width, surf._width);
	SWAP(_height, surf._height);
	SWAP(_vidMode, surf._vidMode);
	SWAP(_ownVidMem, surf._ownVidMem);
	SWAP(_vidMem, surf._vidMem);
}

Video::Video(GobEngine *vm) : _vm(vm) {
	_doRangeClamp = false;
	_videoDriver = 0;

	_surfWidth = 320;
	_surfHeight = 200;

	_scrollOffsetX = 0;
	_scrollOffsetY = 0;

	_splitSurf = 0;
	_splitHeight1 = 200;
	_splitHeight2 = 0;
	_splitStart = 0;

	_screenDeltaX = 0;
	_screenDeltaY = 0;

	_curSparse = 0;
	_lastSparse = 0xFFFFFFFF;
}

char Video::initDriver(int16 vidMode) {
	if (_videoDriver)
		return 1;

	_videoDriver = new VGAVideoDriver();
	return 1;
}

void Video::freeDriver() {
	delete _videoDriver;
}

void Video::initPrimary(int16 mode) {
	if ((mode != 3) && (mode != -1))
		_vm->validateVideoMode(mode);
	_vm->validateVideoMode(_vm->_global->_videoMode);

	if (mode == -1)
		mode = 3;
	_vm->_global->_oldMode = mode;

	if (mode != 3)
		Video::initDriver(mode);

	if (mode != 3) {
		initSurfDesc(mode, _surfWidth, _surfHeight, PRIMARY_SURFACE);

		if (!_vm->_global->_dontSetPalette)
			Video::setFullPalette(_vm->_global->_pPaletteDesc);
	}
}

SurfaceDesc *Video::initSurfDesc(int16 vidMode, int16 width, int16 height,
		int16 flags) {
	SurfaceDesc *descPtr = 0;

	if (flags & PRIMARY_SURFACE)
		assert((width == _surfWidth) && (height == _surfHeight));

	_vm->validateVideoMode(vidMode);

	if (flags & PRIMARY_SURFACE) {
		_vm->_global->_primaryWidth = width;
		_vm->_global->_primaryHeight = height;

		descPtr = _vm->_global->_primarySurfDesc;
		descPtr->resize(width, height);
		descPtr->_vidMode = vidMode;
	} else {
		assert(!(flags & DISABLE_SPR_ALLOC));

		if (!(flags & SCUMMVM_CURSOR))
			width = (width + 7) & 0xFFF8;

		descPtr = new SurfaceDesc(vidMode, width, height);
	}
	return descPtr;
}

void Video::clearScreen() {
	g_system->clearScreen();
}

void Video::setSize(bool defaultTo1XScaler) {
	_vm->_system->beginGFXTransaction();
		_vm->_system->initSize(_vm->_width, _vm->_height);
		_vm->initCommonGFX(defaultTo1XScaler);
	_vm->_system->endGFXTransaction();
}

void Video::retrace(bool mouse) {
	if (mouse)
		CursorMan.showMouse((_vm->_draw->_showCursor & 2) != 0);
	if (_vm->_global->_primarySurfDesc) {
		int screenOffset = _scrollOffsetY * _surfWidth + _scrollOffsetX;
		int screenX = _screenDeltaX;
		int screenY = _screenDeltaY;
		int screenWidth = MIN<int>(_surfWidth - _scrollOffsetX, _vm->_width);
		int screenHeight = MIN<int>(_surfHeight - _splitHeight2 - _scrollOffsetY, _vm->_height);

		g_system->copyRectToScreen(_vm->_global->_primarySurfDesc->getVidMem() + screenOffset,
				_surfWidth, screenX, screenY, screenWidth, screenHeight);

		if (_splitSurf) {

			screenOffset = 0;
			screenX = 0;
			screenY = _vm->_height - _splitSurf->getHeight();
			screenWidth = MIN<int>(_vm->_width, _splitSurf->getWidth());
			screenHeight = _splitSurf->getHeight();

			g_system->copyRectToScreen(_splitSurf->getVidMem() + screenOffset,
					_splitSurf->getWidth(), screenX, screenY, screenWidth, screenHeight);

		} else if (_splitHeight2 > 0) {

			screenOffset = _splitStart * _surfWidth;
			screenX = 0;
			screenY = _vm->_height - _splitHeight2;
			screenWidth = MIN<int>(_surfWidth, _vm->_width);
			screenHeight = _splitHeight2;

			g_system->copyRectToScreen(_vm->_global->_primarySurfDesc->getVidMem() + screenOffset,
					_surfWidth, screenX, screenY, screenWidth, screenHeight);

		}

		g_system->updateScreen();
	}
}

void Video::waitRetrace(bool mouse) {
	uint32 time = _vm->_util->getTimeKey();
	retrace(mouse);
	_vm->_util->delay(MAX(1, 10 - (int)(_vm->_util->getTimeKey() - time)));
}

void Video::sparseRetrace(int max) {
	uint32 timeKey = _vm->_util->getTimeKey();

	if ((_curSparse++ > max) || ((timeKey - _lastSparse) > 1000)) {
		_curSparse = 0;
		retrace(false);
	}

	_lastSparse = timeKey;
}

void Video::putPixel(int16 x, int16 y, int16 color, SurfaceDesc *dest) {
	if ((x >= dest->getWidth()) || (x < 0) ||
	    (y >= dest->getHeight()) || (y < 0))
		return;

	_videoDriver->putPixel(x, y, color, dest);
}

void Video::fillRect(SurfaceDesc *dest, int16 left, int16 top, int16 right,
		int16 bottom, int16 color) {

	if (_doRangeClamp) {
		if (left > right)
			SWAP(left, right);
		if (top > bottom)
			SWAP(top, bottom);

		if ((left >= dest->getWidth()) || (right < 0) ||
		    (top >= dest->getHeight()) || (bottom < 0))
			return;

		left = CLIP(left, (int16) 0, (int16) (dest->getWidth() - 1));
		top = CLIP(top, (int16) 0, (int16) (dest->getHeight() - 1));
		right = CLIP(right, (int16) 0, (int16) (dest->getWidth() - 1));
		bottom = CLIP(bottom, (int16) 0, (int16) (dest->getHeight() - 1));
	}

	_videoDriver->fillRect(dest, left, top, right, bottom, color);
}

void Video::drawLine(SurfaceDesc *dest, int16 x0, int16 y0, int16 x1,
		int16 y1, int16 color) {

	if ((x0 == x1) || (y0 == y1))
		Video::fillRect(dest, x0, y0, x1, y1, color);
	else
		_videoDriver->drawLine(dest, x0, y0, x1, y1, color);
}

/*
 * The original's version of the Bresenham Algorithm was a bit "unclean"
 * and produced strange edges at 45, 135, 225 and 315 degrees, so using the
 * version found in the Wikipedia article about the
 * "Bresenham's line algorithm" instead
 */
void Video::drawCircle(SurfaceDesc *dest, int16 x0, int16 y0,
		int16 radius, int16 color) {
	int16 f = 1 - radius;
	int16 ddFx = 0;
	int16 ddFy = -2 * radius;
	int16 x = 0;
	int16 y = radius;

	putPixel(x0, y0 + radius, color, dest);
	putPixel(x0, y0 - radius, color, dest);
	putPixel(x0 + radius, y0, color, dest);
	putPixel(x0 - radius, y0, color, dest);

	while (x < y) {
		if (f >= 0) {
			y--;
			ddFy += 2;
			f += ddFy;
		}
		x++;
		ddFx += 2;
		f += ddFx + 1;
		putPixel(x0 + x, y0 + y, color, dest);
		putPixel(x0 - x, y0 + y, color, dest);
		putPixel(x0 + x, y0 - y, color, dest);
		putPixel(x0 - x, y0 - y, color, dest);
		putPixel(x0 + y, y0 + x, color, dest);
		putPixel(x0 - y, y0 + x, color, dest);
		putPixel(x0 + y, y0 - x, color, dest);
		putPixel(x0 - y, y0 - x, color, dest);
	}
}

void Video::clearSurf(SurfaceDesc *dest) {
	Video::fillRect(dest, 0, 0, dest->getWidth() - 1, dest->getHeight() - 1, 0);
}

void Video::drawSprite(SurfaceDesc *source, SurfaceDesc *dest,
	    int16 left, int16 top, int16 right, int16 bottom, int16 x, int16 y, int16 transp) {
	int16 destRight;
	int16 destBottom;

	if (_doRangeClamp) {
		if (left > right)
			SWAP(left, right);
		if (top > bottom)
			SWAP(top, bottom);

		if ((left >= source->getWidth()) || (right < 0) ||
		    (top >= source->getHeight()) || (bottom < 0))
			return;

		if (left < 0) {
			x -= left;
			left = 0;
		}
		if (top < 0) {
			y -= top;
			top = 0;
		}
		right = CLIP(right, (int16) 0, (int16) (source->getWidth() - 1));
		bottom = CLIP(bottom, (int16) 0, (int16) (source->getHeight() - 1));
		if (right - left >= source->getWidth())
			right = left + source->getWidth() - 1;
		if (bottom - top >= source->getHeight())
			bottom = top + source->getHeight() - 1;

		if (x < 0) {
			left -= x;
			x = 0;
		}
		if (y < 0) {
			top -= y;
			y = 0;
		}
		if ((x >= dest->getWidth()) || (left > right) ||
		    (y >= dest->getHeight()) || (top > bottom))
			return;

		destRight = x + right - left;
		destBottom = y + bottom - top;
		if (destRight >= dest->getWidth())
			right -= destRight - dest->getWidth() + 1;

		if (destBottom >= dest->getHeight())
			bottom -= destBottom - dest->getHeight() + 1;
	}

	_videoDriver->drawSprite(source, dest, left, top, right, bottom, x, y, transp);
}

void Video::drawLetter(int16 item, int16 x, int16 y, FontDesc *fontDesc,
		int16 color1, int16 color2, int16 transp, SurfaceDesc *dest) {
	byte *dataPtr;
	byte *itemData;
	int16 itemSize;
	int16 newItem;
	int16 curItem;
	int16 newItemPos;
	int16 curItemPos;

	if (fontDesc->endItem == 0) {
		itemSize = fontDesc->itemSize + 3;
		dataPtr = fontDesc->dataPtr;
		//        startItem
		curItem = READ_LE_UINT16(dataPtr - 2) - 1;

		curItemPos = 0;
		do {
			newItemPos = ((curItemPos + curItem) / 2) * itemSize;
			itemData = fontDesc->dataPtr + newItemPos;
			newItem = (READ_LE_UINT16(itemData) & 0x7FFF);
			if (item > newItem)
				curItem = newItemPos - 1;
			else
				curItemPos = newItemPos + 1;
		} while ((newItem != item) && (curItemPos <= curItem));

		if (newItem != item)
			return;

		fontDesc->dataPtr = itemData + 3;
		item = 0;
	}

	_videoDriver->drawLetter((unsigned char) item, x, y, fontDesc, color1, color2, transp, dest);
}

void Video::drawPackedSprite(byte *sprBuf, int16 width, int16 height,
		int16 x, int16 y, int16 transp, SurfaceDesc *dest) {

	if (spriteUncompressor(sprBuf, width, height, x, y, transp, dest))
		return;

	_vm->validateVideoMode(dest->_vidMode);

	_videoDriver->drawPackedSprite(sprBuf, width, height, x, y, transp, dest);
}

void Video::drawPackedSprite(const char *path, SurfaceDesc *dest, int width) {
	byte *data;

	data = _vm->_dataIO->getData(path);
	drawPackedSprite(data, width, dest->getHeight(), 0, 0, 0, dest);
	delete[] data;
}

void Video::setPalElem(int16 index, char red, char green, char blue,
		int16 unused, int16 vidMode) {
	byte pal[4];

	_vm->validateVideoMode(vidMode);

	_vm->_global->_redPalette[index] = red;
	_vm->_global->_greenPalette[index] = green;
	_vm->_global->_bluePalette[index] = blue;
	setPalColor(pal, red, green, blue);

	g_system->setPalette(pal, index, 1);
}

void Video::setPalette(PalDesc *palDesc) {
	byte pal[1024];
	int16 numcolors;

	_vm->validateVideoMode(_vm->_global->_videoMode);

	numcolors = _vm->_global->_setAllPalette ? 256 : 16;
	for (int i = 0; i < numcolors; i++)
		setPalColor(pal + i * 4, palDesc->vgaPal[i]);

	g_system->setPalette(pal, 0, numcolors);
}

void Video::setFullPalette(PalDesc *palDesc) {
	if (_vm->_global->_setAllPalette) {
		byte pal[1024];
		Color *colors = palDesc->vgaPal;

		for (int i = 0; i < 256; i++) {
			_vm->_global->_redPalette[i] = colors[i].red;
			_vm->_global->_greenPalette[i] = colors[i].green;
			_vm->_global->_bluePalette[i] = colors[i].blue;
			setPalColor(pal + i * 4, colors[i]);
		}

		g_system->setPalette(pal, 0, 256);
	} else
		Video::setPalette(palDesc);
}

void Video::setPalette(Color *palette) {
	Color *palBak;
	bool setAllPalBak;

	palBak = _vm->_global->_pPaletteDesc->vgaPal;
	setAllPalBak = _vm->_global->_setAllPalette;

	_vm->_global->_pPaletteDesc->vgaPal = palette;
	_vm->_global->_setAllPalette = true;
	setFullPalette(_vm->_global->_pPaletteDesc);

	_vm->_global->_setAllPalette = setAllPalBak;
	_vm->_global->_pPaletteDesc->vgaPal = palBak;
}

} // End of namespace Gob
