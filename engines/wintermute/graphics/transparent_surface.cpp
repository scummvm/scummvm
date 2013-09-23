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
 */

#include "common/algorithm.h"
#include "common/endian.h"
#include "common/util.h"
#include "common/rect.h"
#include "common/math.h"
#include "common/textconsole.h"
#include "graphics/primitives.h"
#include "engines/wintermute/graphics/transparent_surface.h"
#include "engines/wintermute/graphics/transform_tools.h"

namespace Wintermute {

void doBlitOpaqueFast(byte *ino, byte *outo, uint32 width, uint32 height, uint32 pitch, int32 inStep, int32 inoStep);
void doBlitBinaryFast(byte *ino, byte *outo, uint32 width, uint32 height, uint32 pitch, int32 inStep, int32 inoStep);

// These gather together various blendPixel functions for use with templates.

class BlenderAdditive {
public:
	static void blendPixel(byte ina, byte inr, byte ing, byte inb, byte *outa, byte *outr, byte *outg, byte *outb);
	static void blendPixel(byte ina, byte inr, byte ing, byte inb, byte *outa, byte *outr, byte *outg, byte *outb, byte *ca, byte *cr, byte *cg, byte *cb);
	static void blendPixel(byte *in, byte *out);
	static void blendPixel(byte *in, byte *out, int colorMod);
};

class BlenderSubtractive {
public:
	static void blendPixel(byte ina, byte inr, byte ing, byte inb, byte *outa, byte *outr, byte *outg, byte *outb);
	static void blendPixel(byte ina, byte inr, byte ing, byte inb, byte *outa, byte *outr, byte *outg, byte *outb, byte *ca, byte *cr, byte *cg, byte *cb);
	static void blendPixel(byte *in, byte *out);
	static void blendPixel(byte *in, byte *out, int colorMod);
};

class BlenderNormal {
public:
	static void blendPixel(byte ina, byte inr, byte ing, byte inb, byte *outa, byte *outr, byte *outg, byte *outb);
	static void blendPixel(byte ina, byte inr, byte ing, byte inb, byte *outa, byte *outr, byte *outg, byte *outb, byte *ca, byte *cr, byte *cg, byte *cb);
	static void blendPixel(byte *in, byte *out);
	static void blendPixel(byte *in, byte *out, int colorMod);
};

/**
 * Perform additive blending of a pixel, applying beforehand a given colormod.
 * @param ina, inr, ing, inb: the input pixel, split into its components.
 * @param *outa, *outr, *outg, *outb pointer to the output pixel.
 * @param *outa, *outr, *outg, *outb pointer to the colormod components.
 */

void BlenderAdditive::blendPixel(byte ina, byte inr, byte ing, byte inb, byte *outa, byte *outr, byte *outg, byte *outb, byte *ca, byte *cr, byte *cg, byte *cb) {
	if (*ca != 255) {
		ina = (ina) * (*ca) >> 8;
	}

	if (ina == 0) {
		return;
	} else {
		if (*cb != 255)
			*outb = MIN(*outb + ((inb * (*cb) * ina) >> 16), 255);
		else
			*outb = MIN(*outb + (inb * ina >> 8), 255);

		if (*cg != 255)
			*outg = MIN(*outg + ((ing * (*cg) * ina) >> 16), 255);
		else
			*outg = MIN(*outg + (ing * ina >> 8), 255);

		if (*cr != 255)
			*outr = MIN(*outr + ((inr * (*cr) * ina) >> 16), 255);
		else
			*outr = MIN(*outr + (inr * ina >> 8), 255);
	}
}

/**
 * Perform subtractive blending of a pixel, applying beforehand a given colormod.
 * @param ina, inr, ing, inb: the input pixel, split into its components.
 * @param *outa, *outr, *outg, *outb pointer to the output pixel.
 * @param *outa, *outr, *outg, *outb pointer to the colormod components.
 */

void BlenderSubtractive::blendPixel(byte ina, byte inr, byte ing, byte inb, byte *outa, byte *outr, byte *outg, byte *outb, byte *ca, byte *cr, byte *cg, byte *cb) {
	//if (*ca != 255) {
	//	ina = ina * (*ca) >> 8;
	// }

	// As weird as it is, evidence suggests that alphamod is ignored when doing
	// subtractive...

	// TODO if ina == 255 fast version

	if (ina == 0) {
		return;
	} else {
		if (*cb != 255)
			*outb = MAX(*outb - ((inb * (*cb)  * (*outb) * ina) >> 24), 0);
		else
			*outb = MAX(*outb - (inb * (*outb) * ina >> 16), 0);

		if (*cg != 255)
			*outg = MAX(*outg - ((ing * (*cg)  * (*outg) * ina) >> 24), 0);
		else
			*outg = MAX(*outg - (ing * (*outg) * ina >> 16), 0);

		if (*cr != 255)
			*outr = MAX(*outr - ((inr * (*cr) *  (*outr) * ina) >> 24), 0);
		else
			*outr = MAX(*outr - (inr * (*outr) * ina >> 16), 0);
	}
}

/**
 * Perform "regular" alphablending of a pixel, applying beforehand a given colormod.
 * @param ina, inr, ing, inb: the input pixel, split into its components.
 * @param *outa, *outr, *outg, *outb pointer to the output pixel.
 * @param *outa, *outr, *outg, *outb pointer to the colormod components.
 */

void BlenderNormal::blendPixel(byte ina, byte inr, byte ing, byte inb, byte *outa, byte *outr, byte *outg, byte *outb, byte *ca, byte *cr, byte *cg, byte *cb) {
	if (*ca != 255) {
		ina = ina * (*ca) >> 8;
	}

	if (ina == 0) {
		return;
	} else if (ina == 255) {
		if (*cb != 255)
			*outb = (inb * (*cb)) >> 8;
		else
			*outb = inb;

		if (*cr != 255)
			*outr = (inr * (*cr)) >> 8;
		else
			*outr = inr;

		if (*cg != 255)
			*outg = (ing * (*cg)) >> 8;
		else
			*outg = ing;

		*outa = ina;

		return;

	} else {

		*outa = 255;
		*outb = (*outb * (255 - ina) >> 8);
		*outr = (*outr * (255 - ina) >> 8);
		*outg = (*outg * (255 - ina) >> 8);

		if (*cb == 0)
			*outb = *outb;
		else if (*cb != 255)
			*outb = *outb + (inb * ina * (*cb) >> 16);
		else
			*outb = *outb + (inb * ina >> 8);

		if (*cr == 0)
			*outr = *outr;
		else if (*cr != 255)
			*outr = *outr + (inr * ina * (*cr) >> 16);
		else
			*outr = *outr + (inr * ina >> 8);

		if (*cg == 0)
			*outg = *outg;
		else if (*cg != 255)
			*outg = *outg + (ing * ina * (*cg) >> 16);
		else
			*outg = *outg + (ing * ina >> 8);

		return;
	}
}

/**
 * Perform "regular" alphablending of a pixel.
 * @param ina, inr, ing, inb: the input pixel, split into its components.
 * @param *outa, *outr, *outg, *outb pointer to the output pixel.
 */

void BlenderNormal::blendPixel(byte ina, byte inr, byte ing, byte inb, byte *outa, byte *outr, byte *outg, byte *outb) {

	if (ina == 0) {
		return;
	} else if (ina == 255) {
		*outb = inb;
		*outg = ing;
		*outr = inr;
		*outa = ina;
		return;
	} else {
		*outa = 255;
		*outb = ((inb * ina) + *outb * (255 - ina)) >> 8;
		*outg = ((ing * ina) + *outg * (255 - ina)) >> 8;
		*outr = ((inr * ina) + *outr * (255 - ina)) >> 8;
	}
}

/**
 * Perform subtractive blending of a pixel.
 * @param ina, inr, ing, inb: the input pixel, split into its components.
 * @param *outa, *outr, *outg, *outb pointer to the output pixel.
 */

void BlenderSubtractive::blendPixel(byte ina, byte inr, byte ing, byte inb, byte *outa, byte *outr, byte *outg, byte *outb) {

	if (ina == 0) {
		return;
	} else if (ina == 255) {
		*outa = *outa;
		*outr = *outr - (inr * (*outr) >> 8);
		*outg = *outg - (ing * (*outg) >> 8);
		*outb = *outb - (inb * (*outb) >> 8);
		return;
	} else {
		*outa = *outa;
		*outb = MAX(*outb - ((inb * (*outb)) * ina >> 16), 0);
		*outg = MAX(*outg - ((ing * (*outg)) * ina >> 16), 0);
		*outr = MAX(*outr - ((inr * (*outr)) * ina >> 16), 0);
		return;
	}
}

/**
 * Perform additive blending of a pixel.
 * @param ina, inr, ing, inb: the input pixel, split into its components.
 * @param *outa, *outr, *outg, *outb pointer to the output pixel.
 */

void BlenderAdditive::blendPixel(byte ina, byte inr, byte ing, byte inb, byte *outa, byte *outr, byte *outg, byte *outb) {

	if (ina == 0) {
		return;
	} else if (ina == 255) {
		*outa = *outa;
		*outr = MIN(*outr + inr, 255);
		*outg = MIN(*outg + ing, 255);
		*outb = MIN(*outb + inb, 255);
		return;
	} else {
		*outa = *outa;
		*outb = MIN((inb * ina >> 8) + *outb, 255);
		*outg = MIN((ing * ina >> 8) + *outg, 255);
		*outr = MIN((inr * ina >> 8) + *outr, 255);
		return;
	}
}

#if ENABLE_BILINEAR
void TransparentSurface::copyPixelBilinear(float projX, float projY, int dstX, int dstY, const Common::Rect &srcRect, const Common::Rect &dstRect, const TransparentSurface *src, TransparentSurface *dst) {

	// TODO: Do some optimization on this. This is completely naive.
	
	int srcW = srcRect.width();
	int srcH = srcRect.height();
	int dstW = dstRect.width();
	int dstH = dstRect.height();

	assert(dstX >= 0 && dstX < dstW);
	assert(dstY >= 0 && dstY < dstH);

	float x1 = floor(projX);
	float x2 = ceil(projX);
	float y1 = floor(projY);
	float y2 = ceil(projY);

	uint32 Q11, Q12, Q21, Q22;

	if (x1 >= srcW || x1 < 0 || y1 >= srcH || y1 < 0) {
		Q11 = 0;
	} else {
		Q11 = READ_UINT32((const byte *)src->getBasePtr((int)(x1 + srcRect.left), (int)(y1 + srcRect.top)));
	}

	if (x1 >= srcW || x1 < 0 || y2 >= srcH || y2 < 0) {
		Q12 = 0;
	} else {
		Q12 = READ_UINT32((const byte *)src->getBasePtr((int)(x1 + srcRect.left), (int)(y2 + srcRect.top)));
	}

	if (x2 >= srcW || x2 < 0 || y1 >= srcH || y1 < 0) {
		Q21 = 0;
	} else {
		Q21 = READ_UINT32((const byte *)src->getBasePtr((int)(x2 + srcRect.left), (int)(y1 + srcRect.top)));
	}

	if (x2 >= srcW || x2 < 0 || y2 >= srcH || y2 < 0) {
		Q22 = 0;
	} else {
		Q22 = READ_UINT32((const byte *)src->getBasePtr((int)(x2 + srcRect.left), (int)(y2 + srcRect.top)));
	}

	byte *Q11s = (byte *)&Q11;
	byte *Q12s = (byte *)&Q12;
	byte *Q21s = (byte *)&Q21;
	byte *Q22s = (byte *)&Q22;

	uint32 color;
	byte *dest = (byte *)&color;

	float q11x = (x2 - projX);
	float q11y = (y2 - projY);
	float q21x = (projX - x1);
	float q21y = (y2 - projY);
	float q12x = (x2 - projX);
	float q12y = (projY - y1);

	if (x1 == x2 && y1 == y2) {
		for (int c = 0; c < 4; c++) {
			dest[c] = ((float)Q11s[c]);
		}
	} else {

		if (x1 == x2) {
			q11x = 0.5;
			q12x = 0.5;
			q21x = 0.5;
		} else if (y1 == y2) {
			q11y = 0.5;
			q12y = 0.5;
			q21y = 0.5;
		}

		for (int c = 0; c < 4; c++) {
			dest[c] = (byte)(
			              ((float)Q11s[c]) * q11x * q11y +
			              ((float)Q21s[c]) * q21x * q21y +
			              ((float)Q12s[c]) * q12x * q12y +
			              ((float)Q22s[c]) * (1.0 -
			                                  q11x * q11y -
			                                  q21x * q21y -
			                                  q12x * q12y)
			          );
		}
	}
	WRITE_UINT32((byte *)dst->getBasePtr(dstX + dstRect.left, dstY + dstRect.top), color);
}
#else
void TransparentSurface::copyPixelNearestNeighbor(float projX, float projY, int dstX, int dstY, const Common::Rect &srcRect, const Common::Rect &dstRect, const TransparentSurface *src, TransparentSurface *dst) {

	// TODO: Have the Rect arguments become completely useless at this point?

	int srcW = srcRect.width();
	int srcH = srcRect.height();
	int dstW = dstRect.width();
	int dstH = dstRect.height();

	assert(dstX >= 0 && dstX < dstW);
	assert(dstY >= 0 && dstY < dstH);

	uint32 color;

	if (projX >= srcW || projX < 0 || projY >= srcH || projY < 0) {
		color = 0;
	} else {
		color = READ_UINT32((const byte *)src->getBasePtr((int)projX, (int)projY));
	}

	WRITE_UINT32((byte *)dst->getBasePtr(dstX, dstY), color);
}
#endif

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

void doBlitOpaqueFast(byte *ino, byte *outo, uint32 width, uint32 height, uint32 pitch, int32 inStep, int32 inoStep) {
/**
 * Optimized version of doBlit to be used w/opaque blitting (no alpha).
 */
	byte *in;
	byte *out;

	for (uint32 i = 0; i < height; i++) {
		out = outo;
		in = ino;
		memcpy(out, in, width * 4);
		for (uint32 j = 0; j < width; j++) {
			out[TransparentSurface::kAIndex] = 0xFF;
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
			int a = (pix >> TransparentSurface::kAShift) & 0xff;

			if (a == 0) { // Full transparency
			} else { // Full opacity (Any value not exactly 0 is Opaque here)
				*(uint32 *)out = pix;
				out[TransparentSurface::kAIndex] = 0xFF;
			}
			out += 4;
			in += inStep;
		}
		outo += pitch;
		ino += inoStep;
	}
}

/**
 * What we have here is a template method that calls blendPixel() from a different
 * class - the one we call it with - thus performing a different type of blending.
 *
 * @param *ino a pointer to the input surface
 * @param *outo a pointer to the output surface
 * @param width width of the input surface
 * @param height height of the input surface
 * @param pitch pitch of the output surface - that is, width in bytes of every row, usually bpp * width of the TARGET surface (the area we are blitting to might be smaller, do the math)
 * @inStep size in bytes to skip to address each pixel, usually bpp of the source surface
 * @inoStep width in bytes of every row on the *input* surface / kind of like pitch
 * @color colormod in 0xAARRGGBB format - 0xFFFFFFFF for no colormod
 */

template<class Blender> 
void doBlit(byte *ino, byte *outo, uint32 width, uint32 height, uint32 pitch, int32 inStep, int32 inoStep, uint32 color) {

	byte *in;
	byte *out;

	if (color == 0xffffffff) {

		for (uint32 i = 0; i < height; i++) {
			out = outo;
			in = ino;
			for (uint32 j = 0; j < width; j++) {

				byte *outa = &out[TransparentSurface::kAIndex];
				byte *outr = &out[TransparentSurface::kRIndex];
				byte *outg = &out[TransparentSurface::kGIndex];
				byte *outb = &out[TransparentSurface::kBIndex];

				Blender::blendPixel(in[TransparentSurface::kAIndex],
					in[TransparentSurface::kRIndex],
					in[TransparentSurface::kGIndex],
					in[TransparentSurface::kBIndex],
					outa, outr, outg, outb);

				in += inStep;
				out += 4;
			}
			outo += pitch;
			ino += inoStep;
		}
	} else {

		byte ca = (color >> TransparentSurface::kAModShift) & 0xFF;
		byte cr = (color >> TransparentSurface::kRModShift) & 0xFF;
		byte cg = (color >> TransparentSurface::kGModShift) & 0xFF;
		byte cb = (color >> TransparentSurface::kBModShift) & 0xFF;

		for (uint32 i = 0; i < height; i++) {
			out = outo;
			in = ino;
			for (uint32 j = 0; j < width; j++) {

				byte *outa = &out[TransparentSurface::kAIndex];
				byte *outr = &out[TransparentSurface::kRIndex];
				byte *outg = &out[TransparentSurface::kGIndex];
				byte *outb = &out[TransparentSurface::kBIndex];

				Blender::blendPixel(in[TransparentSurface::kAIndex],
					in[TransparentSurface::kRIndex],
					in[TransparentSurface::kGIndex],
					in[TransparentSurface::kBIndex],
					outa, outr, outg, outb, &ca, &cr, &cg, &cb);
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
	int ca = (color >> 24) & 0xff;

	if (ca == 0)
		return retSize;

	// Create an encapsulating surface for the data
	TransparentSurface srcImage(*this, false);
	// TODO: Is the data really in the screen format?
	if (format.bytesPerPixel != 4) {
		warning("TransparentSurface can only blit 32 bpp images");
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

	if (width == -1)
		width = srcImage.w;
	if (height == -1)
		height = srcImage.h;

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
		img->setPixels((byte *)img->getBasePtr(0, -posY));
		posY = 0;
	}

	if (posX < 0) {
		img->w = MAX(0, (int)img->w - -posX);
		img->setPixels((byte *)img->getBasePtr(-posX, 0));
		posX = 0;
	}

	img->w = CLIP((int)img->w, 0, (int)MAX((int)target.w - posX, 0));
	img->h = CLIP((int)img->h, 0, (int)MAX((int)target.h - posY, 0));

	if ((img->w > 0) && (img->h > 0)) {
		int xp = 0, yp = 0;

		int inStep = 4;
		int inoStep = img->pitch;
		if (flipping & TransparentSurface::FLIP_H) {
			inStep = -inStep;
			xp = img->w - 1;
		}

		if (flipping & TransparentSurface::FLIP_V) {
			inoStep = -inoStep;
			yp = img->h - 1;
		}

		byte *ino= (byte *)img->getBasePtr(xp, yp);
		byte *outo = (byte *)target.getBasePtr(posX, posY);

		if (color == 0xFFFFFF && blendMode == BLEND_NORMAL && _alphaMode == ALPHA_OPAQUE) {
			doBlitOpaqueFast(ino, outo, img->w, img->h, target.pitch, inStep, inoStep);
		} else if (color == 0xFFFFFF && blendMode == BLEND_NORMAL && _alphaMode == ALPHA_BINARY) {
			doBlitBinaryFast(ino, outo, img->w, img->h, target.pitch, inStep, inoStep);
		} else {
			if (blendMode == BLEND_ADDITIVE) {
				doBlit<BlenderAdditive>(ino, outo, img->w, img->h, target.pitch, inStep, inoStep, color);
			} else if (blendMode == BLEND_SUBTRACTIVE) {
				doBlit<BlenderSubtractive>(ino, outo, img->w, img->h, target.pitch, inStep, inoStep, color);
			} else {
				assert(blendMode == BLEND_NORMAL);
				doBlit<BlenderNormal>(ino, outo, img->w, img->h, target.pitch, inStep, inoStep, color);
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

TransparentSurface *TransparentSurface::rotoscale(const TransformStruct &transform) const {

	assert(transform._angle != 0); // This would not be ideal; rotoscale() should never be called in conditional branches where angle = 0 anyway.

	Point32 newHotspot;
	Common::Rect srcRect(0, 0, (int16)w, (int16)h);
	Rect32 rect = TransformTools::newRect(Rect32(srcRect), transform, &newHotspot);
	Common::Rect dstRect(0, 0, (int16)(rect.right - rect.left), (int16)(rect.bottom - rect.top));

	TransparentSurface *target = new TransparentSurface();
	assert(format.bytesPerPixel == 4);

	int dstW = dstRect.width();
	int dstH = dstRect.height();

	target->create((uint16)dstW, (uint16)dstH, this->format);

	uint32 invAngle = 360 - (transform._angle % 360);
	float invCos = cos(invAngle * M_PI / 180.0);
	float invSin = sin(invAngle * M_PI / 180.0);
	float targX;
	float targY;

	for (int y = 0; y < dstH; y++) {
		for (int x = 0; x < dstW; x++) {
			int x1 = x - newHotspot.x;
			int y1 = y - newHotspot.y;

			targX = ((x1 * invCos - y1 * invSin)) * kDefaultZoomX / transform._zoom.x + srcRect.left;
			targY = ((x1 * invSin + y1 * invCos)) * kDefaultZoomY / transform._zoom.y + srcRect.top;

			targX += transform._hotspot.x;
			targY += transform._hotspot.y;

#if ENABLE_BILINEAR
			copyPixelBilinear(targX, targY, x, y, srcRect, dstRect, this, target);
#else
			copyPixelNearestNeighbor(targX, targY, x, y, srcRect, dstRect, this, target);
#endif
		}
	}
	return target;
}

TransparentSurface *TransparentSurface::scale(uint16 newWidth, uint16 newHeight) const {

	Common::Rect srcRect(0, 0, (int16)w, (int16)h);
	Common::Rect dstRect(0, 0, (int16)newWidth, (int16)newHeight);

	TransparentSurface *target = new TransparentSurface();

	assert(format.bytesPerPixel == 4);

	int srcW = srcRect.width();
	int srcH = srcRect.height();
	int dstW = dstRect.width();
	int dstH = dstRect.height();

	target->create((uint16)dstW, (uint16)dstH, this->format);


	float projX;
	float projY;
	for (int y = 0; y < dstH; y++) {
		for (int x = 0; x < dstW; x++) {
			projX = x / (float)dstW * srcW;
			projY = y / (float)dstH * srcH;
#if ENABLE_BILINEAR
			copyPixelBilinear(projX, projY, x, y, srcRect, dstRect, this, target);
#else
			copyPixelNearestNeighbor(projX, projY, x, y, srcRect, dstRect, this, target);
#endif
		}
	}
	return target;

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

} // End of namespace Wintermute
