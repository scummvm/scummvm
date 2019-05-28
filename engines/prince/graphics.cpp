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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
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

GraphicsMan::GraphicsMan(PrinceEngine *vm) : _vm(vm), _changed(false) {
	initGraphics(640, 480);

	_frontScreen = new Graphics::Surface();
	_frontScreen->create(640, 480, Graphics::PixelFormat::createFormatCLUT8());

	_screenForInventory = new Graphics::Surface();
	_screenForInventory->create(640, 480, Graphics::PixelFormat::createFormatCLUT8());

	_mapScreen = new Graphics::Surface();
	_mapScreen->create(640, 480, Graphics::PixelFormat::createFormatCLUT8());

	_shadowTable70 = (byte *)malloc(256);
	_shadowTable50 = (byte *)malloc(256);

	_roomBackground = 0;
}

GraphicsMan::~GraphicsMan() {
	_frontScreen->free();
	delete _frontScreen;

	_screenForInventory->free();
	delete _screenForInventory;

	_mapScreen->free();
	delete _mapScreen;

	free(_shadowTable70);
	free(_shadowTable50);
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
	const byte *src = (const byte *)s->getBasePtr(0, 0);
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

// Black (value = 0) as a primary transparent color, fix for FLC animations
void GraphicsMan::drawTransparentSurface(Graphics::Surface *screen, int32 posX, int32 posY, const Graphics::Surface *s, int secondTransColor) {
	const byte *src1 = (const byte *)s->getBasePtr(0, 0);
	byte *dst1 = (byte *)screen->getBasePtr(posX, posY);
	for (int y = 0; y < s->h; y++) {
		if (y + posY < screen->h && y + posY >= 0) {
			const byte *src2 = src1;
			byte *dst2 = dst1;
			for (int x = 0; x < s->w; x++, src2++, dst2++) {
				if (*src2 && *src2 != secondTransColor) {
					if (x + posX < screen->w && x + posX >= 0) {
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

/**
 * Similar to drawTransparentSurface but with use of shadowTable for color recalculation
 * and kShadowColor (191) as a transparent color.
 */
void GraphicsMan::drawAsShadowSurface(Graphics::Surface *screen, int32 posX, int32 posY, const Graphics::Surface *s, byte *shadowTable) {
	const byte *src1 = (const byte *)s->getBasePtr(0, 0);
	byte *dst1 = (byte *)screen->getBasePtr(posX, posY);
	for (int y = 0; y < s->h; y++) {
		if (y + posY < screen->h && y + posY >= 0) {
			const byte *src2 = src1;
			byte *dst2 = dst1;
			for (int x = 0; x < s->w; x++, src2++, dst2++) {
				if (*src2 == kShadowColor) {
					if (x + posX < screen->w && x + posX >= 0) {
						*dst2 = *(shadowTable + *dst2);
					}
				}
			}
		}
		src1 += s->pitch;
		dst1 += screen->pitch;
	}
}

/**
 * Used in glowing animation for inventory items. Creates special blendTable array of colors,
 * use black (0) as a transparent color.
 */
void GraphicsMan::drawTransparentWithBlendSurface(Graphics::Surface *screen, int32 posX, int32 posY, const Graphics::Surface *s) {
	const byte *src1 = (const byte *)s->getBasePtr(0, 0);
	byte *dst1 = (byte *)screen->getBasePtr(posX, posY);
	byte *blendTable = (byte *)malloc(256);
	for (int i = 0; i < 256; i++) {
		blendTable[i] = 255;
	}
	for (int y = 0; y < s->h; y++) {
		if (y + posY < screen->h && y + posY >= 0) {
			const byte *src2 = src1;
			byte *dst2 = dst1;
			for (int x = 0; x < s->w; x++, src2++, dst2++) {
				if (*src2) {
					if (x + posX < screen->w && x + posX >= 0) {
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

/**
 * Similar to drawTransparentSurface but with with use of DrawNode as argument for Z axis sorting
 * and white (255) as transparent color.
 */
void GraphicsMan::drawTransparentDrawNode(Graphics::Surface *screen, DrawNode *drawNode) {
	byte *src1 = (byte *)drawNode->s->getBasePtr(0, 0);
	byte *dst1 = (byte *)screen->getBasePtr(drawNode->posX, drawNode->posY);
	for (int y = 0; y < drawNode->s->h; y++) {
		if (y + drawNode->posY < screen->h && y + drawNode->posY >= 0) {
			byte *src2 = src1;
			byte *dst2 = dst1;
			for (int x = 0; x < drawNode->s->w; x++, src2++, dst2++) {
				if (*src2 != 255) {
					if (x + drawNode->posX < screen->w && x + drawNode->posX >= 0) {
						*dst2 = *src2;
					}
				}
			}
		}
		src1 += drawNode->s->pitch;
		dst1 += screen->pitch;
	}
}

/**
 * Similar to drawTransparentDrawNode but with additional anti-aliasing code for sprite drawing.
 * Edge smoothing is based on 256 x 256 table of colors transition.
 * Algorithm is checking if currently drawing pixel is located next to the edge of sprite and if it makes jagged line.
 * If it does then this pixel is set with color from transition table calculated of original background pixel color
 * and sprite's edge pixel color.
 */
void GraphicsMan::drawTransparentWithTransDrawNode(Graphics::Surface *screen, DrawNode *drawNode) {
	// pos of first pixel for each row of source sprite
	byte *src1 = (byte *)drawNode->s->getBasePtr(0, 0);
	// pos of drawing first pixel for each row on screen surface
	byte *dst1 = (byte *)screen->getBasePtr(drawNode->posX, drawNode->posY);
	// trasition table for calculating new color value
	byte *transTableData = (byte *)drawNode->data;
	for (int y = 0; y < drawNode->s->h; y++) {
		if (y + drawNode->posY < screen->h && y + drawNode->posY >= 0) {
			// current pixel in row of source sprite
			byte *src2 = src1;
			// current pixel in row of screen surface
			byte *dst2 = dst1;
			for (int x = 0; x < drawNode->s->w; x++, src2++, dst2++) {
				if (x + drawNode->posX < screen->w && x + drawNode->posX >= 0) {
					if (*src2 != 255) {
						// if source sprite pixel is not mask color than draw it normally
						*dst2 = *src2;
					} else {
						// check for making jagged line
						if (x) {
							// not first pixel in row
							if (*(src2 - 1) == 255) {
								// if it has mask color to the left - check right
								if (x != drawNode->s->w - 1) {
									// not last pixel in row
									if (*(src2 + 1) == 255) {
										// pixel to the right with mask color - no anti-alias
										continue;
									}
									// it's not mask color to the right - we continue checking
								} else {
									// last pixel in row, no right check - no anti-alias
									continue;
								}
							}
							// it's not mask color to the left - we continue checking
						} else if (x != drawNode->s->w - 1) {
							// first pixel in row but not last - just right pixel checking
							if (*(src2 + 1) == 255) {
								// pixel to the right with mask color - no anti-alias
								continue;
							}
							// it's not mask color to the right - we continue checking
						} else {
							// it's first and last pixel in row at the same time (width = 1) - no anti-alias
							continue;
						}
						byte value = 0;
						if (y != drawNode->s->h - 1) {
							// not last row
							// check pixel below of current src2 pixel
							value = *(src2 + drawNode->s->pitch);
							if (value == 255) {
								// pixel below with mask color - check above
								if (y) {
									// not first row
									value = *(src2 - drawNode->s->pitch);
									if (value == 255) {
										// pixel above with mask color - no anti-alias
										continue;
									}
									// it's not mask color above - we draw as transition color
								} else {
									// first row - no anti-alias
									continue;
								}
							}
							// it's not mask color below - we draw as transition color
						} else if (y) {
							// last row - just check above
							value = *(src2 - drawNode->s->pitch);
							if (value == 255) {
								// pixel above with mask color - no anti-alias
								continue;
							}
							// it's not mask color above - we draw as transition color
						} else {
							// first and last row at the same time (height = 1) - no anti-alias
							continue;
						}
						// new color value based on original screen surface color and sprite's edge pixel color
						*dst2 = transTableData[*dst2 * 256 + value];
					}
				}
			}
		}
		// adding pitch to jump to next row of pixels
		src1 += drawNode->s->pitch;
		dst1 += screen->pitch;
	}
}

void GraphicsMan::drawMaskDrawNode(Graphics::Surface *screen, DrawNode *drawNode) {
	byte *maskData = (byte *)drawNode->data;
	byte *src1 = (byte *)drawNode->originalRoomSurface->getBasePtr(drawNode->posX, drawNode->posY);
	byte *dst1 = (byte *)screen->getBasePtr(drawNode->posX, drawNode->posY);
	int maskWidth = drawNode->width >> 3;
	int maskPostion = 0;
	int maskCounter = 128;
	for (int y = 0; y < drawNode->height; y++) {
		if (y + drawNode->posY < screen->h && y + drawNode->posY >= 0) {
			byte *src2 = src1;
			byte *dst2 = dst1;
			int tempMaskPostion = maskPostion;
			for (int x = 0; x < drawNode->width; x++, src2++, dst2++) {
				if (x + drawNode->posX < screen->w && x + drawNode->posX >= 0) {
					if ((maskData[tempMaskPostion] & maskCounter) != 0) {
						*dst2 = *src2;
					}
				}
				maskCounter >>= 1;
				if (maskCounter == 0) {
					maskCounter = 128;
					tempMaskPostion++;
				}
			}
		}
		src1 += drawNode->originalRoomSurface->pitch;
		dst1 += screen->pitch;
		maskPostion += maskWidth;
		maskCounter = 128;
	}
}

void GraphicsMan::drawAsShadowDrawNode(Graphics::Surface *screen, DrawNode *drawNode) {
	byte *shadowData = (byte *)drawNode->data;
	byte *src1 = (byte *)drawNode->s->getBasePtr(0, 0);
	byte *dst1 = (byte *)screen->getBasePtr(drawNode->posX, drawNode->posY);
	for (int y = 0; y < drawNode->s->h; y++) {
		if (y + drawNode->posY < screen->h && y + drawNode->posY >= 0) {
			byte *src2 = src1;
			byte *dst2 = dst1;
			for (int x = 0; x < drawNode->s->w; x++, src2++, dst2++) {
				if (*src2 == kShadowColor) {
					if (x + drawNode->posX < screen->w && x + drawNode->posX >= 0) {
						*dst2 = *(shadowData + *dst2);
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
		if (y + drawNode->posY < screen->h && y + drawNode->posY >= 0) {
			byte *src2 = src1;
			byte *dst2 = dst1;
			for (int x = 0; x < drawNode->s->w; x++, src2++, dst2++) {
				if (*src2 != 255) {
					if (x + drawNode->posX < screen->w && x + drawNode->posX >= 0) {
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

byte GraphicsMan::getBlendTableColor(byte pixelColor, byte backgroundPixelColor, byte *blendTable) {
	int currColor = 0;

	if (blendTable[pixelColor] != 255) {
		currColor = blendTable[pixelColor];
	} else {
		const byte *originalPalette = _vm->_roomBmp->getPalette();

		int redFirstOrg = originalPalette[pixelColor * 3] * _vm->_mst_shadow / 256;
		redFirstOrg = CLIP(redFirstOrg, 0, 255);
		if (_vm->_mst_shadow <= 256) {
			int redFirstBack = originalPalette[backgroundPixelColor * 3] * (256 - _vm->_mst_shadow) / 256;
			redFirstBack = CLIP(redFirstBack, 0, 255);
			redFirstOrg += redFirstBack;
			redFirstOrg = CLIP(redFirstOrg, 0, 255);
		}

		int greenFirstOrg = originalPalette[pixelColor * 3 + 1] * _vm->_mst_shadow / 256;
		greenFirstOrg = CLIP(greenFirstOrg, 0, 255);
		if (_vm->_mst_shadow <= 256) {
			int greenFirstBack = originalPalette[backgroundPixelColor * 3 + 1] * (256 - _vm->_mst_shadow) / 256;
			greenFirstBack = CLIP(greenFirstBack, 0, 255);
			greenFirstOrg += greenFirstBack;
			greenFirstOrg = CLIP(greenFirstOrg, 0, 255);
		}

		int blueFirstOrg = originalPalette[pixelColor * 3 + 2] * _vm->_mst_shadow / 256;
		blueFirstOrg = CLIP(blueFirstOrg, 0, 255);
		if (_vm->_mst_shadow <= 256) {
			int blueFirstBack = originalPalette[backgroundPixelColor * 3 + 2] * (256 - _vm->_mst_shadow) / 256;
			blueFirstBack = CLIP(blueFirstBack, 0, 255);
			blueFirstOrg += blueFirstBack;
			blueFirstOrg = CLIP(blueFirstOrg, 0, 255);
		}

		int bigValue = PrinceEngine::kIntMax; // infinity
		for (int j = 0; j < 256; j++) {
			int redSecondOrg = originalPalette[3 * j];
			int redNew = redFirstOrg - redSecondOrg;
			redNew = redNew * redNew;

			int greenSecondOrg = originalPalette[3 * j + 1];
			int greenNew = greenFirstOrg - greenSecondOrg;
			greenNew = greenNew * greenNew;

			int blueSecondOrg = originalPalette[3 * j + 2];
			int blueNew = blueFirstOrg - blueSecondOrg;
			blueNew = blueNew * blueNew;

			int sumOfColorValues = redNew + greenNew + blueNew;

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
	int shadow =  brightness * 256 / 100;
	const byte *originalPalette = _vm->_roomBmp->getPalette();

	for (int i = 0; i < 256; i++) {
		int redFirstOrg = originalPalette[3 * i] * shadow / 256;
		int greenFirstOrg = originalPalette[3 * i + 1] * shadow / 256;
		int blueFirstOrg = originalPalette[3 * i + 2] * shadow / 256;

		int currColor = 0;
		int bigValue = 999999999; // infinity

		for (int j = 0; j < 256; j++) {
			int redSecondOrg = originalPalette[3 * j];
			int redNew = redFirstOrg - redSecondOrg;
			redNew = redNew * redNew;

			int greenSecondOrg = originalPalette[3 * j + 1];
			int greenNew = greenFirstOrg - greenSecondOrg;
			greenNew = greenNew * greenNew;

			int blueSecondOrg = originalPalette[3 * j + 2];
			int blueNew = blueFirstOrg - blueSecondOrg;
			blueNew = blueNew * blueNew;

			int sumOfColorValues = redNew + greenNew + blueNew;

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

} // End of namespace Prince
