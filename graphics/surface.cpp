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
#include "graphics/surface.h"
#include "graphics/conversion.h"

namespace Graphics {

template<typename T>
static void plotPoint(int x, int y, int color, void *data) {
	Surface *s = (Surface *)data;
	if (x >= 0 && x < s->w && y >= 0 && y < s->h) {
		T *ptr = (T *)s->getBasePtr(x, y);
		*ptr = (T)color;
	}
}

void Surface::drawLine(int x0, int y0, int x1, int y1, uint32 color) {
	if (format.bytesPerPixel == 1)
		Graphics::drawLine(x0, y0, x1, y1, color, plotPoint<byte>, this);
	else if (format.bytesPerPixel == 2)
		Graphics::drawLine(x0, y0, x1, y1, color, plotPoint<uint16>, this);
	else if (format.bytesPerPixel == 4)
		Graphics::drawLine(x0, y0, x1, y1, color, plotPoint<uint32>, this);
	else
		error("Surface::drawLine: bytesPerPixel must be 1, 2, or 4");
}

void Surface::drawThickLine(int x0, int y0, int x1, int y1, int penX, int penY, uint32 color) {
	if (format.bytesPerPixel == 1)
		Graphics::drawThickLine(x0, y0, x1, y1, penX, penY, color, plotPoint<byte>, this);
	else if (format.bytesPerPixel == 2)
		Graphics::drawThickLine(x0, y0, x1, y1, penX, penY, color, plotPoint<uint16>, this);
	else if (format.bytesPerPixel == 4)
		Graphics::drawThickLine(x0, y0, x1, y1, penX, penY, color, plotPoint<uint32>, this);
	else
		error("Surface::drawThickLine: bytesPerPixel must be 1, 2, or 4");
}

void Surface::create(uint16 width, uint16 height, const PixelFormat &f) {
	free();

	w = width;
	h = height;
	format = f;
	pitch = w * format.bytesPerPixel;

	if (width && height) {
		pixels = calloc(width * height, format.bytesPerPixel);
		assert(pixels);
	}
}

void Surface::free() {
	::free(pixels);
	pixels = 0;
	w = h = pitch = 0;
	format = PixelFormat();
}

void Surface::copyFrom(const Surface &surf) {
	create(surf.w, surf.h, surf.format);
	memcpy(pixels, surf.pixels, h * pitch);
}

void Surface::hLine(int x, int y, int x2, uint32 color) {
	// Clipping
	if (y < 0 || y >= h)
		return;

	if (x2 < x)
		SWAP(x2, x);

	if (x < 0)
		x = 0;
	if (x2 >= w)
		x2 = w - 1;

	if (x2 < x)
		return;

	if (format.bytesPerPixel == 1) {
		byte *ptr = (byte *)getBasePtr(x, y);
		memset(ptr, (byte)color, x2 - x + 1);
	} else if (format.bytesPerPixel == 2) {
		uint16 *ptr = (uint16 *)getBasePtr(x, y);
		Common::fill(ptr, ptr + (x2 - x + 1), (uint16)color);
	} else if (format.bytesPerPixel == 4) {
		uint32 *ptr = (uint32 *)getBasePtr(x, y);
		Common::fill(ptr, ptr + (x2 - x + 1), color);
	} else {
		error("Surface::hLine: bytesPerPixel must be 1, 2, or 4");
	}
}

void Surface::vLine(int x, int y, int y2, uint32 color) {
	// Clipping
	if (x < 0 || x >= w)
		return;

	if (y2 < y)
		SWAP(y2, y);

	if (y < 0)
		y = 0;
	if (y2 >= h)
		y2 = h - 1;

	if (format.bytesPerPixel == 1) {
		byte *ptr = (byte *)getBasePtr(x, y);
		while (y++ <= y2) {
			*ptr = (byte)color;
			ptr += pitch;
		}
	} else if (format.bytesPerPixel == 2) {
		uint16 *ptr = (uint16 *)getBasePtr(x, y);
		while (y++ <= y2) {
			*ptr = (uint16)color;
			ptr += pitch / 2;
		}

	} else if (format.bytesPerPixel == 4) {
		uint32 *ptr = (uint32 *)getBasePtr(x, y);
		while (y++ <= y2) {
			*ptr = color;
			ptr += pitch / 4;
		}
	} else {
		error("Surface::vLine: bytesPerPixel must be 1, 2, or 4");
	}
}

void Surface::fillRect(Common::Rect r, uint32 color) {
	r.clip(w, h);

	if (!r.isValidRect())
		return;

	int width = r.width();
	int lineLen = width;
	int height = r.height();
	bool useMemset = true;

	if (format.bytesPerPixel == 2) {
		lineLen *= 2;
		if ((uint16)color != ((color & 0xff) | (color & 0xff) << 8))
			useMemset = false;
	} else if (format.bytesPerPixel == 4) {
		useMemset = false;
	} else if (format.bytesPerPixel != 1) {
		error("Surface::fillRect: bytesPerPixel must be 1, 2, or 4");
	}

	if (useMemset) {
		byte *ptr = (byte *)getBasePtr(r.left, r.top);
		while (height--) {
			memset(ptr, (byte)color, lineLen);
			ptr += pitch;
		}
	} else {
		if (format.bytesPerPixel == 2) {
			uint16 *ptr = (uint16 *)getBasePtr(r.left, r.top);
			while (height--) {
				Common::fill(ptr, ptr + width, (uint16)color);
				ptr += pitch / 2;
			}
		} else {
			uint32 *ptr = (uint32 *)getBasePtr(r.left, r.top);
			while (height--) {
				Common::fill(ptr, ptr + width, color);
				ptr += pitch / 4;
			}
		}
	}
}

void Surface::frameRect(const Common::Rect &r, uint32 color) {
	hLine(r.left, r.top, r.right - 1, color);
	hLine(r.left, r.bottom - 1, r.right - 1, color);
	vLine(r.left, r.top, r.bottom - 1, color);
	vLine(r.right - 1, r.top, r.bottom - 1, color);
}

void Surface::move(int dx, int dy, int height) {
	// Short circuit check - do we have to do anything anyway?
	if ((dx == 0 && dy == 0) || height <= 0)
		return;

	if (format.bytesPerPixel != 1 && format.bytesPerPixel != 2 && format.bytesPerPixel != 4)
		error("Surface::move: bytesPerPixel must be 1, 2, or 4");

	byte *src, *dst;
	int x, y;

	// vertical movement
	if (dy > 0) {
		// move down - copy from bottom to top
		dst = (byte *)pixels + (height - 1) * pitch;
		src = dst - dy * pitch;
		for (y = dy; y < height; y++) {
			memcpy(dst, src, pitch);
			src -= pitch;
			dst -= pitch;
		}
	} else if (dy < 0) {
		// move up - copy from top to bottom
		dst = (byte *)pixels;
		src = dst - dy * pitch;
		for (y = -dy; y < height; y++) {
			memcpy(dst, src, pitch);
			src += pitch;
			dst += pitch;
		}
	}

	// horizontal movement
	if (dx > 0) {
		// move right - copy from right to left
		dst = (byte *)pixels + (pitch - format.bytesPerPixel);
		src = dst - (dx * format.bytesPerPixel);
		for (y = 0; y < height; y++) {
			for (x = dx; x < w; x++) {
				if (format.bytesPerPixel == 1) {
					*dst-- = *src--;
				} else if (format.bytesPerPixel == 2) {
					*(uint16 *)dst = *(const uint16 *)src;
					src -= 2;
					dst -= 2;
				} else if (format.bytesPerPixel == 4) {
					*(uint32 *)dst = *(const uint32 *)src;
					src -= 4;
					dst -= 4;
				}
			}
			src += pitch + (pitch - dx * format.bytesPerPixel);
			dst += pitch + (pitch - dx * format.bytesPerPixel);
		}
	} else if (dx < 0)  {
		// move left - copy from left to right
		dst = (byte *)pixels;
		src = dst - (dx * format.bytesPerPixel);
		for (y = 0; y < height; y++) {
			for (x = -dx; x < w; x++) {
				if (format.bytesPerPixel == 1) {
					*dst++ = *src++;
				} else if (format.bytesPerPixel == 2) {
					*(uint16 *)dst = *(const uint16 *)src;
					src += 2;
					dst += 2;
				} else if (format.bytesPerPixel == 4) {
					*(uint32 *)dst = *(const uint32 *)src;
					src += 4;
					dst += 4;
				}
			}
			src += pitch - (pitch + dx * format.bytesPerPixel);
			dst += pitch - (pitch + dx * format.bytesPerPixel);
		}
	}
}

void Surface::convertToInPlace(const PixelFormat &dstFormat, const byte *palette) {
	// Do not convert to the same format and ignore empty surfaces.
	if (format == dstFormat || pixels == 0) {
		return;
	}

	if (format.bytesPerPixel == 0 || format.bytesPerPixel > 4)
		error("Surface::convertToInPlace(): Can only convert from 1Bpp, 2Bpp, 3Bpp, and 4Bpp");

	if (dstFormat.bytesPerPixel != 2 && dstFormat.bytesPerPixel != 4)
		error("Surface::convertToInPlace(): Can only convert to 2Bpp and 4Bpp");

	// In case the surface data needs more space allocate it.
	if (dstFormat.bytesPerPixel > format.bytesPerPixel) {
		void *const newPixels = realloc(pixels, w * h * dstFormat.bytesPerPixel);
		if (!newPixels) {
			error("Surface::convertToInPlace(): Out of memory");
		}
		pixels = newPixels;
	}

	// We take advantage of the fact that pitch is always w * format.bytesPerPixel.
	// This is assured by the logic of Surface::create.

	// We need to handle 1 Bpp surfaces special here.
	if (format.bytesPerPixel == 1) {
		assert(palette);

		for (int y = h; y > 0; --y) {
			const byte *srcRow = (const byte *)pixels + y * pitch - 1;
			byte *dstRow = (byte *)pixels + y * w * dstFormat.bytesPerPixel - dstFormat.bytesPerPixel;

			for (int x = 0; x < w; x++) {
				byte index = *srcRow--;
				byte r = palette[index * 3];
				byte g = palette[index * 3 + 1];
				byte b = palette[index * 3 + 2];

				uint32 color = dstFormat.RGBToColor(r, g, b);

				if (dstFormat.bytesPerPixel == 2)
					*((uint16 *)dstRow) = color;
				else
					*((uint32 *)dstRow) = color;

				dstRow -= dstFormat.bytesPerPixel;
			}
		}
	} else {
		crossBlit((byte *)pixels, (const byte *)pixels, w * dstFormat.bytesPerPixel, pitch, w, h, dstFormat, format);
	}

	// In case the surface data got smaller, free up some memory.
	if (dstFormat.bytesPerPixel < format.bytesPerPixel) {
		void *const newPixels = realloc(pixels, w * h * dstFormat.bytesPerPixel);
		if (!newPixels) {
			error("Surface::convertToInPlace(): Freeing memory failed");
		}
		pixels = newPixels;
	}

	// Update the surface specific data.
	format = dstFormat;
	pitch = w * dstFormat.bytesPerPixel;
}

Graphics::Surface *Surface::convertTo(const PixelFormat &dstFormat, const byte *palette) const {
	assert(pixels);

	Graphics::Surface *surface = new Graphics::Surface();

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

/*
 * The following code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

Common::Rect Surface::blit(Graphics::Surface &target, int posX, int posY, int flipping, const Common::Rect *pPartRect, uint color, int width, int height) const {
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
	Graphics::Surface srcImage;
	srcImage.w = w;
	srcImage.h = h;
	srcImage.pitch = pitch;
	srcImage.pixels = pixels;
	srcImage.format = format;

	// TODO: Is the data really in the screen format?
	if (format.bytesPerPixel != 4) {
		error("TransparentSurface can only blit 32 bpp images");
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
		if (flipping & Surface::FLIP_V) {
			inStep = -inStep;
			xp = img->w - 1;
		}

		if (flipping & Surface::FLIP_H) {
			inoStep = -inoStep;
			yp = img->h - 1;
		}

		byte *ino = (byte *)img->getBasePtr(xp, yp);
		byte *outo = (byte *)target.getBasePtr(posX, posY);
		byte *in, *out;

		// Hardcoded to the pixelFormat used by the PNG-decoder
		// Variables kept intact for future templatization.
		const int bShift = 16;
		const int gShift = 8;
		const int rShift = 0;
		const int aShift = 24;

		const int bShiftTarget = 16;
		const int gShiftTarget = 8;
		const int rShiftTarget = 0;

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
						*(uint32 *)out = target.format.ARGBToColor(o_a, o_r, o_g, o_b);
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
						*(uint32 *)out = target.format.ARGBToColor(o_a, o_r, o_g, o_b);
						out += 4;
				}
			}
			outo += target.pitch;
			ino += inoStep;
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

Graphics::Surface *Surface::scale(int xSize, int ySize) const {
	Graphics::Surface *s = new Graphics::Surface();
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

void Surface::applyColorKey(uint8 rKey, uint8 gKey, uint8 bKey, bool overwriteAlpha) {
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
int *Surface::scaleLine(int size, int srcSize) {
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
