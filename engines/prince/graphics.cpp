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

#include "prince/graphics.h"

#include "prince/prince.h"

#include "graphics/palette.h"

#include "common/memstream.h"

namespace Prince {

GraphicsMan::GraphicsMan(PrinceEngine *vm) 
	: _vm(vm), _changed(false) {
	initGraphics(640, 480, true);
	_frontScreen = new Graphics::Surface();
	_frontScreen->create(640, 480, Graphics::PixelFormat::createFormatCLUT8());
	_shadowTable70 = new byte[256];
	_shadowTable50 = new byte[256];
}

GraphicsMan::~GraphicsMan() {
	_frontScreen->free();
	delete _frontScreen;
	delete[] _shadowTable70;
	delete[] _shadowTable50;
}

void GraphicsMan::update() {
	if (_changed) {
		_vm->_system->copyRectToScreen((byte*)_frontScreen->getBasePtr(0,0), 640, 0, 0, 640, 480);

		_vm->_system->updateScreen();
		_changed = false;
	}
}

void GraphicsMan::setPalette(const byte *palette) {
	_vm->_system->getPaletteManager()->setPalette(palette, 0, 256);
}

void GraphicsMan::change() {
	_changed = true;
}

void GraphicsMan::draw(uint16 posX, uint16 posY, const Graphics::Surface *s) {
	uint16 w = MIN(_frontScreen->w, s->w);
	for (uint y = 0; y < s->h; y++) {
		if (y < _frontScreen->h) {
		   memcpy((byte*)_frontScreen->getBasePtr(0, y), (byte*)s->getBasePtr(0, y), w);
		}
	}
	change();
}

void GraphicsMan::drawTransparent(int32 posX, int32 posY, const Graphics::Surface *s) {
	for (int y = 0; y < s->h; y++) {
		for (int x = 0; x < s->w; x++) {
			byte pixel = *((byte*)s->getBasePtr(x, y));
			if (pixel != 255) {
				if (x + posX < _frontScreen->w && x + posX >= 0) {
					if (y + posY < _frontScreen->h && y + posY >= 0) {
						*((byte*)_frontScreen->getBasePtr(x + posX, y + posY)) = pixel;
					}
				}
			}
		}
	}
	change();
}

void GraphicsMan::drawMask(int32 posX, int32 posY, int32 width, int32 height, byte *maskData, const Graphics::Surface *originalRoomSurface) {
	int maskWidth = width >> 3;
	int maskPostion = 0;
	int maskCounter = 128;
	for (int y = 0; y < height; y++) {
		int tempMaskPostion = maskPostion;
		for (int x = 0; x < width; x++) {
			if (x + posX < _frontScreen->w && x + posX >= 0) {
				if (y + posY < _frontScreen->h && y + posY >= 0) {
					if ((maskData[tempMaskPostion] & maskCounter) != 0) {
						byte orgPixel = *((byte*)originalRoomSurface->getBasePtr(x + posX, y + posY));
						*((byte*)_frontScreen->getBasePtr(x + posX, y + posY)) = orgPixel;
					}
				}
			}
			maskCounter >>= 1;
			if (maskCounter == 0) {
				maskCounter = 128;
				tempMaskPostion++;
			}
		}
		maskPostion += maskWidth;
		maskCounter = 128;
	}
	change();
}

void GraphicsMan::drawAsShadow(int32 posX, int32 posY, const Graphics::Surface *s, byte *shadowTable) {
	for (int y = 0; y < s->h; y++) {
		for (int x = 0; x < s->w; x++) {
			byte pixel = *((byte*)s->getBasePtr(x, y));
			if (pixel == kShadowColor) {
				if (x + posX < _frontScreen->w && x + posX >= 0) {
					if (y + posY < _frontScreen->h && y + posY >= 0) {
						byte *background = (byte *)_frontScreen->getBasePtr(x + posX, y + posY);
						*background = *(shadowTable + *background);
					}
				}
			}
		}
	}
	change();
}

void GraphicsMan::makeShadowTable(int brightness, byte *shadowPalette) {
	int32 redFirstOrg, greenFirstOrg, blueFirstOrg;
	int32 redSecondOrg, greenSecondOrg, blueSecondOrg;
	int32 redNew, greenNew, blueNew;

	int32 sumOfColorValues;
	int32 bigValue;
	int32 currColor;

	int shadow =  brightness * 256 / 100;

	const byte *originalPalette = _vm->_roomBmp->getPalette();

	for (int i = 0; i < 256; i++) {
		redFirstOrg = originalPalette[3 * i] * shadow / 256;
		greenFirstOrg = originalPalette[3 * i + 1] * shadow / 256;
		blueFirstOrg = originalPalette[3 * i + 2] * shadow / 256;

		currColor = 0;
		bigValue = 999999999; // infinity

		for (int j = 0; j < 256; j++) {
			redSecondOrg = originalPalette[3 * j];
			redNew = redFirstOrg - redSecondOrg;
			redNew = redNew * redNew;

			greenSecondOrg = originalPalette[3 * j + 1];
			greenNew = greenFirstOrg - greenSecondOrg;
			greenNew = greenNew * greenNew;

			blueSecondOrg = originalPalette[3 * j + 2];
			blueNew = blueFirstOrg - blueSecondOrg;
			blueNew = blueNew * blueNew;

			sumOfColorValues = redNew + greenNew + blueNew;

			if (sumOfColorValues < bigValue) {
				bigValue = sumOfColorValues;
				currColor = j;
			}

			if (sumOfColorValues == 0) {
				break;
			}
		}
		shadowPalette[i] = currColor;
	}
}

}

/* vim: set tabstop=4 noexpandtab: */
