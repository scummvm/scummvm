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
 */

#include "graphics/palette.h"
#include "neverhood/screen.h"

namespace Neverhood {

Screen::Screen(NeverhoodEngine *vm)
	: _vm(vm), _paletteData(NULL), _paletteChanged(false) {
	
	_ticks = _vm->_system->getMillis();
	
	_backScreen = new Graphics::Surface();
	_backScreen->create(640, 480, Graphics::PixelFormat::createFormatCLUT8());
	
}

Screen::~Screen() {
	delete _backScreen;
}

void Screen::update() {
	updatePalette();
	// TODO: Implement actual code
	_vm->_system->copyRectToScreen((const byte*)_backScreen->pixels, _backScreen->pitch, 0, 0, 640, 480);
	_vm->_system->updateScreen();
}

void Screen::wait() {
	// TODO
	_vm->_system->delayMillis(40);
}

void Screen::setFps(int fps) {
	_frameDelay = 1000 / fps;
}

int Screen::getFps() {
	return 1000 / _frameDelay;
}

void Screen::setPaletteData(byte *paletteData) {
	_paletteChanged = true;
	_paletteData = paletteData;
}

void Screen::unsetPaletteData(byte *paletteData) {
	if (_paletteData == paletteData) {
		_paletteChanged = false;
		_paletteData = NULL;
	}
}

void Screen::testPalette(byte *paletteData) {
	if (_paletteData == paletteData)
		_paletteChanged = true;
}

void Screen::updatePalette() {
	if (_paletteChanged && _paletteData) {
		byte *tempPalette = new byte[768];
		for (int i = 0; i < 256; i++) {
			tempPalette[i * 3 + 0] = _paletteData[i * 4 + 0];
			tempPalette[i * 3 + 1] = _paletteData[i * 4 + 1];
			tempPalette[i * 3 + 2] = _paletteData[i * 4 + 2];
		}
		_vm->_system->getPaletteManager()->setPalette(tempPalette, 0, 256);
		delete[] tempPalette;
		_paletteChanged = false;
	}
}

void Screen::clear() {
	memset(_backScreen->pixels, 0, _backScreen->pitch * _backScreen->h);
}

void Screen::drawSurface2(const Graphics::Surface *surface, NDrawRect &drawRect, NRect &clipRect, bool transparent) {

	int16 destX, destY;
	NRect ddRect;
	
	if (drawRect.x + drawRect.width >= clipRect.x2)
		ddRect.x2 = clipRect.x2 - drawRect.x;
	else
		ddRect.x2 = drawRect.width;
		
	if (drawRect.x <= clipRect.x1) {
		destX = clipRect.x1;
		ddRect.x1 = clipRect.x1 - drawRect.x;
	} else {
		destX = drawRect.x;
		ddRect.x1 = 0;
	}
	
	if (drawRect.y + drawRect.height >= clipRect.y2)
		ddRect.y2 = clipRect.y2 - drawRect.y;
	else
		ddRect.y2 = drawRect.height;
	
	if (drawRect.y <= clipRect.y1) {
		destY = clipRect.y1;
		ddRect.y1 = clipRect.y1 - drawRect.y;
	} else {
		destY = drawRect.y;
		ddRect.y1 = 0;
	}
	
	debug(8, "draw: x = %d; y = %d; (%d, %d, %d, %d)", destX, destY, ddRect.x1, ddRect.y1, ddRect.x2, ddRect.y2);
	
	const byte *source = (const byte*)surface->getBasePtr(ddRect.x1, ddRect.y1);
	byte *dest = (byte*)_backScreen->getBasePtr(destX, destY);
	int width = ddRect.x2 - ddRect.x1;
	int height = ddRect.y2 - ddRect.y1;

	if (!transparent) {
		while (height--) {
			memcpy(dest, source, width);
			source += surface->pitch;
			dest += _backScreen->pitch;
		}
	} else {
		while (height--) {
			for (int xc = 0; xc < width; xc++)
				if (source[xc] != 0)
					dest[xc] = source[xc];
			source += surface->pitch;
			dest += _backScreen->pitch;
		}
	} 
	
}

void Screen::drawDoubleSurface2(const Graphics::Surface *surface, NDrawRect &drawRect) {

	const byte *source = (const byte*)surface->getBasePtr(0, 0);
	byte *dest = (byte*)_backScreen->getBasePtr(drawRect.x, drawRect.y);
	
	for (int16 yc = 0; yc < surface->h; yc++) {
		byte *row = dest;
		for (int16 xc = 0; xc < surface->w; xc++) {
			*row++ = *source;
			*row++ = *source++;
		}
		memcpy(dest + _backScreen->pitch, dest, surface->w * 2);
		dest += _backScreen->pitch;
		dest += _backScreen->pitch;
	}

}

} // End of namespace Neverhood
