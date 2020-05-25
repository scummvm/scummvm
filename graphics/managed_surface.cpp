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

#include "graphics/managed_surface.h"
#include "common/algorithm.h"
#include "common/textconsole.h"

namespace Graphics {

const int SCALE_THRESHOLD = 0x100;

ManagedSurface::ManagedSurface() :
		w(_innerSurface.w), h(_innerSurface.h), pitch(_innerSurface.pitch), format(_innerSurface.format),
		_disposeAfterUse(DisposeAfterUse::NO), _owner(nullptr),
		_transparentColor(0),_transparentColorSet(false), _paletteSet(false) {
	Common::fill(&_palette[0], &_palette[256], 0);
}

ManagedSurface::ManagedSurface(const ManagedSurface &surf) :
		w(_innerSurface.w), h(_innerSurface.h), pitch(_innerSurface.pitch), format(_innerSurface.format),
		_disposeAfterUse(DisposeAfterUse::NO), _owner(nullptr),
		_transparentColor(0), _transparentColorSet(false), _paletteSet(false) {
	Common::fill(&_palette[0], &_palette[256], 0);
	*this = surf;
}

ManagedSurface::ManagedSurface(int width, int height) :
		w(_innerSurface.w), h(_innerSurface.h), pitch(_innerSurface.pitch), format(_innerSurface.format),
		_disposeAfterUse(DisposeAfterUse::NO), _owner(nullptr),
		_transparentColor(0), _transparentColorSet(false), _paletteSet(false) {
	Common::fill(&_palette[0], &_palette[256], 0);
	create(width, height);
}

ManagedSurface::ManagedSurface(int width, int height, const Graphics::PixelFormat &pixelFormat) :
		w(_innerSurface.w), h(_innerSurface.h), pitch(_innerSurface.pitch), format(_innerSurface.format),
		_disposeAfterUse(DisposeAfterUse::NO), _owner(nullptr),
		_transparentColor(0), _transparentColorSet(false), _paletteSet(false) {
	Common::fill(&_palette[0], &_palette[256], 0);
	create(width, height, pixelFormat);
}

ManagedSurface::ManagedSurface(ManagedSurface &surf, const Common::Rect &bounds) :
		w(_innerSurface.w), h(_innerSurface.h), pitch(_innerSurface.pitch), format(_innerSurface.format),
		_disposeAfterUse(DisposeAfterUse::NO), _owner(nullptr),
		_transparentColor(0), _transparentColorSet(false), _paletteSet(false) {
	Common::fill(&_palette[0], &_palette[256], 0);
	create(surf, bounds);
}

ManagedSurface::~ManagedSurface() {
	free();
}

ManagedSurface &ManagedSurface::operator=(const ManagedSurface &surf) {
	// Free any current surface
	free();

	if (surf._disposeAfterUse == DisposeAfterUse::YES) {
		// Create a new surface and copy the pixels from the source surface
		create(surf.w, surf.h, surf.format);
		Common::copy((const byte *)surf.getPixels(), (const byte *)surf.getPixels() +
			surf.w * surf.h * surf.format.bytesPerPixel, (byte *)this->getPixels());
	} else {
		// Source isn't managed, so simply copy its fields
		_owner = surf._owner;
		_offsetFromOwner = surf._offsetFromOwner;
		void *srcPixels = const_cast<void *>(surf._innerSurface.getPixels());
		_innerSurface.setPixels(srcPixels);
		_innerSurface.w = surf.w;
		_innerSurface.h = surf.h;
		_innerSurface.pitch = surf.pitch;
		this->format = surf.format;
	}

	return *this;
}

void ManagedSurface::setPixels(void *newPixels) {
	free();
	_innerSurface.setPixels(newPixels);
}

void ManagedSurface::create(uint16 width, uint16 height) {
	create(width, height, PixelFormat::createFormatCLUT8());
}

void ManagedSurface::create(uint16 width, uint16 height, const PixelFormat &pixelFormat) {
	free();
	_innerSurface.create(width, height, pixelFormat);

	_disposeAfterUse = DisposeAfterUse::YES;
	markAllDirty();
}

void ManagedSurface::create(ManagedSurface &surf, const Common::Rect &bounds) {
	free();

	_offsetFromOwner = Common::Point(bounds.left, bounds.top);
	_innerSurface.setPixels(surf.getBasePtr(bounds.left, bounds.top));
	_innerSurface.pitch = surf.pitch;
	_innerSurface.format = surf.format;
	_innerSurface.w = bounds.width();
	_innerSurface.h = bounds.height();
	_owner = &surf;
	_disposeAfterUse = DisposeAfterUse::NO;
}

void ManagedSurface::free() {
	if (_disposeAfterUse == DisposeAfterUse::YES)
		_innerSurface.free();

	_disposeAfterUse = DisposeAfterUse::NO;
	_owner = nullptr;
	_offsetFromOwner = Common::Point(0, 0);
	_transparentColorSet = false;
	_paletteSet = false;
}

void ManagedSurface::copyFrom(const ManagedSurface &surf) {
	// Surface::copyFrom frees pixel pointer so let's free up ManagedSurface to be coherent
	free();

	// Copy the surface
	_innerSurface.copyFrom(surf._innerSurface);
	markAllDirty();

	// Pixels data is now owned by us
	_disposeAfterUse = DisposeAfterUse::YES;

	// Copy miscellaneous properties
	_transparentColorSet = surf._transparentColorSet;
	_transparentColor = surf._transparentColor;
	_paletteSet = surf._paletteSet;
	Common::copy(&surf._palette[0], &surf._palette[256], _palette);
}

bool ManagedSurface::clip(Common::Rect &srcBounds, Common::Rect &destBounds) {
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

void ManagedSurface::blitFrom(const Surface &src) {
	blitFrom(src, Common::Rect(0, 0, src.w, src.h), Common::Point(0, 0));
}

void ManagedSurface::blitFrom(const Surface &src, const Common::Point &destPos) {
	blitFrom(src, Common::Rect(0, 0, src.w, src.h), destPos);
}

void ManagedSurface::blitFrom(const Surface &src, const Common::Rect &srcRect,
		const Common::Point &destPos) {
	blitFromInner(src, srcRect, destPos, nullptr);
}

void ManagedSurface::blitFrom(const ManagedSurface &src) {
	blitFrom(src, Common::Rect(0, 0, src.w, src.h), Common::Point(0, 0));
}

void ManagedSurface::blitFrom(const ManagedSurface &src, const Common::Point &destPos) {
	blitFrom(src, Common::Rect(0, 0, src.w, src.h), destPos);
}

void ManagedSurface::blitFrom(const ManagedSurface &src, const Common::Rect &srcRect,
		const Common::Point &destPos) {
	if (src._transparentColorSet)
		transBlitFrom(src, srcRect, destPos);
	else
		blitFromInner(src._innerSurface, srcRect, destPos, src._paletteSet ? &src._palette[0] : nullptr);
}

void ManagedSurface::blitFromInner(const Surface &src, const Common::Rect &srcRect,
		const Common::Point &destPos, const uint32 *palette) {
	Common::Rect srcBounds = srcRect;
	Common::Rect destBounds(destPos.x, destPos.y, destPos.x + srcRect.width(),
		destPos.y + srcRect.height());
	uint destPixel;
	byte rSrc, gSrc, bSrc, aSrc;
	byte rDest = 0, gDest = 0, bDest = 0;
	double alpha;

	if (!srcRect.isValidRect() || !clip(srcBounds, destBounds))
		return;

	if (format != src.format) {
		// When the pixel format differs, the destination must be 2 or 4 bytes per pixel,
		// and the source be 2/4 bytes as well or be paletted
		assert(format.bytesPerPixel == 2 || format.bytesPerPixel == 4);
		assert(src.format.bytesPerPixel == 2 || src.format.bytesPerPixel == 4
			|| (src.format.bytesPerPixel == 1 && palette));
	}

	for (int y = 0; y < srcBounds.height(); ++y) {
		const byte *srcP = (const byte *)src.getBasePtr(srcBounds.left, srcBounds.top + y);
		byte *destP = (byte *)getBasePtr(destBounds.left, destBounds.top + y);

		if (src.format == format) {
			// Matching surface formats, so we can do a straight copy
			Common::copy(srcP, srcP + srcBounds.width() * format.bytesPerPixel, destP);
		} else {
			for (int x = 0; x < srcBounds.width(); ++x,
					srcP += src.format.bytesPerPixel,
					destP += format.bytesPerPixel) {

				if (src.format.bytesPerPixel == 1) {
					// Get the palette color
					const uint32 col = palette[*srcP];
					rSrc = col & 0xff;
					gSrc = (col >> 8) & 0xff;
					bSrc = (col >> 16) & 0xff;
					aSrc = (col >> 24) & 0xff;
				} else {
					// Use the src's pixel format to split up the source pixel
					src.format.colorToARGB(src.format.bytesPerPixel == 2
						? *(const uint16 *)srcP : *(const uint32 *)srcP,
						aSrc, rSrc, gSrc, bSrc);
				}

				if (aSrc == 0) {
					// Completely transparent, so skip
					continue;
				} else if (aSrc == 0xff) {
					// Completely opaque, so copy RGB values over
					rDest = rSrc;
					gDest = gSrc;
					bDest = bSrc;
				} else {
					// Partially transparent, so calculate new pixel colors
					alpha = (double)aSrc / 255.0;
					rDest = static_cast<byte>((rSrc * alpha) + (rDest * (1.0 - alpha)));
					gDest = static_cast<byte>((gSrc * alpha) + (gDest * (1.0 - alpha)));
					bDest = static_cast<byte>((bSrc * alpha) + (bDest * (1.0 - alpha)));
				}

				destPixel = format.ARGBToColor(0xff, rDest, gDest, bDest);
				if (format.bytesPerPixel == 2)
					*(uint16 *)destP = destPixel;
				else
					*(uint32 *)destP = destPixel;
			}
		}
	}

	addDirtyRect(Common::Rect(0, 0, this->w, this->h));
}

void ManagedSurface::transBlitFrom(const Surface &src, uint transColor, bool flipped,
		uint overrideColor, uint srcAlpha) {
	transBlitFrom(src, Common::Rect(0, 0, src.w, src.h), Common::Rect(0, 0, this->w, this->h),
		transColor, flipped, overrideColor, srcAlpha);
}

void ManagedSurface::transBlitFrom(const Surface &src, const Common::Point &destPos,
		uint transColor, bool flipped, uint overrideColor, uint srcAlpha) {
	transBlitFrom(src, Common::Rect(0, 0, src.w, src.h), Common::Rect(destPos.x, destPos.y,
		destPos.x + src.w, destPos.y + src.h), transColor, flipped, overrideColor, srcAlpha);
}

void ManagedSurface::transBlitFrom(const Surface &src, const Common::Point &destPos,
		const ManagedSurface &mask) {
	transBlitFrom(src, Common::Rect(0, 0, src.w, src.h), Common::Rect(destPos.x, destPos.y,
		destPos.x + src.w, destPos.y + src.h), 0, false, 0, 0xff, &mask._innerSurface, true);
}

void ManagedSurface::transBlitFrom(const Surface &src, const Common::Point &destPos,
		const Surface &mask) {
	transBlitFrom(src, Common::Rect(0, 0, src.w, src.h), Common::Rect(destPos.x, destPos.y,
		destPos.x + src.w, destPos.y + src.h), 0, false, 0, 0xff, &mask, true);
}

void ManagedSurface::transBlitFrom(const Surface &src, const Common::Rect &srcRect,
		const Common::Point &destPos, uint transColor, bool flipped, uint overrideColor, uint srcAlpha) {
	transBlitFrom(src, srcRect, Common::Rect(destPos.x, destPos.y,
		destPos.x + srcRect.width(), destPos.y + srcRect.height()), transColor, flipped, overrideColor, srcAlpha);
}

void ManagedSurface::transBlitFrom(const Surface &src, const Common::Rect &srcRect,
		const Common::Rect &destRect, uint transColor, bool flipped, uint overrideColor, uint srcAlpha,
		const Surface *mask, bool maskOnly) {
	transBlitFromInner(src, srcRect, destRect, transColor, flipped, overrideColor, srcAlpha,
		nullptr, mask, maskOnly);
}

void ManagedSurface::transBlitFrom(const ManagedSurface &src, uint transColor, bool flipped,
		uint overrideColor, uint srcAlpha) {
	transBlitFrom(src, Common::Rect(0, 0, src.w, src.h), Common::Rect(0, 0, this->w, this->h),
		transColor, flipped, overrideColor, srcAlpha);
}

void ManagedSurface::transBlitFrom(const ManagedSurface &src, const Common::Point &destPos,
		uint transColor, bool flipped, uint overrideColor, uint srcAlpha) {
	transBlitFrom(src, Common::Rect(0, 0, src.w, src.h), Common::Rect(destPos.x, destPos.y,
		destPos.x + src.w, destPos.y + src.h), transColor, flipped, overrideColor, srcAlpha);
}

void ManagedSurface::transBlitFrom(const ManagedSurface &src, const Common::Point &destPos,
		const ManagedSurface &mask) {
	transBlitFrom(src, Common::Rect(0, 0, src.w, src.h), Common::Rect(destPos.x, destPos.y,
		destPos.x + src.w, destPos.y + src.h), 0, false, 0, 0xff, &mask._innerSurface, true);
}

void ManagedSurface::transBlitFrom(const ManagedSurface &src, const Common::Rect &srcRect,
		const Common::Point &destPos, uint transColor, bool flipped, uint overrideColor, uint srcAlpha) {
	uint tColor = !transColor && src._transparentColorSet ? src._transparentColor : transColor;

	transBlitFrom(src, srcRect, Common::Rect(destPos.x, destPos.y, destPos.x + srcRect.width(),
		destPos.y + srcRect.height()), tColor, flipped, overrideColor, srcAlpha);
}

void ManagedSurface::transBlitFrom(const ManagedSurface &src, const Common::Rect &srcRect,
		const Common::Rect &destRect, uint transColor, bool flipped, uint overrideColor, uint srcAlpha,
		const Surface *mask, bool maskOnly) {
	if (transColor == (uint)-1 && src._transparentColorSet)
		transColor = src._transparentColor;
	const uint32 *palette = src._paletteSet ? src._palette : nullptr;

	transBlitFromInner(src._innerSurface, srcRect, destRect, transColor, flipped, overrideColor,
		srcAlpha, palette, mask, maskOnly);
}

template<typename TSRC, typename TDEST>
void transBlit(const Surface &src, const Common::Rect &srcRect, Surface &dest, const Common::Rect &destRect,
		TSRC transColor, bool flipped, uint overrideColor, uint srcAlpha, const uint32 *palette,
		const Surface *mask, bool maskOnly) {
	int scaleX = SCALE_THRESHOLD * srcRect.width() / destRect.width();
	int scaleY = SCALE_THRESHOLD * srcRect.height() / destRect.height();
	const Graphics::PixelFormat &srcFormat = src.format;
	const Graphics::PixelFormat &destFormat = dest.format;
	byte aSrc, rSrc, gSrc, bSrc;
	byte rDest, gDest, bDest;
	double alpha;

	// Loop through drawing output lines
	for (int destY = destRect.top, scaleYCtr = 0; destY < destRect.bottom; ++destY, scaleYCtr += scaleY) {
		if (destY < 0 || destY >= dest.h)
			continue;
		const TSRC *srcLine = (const TSRC *)src.getBasePtr(srcRect.left, scaleYCtr / SCALE_THRESHOLD + srcRect.top);
		const TSRC *mskLine = nullptr;

		if (mask)
			mskLine = (const TSRC *)mask->getBasePtr(srcRect.left, scaleYCtr / SCALE_THRESHOLD + srcRect.top);

		TDEST *destLine = (TDEST *)dest.getBasePtr(destRect.left, destY);

		// Loop through drawing the pixels of the row
		for (int destX = destRect.left, xCtr = 0, scaleXCtr = 0; destX < destRect.right; ++destX, ++xCtr, scaleXCtr += scaleX) {
			if (destX < 0 || destX >= dest.w)
				continue;

			TSRC srcVal = srcLine[flipped ? src.w - scaleXCtr / SCALE_THRESHOLD - 1 : scaleXCtr / SCALE_THRESHOLD];
			if (srcVal == transColor && !maskOnly)
				continue;

			if (mask) {
				TSRC mskVal = mskLine[flipped ? src.w - scaleXCtr / SCALE_THRESHOLD - 1 : scaleXCtr / SCALE_THRESHOLD];
				if (!mskVal)
					continue;
			}

			if (srcFormat == destFormat && srcAlpha == 0xff) {
				// Matching formats, so we can do a straight copy
				destLine[xCtr] = overrideColor ? overrideColor : srcVal;
			} else {
				// Otherwise we have to manually decode and re-encode each pixel
				if (srcFormat.bytesPerPixel == 1) {
					assert(palette != nullptr);	// Catch the cases when palette is missing

					// Get the palette color
					const uint32 col = palette[srcVal];
					rSrc = col & 0xff;
					gSrc = (col >> 8) & 0xff;
					bSrc = (col >> 16) & 0xff;
					aSrc = (col >> 24) & 0xff;
				} else {
					srcFormat.colorToARGB(srcVal, aSrc, rSrc, gSrc, bSrc);
				}
				destFormat.colorToRGB(destLine[xCtr], rDest, gDest, bDest);

				if (srcAlpha != 0xff) {
					aSrc = aSrc * srcAlpha / 255;
				}

				if (aSrc == 0) {
					// Completely transparent, so skip
					continue;
				}
				else if (aSrc == 0xff) {
					// Completely opaque, so copy RGB values over
					rDest = rSrc;
					gDest = gSrc;
					bDest = bSrc;
				}
				else {
					// Partially transparent, so calculate new pixel colors
					alpha = (double)aSrc / 255.0;
					rDest = static_cast<byte>((rSrc * alpha) + (rDest * (1.0 - alpha)));
					gDest = static_cast<byte>((gSrc * alpha) + (gDest * (1.0 - alpha)));
					bDest = static_cast<byte>((bSrc * alpha) + (bDest * (1.0 - alpha)));
				}

				destLine[xCtr] = destFormat.ARGBToColor(0xff, rDest, gDest, bDest);
			}
		}
	}
}

#define HANDLE_BLIT(SRC_BYTES, DEST_BYTES, SRC_TYPE, DEST_TYPE) \
	if (src.format.bytesPerPixel == SRC_BYTES && format.bytesPerPixel == DEST_BYTES) \
		transBlit<SRC_TYPE, DEST_TYPE>(src, srcRect, _innerSurface, destRect, transColor, flipped, overrideColor, srcAlpha, palette, mask, maskOnly); \
	else

void ManagedSurface::transBlitFromInner(const Surface &src, const Common::Rect &srcRect,
		const Common::Rect &destRect, uint transColor, bool flipped, uint overrideColor,
		uint srcAlpha, const uint32 *palette, const Surface *mask, bool maskOnly) {
	if (src.w == 0 || src.h == 0 || destRect.width() == 0 || destRect.height() == 0)
		return;

	if (mask) {
		if (mask->w != src.w || mask->h != src.h)
			error("Surface::transBlitFrom: mask dimensions do not match src");
	}

	HANDLE_BLIT(1, 1, byte, byte)
	HANDLE_BLIT(1, 2, byte, uint16)
	HANDLE_BLIT(1, 4, byte, uint32)
	HANDLE_BLIT(2, 2, uint16, uint16)
	HANDLE_BLIT(4, 4, uint32, uint32)
	HANDLE_BLIT(2, 4, uint16, uint32)
	HANDLE_BLIT(4, 2, uint32, uint16)
	error("Surface::transBlitFrom: bytesPerPixel must be 1, 2, or 4");

	// Mark the affected area
	addDirtyRect(destRect);
}

#undef HANDLE_BLIT

void ManagedSurface::markAllDirty() {
	addDirtyRect(Common::Rect(0, 0, this->w, this->h));
}

void ManagedSurface::addDirtyRect(const Common::Rect &r) {
	if (_owner) {
		Common::Rect bounds = r;
		bounds.clip(Common::Rect(0, 0, this->w, this->h));
		bounds.translate(_offsetFromOwner.x, _offsetFromOwner.y);
		_owner->addDirtyRect(bounds);
	}
}

void ManagedSurface::clear(uint color) {
	if (!empty())
		fillRect(getBounds(), color);
}

void ManagedSurface::setPalette(const byte *colors, uint start, uint num) {
	assert(start < 256 && (start + num) <= 256);
	uint32 *dest = &_palette[start];

	for (; num > 0; --num, colors += 3) {
		*dest++ = colors[0] | (colors[1] << 8) | (colors[2] << 16) | (0xff << 24);
	}

	_paletteSet = true;
}

void ManagedSurface::setPalette(const uint32 *colors, uint start, uint num) {
	assert(start < 256 && (start + num) <= 256);
	Common::copy(colors, colors + num, &_palette[start]);
	_paletteSet = true;
}

} // End of namespace Graphics
