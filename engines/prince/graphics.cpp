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
	_mapScreen = new Graphics::Surface();
	_mapScreen->create(640, 480, Graphics::PixelFormat::createFormatCLUT8());
	_shadowTable70 = new byte[256];
	_shadowTable50 = new byte[256];
}

GraphicsMan::~GraphicsMan() {
	_frontScreen->free();
	delete _frontScreen;
	_screenForInventory->free();
	delete _screenForInventory;
	_mapScreen->free();
	delete _mapScreen;
	delete[] _shadowTable70;
	delete[] _shadowTable50;
}

void GraphicsMan::update(Graphics::Surface *screen) {
	if (_changed) {
		_vm->_system->copyRectToScreen((byte *)screen->getBasePtr(0, 0), 640, 0, 0, 640, 480);

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

void GraphicsMan::draw(Graphics::Surface *screen, const Graphics::Surface *s) {
	uint16 w = MIN(screen->w, s->w);
	byte *src = (byte *)s->getBasePtr(0, 0);
	byte *dst = (byte *)screen->getBasePtr(0, 0);
	for (uint y = 0; y < s->h; y++) {
		if (y < screen->h) {
			memcpy(dst, src, w);
		}
		src += s->pitch;
		dst += screen->pitch;
	}
	change();
}

void GraphicsMan::drawTransparentSurface(Graphics::Surface *screen, int32 posX, int32 posY, const Graphics::Surface *s, int transColor) {
	byte *src1 = (byte *)s->getBasePtr(0, 0);
	byte *dst1 = (byte *)screen->getBasePtr(posX, posY);
	for (int y = 0; y < s->h; y++) {
		byte *src2 = src1;
		byte *dst2 = dst1;
		for (int x = 0; x < s->w; x++, src2++, dst2++) {
			if (*src2 != transColor) {
				if (x + posX < screen->w && x + posX >= 0) {
					if (y + posY < screen->h && y + posY >= 0) {
						*dst2 = *src2;
					}
				}
			}
		}
		src1 += s->pitch;
		dst1 += screen->pitch;
	}
	change();
}

void GraphicsMan::drawAsShadowSurface(Graphics::Surface *screen, int32 posX, int32 posY, const Graphics::Surface *s, byte *shadowTable) {
	byte *src1 = (byte *)s->getBasePtr(0, 0);
	byte *dst1 = (byte *)screen->getBasePtr(posX, posY);

	for (int y = 0; y < s->h; y++) {
		byte *src2 = src1;
		byte *dst2 = dst1;
		for (int x = 0; x < s->w; x++, src2++, dst2++) {
			if (*src2 == kShadowColor) {
				if (x + posX < screen->w && x + posX >= 0) {
					if (y + posY < screen->h && y + posY >= 0) {
						*dst2 = *(shadowTable + *dst2);
					}
				}
			}
		}
		src1 += s->pitch;
		dst1 += screen->pitch;
	}
}

void GraphicsMan::drawTransparentWithBlendSurface(Graphics::Surface *screen, int32 posX, int32 posY, const Graphics::Surface *s, int transColor) {
	byte *src1 = (byte *)s->getBasePtr(0, 0);
	byte *dst1 = (byte *)screen->getBasePtr(posX, posY);
	byte *blendTable = (byte *)malloc(256);
	for (int i = 0; i < 256; i++) {
		blendTable[i] = 255;
	}
	for (int y = 0; y < s->h; y++) {
		byte *src2 = src1;
		byte *dst2 = dst1;
		for (int x = 0; x < s->w; x++, src2++, dst2++) {
			if (*src2 != transColor) {
				if (x + posX < screen->w && x + posX >= 0) {
					if (y + posY < screen->h && y + posY >= 0) {
						*dst2 = getBlendTableColor(*src2, *dst2, blendTable);
					}
				}
			}
		}
		src1 += s->pitch;
		dst1 += screen->pitch;
	}
	free(blendTable);
	change();
}

void GraphicsMan::drawTransparentDrawNode(Graphics::Surface *screen, DrawNode *drawNode) {
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

void GraphicsMan::drawMaskDrawNode(Graphics::Surface *screen, DrawNode *drawNode) {
	byte *src1 = (byte *)drawNode->originalRoomSurface->getBasePtr(drawNode->posX, drawNode->posY);
	byte *dst1 = (byte *)screen->getBasePtr(drawNode->posX, drawNode->posY);
	int maskWidth = drawNode->width >> 3;
	int maskPostion = 0;
	int maskCounter = 128;

	for (int y = 0; y < drawNode->height; y++) {
		byte *src2 = src1;
		byte *dst2 = dst1;
		int tempMaskPostion = maskPostion;
		for (int x = 0; x < drawNode->width; x++, src2++, dst2++) {
			if (x + drawNode->posX < screen->w && x + drawNode->posX >= 0) {
				if (y + drawNode->posY < screen->h && y + drawNode->posY >= 0) {
					if ((drawNode->data[tempMaskPostion] & maskCounter) != 0) {
						*dst2 = *src2;
						//*dst2 = 0; // for debugging
					}
				}
			}
			maskCounter >>= 1;
			if (maskCounter == 0) {
				maskCounter = 128;
				tempMaskPostion++;
			}
		}
		src1 += drawNode->originalRoomSurface->pitch;
		dst1 += screen->pitch;
		maskPostion += maskWidth;
		maskCounter = 128;
	}
}

void GraphicsMan::drawAsShadowDrawNode(Graphics::Surface *screen, DrawNode *drawNode) {
	byte *src1 = (byte *)drawNode->s->getBasePtr(0, 0);
	byte *dst1 = (byte *)screen->getBasePtr(drawNode->posX, drawNode->posY);

	for (int y = 0; y < drawNode->s->h; y++) {
		byte *src2 = src1;
		byte *dst2 = dst1;
		for (int x = 0; x < drawNode->s->w; x++, src2++, dst2++) {
			if (*src2 == kShadowColor) {
				if (x + drawNode->posX < screen->w && x + drawNode->posX >= 0) {
					if (y + drawNode->posY < screen->h && y + drawNode->posY >= 0) {
						*dst2 = *(drawNode->data + *dst2);
					}
				}
			}
		}
		src1 += drawNode->s->pitch;
		dst1 += screen->pitch;
	}
}

void GraphicsMan::drawBackSpriteDrawNode(Graphics::Surface *screen, DrawNode *drawNode) {
	byte *src1 = (byte *)drawNode->s->getBasePtr(0, 0);
	byte *dst1 = (byte *)screen->getBasePtr(drawNode->posX, drawNode->posY);

	for (int y = 0; y < drawNode->s->h; y++) {
		byte *src2 = src1;
		byte *dst2 = dst1;
		for (int x = 0; x < drawNode->s->w; x++, src2++, dst2++) {
			if (*src2 != 255) {
				 if (x + drawNode->posX < screen->w && x + drawNode->posX >= 0) {
					 if (y + drawNode->posY < screen->h && y + drawNode->posY >= 0) {
						 if (*dst2 == 255) {
							 *dst2 = *src2;
						 }
					 }
				 }
			}
		}
		src1 += drawNode->s->pitch;
		dst1 += screen->pitch;
	}
}

void GraphicsMan::drawPixel(Graphics::Surface *screen, int32 posX, int32 posY) {
	byte *dst = (byte *)screen->getBasePtr(posX, posY);
	*dst = 255;
	change();
}

byte GraphicsMan::getBlendTableColor(byte pixelColor, byte backgroundPixelColor, byte *blendTable) {
	int32 redFirstOrg, greenFirstOrg, blueFirstOrg;
	int32 redFirstBack, greenFirstBack, blueFirstBack;
	int32 redSecondOrg, greenSecondOrg, blueSecondOrg;
	int32 redNew, greenNew, blueNew;

	int32 sumOfColorValues;
	int32 bigValue;
	int32 currColor;

	if (blendTable[pixelColor] != 255) {
		currColor = blendTable[pixelColor];
	} else {
		const byte *originalPalette = _vm->_roomBmp->getPalette();

		redFirstOrg = originalPalette[pixelColor * 3] * _vm->_mst_shadow / 256;
		CLIP(redFirstOrg, 0, 255);
		if (_vm->_mst_shadow <= 256) {
			redFirstBack = originalPalette[backgroundPixelColor * 3] * (256 - _vm->_mst_shadow) / 256;
			CLIP(redFirstBack, 0, 255);
			redFirstOrg += redFirstBack;
			CLIP(redFirstOrg, 0, 255);
		}

		greenFirstOrg = originalPalette[pixelColor * 3 + 1] * _vm->_mst_shadow / 256;
		CLIP(greenFirstOrg, 0, 255);
		if (_vm->_mst_shadow <= 256) {
			greenFirstBack = originalPalette[backgroundPixelColor * 3 + 1] * (256 - _vm->_mst_shadow) / 256;
			CLIP(greenFirstBack, 0, 255);
			greenFirstOrg += greenFirstBack;
			CLIP(greenFirstOrg, 0, 255);
		}

		blueFirstOrg = originalPalette[pixelColor * 3 + 2] * _vm->_mst_shadow / 256;
		CLIP(blueFirstOrg, 0, 255);
		if (_vm->_mst_shadow <= 256) {
			blueFirstBack = originalPalette[backgroundPixelColor * 3 + 2] * (256 - _vm->_mst_shadow) / 256;
			CLIP(blueFirstBack, 0, 255);
			blueFirstOrg += blueFirstBack;
			CLIP(blueFirstOrg, 0, 255);
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
		blendTable[pixelColor] = currColor;
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
