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


#if ENABLE_BILINEAR
void TransparentSurface::copyPixelBilinear(float projX, float projY, int dstX, int dstY, const Common::Rect &srcRect, const Common::Rect &dstRect, const TransparentSurface *src, TransparentSurface *dst) {
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

TransparentSurface::TransparentSurface() : Surface(), _enableAlphaBlit(true) {}

TransparentSurface::TransparentSurface(const Surface &surf, bool copyData) : Surface(), _enableAlphaBlit(true) {
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

void doBlitOpaque(byte *ino, byte *outo, uint32 width, uint32 height, uint32 pitch, int32 inStep, int32 inoStep) {
	byte *in, *out;

#ifdef SCUMM_LITTLE_ENDIAN
	const int aIndex = 3;
#else
	const int aIndex = 0;
#endif

	for (uint32 i = 0; i < height; i++) {
		out = outo;
		in = ino;
		memcpy(out, in, width * 4);
		for (uint32 j = 0; j < width; j++) {
			out[aIndex] = 0xFF;
			out += 4;
		}
		outo += pitch;
		ino += inoStep;
	}
}

void TransparentSurface::doBlitAlpha(byte *ino, byte *outo, uint32 width, uint32 height, uint32 pitch, int32 inStep, int32 inoStep, TSpriteBlendMode blendMode) {
	byte *in, *out;

#ifdef SCUMM_LITTLE_ENDIAN
	const int aIndex = 3;
	const int bIndex = 0;
	const int gIndex = 1;
	const int rIndex = 2;
#else
	const int aIndex = 0;
	const int bIndex = 3;
	const int gIndex = 2;
	const int rIndex = 1;
#endif

	const int bShift = 0;//img->format.bShift;
	const int gShift = 8;//img->format.gShift;
	const int rShift = 16;//img->format.rShift;
	const int aShift = 24;//img->format.aShift;

	const int bShiftTarget = 0;//target.format.bShift;
	const int gShiftTarget = 8;//target.format.gShift;
	const int rShiftTarget = 16;//target.format.rShift;

	for (uint32 i = 0; i < height; i++) {
		out = outo;
		in = ino;
		for (uint32 j = 0; j < width; j++) {
			uint32 pix = *(uint32 *)in;
			uint32 oPix = *(uint32 *) out;
			int b = (pix >> bShift) & 0xff;
			int g = (pix >> gShift) & 0xff;
			int r = (pix >> rShift) & 0xff;
			int a = (pix >> aShift) & 0xff;
			int outb, outg, outr, outa;
			in += inStep;

			switch (a) {
				case 0: // Full transparency
					out += 4;
					break;
				case 255: // Full opacity
					outb = b;
					outg = g;
					outr = r;
					outa = a;

					out[aIndex] = outa;
					out[bIndex] = outb;
					out[gIndex] = outg;
					out[rIndex] = outr;
					out += 4;
					break;

				default: // alpha blending
					// TODO: turn special case into something more fleshed out
					if (blendMode == BLEND_ADDITIVE) {
						outa = a;
						outb = ((b * 255) + ((oPix >> bShiftTarget) & 0xff) * (255-a)) >> 8;
						outg = ((g * 255) + ((oPix >> gShiftTarget) & 0xff) * (255-a)) >> 8;
						outr = ((r * 255) + ((oPix >> rShiftTarget) & 0xff) * (255-a)) >> 8;

						out[aIndex] = outa;
						out[bIndex] = outb;
						out[gIndex] = outg;
						out[rIndex] = outr;
						out += 4;
					} else {
						outa = 255;
						outb = ((b * a) + ((oPix >> bShiftTarget) & 0xff) * (255-a)) >> 8;
						outg = ((g * a) + ((oPix >> gShiftTarget) & 0xff) * (255-a)) >> 8;
						outr = ((r * a) + ((oPix >> rShiftTarget) & 0xff) * (255-a)) >> 8;

						out[aIndex] = outa;
						out[bIndex] = outb;
						out[gIndex] = outg;
						out[rIndex] = outr;
						out += 4;
					}
			}
		}
		outo += pitch;
		ino += inoStep;
	}
}


Common::Rect TransparentSurface::blit(Graphics::Surface &target, int posX, int posY, int flipping, Common::Rect *pPartRect, uint color, int width, int height, TSpriteBlendMode blendMode) {
	int ca = (color >> 24) & 0xff;

	Common::Rect retSize;
	retSize.top = 0;
	retSize.left = 0;
	retSize.setWidth(0);
	retSize.setHeight(0);
	// Check if we need to draw anything at all
	if (ca == 0)
		return retSize;

	int cr = (color >> 16) & 0xff;
	int cg = (color >> 8) & 0xff;
	int cb = (color >> 0) & 0xff;

	// Compensate for transparency. Since we're coming
	// down to 255 alpha, we just compensate for the colors here
	if (ca != 255) {
		cr = cr * ca >> 8;
		cg = cg * ca >> 8;
		cb = cb * ca >> 8;
	}

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

		byte *ino = (byte *)img->getBasePtr(xp, yp);
		byte *outo = (byte *)target.getBasePtr(posX, posY);
		byte *in, *out;

#ifdef SCUMM_LITTLE_ENDIAN
		const int aIndex = 3;
		const int bIndex = 0;
		const int gIndex = 1;
		const int rIndex = 2;
#else
		const int aIndex = 0;
		const int bIndex = 3;
		const int gIndex = 2;
		const int rIndex = 1;
#endif
		const int bShift = 0;//img->format.bShift;
		const int gShift = 8;//img->format.gShift;
		const int rShift = 16;//img->format.rShift;
		const int aShift = 24;//img->format.aShift;

		const int bShiftTarget = 0;//target.format.bShift;
		const int gShiftTarget = 8;//target.format.gShift;
		const int rShiftTarget = 16;//target.format.rShift;

		if (ca == 255 && cb == 255 && cg == 255 && cr == 255) {
			if (_enableAlphaBlit) {
				doBlitAlpha(ino, outo, img->w, img->h, target.pitch, inStep, inoStep, blendMode);
			} else {
				doBlitOpaque(ino, outo, img->w, img->h, target.pitch, inStep, inoStep);
			}
		} else {
			for (int i = 0; i < img->h; i++) {
				out = outo;
				in = ino;
				for (int j = 0; j < img->w; j++) {
					uint32 pix = *(uint32 *)in;
					uint32 o_pix = *(uint32 *) out;
					int b = (pix >> bShift) & 0xff;
					int g = (pix >> gShift) & 0xff;
					int r = (pix >> rShift) & 0xff;
					int a = (pix >> aShift) & 0xff;
					int outb, outg, outr, outa;
					in += inStep;

					if (ca != 255) {
						a = a * ca >> 8;
					}

					switch (a) {
					case 0: // Full transparency
						out += 4;
						break;
					case 255: // Full opacity
						if (cb != 255)
							outb = (b * cb) >> 8;
						else
							outb = b;

						if (cg != 255)
							outg = (g * cg) >> 8;
						else
							outg = g;

						if (cr != 255)
							outr = (r * cr) >> 8;
						else
							outr = r;
						outa = a;
						out[aIndex] = outa;
						out[bIndex] = outb;
						out[gIndex] = outg;
						out[rIndex] = outr;
						out += 4;
						break;

					default: // alpha blending
						outa = 255;
						outb = (o_pix >> bShiftTarget) & 0xff;
						outg = (o_pix >> gShiftTarget) & 0xff;
						outr = (o_pix >> rShiftTarget) & 0xff;
						if (cb == 0)
							outb = 0;
						else if (cb != 255)
							outb += ((b - outb) * a * cb) >> 16;
						else
							outb += ((b - outb) * a) >> 8;
						if (cg == 0)
							outg = 0;
						else if (cg != 255)
							outg += ((g - outg) * a * cg) >> 16;
						else
							outg += ((g - outg) * a) >> 8;
						if (cr == 0)
							outr = 0;
						else if (cr != 255)
							outr += ((r - outr) * a * cr) >> 16;
						else
							outr += ((r - outr) * a) >> 8;
						out[aIndex] = outa;
						out[bIndex] = outb;
						out[gIndex] = outg;
						out[rIndex] = outr;
						out += 4;
					}
				}
				outo += target.pitch;
				ino += inoStep;
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
