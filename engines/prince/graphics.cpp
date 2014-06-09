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

#include "prince/mhwanh.h"

#include "graphics/palette.h"

#include "common/memstream.h"

namespace Prince {

GraphicsMan::GraphicsMan(PrinceEngine *vm) 
	: _vm(vm), _changed(false) {
	initGraphics(640, 480, true);
	_frontScreen = new Graphics::Surface();
	_frontScreen->create(640, 480, Graphics::PixelFormat::createFormatCLUT8());
	_screenForInventory = new Graphics::Surface();
	_screenForInventory->create(640, 480, Graphics::PixelFormat::createFormatCLUT8());
	_shadowTable70 = new byte[256];
	_shadowTable50 = new byte[256];
}

GraphicsMan::~GraphicsMan() {
	_frontScreen->free();
	delete _frontScreen;
	delete[] _shadowTable70;
	delete[] _shadowTable50;
}

void GraphicsMan::update(Graphics::Surface *screen) {
	if (_changed) {
		_vm->_system->copyRectToScreen((byte*)screen->getBasePtr(0,0), 640, 0, 0, 640, 480);

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

void GraphicsMan::draw(Graphics::Surface *screen, uint16 posX, uint16 posY, const Graphics::Surface *s) {
	uint16 w = MIN(screen->w, s->w);
	for (uint y = 0; y < s->h; y++) {
		if (y < screen->h) {
		   memcpy((byte*)screen->getBasePtr(0, y), (byte*)s->getBasePtr(0, y), w);
		}
	}
	change();
}

void GraphicsMan::drawTransparentSurface(Graphics::Surface *screen, int32 posX, int32 posY, const Graphics::Surface *s, int transColor) {
	for (int y = 0; y < s->h; y++) {
		for (int x = 0; x < s->w; x++) {
			byte pixel = *((byte*)s->getBasePtr(x, y));
			if (pixel != transColor) {
				if (x + posX < screen->w && x + posX >= 0) {
					if (y + posY < screen->h && y + posY >= 0) {
						*((byte*)screen->getBasePtr(x + posX, y + posY)) = pixel;
					}
				}
			}
		}
	}
	change();
}

void GraphicsMan::drawTransparentWithBlend(Graphics::Surface *screen, int32 posX, int32 posY, const Graphics::Surface *s, int transColor) {
	_blendTable = new byte[256];
	for (int i = 0; i < 256; i++) {
		_blendTable[i] = 255;
	}
	for (int y = 0; y < s->h; y++) {
		for (int x = 0; x < s->w; x++) {
			byte pixel = *((byte*)s->getBasePtr(x, y));
			if (pixel != transColor) {
				if (x + posX < screen->w && x + posX >= 0) {
					if (y + posY < screen->h && y + posY >= 0) {
						byte backgroundPixel = *((byte*)screen->getBasePtr(x + posX, y + posY));
						byte blendPixel = getBlendTableColor(pixel, backgroundPixel);
						*((byte*)screen->getBasePtr(x + posX, y + posY)) = blendPixel;
					}
				}
			}
		}
	}
	delete _blendTable;
	change();
}

void GraphicsMan::drawTransparent(Graphics::Surface *screen, DrawNode *drawNode) {
	byte *src1 = (byte *)drawNode->s->getBasePtr(0, 0);
	byte *dst1 = (byte *)screen->getBasePtr(drawNode->posX, drawNode->posY);

	for (int y = 0; y < drawNode->s->h; y++) {
		byte *src2 = src1;
		byte *dst2 = dst1;
		for (int x = 0; x < drawNode->s->w; x++, src2++, dst2++) {
			if (*src2 != 255) {
				 if (x + drawNode->posX < screen->w && x + drawNode->posX >= 0) {
					 if (y + drawNode->posY < screen->h && y + drawNode->posY >= 0) {
					   *dst2 = *src2;
					 }
				 }
			}
		}
		src1 += drawNode->s->pitch;
		dst1 += screen->pitch;
	}
}

void GraphicsMan::drawMask(Graphics::Surface *screen, DrawNode *drawNode) {
	int maskWidth = drawNode->width >> 3;
	int maskPostion = 0;
	int maskCounter = 128;
	for (int y = 0; y < drawNode->height; y++) {
		int tempMaskPostion = maskPostion;
		for (int x = 0; x < drawNode->width; x++) {
			if (x + drawNode->posX < screen->w && x + drawNode->posX >= 0) {
				if (y + drawNode->posY < screen->h && y + drawNode->posY >= 0) {
					if ((drawNode->data[tempMaskPostion] & maskCounter) != 0) {
						byte orgPixel = *((byte*)drawNode->originalRoomSurface->getBasePtr(x + drawNode->posX, y + drawNode->posY));
						*((byte*)screen->getBasePtr(x + drawNode->posX, y + drawNode->posY)) = orgPixel;
						//*((byte*)screen->getBasePtr(x + drawNode->posX, y + drawNode->posY)) = 0; // for debugging
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
}

void GraphicsMan::drawAsShadow(Graphics::Surface *screen, DrawNode *drawNode) {
	for (int y = 0; y < drawNode->s->h; y++) {
		for (int x = 0; x < drawNode->s->w; x++) {
			byte pixel = *((byte*)drawNode->s->getBasePtr(x, y));
			if (pixel == kShadowColor) {
				if (x + drawNode->posX < screen->w && x + drawNode->posX >= 0) {
					if (y + drawNode->posY < screen->h && y + drawNode->posY >= 0) {
						byte *background = (byte *)screen->getBasePtr(x + drawNode->posX, y + drawNode->posY);
						*background = *(drawNode->data + *background);
					}
				}
			}
		}
	}
}

byte GraphicsMan::getBlendTableColor(byte pixelColor, byte backgroundPixelColor) {
	int32 redFirstOrg, greenFirstOrg, blueFirstOrg;
	int32 redFirstBack, greenFirstBack, blueFirstBack;
	int32 redSecondOrg, greenSecondOrg, blueSecondOrg;
	int32 redNew, greenNew, blueNew;

	int32 sumOfColorValues;
	int32 bigValue;
	int32 currColor;

	if (_blendTable[pixelColor] != 255) {
		currColor = _blendTable[pixelColor];
	} else {
		const byte *originalPalette = _vm->_roomBmp->getPalette();

		redFirstOrg = originalPalette[pixelColor * 3] * _vm->_mst_shadow / 256;
		if (redFirstOrg >= 256) {
			redFirstOrg = 255;
		}
		if (_vm->_mst_shadow <= 256) {
			redFirstBack = originalPalette[backgroundPixelColor * 3] * (256 - _vm->_mst_shadow) / 256;
			if (redFirstBack >= 256) {
				redFirstBack = 255;
			}
			redFirstOrg += redFirstBack;
			if (redFirstOrg >= 256) {
				redFirstOrg = 255;
			}
		}

		greenFirstOrg = originalPalette[pixelColor * 3 + 1] * _vm->_mst_shadow / 256;
		if (greenFirstOrg >= 256) {
			greenFirstOrg = 255;
		}
		if (_vm->_mst_shadow <= 256) {
			greenFirstBack = originalPalette[backgroundPixelColor * 3 + 1] * (256 - _vm->_mst_shadow) / 256;
			if (greenFirstBack >= 256) {
				greenFirstBack = 255;
			}
			greenFirstOrg += greenFirstBack;
			if (greenFirstOrg >= 256) {
				greenFirstOrg = 255;
			}
		}

		blueFirstOrg = originalPalette[pixelColor * 3 + 2] * _vm->_mst_shadow / 256;
		if (blueFirstOrg >= 256) {
			blueFirstOrg = 255;
		}
		if (_vm->_mst_shadow <= 256) {
			blueFirstBack = originalPalette[backgroundPixelColor * 3 + 2] * (256 - _vm->_mst_shadow) / 256;
			if (blueFirstBack >= 256) {
				blueFirstBack = 255;
			}
			blueFirstOrg += blueFirstBack;
			if (blueFirstOrg >= 256) {
				blueFirstOrg = 255;
			}
		}

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
		_blendTable[pixelColor] = currColor;
	}
	return currColor;
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
