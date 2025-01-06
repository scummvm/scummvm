/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/algorithm.h"
#include "common/endian.h"
#include "common/memory.h"
#include "common/util.h"
#include "common/rect.h"
#include "common/textconsole.h"
#include "graphics/blit.h"
#include "graphics/palette.h"
#include "graphics/primitives.h"
#include "graphics/surface.h"
#include "graphics/transform_tools.h"

namespace Graphics {

template<typename T>
class SurfacePrimitives final : public Primitives {
public:
        void drawPoint(int x, int y, uint32 color, void *data) override {
		Surface *s = (Surface *)data;
		if (x >= 0 && x < s->w && y >= 0 && y < s->h) {
			T *ptr = (T *)s->getBasePtr(x, y);
			*ptr = (T)color;
		}
	}

        void drawHLine(int x1, int x2, int y, uint32 color, void *data) override {
		Surface *s = (Surface *)data;
		s->hLine(x1, y, x2, color);
	}

        void drawVLine(int x, int y1, int y2, uint32 color, void *data) override {
		Surface *s = (Surface *)data;
		s->vLine(x, y1, y2, color);
	}

	void drawFilledRect(const Common::Rect &rect, uint32 color, void *data) override {
		Surface *s = (Surface *)data;
		s->fillRect(rect, color);
	}

	void drawFilledRect1(const Common::Rect &rect, uint32 color, void *data) override {
		Common::Rect r(rect.left, rect.top, rect.right + 1, rect.bottom + 1);

		Surface *s = (Surface *)data;
		s->fillRect(r, color);
	}
};

void Surface::drawLine(int x0, int y0, int x1, int y1, uint32 color) {
	if (format.bytesPerPixel == 1) {
		SurfacePrimitives<byte> primitives;
		primitives.drawLine(x0, y0, x1, y1, color, this);
	} else if (format.bytesPerPixel == 2) {
		SurfacePrimitives<uint16> primitives;
		primitives.drawLine(x0, y0, x1, y1, color, this);
	} else if (format.bytesPerPixel == 4) {
		SurfacePrimitives<uint32> primitives;
		primitives.drawLine(x0, y0, x1, y1, color, this);
	} else
		error("Surface::drawLine: bytesPerPixel must be 1, 2, or 4, got %d", format.bytesPerPixel);
}

void Surface::drawThickLine(int x0, int y0, int x1, int y1, int penX, int penY, uint32 color) {
	if (format.bytesPerPixel == 1) {
		SurfacePrimitives<byte> primitives;
		primitives.drawThickLine(x0, y0, x1, y1, penX, penY, color, this);
	} else if (format.bytesPerPixel == 2) {
		SurfacePrimitives<uint16> primitives;
		primitives.drawThickLine(x0, y0, x1, y1, penX, penY, color, this);
	} else if (format.bytesPerPixel == 4) {
		SurfacePrimitives<uint32> primitives;
		primitives.drawThickLine(x0, y0, x1, y1, penX, penY, color, this);
	} else
		error("Surface::drawThickLine: bytesPerPixel must be 1, 2, or 4, got %d", format.bytesPerPixel);
}

void Surface::drawRoundRect(const Common::Rect &rect, int arc, uint32 color, bool filled) {
	if (format.bytesPerPixel == 1) {
		SurfacePrimitives<byte> primitives;
		primitives.drawRoundRect(rect, arc, color, filled, this);
	} else if (format.bytesPerPixel == 2) {
		SurfacePrimitives<uint16> primitives;
		primitives.drawRoundRect(rect, arc, color, filled, this);
	} else if (format.bytesPerPixel == 4) {
		SurfacePrimitives<uint32> primitives;
		primitives.drawRoundRect(rect, arc, color, filled, this);
	} else
		error("Surface::drawRoundRect: bytesPerPixel must be 1, 2, or 4, got %d", format.bytesPerPixel);
}

// see graphics/blit/blit-atari.cpp
#ifndef ATARI
void Surface::create(int16 width, int16 height, const PixelFormat &f) {
	assert(width >= 0 && height >= 0);
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
#endif

void Surface::init(int16 width, int16 height, int16 newPitch, void *newPixels, const PixelFormat &f) {
	w = width;
	h = height;
	pitch = newPitch;
	pixels = newPixels;
	format = f;
}

void Surface::copyFrom(const Surface &surf) {
	create(surf.w, surf.h, surf.format);
	copyBlit((byte *)pixels, (const byte *)surf.pixels, pitch, surf.pitch, w, h, format.bytesPerPixel);
}

void Surface::convertFrom(const Surface &surf, const PixelFormat &f) {
	create(surf.w, surf.h, f);
	crossBlit((byte *)pixels, (const byte *)surf.pixels, pitch, surf.pitch, w, h, format, surf.format);
}

Surface Surface::getSubArea(const Common::Rect &area) {
	Common::Rect effectiveArea(area);
	effectiveArea.clip(w, h);

	Surface subSurface;
	subSurface.w = effectiveArea.width();
	subSurface.h = effectiveArea.height();
	subSurface.pitch = pitch;
	subSurface.pixels = getBasePtr(area.left, area.top);
	subSurface.format = format;
	return subSurface;
}

const Surface Surface::getSubArea(const Common::Rect &area) const {
	Common::Rect effectiveArea(area);
	effectiveArea.clip(w, h);

	Surface subSurface;
	subSurface.w = effectiveArea.width();
	subSurface.h = effectiveArea.height();
	subSurface.pitch = pitch;
	// We need to cast the const away here because a Surface always has a
	// pointer to modifiable pixel data.
	subSurface.pixels = const_cast<void *>(getBasePtr(area.left, area.top));
	subSurface.format = format;
	return subSurface;
}

bool Surface::clip(Common::Rect &srcBounds, Common::Rect &destBounds) const {
	if (destBounds.left >= this->w || destBounds.top >= this->h ||
		destBounds.right <= 0 || destBounds.bottom <= 0)
		return false;

	// Clip the bounds if necessary to fit on-screen
	if (destBounds.right > this->w) {
		srcBounds.right -= destBounds.right - this->w;
		destBounds.right = this->w;
	}

	if (destBounds.bottom > this->h) {
		srcBounds.bottom -= destBounds.bottom - this->h;
		destBounds.bottom = this->h;
	}

	if (destBounds.top < 0) {
		srcBounds.top += -destBounds.top;
		destBounds.top = 0;
	}

	if (destBounds.left < 0) {
		srcBounds.left += -destBounds.left;
		destBounds.left = 0;
	}

	return true;
}

void Surface::copyRectToSurface(const void *buffer, int srcPitch, int destX, int destY, int width, int height) {
	assert(buffer);

	assert(destX >= 0 && destX < w);
	assert(destY >= 0 && destY < h);
	assert(height > 0 && destY + height <= h);
	assert(width > 0 && destX + width <= w);

	// Copy buffer data to internal buffer
	const byte *src = (const byte *)buffer;
	byte *dst = (byte *)getBasePtr(destX, destY);
	copyBlit(dst, src, pitch, srcPitch, width, height, format.bytesPerPixel);
}

void Surface::copyRectToSurface(const Graphics::Surface &srcSurface, int destX, int destY, const Common::Rect subRect) {
	assert(srcSurface.format == format);

	copyRectToSurface(srcSurface.getBasePtr(subRect.left, subRect.top), srcSurface.pitch, destX, destY, subRect.width(), subRect.height());
}

void Surface::copyRectToSurfaceWithKey(const void *buffer, int srcPitch, int destX, int destY, int width, int height, uint32 key) {
	assert(buffer);

	assert(destX >= 0 && destX < w);
	assert(destY >= 0 && destY < h);
	assert(height > 0 && destY + height <= h);
	assert(width > 0 && destX + width <= w);

	// Copy buffer data to internal buffer
	const byte *src = (const byte *)buffer;
	byte *dst = (byte *)getBasePtr(destX, destY);
	Graphics::keyBlit(dst, src, pitch, srcPitch, width, height, format.bytesPerPixel, key);
}

void Surface::copyRectToSurfaceWithKey(const Graphics::Surface &srcSurface, int destX, int destY, const Common::Rect subRect, uint32 key) {
	assert(srcSurface.format == format);

	copyRectToSurfaceWithKey(srcSurface.getBasePtr(subRect.left, subRect.top), srcSurface.pitch, destX, destY, subRect.width(), subRect.height(), key);
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
		Common::memset16(ptr, (uint16)color, x2 - x + 1);
	} else if (format.bytesPerPixel == 4) {
		uint32 *ptr = (uint32 *)getBasePtr(x, y);
		Common::memset32(ptr, (uint32)color, x2 - x + 1);
	} else {
		error("Surface::hLine: bytesPerPixel must be 1, 2, or 4, got %d", format.bytesPerPixel);
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
		error("Surface::vLine: bytesPerPixel must be 1, 2, or 4, got %d", format.bytesPerPixel);
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
		lineLen *= 4;
		if ((uint32)color != ((color & 0xff) | (color & 0xff) << 8 | (color & 0xff) << 16 | (color & 0xff) << 24))
			useMemset = false;
	} else if (format.bytesPerPixel != 1) {
		error("Surface::fillRect: bytesPerPixel must be 1, 2, or 4, got %d", format.bytesPerPixel);
	}

	byte *ptr = (byte *)getBasePtr(r.left, r.top);
	if (useMemset) {
		while (height--) {
			memset(ptr, (byte)color, lineLen);
			ptr += pitch;
		}
	} else {
		if (format.bytesPerPixel == 2) {
			while (height--) {
				Common::memset16((uint16 *)ptr, (uint16)color, width);
				ptr += pitch;
			}
		} else {
			while (height--) {
				Common::memset32((uint32 *)ptr, (uint32)color, width);
				ptr += pitch;
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
		error("Surface::move: bytesPerPixel must be 1, 2, or 4, got %d", format.bytesPerPixel);

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

void Surface::flipVertical(const Common::Rect &r) {
	const int width = r.width() * format.bytesPerPixel;
	byte *temp = new byte[width];
	for (int y = r.top; y < r.bottom / 2; y++) {
		byte *row1 = (byte *)getBasePtr(r.left, y);
		byte *row2 = (byte *)getBasePtr(r.left, r.bottom - y - 1);

		memcpy(temp, row1, width);
		memcpy(row1, row2, width);
		memcpy(row2, temp, width);
	}
	delete[] temp;
}

void Surface::flipHorizontal(const Common::Rect &r) {
	uint32 tmp = 0;
	const int width = r.width() * format.bytesPerPixel;
	for (int y = r.top; y < r.bottom; ++y) {
		byte *row = (byte *)getBasePtr(r.left, y);
		for (int x = 0; x < width / 2; x += format.bytesPerPixel) {
			memcpy(&tmp, row + x, format.bytesPerPixel);
			memcpy(row + x, row + width - format.bytesPerPixel - x, format.bytesPerPixel);
			memcpy(row + width - format.bytesPerPixel - x, &tmp, format.bytesPerPixel);
		}
	}
}

bool Surface::applyColorKey(uint8 rKey, uint8 gKey, uint8 bKey, bool overwriteAlpha) {
	return Graphics::applyColorKey((byte *)pixels, (const byte *)pixels, pitch, pitch, w, h, format,
	                               overwriteAlpha, rKey, gKey, bKey, rKey, gKey, bKey);
}

bool Surface::applyColorKey(uint8 rKey, uint8 gKey, uint8 bKey, bool overwriteAlpha,
                            uint8 rNew, uint8 gNew, uint8 bNew) {
	return Graphics::applyColorKey((byte *)pixels, (const byte *)pixels, pitch, pitch, w, h, format,
	                               overwriteAlpha, rKey, gKey, bKey, rNew, gNew, bNew);
}

bool Surface::setAlpha(uint8 alpha, bool skipTransparent) {
	return Graphics::setAlpha((byte *)pixels, (const byte *)pixels, pitch, pitch, w, h, format,
	                          skipTransparent, alpha);
}

AlphaType Surface::detectAlpha() const {
	if (format.isCLUT8() || format.aBits() == 0)
		return ALPHA_OPAQUE;

	const uint32 mask = format.ARGBToColor(0xff, 0, 0, 0);
	AlphaType alphaType = ALPHA_OPAQUE;

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			uint32 pixel = getPixel(x, y);
			if ((pixel & mask) != mask) {
				if ((pixel & mask) == 0)
					alphaType = ALPHA_BINARY;
				else
					return ALPHA_FULL;
			}
		}
	}

	return alphaType;
}

Graphics::Surface *Surface::scale(int16 newWidth, int16 newHeight, bool filtering) const {
	Graphics::Surface *target = new Graphics::Surface();

	target->create(newWidth, newHeight, format);

	if (filtering) {
		scaleBlitBilinear((byte *)target->getPixels(), (const byte *)getPixels(), target->pitch, pitch, target->w, target->h, w, h, format);
	} else {
		scaleBlit((byte *)target->getPixels(), (const byte *)getPixels(), target->pitch, pitch, target->w, target->h, w, h, format);
	}

	return target;
}

Graphics::Surface *Surface::rotoscale(const TransformStruct &transform, bool filtering) const {

	Common::Point newHotspot;
	Common::Rect rect = TransformTools::newRect(Common::Rect((int16)w, (int16)h), transform, &newHotspot);

	Graphics::Surface *target = new Graphics::Surface();

	target->create((uint16)rect.right - rect.left, (uint16)rect.bottom - rect.top, this->format);

	if (filtering) {
		rotoscaleBlitBilinear((byte *)target->getPixels(), (const byte *)getPixels(), target->pitch, pitch, target->w, target->h, w, h, format, transform, newHotspot);
	} else {
		rotoscaleBlit((byte *)target->getPixels(), (const byte *)getPixels(), target->pitch, pitch, target->w, target->h, w, h, format, transform, newHotspot);
	}

	return target;
}

void Surface::convertToInPlace(const PixelFormat &dstFormat, const byte *palette, uint16 paletteCount) {
	// Do not convert to the same format and ignore empty surfaces.
	if (format == dstFormat || pixels == 0) {
		return;
	}

	if (format.bytesPerPixel == 0 || format.bytesPerPixel > 4)
		error("Surface::convertToInPlace(): Can only convert from 1Bpp, 2Bpp, 3Bpp, and 4Bpp but have %dbpp", format.bytesPerPixel);

	if (dstFormat.bytesPerPixel == 0 || dstFormat.bytesPerPixel == 1 || dstFormat.bytesPerPixel > 4)
		error("Surface::convertToInPlace(): Can only convert to 2Bpp, 3Bpp and 4Bpp but requested %dbpp", dstFormat.bytesPerPixel);

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
		uint32 map[256];
		assert(palette);

		convertPaletteToMap(map, palette, paletteCount, dstFormat);
		crossBlitMap((byte *)pixels, (const byte *)pixels, w * dstFormat.bytesPerPixel, pitch, w, h, dstFormat.bytesPerPixel, map);
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

Graphics::Surface *Surface::convertTo(const PixelFormat &dstFormat, const byte *srcPalette, int srcPaletteCount, const byte *dstPalette, int dstPaletteCount, DitherMethod method) const {
	assert(pixels);

	Graphics::Surface *surface = new Graphics::Surface();

	// If the target format is the same, just copy
	if (format == dstFormat) {
		if (dstFormat.bytesPerPixel == 1) { // Checking if dithering could be skipped
			if (!srcPalette // No palette is specified
					|| !dstPalette // No dst palette
					|| (srcPaletteCount == dstPaletteCount // palettes are the same
						&& !memcmp(srcPalette, dstPalette, srcPaletteCount * 3))) {
				surface->copyFrom(*this);
				return surface;
			}
		}
	}

	if (format.bytesPerPixel == 0 || format.bytesPerPixel > 4)
		error("Surface::convertTo(): Can only convert from 1Bpp, 2Bpp, 3Bpp, and 4Bpp but have %dbpp", format.bytesPerPixel);

	if (dstFormat.bytesPerPixel == 0 || dstFormat.bytesPerPixel > 4)
		error("Surface::convertTo(): Can only convert to 1Bpp, 2Bpp, 3Bpp and 4Bpp but requested %dbpp", dstFormat.bytesPerPixel);

	surface->create(w, h, dstFormat);

	// We are here when we are converting from a higher bpp or palettes are different
	if (dstFormat.bytesPerPixel == 1) {
		ditherFloyd(srcPalette, srcPaletteCount, surface, dstPalette, dstPaletteCount, method,
			    dstFormat);
		return surface;
	}

	const byte *src = (const byte *)getPixels();
	byte *dst = (byte *)surface->getPixels();

	if (format.bytesPerPixel == 1) {
		// Converting from paletted to high color
		assert(srcPalette);
		uint32 map[256];

		convertPaletteToMap(map, srcPalette, srcPaletteCount, dstFormat);
		crossBlitMap(dst, src, surface->pitch, pitch, w, h, dstFormat.bytesPerPixel, map);
	} else {
		// Converting from high color to high color
		crossBlit(dst, src, surface->pitch, pitch, w, h, dstFormat, format);
	}

	return surface;
}

void Surface::debugPrint(int debuglevel, int width, int height, int x, int y, int scale, int maxwidth, const byte *palette) const {
	//                      012 3456789abcdef
	const char *gradient = " .:\';+*<?F7RQ&%#";

	if (width <= 0) width = w;
	if (height <= 0) height = h;
	if (x < 0) x = 0;
	if (y < 0) 	y = 0;

	maxwidth -= 2; // Compensate for the frame
	if (maxwidth < 0) maxwidth = 80;

	if (scale < 1) {
		scale = MAX(1, (width + maxwidth - 1) / maxwidth);
	}

	x = MIN<int>(x, w);
	y = MIN<int>(y, h);

	int tox = MIN<int>(x + width, w);
	int toy = MIN<int>(y + height, h);

	debug(debuglevel, "Surface: %d x %d, bpp: %d, format: %s, address: %p", w, h, format.bytesPerPixel, format.toString().c_str(), (const void *)this);
	debug(debuglevel, "displaying: %d x %d @ %d,%d, scale: %d", width, height, x, y, scale);
	debugN(debuglevel, "+");
	for (int xx = x; xx < tox; xx += scale)
		debugN(debuglevel, "-");
	debug(debuglevel, "+");

	for (int yy = y; yy < toy; yy += scale) {
		debugN(debuglevel, "|");
		for (int xx = x; xx < tox; xx += scale) {
			double grayscale = 0.0;
			int pixelcount = 0;

			for (int ys = 0; ys < scale && yy + ys < h; ys++) {
				const byte *srcRow = (const byte *)getBasePtr(xx, yy + ys);

				for (int xs = 0; xs < scale && xx + xs < w; xs++) {
					byte r = 0, g = 0, b = 0, a = 0;
					uint32 color = 0;

					switch (format.bytesPerPixel) {
					case 1: {
						byte index = *srcRow;

						if (palette) {
							r = palette[index * 3];
							g = palette[index * 3 + 1];
							b = palette[index * 3 + 2];
						} else {
							r = g = b = index;
						}

						a = 0xff;
					    }
						break;
					case 2:
						color = READ_UINT16(srcRow);
						break;
					case 3:
						color = READ_UINT24(srcRow);
						break;
					case 4:
						color = READ_UINT32(srcRow);
						break;
					default:
						error("Surface::debugPrint: Unsupported bpp: %d", format.bytesPerPixel);
					}

					if (format.bytesPerPixel > 1)
						format.colorToARGB(color, a, r, g, b);

					grayscale += (0.29 * r + 0.58 * g + 0.11 * b) / 3.0;
					pixelcount++;

					srcRow += format.bytesPerPixel;
				}
			}

			debugN(debuglevel, "%c", gradient[(int)(grayscale / pixelcount / 16)]);
		}
		debug(debuglevel, "|");
	}
	debugN(debuglevel, "+");
	for (int xx = x; xx < tox; xx += scale)
		debugN(debuglevel, "-");
	debug(debuglevel, "+");
}

/*******************************************
 *
 * Dithering
 *
 *******************************************/

static void updatePixel(byte *surf, int x, int y, int w, int h, int qr, int qg, int qb, int qq, int qdiv) {
	if (x >= w || y >= h)
		return;

	byte *ptr = &surf[x * 3 + y * w * 3];

	ptr[0] = CLIP(ptr[0] + qr * qq / qdiv, 0, 255);
	ptr[1] = CLIP(ptr[1] + qg * qq / qdiv, 0, 255);
	ptr[2] = CLIP(ptr[2] + qb * qq / qdiv, 0, 255);
}

void Surface::ditherFloyd(const byte *srcPalette, int srcPaletteCount, Surface *dstSurf, const byte *dstPalette, int dstPaletteCount,
			  DitherMethod method, const PixelFormat &dstFormat) const {
	byte *tmpSurf = (byte *)malloc(w * h * 3);

	int bpp = format.bytesPerPixel;

	for (int y = 0; y < h; y++) {
		const byte *src = (const byte *)getBasePtr(0, y);
		byte *dst = &tmpSurf[y * w * 3];

		byte r, g, b;

		for (int x = 0; x < w; x++) {
			uint32 color;

			switch (bpp) {
			case 1:
				color = *src * 3;
				src += 1;
				r = srcPalette[color + 0]; g = srcPalette[color + 1]; b = srcPalette[color + 2];
				break;
			case 2:
				color = *((const uint16 *)src);
				src += 2;
				format.colorToRGB(color, r, g, b);
				break;
			case 3:
				color = *((const uint32 *)src);
				color >>= 8;
				src += 3;
				format.colorToRGB(color, r, g, b);
				break;
			case 4:
				color = *((const uint32 *)src);
				src += 4;
				format.colorToRGB(color, r, g, b);
				break;
			default:
				error("Surface::ditherFloydImage(): Unsupported bit depth: %d", bpp);
			}

			dst[0] = r; dst[1] = g; dst[2] = b;
			dst += 3;
		}
	}

	struct DitherParams {
		int dy, dx, qq;
	};

	const DitherParams paramsNaive[] = {
		{ 0, 0, 0 }
	};

	const DitherParams paramsFloyd[] = {
		{ 0, +1, 7 },
		{ 1, -1, 3 },
		{ 1,  0, 5 },
		{ 1, +1, 1 },
		{ 0,  0, 0 }
	};

	const DitherParams paramsAtkinson[] = {
		{ 0, +1, 1 },
		{ 0, +2, 1 },
		{ 1, -1, 1 },
		{ 1,  0, 1 },
		{ 1, +1, 1 },
		{ 2,  0, 1 },
		{ 0,  0, 0 }
	};

	const DitherParams paramsBurkes[] = {
		{ 0, +1, 8 },
		{ 0, +2, 4 },
		{ 1, -2, 2 },
		{ 1, -1, 4 },
		{ 1,  0, 8 },
		{ 1, +1, 4 },
		{ 1, +2, 2 },
		{ 0,  0, 0 }
	};

	const DitherParams paramsFalseFloyd[] = {
		{ 0, +1, 3 },
		{ 1,  0, 3 },
		{ 1, +1, 2 },
		{ 0,  0, 0 }
	};

    const DitherParams paramsSierra[] = {
		{ 0,  1, 5 },
		{ 0,  2, 3 },
		{ 1, -2, 2 },
		{ 1, -1, 4 },
		{ 1,  0, 5 },
		{ 1,  1, 4 },
		{ 1,  2, 2 },
		{ 2, -1, 2 },
		{ 2,  0, 3 },
		{ 2,  1, 2 },
		{ 0,  0, 0 }
    };

    const DitherParams paramsSierraTwoRow[] = {
		{ 0,  1, 4 },
		{ 0,  2, 3 },
		{ 1, -2, 1 },
		{ 1, -1, 2 },
		{ 1,  0, 3 },
		{ 1,  1, 2 },
		{ 1,  2, 1 },
		{ 0,  0, 0 }
    };

    const DitherParams paramsSierraLite[] = {
		{ 0,  1, 2 },
		{ 1, -1, 1 },
		{ 1,  0, 1 },
		{ 0,  0, 0 }
    };

    const DitherParams paramsStucki[] = {
		{ 0,  1, 8 },
		{ 0,  2, 4 },
		{ 1, -2, 2 },
		{ 1, -1, 4 },
		{ 1,  0, 8 },
		{ 1,  1, 4 },
		{ 1,  2, 2 },
		{ 2, -2, 1 },
		{ 2, -1, 2 },
		{ 2,  0, 4 },
		{ 2,  1, 2 },
		{ 2,  2, 1 },
		{ 0,  0, 0 }
    };

    const DitherParams paramsJarvis[] = {
		{ 0,  1, 7 },
		{ 0,  2, 5 },
		{ 1, -2, 3 },
		{ 1, -1, 5 },
		{ 1,  0, 7 },
		{ 1,  1, 5 },
		{ 1,  2, 3 },
		{ 2, -2, 1 },
		{ 2, -1, 3 },
		{ 2,  0, 5 },
		{ 2,  1, 3 },
		{ 2,  2, 1 },
		{ 0,  0, 0 }
    };

	struct DitherAlgos {
		const char *name;
		const DitherParams *params;
		int qdiv;
	} const algos[] = {
		{ "Naive",                paramsNaive,         1 },
		{ "Floyd-Steinberg",      paramsFloyd,        16 },
		{ "Atkinson",             paramsAtkinson,      8 },
		{ "Burkes",               paramsBurkes,       32 },
		{ "False Floyd-Steinberg",paramsFalseFloyd,    8 },
		{ "Sierra",               paramsSierra,       32 },
		{ "Sierra 2",             paramsSierraTwoRow, 16 },
		{ "Sierra Lite",          paramsSierraLite,    4 },
		{ "Stucki",               paramsStucki,       42 },
		{ "Jarvis-Judice-Ninke ", paramsJarvis,       48 },
		{ nullptr, nullptr, 0 }
	};

	if (dstPalette) {
		PaletteLookup _paletteLookup;

		_paletteLookup.setPalette(dstPalette, dstPaletteCount);

		for (int y = 0; y < h; y++) {
			const byte *src = &tmpSurf[y * w * 3];
			byte *dst = (byte *)dstSurf->getBasePtr(0, y);

			for (int x = 0; x < w; x++) {
				byte r = src[0], g = src[1], b = src[2];
				byte col = _paletteLookup.findBestColor(r, g, b);

				*dst = col;

				int qr = r - dstPalette[col * 3 + 0];
				int qg = g - dstPalette[col * 3 + 1];
				int qb = b - dstPalette[col * 3 + 2];

				const DitherParams *params = algos[method].params;

				for (int i = 0; params[i].dx != 0 || params[i].dy != 0; i++)
					updatePixel(tmpSurf, x + params[i].dx, y + params[i].dy, w, h, qr, qg, qb, params[i].qq, algos[method].qdiv);

				src += 3;
				dst++;
			}
		}
	} else if (dstFormat == PixelFormat(1, 3, 3, 2, 0, 5, 2, 0, 0) || dstFormat == PixelFormat(1, 1, 2, 1, 0, 3, 1, 0, 0)) {
		const int rShift = dstFormat.rLoss - dstFormat.rShift;
		const int gShift = dstFormat.gLoss - dstFormat.gShift;
		const int bShift = dstFormat.bLoss - dstFormat.bShift;

		const int rMask = dstFormat.rMax() << dstFormat.rShift;
		const int gMask = dstFormat.gMax() << dstFormat.gShift;
		const int bMask = dstFormat.bMax() << dstFormat.bShift;

		const int rLossMask = (1 << dstFormat.rLoss) - 1;
		const int gLossMask = (1 << dstFormat.gLoss) - 1;
		const int bLossMask = (1 << dstFormat.bLoss) - 1;

		for (int y = 0; y < h; y++) {
			const byte *src = &tmpSurf[y * w * 3];
			byte *dst = (byte *)dstSurf->getBasePtr(0, y);

			for (int x = 0; x < w; x++) {
				byte r = src[0], g = src[1], b = src[2];

				*dst = ((r >> rShift) & rMask) | ((g >> gShift) & gMask) | ((b >> bShift) & bMask);

				int qr = r & rLossMask;
				int qg = g & gLossMask;
				int qb = b & bLossMask;

				const DitherParams *params = algos[method].params;

				for (int i = 0; params[i].dx != 0 || params[i].dy != 0; i++)
					updatePixel(tmpSurf, x + params[i].dx, y + params[i].dy, w, h, qr, qg, qb, params[i].qq, algos[method].qdiv);

				src += 3;
				dst++;
			}
		}
	} else
		error("Unsupported dithering target format or missing palette");

	::free(tmpSurf);
}

/*******************************************
 *
 * Flood Fill
 *
 *******************************************/

FloodFill::FloodFill(Graphics::Surface *surface, uint32 oldColor, uint32 fillColor, bool maskMode) {
	_surface = surface;
	_oldColor = oldColor;
	_fillColor = fillColor;
	_w = surface->w;
	_h = surface->h;

	_mask = nullptr;
	_maskMode = maskMode;

	if (_maskMode) {
		_mask = new Graphics::Surface();
		_mask->create(_w, _h, surface->format); // Uses calloc()
	}

	_visited = (byte *)calloc(_w * _h, 1);
}

FloodFill::~FloodFill() {
	while(!_queue.empty()) {
		Common::Point *p = _queue.front();

		delete p;
		_queue.pop_front();
	}

	free(_visited);

	if (_mask) {
		_mask->free();
		delete _mask;
	}
}

void FloodFill::addSeed(int x, int y) {
	if (x >= 0 && x < _w && y >= 0 && y < _h) {
		if (!_visited[y * _w + x]) {
			_visited[y * _w + x] = 1;
			void *src = _surface->getBasePtr(x, y);
			void *dst;
			bool changed = false;

			if (_maskMode)
				dst = _mask->getBasePtr(x, y);
			else
				dst = src;

			if (_surface->format.bytesPerPixel == 1) {
				if (*((byte *)src) == _oldColor) {
					*((byte *)dst) = _maskMode ? 255 : _fillColor;
					changed = true;
				}
			} else if (_surface->format.bytesPerPixel == 2) {
				if (READ_UINT16(src) == _oldColor) {
					if (!_maskMode)
						WRITE_UINT16(src, _fillColor);
					else
						*((uint16 *)dst) = 0xffff;

					changed = true;
				}
			} else if (_surface->format.bytesPerPixel == 4) {
				if (READ_UINT32(src) == _oldColor) {
					if (!_maskMode)
						WRITE_UINT32(src, _fillColor);
					else
						*((uint32 *)dst) = 0xffffffff;

					changed = true;
				}
			} else {
				error("Unsupported bpp in FloodFill, got %d", _surface->format.bytesPerPixel);
			}

			if (changed) {
				Common::Point *pt = new Common::Point(x, y);

				_queue.push_back(pt);
			}
		}
	}
}

void FloodFill::fill() {
	while (!_queue.empty()) {
		Common::Point *p = _queue.front();
		_queue.pop_front();
		addSeed(p->x    , p->y - 1);
		addSeed(p->x - 1, p->y    );
		addSeed(p->x    , p->y + 1);
		addSeed(p->x + 1, p->y    );

		delete p;
	}
}

void FloodFill::fillMask() {
	_maskMode = true;

	if (!_mask) {
		_mask = new Graphics::Surface();
		_mask->create(_w, _h, _surface->format); // Uses calloc()
	}

	fill();
}

} // End of namespace Graphics
