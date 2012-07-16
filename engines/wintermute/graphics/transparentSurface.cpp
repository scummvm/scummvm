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
#include "common/textconsole.h"
#include "graphics/primitives.h"
#include "engines/wintermute/graphics/transparentSurface.h"

namespace WinterMute {

TransparentSurface::TransparentSurface() : Surface() {}

TransparentSurface::TransparentSurface(const Surface &surf, bool copyData) : Surface() {
	if (copyData) {
		copyFrom(surf);
	} else {
		w = surf.w;
		h = surf.h;
		pitch = surf.pitch;
		format = surf.format;
		pixels = surf.pixels;
	}
}

void doBlit(byte *ino, byte* outo, uint32 width, uint32 height, uint32 pitch, int32 inStep, int32 inoStep) {
	byte *in, *out;
	
	const int bShift = 8;//img->format.bShift;
	const int gShift = 16;//img->format.gShift;
	const int rShift = 24;//img->format.rShift;
	const int aShift = 0;//img->format.aShift;
	
	const int bShiftTarget = 8;//target.format.bShift;
	const int gShiftTarget = 16;//target.format.gShift;
	const int rShiftTarget = 24;//target.format.rShift;
	
	for (int i = 0; i < height; i++) {
		out = outo;
		in = ino;
		for (int j = 0; j < width; j++) {
			uint32 pix = *(uint32 *)in;
			uint32 o_pix = *(uint32 *) out;
			int b = (pix >> bShift) & 0xff;
			int g = (pix >> gShift) & 0xff;
			int r = (pix >> rShift) & 0xff;
			int a = (pix >> aShift) & 0xff;
			int o_b, o_g, o_r, o_a;
			in += inStep;
			
	/*		if (ca != 255) {
				a = a * ca >> 8;
			}*/
			
			switch (a) {
				case 0: // Full transparency
					out += 4;
					break;
				case 255: // Full opacity
					o_b = b;
					o_g = g;
					o_r = r;
					o_a = a;
					//*(uint32 *)out = target.format.ARGBToColor(o_a, o_r, o_g, o_b);
					out[0] = o_a;
					out[1] = o_b;
					out[2] = o_g;
					out[3] = o_r;
					out += 4;
					break;
					
				default: // alpha blending
					o_a = 255;
					o_b = (o_pix >> bShiftTarget) & 0xff;
					o_g = (o_pix >> gShiftTarget) & 0xff;
					o_r = (o_pix >> rShiftTarget) & 0xff;
					o_b += ((b - o_b) * a) >> 8;
					o_g += ((g - o_g) * a) >> 8;
					o_r += ((r - o_r) * a) >> 8;
					//*(uint32 *)out = target.format.ARGBToColor(o_a, o_r, o_g, o_b);
					out[0] = o_a;
					out[1] = o_b;
					out[2] = o_g;
					out[3] = o_r;
					out += 4;
			}
		}
		outo += pitch;
		ino += inoStep;
	}
}


Common::Rect TransparentSurface::blit(Graphics::Surface &target, int posX, int posY, int flipping, Common::Rect *pPartRect, uint color, int width, int height) {
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
		srcImage.pixels = &((char *)pixels)[pPartRect->top * srcImage.pitch + pPartRect->left * 4];
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

	Graphics::Surface *img;
	Graphics::Surface *imgScaled = NULL;
	byte *savedPixels = NULL;
	if ((width != srcImage.w) || (height != srcImage.h)) {
		// Scale the image
		img = imgScaled = srcImage.scale(width, height);
		savedPixels = (byte *)img->pixels;
	} else {
		img = &srcImage;
	}

	// Handle off-screen clipping
	if (posY < 0) {
		img->h = MAX(0, (int)img->h - -posY);
		img->pixels = (byte *)img->pixels + img->pitch * -posY;
		posY = 0;
	}

	if (posX < 0) {
		img->w = MAX(0, (int)img->w - -posX);
		img->pixels = (byte *)img->pixels + (-posX * 4);
		posX = 0;
	}

	img->w = CLIP((int)img->w, 0, (int)MAX((int)target.w - posX, 0));
	img->h = CLIP((int)img->h, 0, (int)MAX((int)target.h - posY, 0));

	if ((img->w > 0) && (img->h > 0)) {
		int xp = 0, yp = 0;

		int inStep = 4;
		int inoStep = img->pitch;
		if (flipping & TransparentSurface::FLIP_V) {
			inStep = -inStep;
			xp = img->w - 1;
		}

		if (flipping & TransparentSurface::FLIP_H) {
			inoStep = -inoStep;
			yp = img->h - 1;
		}

		byte *ino = (byte *)img->getBasePtr(xp, yp);
		byte *outo = (byte *)target.getBasePtr(posX, posY);
		byte *in, *out;

		const int bShift = 8;//img->format.bShift;
		const int gShift = 16;//img->format.gShift;
		const int rShift = 24;//img->format.rShift;
		const int aShift = 0;//img->format.aShift;

		const int bShiftTarget = 8;//target.format.bShift;
		const int gShiftTarget = 16;//target.format.gShift;
		const int rShiftTarget = 24;//target.format.rShift;

		if (ca == 255 && cb == 255 && cg == 255 && cr == 255) {
			doBlit(ino, outo, img->w, img->h, target.pitch, inStep, inoStep);
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
					int o_b, o_g, o_r, o_a;
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
							o_b = (b * cb) >> 8;
						else
							o_b = b;

						if (cg != 255)
							o_g = (g * cg) >> 8;
						else
							o_g = g;

						if (cr != 255)
							o_r = (r * cr) >> 8;
						else
							o_r = r;
						o_a = a;
						//*(uint32 *)out = target.format.ARGBToColor(o_a, o_r, o_g, o_b);
						out[0] = o_a;
						out[1] = o_b;
						out[2] = o_g;
						out[3] = o_r;
						out += 4;
						break;

					default: // alpha blending
						o_a = 255;
						o_b = (o_pix >> bShiftTarget) & 0xff;
						o_g = (o_pix >> gShiftTarget) & 0xff;
						o_r = (o_pix >> rShiftTarget) & 0xff;
						if (cb == 0)
							o_b = 0;
						else if (cb != 255)
							o_b += ((b - o_b) * a * cb) >> 16;
						else
							o_b += ((b - o_b) * a) >> 8;
						if (cg == 0)
							o_g = 0;
						else if (cg != 255)
							o_g += ((g - o_g) * a * cg) >> 16;
						else
							o_g += ((g - o_g) * a) >> 8;
						if (cr == 0)
							o_r = 0;
						else if (cr != 255)
							o_r += ((r - o_r) * a * cr) >> 16;
						else
							o_r += ((r - o_r) * a) >> 8;
						//*(uint32 *)out = target.format.ARGBToColor(o_a, o_r, o_g, o_b);
						out[0] = o_a;
						out[1] = o_b;
						out[2] = o_g;
						out[3] = o_r;
						out += 4;
					}
				}
				outo += target.pitch;
				ino += inoStep;
			}
		}
	}

	if (imgScaled) {
		imgScaled->pixels = savedPixels;
		imgScaled->free();
		delete imgScaled;
	}

	retSize.setWidth(img->w);
	retSize.setHeight(img->h);
	return retSize;
}

/**
 * Scales a passed surface, creating a new surface with the result
 * @param srcImage      Source image to scale
 * @param scaleFactor   Scale amount. Must be between 0 and 1.0 (but not zero)
 * @remarks Caller is responsible for freeing the returned surface
 */
TransparentSurface *TransparentSurface::scale(int xSize, int ySize) const {
	TransparentSurface *s = new TransparentSurface();
	s->create(xSize, ySize, this->format);

	int *horizUsage = scaleLine(xSize, this->w);
	int *vertUsage = scaleLine(ySize, this->h);

	// Loop to create scaled version
	for (int yp = 0; yp < ySize; ++yp) {
		const byte *srcP = (const byte *)this->getBasePtr(0, vertUsage[yp]);
		byte *destP = (byte *)s->getBasePtr(0, yp);

		for (int xp = 0; xp < xSize; ++xp) {
			const byte *tempSrcP = srcP + (horizUsage[xp] * this->format.bytesPerPixel);
			for (int byteCtr = 0; byteCtr < this->format.bytesPerPixel; ++byteCtr) {
				*destP++ = *tempSrcP++;
			}
		}
	}

	// Delete arrays and return surface
	delete[] horizUsage;
	delete[] vertUsage;
	return s;
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
 * Returns an array indicating which pixels of a source image horizontally or vertically get
 * included in a scaled image
 */
int *TransparentSurface::scaleLine(int size, int srcSize) {
	int scale = 100 * size / srcSize;
	assert(scale > 0);
	int *v = new int[size];
	Common::fill(v, &v[size], 0);

	int distCtr = 0;
	int *destP = v;
	for (int distIndex = 0; distIndex < srcSize; ++distIndex) {
		distCtr += scale;
		while (distCtr >= 100) {
			assert(destP < &v[size]);
			*destP++ = distIndex;
			distCtr -= 100;
		}
	}

	return v;
}


} // End of namespace Graphics
