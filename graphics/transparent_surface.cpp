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
 *
 * The bottom part of this is file is adapted from SDL_rotozoom.c. The
 * relevant copyright notice for those specific functions can be found at the
 * top of that section.
 *
 */



#include "common/algorithm.h"
#include "common/endian.h"
#include "common/util.h"
#include "common/rect.h"
#include "common/math.h"
#include "common/textconsole.h"
#include "graphics/conversion.h"
#include "graphics/primitives.h"
#include "graphics/transparent_surface.h"
#include "graphics/transform_tools.h"

namespace Graphics {

static const int kBModShift = 0;//img->format.bShift;
static const int kGModShift = 8;//img->format.gShift;
static const int kRModShift = 16;//img->format.rShift;
static const int kAModShift = 24;//img->format.aShift;

#ifdef SCUMM_LITTLE_ENDIAN
static const int kAIndex = 0;
static const int kBIndex = 1;
static const int kGIndex = 2;
static const int kRIndex = 3;

#else
static const int kAIndex = 3;
static const int kBIndex = 2;
static const int kGIndex = 1;
static const int kRIndex = 0;
#endif

void doBlitOpaqueFast(byte *ino, byte *outo, uint32 width, uint32 height, uint32 pitch, int32 inStep, int32 inoStep);
void doBlitBinaryFast(byte *ino, byte *outo, uint32 width, uint32 height, uint32 pitch, int32 inStep, int32 inoStep);
void doBlitAlphaBlend(byte *ino, byte *outo, uint32 width, uint32 height, uint32 pitch, int32 inStep, int32 inoStep, uint32 color);
void doBlitAdditiveBlend(byte *ino, byte *outo, uint32 width, uint32 height, uint32 pitch, int32 inStep, int32 inoStep, uint32 color);
void doBlitSubtractiveBlend(byte *ino, byte *outo, uint32 width, uint32 height, uint32 pitch, int32 inStep, int32 inoStep, uint32 color);
void doBlitMultiplyBlend(byte *ino, byte *outo, uint32 width, uint32 height, uint32 pitch, int32 inStep, int32 inoStep, uint32 color);

TransparentSurface::TransparentSurface() : Surface(), _alphaMode(ALPHA_FULL) {}

TransparentSurface::TransparentSurface(const Surface &surf, bool copyData) : Surface(), _alphaMode(ALPHA_FULL) {
	if (copyData) {
		copyFrom(surf);
	} else {
		w = surf.w;
		h = surf.h;
		pitch = surf.pitch;
		format = surf.format;
		// We need to cast the const qualifier away here because 'pixels'
		// always needs to be writable. 'surf' however is a constant Surface,
		// thus getPixels will always return const pixel data.
		pixels = const_cast<void *>(surf.getPixels());
	}
}

/**
 * Optimized version of doBlit to be used w/opaque blitting (no alpha).
 */
void doBlitOpaqueFast(byte *ino, byte *outo, uint32 width, uint32 height, uint32 pitch, int32 inStep, int32 inoStep) {

	byte *in;
	byte *out;

	for (uint32 i = 0; i < height; i++) {
		out = outo;
		in = ino;
		memcpy(out, in, width * 4);
		for (uint32 j = 0; j < width; j++) {
			out[kAIndex] = 0xFF;
			out += 4;
		}
		outo += pitch;
		ino += inoStep;
	}
}

/**
 * Optimized version of doBlit to be used w/binary blitting (blit or no-blit, no blending).
 */
void doBlitBinaryFast(byte *ino, byte *outo, uint32 width, uint32 height, uint32 pitch, int32 inStep, int32 inoStep) {

	byte *in;
	byte *out;

	for (uint32 i = 0; i < height; i++) {
		out = outo;
		in = ino;
		for (uint32 j = 0; j < width; j++) {
			uint32 pix = *(uint32 *)in;
			int a = in[kAIndex];

			if (a != 0) {   // Full opacity (Any value not exactly 0 is Opaque here)
				*(uint32 *)out = pix;
				out[kAIndex] = 0xFF;
			}
			out += 4;
			in += inStep;
		}
		outo += pitch;
		ino += inoStep;
	}
}

/**
 * Optimized version of doBlit to be used with alpha blended blitting
 * @param ino a pointer to the input surface
 * @param outo a pointer to the output surface
 * @param width width of the input surface
 * @param height height of the input surface
 * @param pitch pitch of the output surface - that is, width in bytes of every row, usually bpp * width of the TARGET surface (the area we are blitting to might be smaller, do the math)
 * @inStep size in bytes to skip to address each pixel, usually bpp of the source surface
 * @inoStep width in bytes of every row on the *input* surface / kind of like pitch
 * @color colormod in 0xAARRGGBB format - 0xFFFFFFFF for no colormod
 */
void doBlitAlphaBlend(byte *ino, byte *outo, uint32 width, uint32 height, uint32 pitch, int32 inStep, int32 inoStep, uint32 color) {
	byte *in;
	byte *out;

	if (color == 0xffffffff) {

		for (uint32 i = 0; i < height; i++) {
			out = outo;
			in = ino;
			for (uint32 j = 0; j < width; j++) {

				if (in[kAIndex] != 0) {
					out[kAIndex] = 255;
					out[kRIndex] = ((in[kRIndex] * in[kAIndex]) + out[kRIndex] * (255 - in[kAIndex])) >> 8;
					out[kGIndex] = ((in[kGIndex] * in[kAIndex]) + out[kGIndex] * (255 - in[kAIndex])) >> 8;
					out[kBIndex] = ((in[kBIndex] * in[kAIndex]) + out[kBIndex] * (255 - in[kAIndex])) >> 8;
				}

				in += inStep;
				out += 4;
			}
			outo += pitch;
			ino += inoStep;
		}
	} else {

		byte ca = (color >> kAModShift) & 0xFF;
		byte cr = (color >> kRModShift) & 0xFF;
		byte cg = (color >> kGModShift) & 0xFF;
		byte cb = (color >> kBModShift) & 0xFF;

		for (uint32 i = 0; i < height; i++) {
			out = outo;
			in = ino;
			for (uint32 j = 0; j < width; j++) {

				uint32 ina = in[kAIndex] * ca >> 8;

				if (ina != 0) {
					out[kAIndex] = 255;
					out[kBIndex] = (out[kBIndex] * (255 - ina) >> 8);
					out[kGIndex] = (out[kGIndex] * (255 - ina) >> 8);
					out[kRIndex] = (out[kRIndex] * (255 - ina) >> 8);

					out[kBIndex] = out[kBIndex] + (in[kBIndex] * ina * cb >> 16);
					out[kGIndex] = out[kGIndex] + (in[kGIndex] * ina * cg >> 16);
					out[kRIndex] = out[kRIndex] + (in[kRIndex] * ina * cr >> 16);
				}

				in += inStep;
				out += 4;
			}
			outo += pitch;
			ino += inoStep;
		}
	}
}

/**
 * Optimized version of doBlit to be used with additive blended blitting
 */
void doBlitAdditiveBlend(byte *ino, byte *outo, uint32 width, uint32 height, uint32 pitch, int32 inStep, int32 inoStep, uint32 color) {
	byte *in;
	byte *out;

	if (color == 0xffffffff) {

		for (uint32 i = 0; i < height; i++) {
			out = outo;
			in = ino;
			for (uint32 j = 0; j < width; j++) {

				if (in[kAIndex] != 0) {
					out[kRIndex] = MIN((in[kRIndex] * in[kAIndex] >> 8) + out[kRIndex], 255);
					out[kGIndex] = MIN((in[kGIndex] * in[kAIndex] >> 8) + out[kGIndex], 255);
					out[kBIndex] = MIN((in[kBIndex] * in[kAIndex] >> 8) + out[kBIndex], 255);
				}

				in += inStep;
				out += 4;
			}
			outo += pitch;
			ino += inoStep;
		}
	} else {

		byte ca = (color >> kAModShift) & 0xFF;
		byte cr = (color >> kRModShift) & 0xFF;
		byte cg = (color >> kGModShift) & 0xFF;
		byte cb = (color >> kBModShift) & 0xFF;

		for (uint32 i = 0; i < height; i++) {
			out = outo;
			in = ino;
			for (uint32 j = 0; j < width; j++) {

				uint32 ina = in[kAIndex] * ca >> 8;

				if (cb != 255) {
					out[kBIndex] = MIN<uint>(out[kBIndex] + ((in[kBIndex] * cb * ina) >> 16), 255u);
				} else {
					out[kBIndex] = MIN<uint>(out[kBIndex] + (in[kBIndex] * ina >> 8), 255u);
				}

				if (cg != 255) {
					out[kGIndex] = MIN<uint>(out[kGIndex] + ((in[kGIndex] * cg * ina) >> 16), 255u);
				} else {
					out[kGIndex] = MIN<uint>(out[kGIndex] + (in[kGIndex] * ina >> 8), 255u);
				}

				if (cr != 255) {
					out[kRIndex] = MIN<uint>(out[kRIndex] + ((in[kRIndex] * cr * ina) >> 16), 255u);
				} else {
					out[kRIndex] = MIN<uint>(out[kRIndex] + (in[kRIndex] * ina >> 8), 255u);
				}

				in += inStep;
				out += 4;
			}
			outo += pitch;
			ino += inoStep;
		}
	}
}

/**
 * Optimized version of doBlit to be used with subtractive blended blitting
 */
void doBlitSubtractiveBlend(byte *ino, byte *outo, uint32 width, uint32 height, uint32 pitch, int32 inStep, int32 inoStep, uint32 color) {
	byte *in;
	byte *out;

	if (color == 0xffffffff) {

		for (uint32 i = 0; i < height; i++) {
			out = outo;
			in = ino;
			for (uint32 j = 0; j < width; j++) {

				if (in[kAIndex] != 0) {
					out[kRIndex] = MAX(out[kRIndex] - ((in[kRIndex] * out[kRIndex]) * in[kAIndex] >> 16), 0);
					out[kGIndex] = MAX(out[kGIndex] - ((in[kGIndex] * out[kGIndex]) * in[kAIndex] >> 16), 0);
					out[kBIndex] = MAX(out[kBIndex] - ((in[kBIndex] * out[kBIndex]) * in[kAIndex] >> 16), 0);
				}

				in += inStep;
				out += 4;
			}
			outo += pitch;
			ino += inoStep;
		}
	} else {

		byte cr = (color >> kRModShift) & 0xFF;
		byte cg = (color >> kGModShift) & 0xFF;
		byte cb = (color >> kBModShift) & 0xFF;

		for (uint32 i = 0; i < height; i++) {
			out = outo;
			in = ino;
			for (uint32 j = 0; j < width; j++) {

				out[kAIndex] = 255;
				if (cb != 255) {
					out[kBIndex] = MAX(out[kBIndex] - ((in[kBIndex] * cb  * (out[kBIndex]) * in[kAIndex]) >> 24), 0);
				} else {
					out[kBIndex] = MAX(out[kBIndex] - (in[kBIndex] * (out[kBIndex]) * in[kAIndex] >> 16), 0);
				}

				if (cg != 255) {
					out[kGIndex] = MAX(out[kGIndex] - ((in[kGIndex] * cg  * (out[kGIndex]) * in[kAIndex]) >> 24), 0);
				} else {
					out[kGIndex] = MAX(out[kGIndex] - (in[kGIndex] * (out[kGIndex]) * in[kAIndex] >> 16), 0);
				}

				if (cr != 255) {
					out[kRIndex] = MAX(out[kRIndex] - ((in[kRIndex] * cr * (out[kRIndex]) * in[kAIndex]) >> 24), 0);
				} else {
					out[kRIndex] = MAX(out[kRIndex] - (in[kRIndex] * (out[kRIndex]) * in[kAIndex] >> 16), 0);
				}

				in += inStep;
				out += 4;
			}
			outo += pitch;
			ino += inoStep;
		}
	}
}

/**
 * Optimized version of doBlit to be used with multiply blended blitting
 */
void doBlitMultiplyBlend(byte *ino, byte *outo, uint32 width, uint32 height, uint32 pitch, int32 inStep, int32 inoStep, uint32 color) {
	byte *in;
	byte *out;

	if (color == 0xffffffff) {
		for (uint32 i = 0; i < height; i++) {
			out = outo;
			in = ino;
			for (uint32 j = 0; j < width; j++) {

				if (in[kAIndex] != 0) {
					out[kRIndex] = MIN((in[kRIndex] * in[kAIndex] >> 8) * out[kRIndex] >> 8, 255);
					out[kGIndex] = MIN((in[kGIndex] * in[kAIndex] >> 8) * out[kGIndex] >> 8, 255);
					out[kBIndex] = MIN((in[kBIndex] * in[kAIndex] >> 8) * out[kBIndex] >> 8, 255);
				}

				in += inStep;
				out += 4;
			}
			outo += pitch;
			ino += inoStep;
		}
	} else {
		byte ca = (color >> kAModShift) & 0xFF;
		byte cr = (color >> kRModShift) & 0xFF;
		byte cg = (color >> kGModShift) & 0xFF;
		byte cb = (color >> kBModShift) & 0xFF;

		for (uint32 i = 0; i < height; i++) {
			out = outo;
			in = ino;
			for (uint32 j = 0; j < width; j++) {

				uint32 ina = in[kAIndex] * ca >> 8;

				if (cb != 255) {
					out[kBIndex] = MIN<uint>(out[kBIndex] * ((in[kBIndex] * cb * ina) >> 16) >> 8, 255u);
				} else {
					out[kBIndex] = MIN<uint>(out[kBIndex] * (in[kBIndex] * ina >> 8) >> 8, 255u);
				}

				if (cg != 255) {
					out[kGIndex] = MIN<uint>(out[kGIndex] * ((in[kGIndex] * cg * ina) >> 16) >> 8, 255u);
				} else {
					out[kGIndex] = MIN<uint>(out[kGIndex] * (in[kGIndex] * ina >> 8) >> 8, 255u);
				}

				if (cr != 255) {
					out[kRIndex] = MIN<uint>(out[kRIndex] * ((in[kRIndex] * cr * ina) >> 16) >> 8, 255u);
				} else {
					out[kRIndex] = MIN<uint>(out[kRIndex] * (in[kRIndex] * ina >> 8) >> 8, 255u);
				}

				in += inStep;
				out += 4;
			}
			outo += pitch;
			ino += inoStep;
		}
	}

}

Common::Rect TransparentSurface::blit(Graphics::Surface &target, int posX, int posY, int flipping, Common::Rect *pPartRect, uint color, int width, int height, TSpriteBlendMode blendMode) {

	Common::Rect retSize;
	retSize.top = 0;
	retSize.left = 0;
	retSize.setWidth(0);
	retSize.setHeight(0);
	// Check if we need to draw anything at all
	int ca = (color >> kAModShift) & 0xff;

	if (ca == 0) {
		return retSize;
	}

	// Create an encapsulating surface for the data
	TransparentSurface srcImage(*this, false);
	// TODO: Is the data really in the screen format?
	if (format.bytesPerPixel != 4) {
		warning("TransparentSurface can only blit 32bpp images, but got %d", format.bytesPerPixel * 8);
		return retSize;
	}

	if (pPartRect) {

		int xOffset = pPartRect->left;
		int yOffset = pPartRect->top;

		if (flipping & FLIP_V) {
			yOffset = srcImage.h - pPartRect->bottom;
		}

		if (flipping & FLIP_H) {
			xOffset = srcImage.w - pPartRect->right;
		}

		srcImage.pixels = getBasePtr(xOffset, yOffset);
		srcImage.w = pPartRect->width();
		srcImage.h = pPartRect->height();

		debug(6, "Blit(%d, %d, %d, [%d, %d, %d, %d], %08x, %d, %d)", posX, posY, flipping,
			  pPartRect->left,  pPartRect->top, pPartRect->width(), pPartRect->height(), color, width, height);
	} else {

		debug(6, "Blit(%d, %d, %d, [%d, %d, %d, %d], %08x, %d, %d)", posX, posY, flipping, 0, 0,
			  srcImage.w, srcImage.h, color, width, height);
	}

	if (width == -1) {
		width = srcImage.w;
	}
	if (height == -1) {
		height = srcImage.h;
	}

#ifdef SCALING_TESTING
	// Hardcode scaling to 66% to test scaling
	width = width * 2 / 3;
	height = height * 2 / 3;
#endif

	Graphics::Surface *img = nullptr;
	Graphics::Surface *imgScaled = nullptr;
	byte *savedPixels = nullptr;
	if ((width != srcImage.w) || (height != srcImage.h)) {
		// Scale the image
		img = imgScaled = srcImage.scale(width, height);
		savedPixels = (byte *)img->getPixels();
	} else {
		img = &srcImage;
	}

	// Handle off-screen clipping
	if (posY < 0) {
		img->h = MAX(0, (int)img->h - -posY);
		if (!(flipping & FLIP_V))
			img->setPixels((byte *)img->getBasePtr(0, -posY));
		posY = 0;
	}

	if (posX < 0) {
		img->w = MAX(0, (int)img->w - -posX);
		if (!(flipping & FLIP_H))
			img->setPixels((byte *)img->getBasePtr(-posX, 0));
		posX = 0;
	}

	if (img->w > target.w - posX) {
		if (flipping & FLIP_H)
			img->setPixels((byte *)img->getBasePtr(img->w - target.w + posX, 0));
		img->w = CLIP((int)img->w, 0, (int)MAX((int)target.w - posX, 0));
	}

	if (img->h > target.h - posY) {
		if (flipping & FLIP_V)
			img->setPixels((byte *)img->getBasePtr(0, img->h - target.h + posY));
		img->h = CLIP((int)img->h, 0, (int)MAX((int)target.h - posY, 0));
	}

	// Flip surface
	if ((img->w > 0) && (img->h > 0)) {
		int xp = 0, yp = 0;

		int inStep = 4;
		int inoStep = img->pitch;
		if (flipping & FLIP_H) {
			inStep = -inStep;
			xp = img->w - 1;
		}

		if (flipping & FLIP_V) {
			inoStep = -inoStep;
			yp = img->h - 1;
		}

		byte *ino = (byte *)img->getBasePtr(xp, yp);
		byte *outo = (byte *)target.getBasePtr(posX, posY);

		if (color == 0xFFFFFFFF && blendMode == BLEND_NORMAL && _alphaMode == ALPHA_OPAQUE) {
			doBlitOpaqueFast(ino, outo, img->w, img->h, target.pitch, inStep, inoStep);
		} else if (color == 0xFFFFFFFF && blendMode == BLEND_NORMAL && _alphaMode == ALPHA_BINARY) {
			doBlitBinaryFast(ino, outo, img->w, img->h, target.pitch, inStep, inoStep);
		} else {
			if (blendMode == BLEND_ADDITIVE) {
				doBlitAdditiveBlend(ino, outo, img->w, img->h, target.pitch, inStep, inoStep, color);
			} else if (blendMode == BLEND_SUBTRACTIVE) {
				doBlitSubtractiveBlend(ino, outo, img->w, img->h, target.pitch, inStep, inoStep, color);
			} else if (blendMode == BLEND_MULTIPLY) {
				doBlitMultiplyBlend(ino, outo, img->w, img->h, target.pitch, inStep, inoStep, color);
			} else {
				assert(blendMode == BLEND_NORMAL);
				doBlitAlphaBlend(ino, outo, img->w, img->h, target.pitch, inStep, inoStep, color);
			}
		}

	}

	retSize.setWidth(img->w);
	retSize.setHeight(img->h);

	if (imgScaled) {
		imgScaled->setPixels(savedPixels);
		imgScaled->free();
		delete imgScaled;
	}

	return retSize;
}

Common::Rect TransparentSurface::blitClip(Graphics::Surface &target, Common::Rect clippingArea, int posX, int posY, int flipping, Common::Rect *pPartRect, uint color, int width, int height, TSpriteBlendMode blendMode) {
	Common::Rect retSize;
	retSize.top = 0;
	retSize.left = 0;
	retSize.setWidth(0);
	retSize.setHeight(0);
	// Check if we need to draw anything at all
	int ca = (color >> kAModShift) & 0xff;

	if (ca == 0) {
		return retSize;
	}

	// Create an encapsulating surface for the data
	TransparentSurface srcImage(*this, false);
	// TODO: Is the data really in the screen format?
	if (format.bytesPerPixel != 4) {
		warning("TransparentSurface can only blit 32bpp images, but got %d", format.bytesPerPixel * 8);
		return retSize;
	}

	if (pPartRect) {

		int xOffset = pPartRect->left;
		int yOffset = pPartRect->top;

		if (flipping & FLIP_V) {
			yOffset = srcImage.h - pPartRect->bottom;
		}

		if (flipping & FLIP_H) {
			xOffset = srcImage.w - pPartRect->right;
		}

		srcImage.pixels = getBasePtr(xOffset, yOffset);
		srcImage.w = pPartRect->width();
		srcImage.h = pPartRect->height();

		debug(6, "Blit(%d, %d, %d, [%d, %d, %d, %d], %08x, %d, %d)", posX, posY, flipping,
			pPartRect->left, pPartRect->top, pPartRect->width(), pPartRect->height(), color, width, height);
	} else {

		debug(6, "Blit(%d, %d, %d, [%d, %d, %d, %d], %08x, %d, %d)", posX, posY, flipping, 0, 0,
			srcImage.w, srcImage.h, color, width, height);
	}

	if (width == -1) {
		width = srcImage.w;
	}
	if (height == -1) {
		height = srcImage.h;
	}

#ifdef SCALING_TESTING
	// Hardcode scaling to 66% to test scaling
	width = width * 2 / 3;
	height = height * 2 / 3;
#endif

	Graphics::Surface *img = nullptr;
	Graphics::Surface *imgScaled = nullptr;
	byte *savedPixels = nullptr;
	if ((width != srcImage.w) || (height != srcImage.h)) {
		// Scale the image
		img = imgScaled = srcImage.scale(width, height);
		savedPixels = (byte *)img->getPixels();
	} else {
		img = &srcImage;
	}

	// Handle off-screen clipping
	if (posY < clippingArea.top) {
		img->h = MAX(0, (int)img->h - (clippingArea.top - posY));
		if (!(flipping & FLIP_V))
			img->setPixels((byte *)img->getBasePtr(0, clippingArea.top - posY));
		posY = clippingArea.top;
	}

	if (posX < clippingArea.left) {
		img->w = MAX(0, (int)img->w - (clippingArea.left - posX));
		if (!(flipping & FLIP_H))
			img->setPixels((byte *)img->getBasePtr(clippingArea.left - posX, 0));
		posX = clippingArea.left;
	}

	if (img->w > clippingArea.right - posX) {
		if (flipping & FLIP_H)
			img->setPixels((byte *)img->getBasePtr(img->w - clippingArea.right + posX, 0));
		img->w = CLIP((int)img->w, 0, (int)MAX((int)clippingArea.right - posX, 0));
	}

	if (img->h > clippingArea.bottom - posY) {
		if (flipping & FLIP_V)
			img->setPixels((byte *)img->getBasePtr(0, img->h - clippingArea.bottom + posY));
		img->h = CLIP((int)img->h, 0, (int)MAX((int)clippingArea.bottom - posY, 0));
	}

	// Flip surface
	if ((img->w > 0) && (img->h > 0)) {
		int xp = 0, yp = 0;

		int inStep = 4;
		int inoStep = img->pitch;
		if (flipping & FLIP_H) {
			inStep = -inStep;
			xp = img->w - 1;
		}

		if (flipping & FLIP_V) {
			inoStep = -inoStep;
			yp = img->h - 1;
		}

		byte *ino = (byte *)img->getBasePtr(xp, yp);
		byte *outo = (byte *)target.getBasePtr(posX, posY);

		if (color == 0xFFFFFFFF && blendMode == BLEND_NORMAL && _alphaMode == ALPHA_OPAQUE) {
			doBlitOpaqueFast(ino, outo, img->w, img->h, target.pitch, inStep, inoStep);
		} else if (color == 0xFFFFFFFF && blendMode == BLEND_NORMAL && _alphaMode == ALPHA_BINARY) {
			doBlitBinaryFast(ino, outo, img->w, img->h, target.pitch, inStep, inoStep);
		} else {
			if (blendMode == BLEND_ADDITIVE) {
				doBlitAdditiveBlend(ino, outo, img->w, img->h, target.pitch, inStep, inoStep, color);
			} else if (blendMode == BLEND_SUBTRACTIVE) {
				doBlitSubtractiveBlend(ino, outo, img->w, img->h, target.pitch, inStep, inoStep, color);
			} else if (blendMode == BLEND_MULTIPLY) {
				doBlitMultiplyBlend(ino, outo, img->w, img->h, target.pitch, inStep, inoStep, color);
			} else {
				assert(blendMode == BLEND_NORMAL);
				doBlitAlphaBlend(ino, outo, img->w, img->h, target.pitch, inStep, inoStep, color);
			}
		}

	}

	retSize.setWidth(img->w);
	retSize.setHeight(img->h);

	if (imgScaled) {
		imgScaled->setPixels(savedPixels);
		imgScaled->free();
		delete imgScaled;
	}

	return retSize;
}

/**
 * Writes a color key to the alpha channel of the surface
 * @param rKey  the red component of the color key
 * @param gKey  the green component of the color key
 * @param bKey  the blue component of the color key
 * @param overwriteAlpha if true, all other alpha will be set fully opaque
 */
void TransparentSurface::applyColorKey(uint8 rKey, uint8 gKey, uint8 bKey, bool overwriteAlpha) {
	assert(format.bytesPerPixel == 4);
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			uint32 pix = ((uint32 *)pixels)[i * w + j];
			uint8 r, g, b, a;
			format.colorToARGB(pix, a, r, g, b);
			if (r == rKey && g == gKey && b == bKey) {
				a = 0;
				((uint32 *)pixels)[i * w + j] = format.ARGBToColor(a, r, g, b);
			} else if (overwriteAlpha) {
				a = 255;
				((uint32 *)pixels)[i * w + j] = format.ARGBToColor(a, r, g, b);
			}
		}
	}
}

/**
 * Sets alpha channel for all pixels to specified value
 * @param alpha  value of the alpha channel to set
 * @param skipTransparent  if set to true, then do not touch pixels with alpha=0
 */
void TransparentSurface::setAlpha(uint8 alpha, bool skipTransparent) {
	assert(format.bytesPerPixel == 4);
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			uint32 pix = ((uint32 *)pixels)[i * w + j];
			uint8 r, g, b, a;
			format.colorToARGB(pix, a, r, g, b);
			if (!skipTransparent || a)
				a = alpha;
			((uint32 *)pixels)[i * w + j] = format.ARGBToColor(a, r, g, b);
		}
	}
}

AlphaType TransparentSurface::getAlphaMode() const {
	return _alphaMode;
}

void TransparentSurface::setAlphaMode(AlphaType mode) {
	_alphaMode = mode;
}






/*

The function below is adapted from SDL_rotozoom.c,
taken from SDL_gfx-2.0.18.

Its copyright notice:

=============================================================================
SDL_rotozoom.c: rotozoomer, zoomer and shrinker for 32bit or 8bit surfaces

Copyright (C) 2001-2012  Andreas Schiffler

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.

Andreas Schiffler -- aschiffler at ferzkopp dot net
=============================================================================


The functions have been adapted for different structures and coordinate
systems.

*/




struct tColorRGBA { byte r; byte g; byte b; byte a; };

template <TFilteringMode filteringMode>
TransparentSurface *TransparentSurface::rotoscaleT(const TransformStruct &transform) const {

	assert(transform._angle != 0); // This would not be ideal; rotoscale() should never be called in conditional branches where angle = 0 anyway.

	Common::Point newHotspot;
	Common::Rect srcRect(0, 0, (int16)w, (int16)h);
	Common::Rect rect = TransformTools::newRect(Common::Rect(srcRect), transform, &newHotspot);
	Common::Rect dstRect(0, 0, (int16)(rect.right - rect.left), (int16)(rect.bottom - rect.top));

	TransparentSurface *target = new TransparentSurface();
	assert(format.bytesPerPixel == 4);

	int srcW = w;
	int srcH = h;
	int dstW = dstRect.width();
	int dstH = dstRect.height();

	target->create((uint16)dstW, (uint16)dstH, this->format);

	if (transform._zoom.x == 0 || transform._zoom.y == 0) {
		return target;
	}

	uint32 invAngle = 360 - (transform._angle % 360);
	float invAngleRad = Common::deg2rad<uint32,float>(invAngle);
	float invCos = cos(invAngleRad);
	float invSin = sin(invAngleRad);

	int icosx = (int)(invCos * (65536.0f * kDefaultZoomX / transform._zoom.x));
	int isinx = (int)(invSin * (65536.0f * kDefaultZoomX / transform._zoom.x));
	int icosy = (int)(invCos * (65536.0f * kDefaultZoomY / transform._zoom.y));
	int isiny = (int)(invSin * (65536.0f * kDefaultZoomY / transform._zoom.y));


	bool flipx = false, flipy = false; // TODO: See mirroring comment in RenderTicket ctor

	int xd = (srcRect.left + transform._hotspot.x) << 16;
	int yd = (srcRect.top + transform._hotspot.y) << 16;
	int cx = newHotspot.x;
	int cy = newHotspot.y;

	int ax = -icosx * cx;
	int ay = -isiny * cx;
	int sw = srcW - 1;
	int sh = srcH - 1;

	tColorRGBA *pc = (tColorRGBA*)target->getBasePtr(0, 0);

	for (int y = 0; y < dstH; y++) {
		int t = cy - y;
		int sdx = ax + (isinx * t) + xd;
		int sdy = ay - (icosy * t) + yd;
		for (int x = 0; x < dstW; x++) {
			int dx = (sdx >> 16);
			int dy = (sdy >> 16);
			if (flipx) {
				dx = sw - dx;
			}
			if (flipy) {
				dy = sh - dy;
			}

			if (filteringMode == FILTER_BILINEAR) {
				if ((dx > -1) && (dy > -1) && (dx < sw) && (dy < sh)) {
					const tColorRGBA *sp = (const tColorRGBA *)getBasePtr(dx, dy);
					tColorRGBA c00, c01, c10, c11, cswap;
					c00 = *sp;
					sp += 1;
					c01 = *sp;
					sp += (this->pitch / 4);
					c11 = *sp;
					sp -= 1;
					c10 = *sp;
					if (flipx) {
						cswap = c00; c00=c01; c01=cswap;
						cswap = c10; c10=c11; c11=cswap;
					}
					if (flipy) {
						cswap = c00; c00=c10; c10=cswap;
						cswap = c01; c01=c11; c11=cswap;
					}
					/*
					* Interpolate colors
					*/
					int ex = (sdx & 0xffff);
					int ey = (sdy & 0xffff);
					int t1, t2;
					t1 = ((((c01.r - c00.r) * ex) >> 16) + c00.r) & 0xff;
					t2 = ((((c11.r - c10.r) * ex) >> 16) + c10.r) & 0xff;
					pc->r = (((t2 - t1) * ey) >> 16) + t1;
					t1 = ((((c01.g - c00.g) * ex) >> 16) + c00.g) & 0xff;
					t2 = ((((c11.g - c10.g) * ex) >> 16) + c10.g) & 0xff;
					pc->g = (((t2 - t1) * ey) >> 16) + t1;
					t1 = ((((c01.b - c00.b) * ex) >> 16) + c00.b) & 0xff;
					t2 = ((((c11.b - c10.b) * ex) >> 16) + c10.b) & 0xff;
					pc->b = (((t2 - t1) * ey) >> 16) + t1;
					t1 = ((((c01.a - c00.a) * ex) >> 16) + c00.a) & 0xff;
					t2 = ((((c11.a - c10.a) * ex) >> 16) + c10.a) & 0xff;
					pc->a = (((t2 - t1) * ey) >> 16) + t1;
				}
			} else {
				if ((dx >= 0) && (dy >= 0) && (dx < srcW) && (dy < srcH)) {
					const tColorRGBA *sp = (const tColorRGBA *)getBasePtr(dx, dy);
					*pc = *sp;
				}
			}
			sdx += icosx;
			sdy += isiny;
			pc++;
		}
	}
	return target;
}

TransparentSurface *TransparentSurface::scale(uint16 newWidth, uint16 newHeight, bool filtering) const {

	TransparentSurface *target = new TransparentSurface();

	target->create(newWidth, newHeight, format);

	if (filtering) {
		scaleBlitBilinear((byte *)target->getPixels(), (const byte *)getPixels(), target->pitch, pitch, target->w, target->h, w, h, format);
	} else {
		scaleBlit((byte *)target->getPixels(), (const byte *)getPixels(), target->pitch, pitch, target->w, target->h, w, h, format);
	}

	return target;
}

TransparentSurface *TransparentSurface::convertTo(const PixelFormat &dstFormat, const byte *palette) const {
	assert(pixels);

	TransparentSurface *surface = new TransparentSurface();

	// If the target format is the same, just copy
	if (format == dstFormat) {
		surface->copyFrom(*this);
		return surface;
	}

	if (format.bytesPerPixel == 0 || format.bytesPerPixel > 4)
		error("Surface::convertTo(): Can only convert from 1Bpp, 2Bpp, 3Bpp, and 4Bpp");

	if (dstFormat.bytesPerPixel != 2 && dstFormat.bytesPerPixel != 4)
		error("Surface::convertTo(): Can only convert to 2Bpp and 4Bpp");

	surface->create(w, h, dstFormat);

	if (format.bytesPerPixel == 1) {
		// Converting from paletted to high color
		assert(palette);

		for (int y = 0; y < h; y++) {
			const byte *srcRow = (const byte *)getBasePtr(0, y);
			byte *dstRow = (byte *)surface->getBasePtr(0, y);

			for (int x = 0; x < w; x++) {
				byte index = *srcRow++;
				byte r = palette[index * 3];
				byte g = palette[index * 3 + 1];
				byte b = palette[index * 3 + 2];

				uint32 color = dstFormat.RGBToColor(r, g, b);

				if (dstFormat.bytesPerPixel == 2)
					*((uint16 *)dstRow) = color;
				else
					*((uint32 *)dstRow) = color;

				dstRow += dstFormat.bytesPerPixel;
			}
		}
	} else {
		// Converting from high color to high color
		for (int y = 0; y < h; y++) {
			const byte *srcRow = (const byte *)getBasePtr(0, y);
			byte *dstRow = (byte *)surface->getBasePtr(0, y);

			for (int x = 0; x < w; x++) {
				uint32 srcColor;
				if (format.bytesPerPixel == 2)
					srcColor = READ_UINT16(srcRow);
				else if (format.bytesPerPixel == 3)
					srcColor = READ_UINT24(srcRow);
				else
					srcColor = READ_UINT32(srcRow);

				srcRow += format.bytesPerPixel;

				// Convert that color to the new format
				byte r, g, b, a;
				format.colorToARGB(srcColor, a, r, g, b);
				uint32 color = dstFormat.ARGBToColor(a, r, g, b);

				if (dstFormat.bytesPerPixel == 2)
					*((uint16 *)dstRow) = color;
				else
					*((uint32 *)dstRow) = color;

				dstRow += dstFormat.bytesPerPixel;
			}
		}
	}

	return surface;
}

template TransparentSurface *TransparentSurface::rotoscaleT<FILTER_NEAREST>(const TransformStruct &transform) const;
template TransparentSurface *TransparentSurface::rotoscaleT<FILTER_BILINEAR>(const TransformStruct &transform) const;

TransparentSurface *TransparentSurface::rotoscale(const TransformStruct &transform) const {
	return rotoscaleT<FILTER_BILINEAR>(transform);
}

} // End of namespace Graphics
