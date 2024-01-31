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

#include "graphics/managed_surface.h"
#include "graphics/palette.h"
#include "graphics/blit.h"
#include "common/algorithm.h"
#include "common/textconsole.h"
#include "common/endian.h"

namespace Graphics {

const int SCALE_THRESHOLD = 0x100;

ManagedSurface::ManagedSurface() :
		w(_innerSurface.w), h(_innerSurface.h), pitch(_innerSurface.pitch), format(_innerSurface.format),
		_disposeAfterUse(DisposeAfterUse::NO), _owner(nullptr),
		_transparentColor(0),_transparentColorSet(false), _palette(nullptr) {
}

ManagedSurface::ManagedSurface(const ManagedSurface &surf) :
		w(_innerSurface.w), h(_innerSurface.h), pitch(_innerSurface.pitch), format(_innerSurface.format),
		_disposeAfterUse(DisposeAfterUse::NO), _owner(nullptr),
		_transparentColor(0), _transparentColorSet(false), _palette(nullptr) {
	*this = surf;
}

ManagedSurface::ManagedSurface(int width, int height) :
		w(_innerSurface.w), h(_innerSurface.h), pitch(_innerSurface.pitch), format(_innerSurface.format),
		_disposeAfterUse(DisposeAfterUse::NO), _owner(nullptr),
		_transparentColor(0), _transparentColorSet(false), _palette(nullptr) {
	create(width, height);
}

ManagedSurface::ManagedSurface(int width, int height, const Graphics::PixelFormat &pixelFormat) :
		w(_innerSurface.w), h(_innerSurface.h), pitch(_innerSurface.pitch), format(_innerSurface.format),
		_disposeAfterUse(DisposeAfterUse::NO), _owner(nullptr),
		_transparentColor(0), _transparentColorSet(false), _palette(nullptr) {
	create(width, height, pixelFormat);
}

ManagedSurface::ManagedSurface(ManagedSurface &surf, const Common::Rect &bounds) :
		w(_innerSurface.w), h(_innerSurface.h), pitch(_innerSurface.pitch), format(_innerSurface.format),
		_disposeAfterUse(DisposeAfterUse::NO), _owner(nullptr),
		_transparentColor(0), _transparentColorSet(false), _palette(nullptr) {
	create(surf, bounds);
}

ManagedSurface::ManagedSurface(Surface *surf, DisposeAfterUse::Flag disposeAfterUse) :
		w(_innerSurface.w), h(_innerSurface.h), pitch(_innerSurface.pitch), format(_innerSurface.format),
		_owner(nullptr), _transparentColor(0), _transparentColorSet(false), _palette(nullptr) {
	if (!surf) {
		_disposeAfterUse = DisposeAfterUse::YES;

		return;
	}

	_disposeAfterUse = disposeAfterUse;

	if (disposeAfterUse == DisposeAfterUse::YES) {
		_innerSurface.w = surf->w;
		_innerSurface.h = surf->h;
		_innerSurface.pitch = surf->pitch;
		_innerSurface.format = surf->format;
		_innerSurface.setPixels(surf->getPixels());

		delete surf;
	} else {
		copyFrom(*surf);
	}
}

ManagedSurface::ManagedSurface(const Surface *surf) :
		w(_innerSurface.w), h(_innerSurface.h), pitch(_innerSurface.pitch), format(_innerSurface.format),
		_owner(nullptr), _transparentColor(0), _transparentColorSet(false), _palette(nullptr) {
	if (!surf)  {
		_disposeAfterUse = DisposeAfterUse::YES;

		return;
	}

	_disposeAfterUse = DisposeAfterUse::NO;
	copyFrom(*surf);
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

		// Copy miscellaneous properties
		_transparentColorSet = surf._transparentColorSet;
		_transparentColor = surf._transparentColor;
		_palette = surf._palette ? new Palette(*surf._palette) : nullptr;
	}

	return *this;
}

void ManagedSurface::setPixels(void *newPixels) {
	free();
	_innerSurface.setPixels(newPixels);
}

void ManagedSurface::create(int16 width, int16 height) {
	create(width, height, PixelFormat::createFormatCLUT8());
}

void ManagedSurface::create(int16 width, int16 height, const PixelFormat &pixelFormat) {
	free();
	_innerSurface.create(width, height, pixelFormat);

	// For pixel formats with an alpha channel, we need to do a clear
	// so that all the pixels will have full alpha (0xff)
	if (pixelFormat.aBits() != 0)
		clear(0);

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

	// Copy miscellaneous properties
	_transparentColorSet = surf._transparentColorSet;
	_transparentColor = surf._transparentColor;
	_palette = surf._palette ? new Palette(*surf._palette) : nullptr;
}

void ManagedSurface::free() {
	if (_disposeAfterUse == DisposeAfterUse::YES) {
		_innerSurface.free();
	} else {
		_innerSurface.setPixels(nullptr);
	}

	_disposeAfterUse = DisposeAfterUse::NO;
	_owner = nullptr;
	_offsetFromOwner = Common::Point(0, 0);
	_transparentColorSet = false;
	if (_palette) {
		delete _palette;
		_palette = nullptr;
	}
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
	_palette = surf._palette ? new Palette(*surf._palette) : nullptr;
}

void ManagedSurface::copyFrom(const Surface &surf) {
	// Surface::copyFrom frees pixel pointer so let's free up ManagedSurface to be coherent
	free();

	// Copy the surface
	_innerSurface.copyFrom(surf);
	markAllDirty();

	// Pixels data is now owned by us
	_disposeAfterUse = DisposeAfterUse::YES;

	// Set miscellaneous properties to sane values
	_transparentColorSet = false;
	_transparentColor = 0;
	if (_palette) {
		delete _palette;
		_palette = nullptr;
	}
}

void ManagedSurface::blitFrom(const Surface &src, const byte *srcPalette) {
	blitFrom(src, Common::Rect(0, 0, src.w, src.h), Common::Point(0, 0), srcPalette);
}

void ManagedSurface::blitFrom(const Surface &src, const Common::Point &destPos, const byte *srcPalette) {
	blitFrom(src, Common::Rect(0, 0, src.w, src.h), destPos, srcPalette);
}

void ManagedSurface::blitFrom(const Surface &src, const Common::Rect &srcRect,
		const Common::Point &destPos, const byte *srcPalette) {
	blitFromInner(src, srcRect, Common::Rect(destPos.x, destPos.y, destPos.x + srcRect.width(),
		destPos.y + srcRect.height()), srcPalette);
}

void ManagedSurface::blitFrom(const Surface &src, const Common::Rect &srcRect,
		const Common::Rect &destRect, const byte *srcPalette) {
	blitFromInner(src, srcRect, destRect, srcPalette);
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
		blitFromInner(src._innerSurface, srcRect, Common::Rect(destPos.x, destPos.y, destPos.x + srcRect.width(),
			destPos.y + srcRect.height()), src._palette ? src._palette->data : nullptr);
}

void ManagedSurface::blitFrom(const ManagedSurface &src, const Common::Rect &srcRect,
		const Common::Rect &destRect) {
	if (src._transparentColorSet)
		transBlitFrom(src, srcRect, destRect, (uint32)-1);
	else
		blitFromInner(src._innerSurface, srcRect, destRect, src._palette ? src._palette->data : nullptr);
}

void ManagedSurface::blitFromInner(const Surface &src, const Common::Rect &srcRect,
		const Common::Rect &destRect, const byte *srcPalette) {

	if (destRect.isEmpty())
		return;

	const int scaleX = SCALE_THRESHOLD * srcRect.width() / destRect.width();
	const int scaleY = SCALE_THRESHOLD * srcRect.height() / destRect.height();

	if (!srcRect.isValidRect())
		return;

	// Copy format so compiler can optimize better.
	// This should allow it to do some loop optimizations and condition hoisting as it can tell nothing
	// inside of the loop will clobber the format.
	Graphics::PixelFormat destFormat = format;
	Graphics::PixelFormat srcFormat = src.format;

	bool isSameFormat = (destFormat == srcFormat);
	if (!isSameFormat) {
		assert(destFormat.bytesPerPixel == 1 || destFormat.bytesPerPixel == 2 || destFormat.bytesPerPixel == 3 || destFormat.bytesPerPixel == 4);
		assert(srcFormat.bytesPerPixel == 1 || srcFormat.bytesPerPixel == 2 || srcFormat.bytesPerPixel == 3 || srcFormat.bytesPerPixel == 4);
		if (srcFormat.bytesPerPixel == 1) {
			// When the pixel format differs, the destination must be non-paletted
			assert(!destFormat.isCLUT8() && srcPalette);
		}
	}


	uint32 alphaMask = 0;
	if (srcFormat.aBits() > 0)
		alphaMask = (((static_cast<uint32>(1) << (srcFormat.aBits() - 1)) - 1) * 2 + 1) << srcFormat.aShift;

	const bool noScale = scaleX == SCALE_THRESHOLD && scaleY == SCALE_THRESHOLD;
	for (int destY = destRect.top, scaleYCtr = 0; destY < destRect.bottom; ++destY, scaleYCtr += scaleY) {
		if (destY < 0 || destY >= h)
			continue;
		const byte *srcP = (const byte *)src.getBasePtr(srcRect.left, scaleYCtr / SCALE_THRESHOLD + srcRect.top);
		byte *destP = (byte *)getBasePtr(destRect.left, destY);

		// For paletted format, assume the palette is the same and there is no transparency.
		// We can thus do a straight copy of the pixels.
		if (destFormat.isCLUT8() && noScale) {
			int width = srcRect.width();
			if (destRect.left + width > w)
				width = w - destRect.left;
			if (destRect.left < 0) {
				srcP -= destRect.left;
				destP -= destRect.left;
				width += destRect.left;
			}
			if (width > 0)
				Common::copy(srcP, srcP + width, destP);
			continue;
		}

		// Loop through drawing the pixels of the row
		for (int destX = destRect.left, xCtr = 0, scaleXCtr = 0; destX < destRect.right; ++destX, ++xCtr, scaleXCtr += scaleX) {
			if (destX < 0 || destX >= w)
				continue;

			const byte *srcVal = &srcP[scaleXCtr / SCALE_THRESHOLD * srcFormat.bytesPerPixel];
			byte *destVal = &destP[xCtr * destFormat.bytesPerPixel];
			if (destFormat.isCLUT8()) {
				*destVal = *srcVal;
				continue;
			}

			uint32 col = 0;
			// Use the src's pixel format to split up the source pixel
			if (srcFormat.bytesPerPixel == 1)
				col = *reinterpret_cast<const uint8 *>(srcVal);
			else if (srcFormat.bytesPerPixel == 2)
				col = *reinterpret_cast<const uint16 *>(srcVal);
			else if (srcFormat.bytesPerPixel == 4)
				col = *reinterpret_cast<const uint32 *>(srcVal);
			else
				col = READ_UINT24(srcVal);

			const bool isOpaque = srcFormat.isCLUT8() ? true : ((col & alphaMask) == alphaMask);
			const bool isTransparent = srcFormat.isCLUT8() ? false : ((col & alphaMask) == 0);

			uint32 destPixel = 0;

			// Need to check isOpaque in case alpha mask is 0
			if (!isOpaque && isTransparent) {
				// Completely transparent, so skip
				continue;
			} else if (isOpaque && isSameFormat) {
				// Completely opaque, same format, copy the entire value
				destPixel = col;
			} else {
				byte rSrc, gSrc, bSrc, aSrc;
				byte aDest = 0, rDest = 0, gDest = 0, bDest = 0;

				// Different format or partially transparent
				if (srcFormat.isCLUT8()) {
					rSrc = srcPalette[(col * 3) + 0];
					gSrc = srcPalette[(col * 3) + 1];
					bSrc = srcPalette[(col * 3) + 2];
					aSrc = 0xff;
				} else {
					srcFormat.colorToARGB(col, aSrc, rSrc, gSrc, bSrc);
				}

				if (isOpaque) {
					aDest = aSrc;
					rDest = rSrc;
					gDest = gSrc;
					bDest = bSrc;
				} else {
					// Partially transparent, so calculate new pixel colors
					uint32 destColor;
					if (destFormat.bytesPerPixel == 1)
						destColor = *reinterpret_cast<uint8 *>(destVal);
					else if (destFormat.bytesPerPixel == 2)
						destColor = *reinterpret_cast<uint16 *>(destVal);
					else if (destFormat.bytesPerPixel == 4)
						destColor = *reinterpret_cast<uint32 *>(destVal);
					else
						destColor = READ_UINT24(destVal);

					destFormat.colorToARGB(destColor, aDest, rDest, gDest, bDest);

					if (aDest == 0xff) {
						// Opaque target
						rDest = static_cast<uint8>((((rDest * (255U - aSrc) + rSrc * aSrc) * (257U * 257U)) >> 24) & 0xff);
						gDest = static_cast<uint8>((((gDest * (255U - aSrc) + gSrc * aSrc) * (257U * 257U)) >> 24) & 0xff);
						bDest = static_cast<uint8>((((bDest * (255U - aSrc) + bSrc * aSrc) * (257U * 257U)) >> 24) & 0xff);
					} else {
						// Translucent target
						double sAlpha = (double)aSrc / 255.0;
						double dAlpha = (double)aDest / 255.0;
						dAlpha *= (1.0 - sAlpha);
						rDest = static_cast<uint8>((rSrc * sAlpha + rDest * dAlpha) / (sAlpha + dAlpha));
						gDest = static_cast<uint8>((gSrc * sAlpha + gDest * dAlpha) / (sAlpha + dAlpha));
						bDest = static_cast<uint8>((bSrc * sAlpha + bDest * dAlpha) / (sAlpha + dAlpha));
						aDest = static_cast<uint8>(255. * (sAlpha + dAlpha));
					}
				}

				destPixel = destFormat.ARGBToColor(aDest, rDest, gDest, bDest);
			}

			if (destFormat.bytesPerPixel == 1)
				*(uint8 *)destVal = destPixel;
			else if (destFormat.bytesPerPixel == 2)
				*(uint16 *)destVal = destPixel;
			else if (destFormat.bytesPerPixel == 4)
				*(uint32 *)destVal = destPixel;
			else
				WRITE_UINT24(destVal, destPixel);
		}
	}

	addDirtyRect(Common::Rect(0, 0, this->w, this->h));
}

void ManagedSurface::transBlitFrom(const Surface &src, uint32 transColor, bool flipped,
		uint32 overrideColor, uint32 srcAlpha, const byte *srcPalette) {
	transBlitFrom(src, Common::Rect(0, 0, src.w, src.h), Common::Rect(0, 0, this->w, this->h),
		transColor, flipped, overrideColor, srcAlpha, nullptr, false, srcPalette);
}

void ManagedSurface::transBlitFrom(const Surface &src, const Common::Point &destPos,
		uint32 transColor, bool flipped, uint32 overrideColor, uint32 srcAlpha, const byte *srcPalette) {
	transBlitFrom(src, Common::Rect(0, 0, src.w, src.h), Common::Rect(destPos.x, destPos.y,
		destPos.x + src.w, destPos.y + src.h), transColor, flipped, overrideColor, srcAlpha, nullptr, false, srcPalette);
}

void ManagedSurface::transBlitFrom(const Surface &src, const Common::Point &destPos,
		const ManagedSurface &mask, const byte *srcPalette) {
	transBlitFrom(src, Common::Rect(0, 0, src.w, src.h), Common::Rect(destPos.x, destPos.y,
		destPos.x + src.w, destPos.y + src.h), 0, false, 0, 0xff, &mask._innerSurface, true, srcPalette);
}

void ManagedSurface::transBlitFrom(const Surface &src, const Common::Point &destPos,
		const Surface &mask, const byte *srcPalette) {
	transBlitFrom(src, Common::Rect(0, 0, src.w, src.h), Common::Rect(destPos.x, destPos.y,
		destPos.x + src.w, destPos.y + src.h), 0, false, 0, 0xff, &mask, true, srcPalette);
}

void ManagedSurface::transBlitFrom(const Surface &src, const Common::Rect &srcRect,
		const Common::Point &destPos, uint32 transColor, bool flipped, uint32 overrideColor, uint32 srcAlpha, const byte *srcPalette) {
	transBlitFrom(src, srcRect, Common::Rect(destPos.x, destPos.y,
		destPos.x + srcRect.width(), destPos.y + srcRect.height()), transColor, flipped, overrideColor, srcAlpha, nullptr, false, srcPalette);
}

void ManagedSurface::transBlitFrom(const Surface &src, const Common::Rect &srcRect, const Common::Rect &destRect, const byte *srcPalette) {
	transBlitFrom(src, srcRect, destRect, 0, false, 0, 0xff, nullptr, false, srcPalette);
}

void ManagedSurface::transBlitFrom(const Surface &src, const Common::Rect &srcRect,
		const Common::Rect &destRect, uint32 transColor, bool flipped, uint32 overrideColor, uint32 srcAlpha,
		const Surface *mask, bool maskOnly, const byte *srcPalette) {
	transBlitFromInner(src, srcRect, destRect, transColor, flipped, overrideColor, srcAlpha,
		srcPalette, nullptr, mask, maskOnly);
}

void ManagedSurface::transBlitFrom(const ManagedSurface &src, uint32 transColor, bool flipped,
		uint32 overrideColor, uint32 srcAlpha) {
	transBlitFrom(src, Common::Rect(0, 0, src.w, src.h), Common::Rect(0, 0, this->w, this->h),
		transColor, flipped, overrideColor, srcAlpha);
}

void ManagedSurface::transBlitFrom(const ManagedSurface &src, const Common::Point &destPos,
		uint32 transColor, bool flipped, uint32 overrideColor, uint32 srcAlpha) {
	transBlitFrom(src, Common::Rect(0, 0, src.w, src.h), Common::Rect(destPos.x, destPos.y,
		destPos.x + src.w, destPos.y + src.h), transColor, flipped, overrideColor, srcAlpha);
}

void ManagedSurface::transBlitFrom(const ManagedSurface &src, const Common::Point &destPos,
		const ManagedSurface &mask) {
	transBlitFrom(src, Common::Rect(0, 0, src.w, src.h), Common::Rect(destPos.x, destPos.y,
		destPos.x + src.w, destPos.y + src.h), 0, false, 0, 0xff, &mask._innerSurface, true);
}

void ManagedSurface::transBlitFrom(const ManagedSurface &src, const Common::Rect &srcRect,
		const Common::Point &destPos, uint32 transColor, bool flipped, uint32 overrideColor, uint32 srcAlpha) {
	uint32 tColor = !transColor && src._transparentColorSet ? src._transparentColor : transColor;

	transBlitFrom(src, srcRect, Common::Rect(destPos.x, destPos.y, destPos.x + srcRect.width(),
		destPos.y + srcRect.height()), tColor, flipped, overrideColor, srcAlpha);
}

void ManagedSurface::transBlitFrom(const ManagedSurface &src, const Common::Rect &srcRect,
		const Common::Rect &destRect, uint32 transColor, bool flipped, uint32 overrideColor, uint32 srcAlpha,
		const Surface *mask, bool maskOnly) {
	if (transColor == (uint32)-1 && src._transparentColorSet)
		transColor = src._transparentColor;
	const byte *srcPalette = src._palette ? src._palette->data : nullptr;
	const byte *dstPalette = _palette ? _palette->data : nullptr;

	transBlitFromInner(src._innerSurface, srcRect, destRect, transColor, flipped, overrideColor,
		srcAlpha, srcPalette, dstPalette, mask, maskOnly);
}

void ManagedSurface::rawBlitFrom(const ManagedSurface &src, const Common::Rect &srcRect,
				 const Common::Point &destPos) {
	Common::Rect dstRect(destPos.x, destPos.y, destPos.x + srcRect.width(), destPos.y + srcRect.height());
	blitFromInner(src._innerSurface, srcRect, dstRect, src._palette ? src._palette->data : nullptr);
}

static uint findBestColor(const byte *palette, byte cr, byte cg, byte cb) {
	uint bestColor = 0;
	double min = 0xFFFFFFFF;

	for (uint i = 0; i < 256; ++i) {
		int rmean = (*(palette + 3 * i + 0) + cr) / 2;
		int r = *(palette + 3 * i + 0) - cr;
		int g = *(palette + 3 * i + 1) - cg;
		int b = *(palette + 3 * i + 2) - cb;

		double dist = sqrt((((512 + rmean) * r * r) >> 8) + 4 * g * g + (((767 - rmean) * b * b) >> 8));
		if (min > dist) {
			bestColor = i;
			min = dist;
		}
	}

	return bestColor;
}

static byte *createPaletteLookup(const byte *srcPalette, const byte *dstPalette) {
	byte *lookup = new byte[256];

	for (int i = 0; i < 256; i++) {
		byte rSrc = srcPalette[(i * 3) + 0];
		byte gSrc = srcPalette[(i * 3) + 1];
		byte bSrc = srcPalette[(i * 3) + 2];
		byte rDst = dstPalette[(i * 3) + 0];
		byte gDst = dstPalette[(i * 3) + 1];
		byte bDst = dstPalette[(i * 3) + 2];

		if (rSrc == rDst && gSrc == gDst && bSrc == bDst) {
			lookup[i] = i;
		} else {
			lookup[i] = findBestColor(dstPalette, rSrc, gSrc, bSrc);
		}
	}

	return lookup;
}

template<typename TSRC, typename TDEST>
void transBlitPixel(TSRC srcVal, TDEST &destVal, const Graphics::PixelFormat &srcFormat, const Graphics::PixelFormat &destFormat,
		uint32 overrideColor, uint32 srcAlpha, const byte *srcPalette, const byte *lookup) {
	// Decode and re-encode each pixel
	byte aSrc, rSrc, gSrc, bSrc;
	if (srcFormat.isCLUT8()) {
		assert(srcPalette != nullptr);	// Catch the cases when palette is missing

		// Get the palette color
		rSrc = srcPalette[(srcVal * 3) + 0];
		gSrc = srcPalette[(srcVal * 3) + 1];
		bSrc = srcPalette[(srcVal * 3) + 2];
		aSrc = 0xff;
	} else {
		srcFormat.colorToARGB(srcVal, aSrc, rSrc, gSrc, bSrc);
	}

	if (srcAlpha != 0xff) {
		aSrc = aSrc * srcAlpha / 255;
	}

	byte aDest, rDest, gDest, bDest;
	if (aSrc == 0) {
		// Completely transparent, so skip
		return;
	} else if (aSrc == 0xff) {
		// Completely opaque, so copy RGB values over
		rDest = rSrc;
		gDest = gSrc;
		bDest = bSrc;
		aDest = 0xff;
	} else {
		// Partially transparent, so calculate new pixel colors
		destFormat.colorToARGB(destVal, aDest, rDest, gDest, bDest);
		double sAlpha = (double)aSrc / 255.0;
		double dAlpha = (double)aDest / 255.0;
		dAlpha *= (1.0 - sAlpha);
		rDest = static_cast<uint8>((rSrc * sAlpha + rDest * dAlpha) / (sAlpha + dAlpha));
		gDest = static_cast<uint8>((gSrc * sAlpha + gDest * dAlpha) / (sAlpha + dAlpha));
		bDest = static_cast<uint8>((bSrc * sAlpha + bDest * dAlpha) / (sAlpha + dAlpha));
		aDest = static_cast<uint8>(255. * (sAlpha + dAlpha));
	}

	destVal = destFormat.ARGBToColor(aDest, rDest, gDest, bDest);
}

template<>
void transBlitPixel<byte, byte>(byte srcVal, byte &destVal, const Graphics::PixelFormat &srcFormat, const Graphics::PixelFormat &destFormat,
		uint32 overrideColor, uint32 srcAlpha, const byte *srcPalette, const byte *lookup) {
	if (srcAlpha == 0) {
		// Completely transparent, so skip
		return;
	}

	destVal = overrideColor ? overrideColor : srcVal;

	if (lookup)
		destVal = lookup[destVal];
}

template<typename TSRC, typename TDEST>
void transBlit(const Surface &src, const Common::Rect &srcRect, ManagedSurface &dest, const Common::Rect &destRect,
		TSRC transColor, bool flipped, uint32 overrideColor, uint32 srcAlpha, const byte *srcPalette,
		const byte *dstPalette, const Surface *mask, bool maskOnly) {
	int scaleX = SCALE_THRESHOLD * srcRect.width() / destRect.width();
	int scaleY = SCALE_THRESHOLD * srcRect.height() / destRect.height();
	byte rst = 0, gst = 0, bst = 0, rdt = 0, gdt = 0, bdt = 0;
	byte r = 0, g = 0, b = 0;

	byte *lookup = nullptr;
	if (srcPalette && dstPalette)
		lookup = createPaletteLookup(srcPalette, dstPalette);

	// If we're dealing with a 32-bit source surface, we need to split up the RGB,
	// since we'll want to find matching RGB pixels irrespective of the alpha
	bool isSrcTrans32 = src.format.aBits() != 0 && transColor != (uint32)-1 && transColor > 0;
	if (isSrcTrans32) {
		src.format.colorToRGB(transColor, rst, gst, bst);
	}
	bool isDestTrans32 = dest.format.aBits() != 0 && dest.hasTransparentColor();
	if (isDestTrans32) {
		dest.format.colorToRGB(dest.getTransparentColor(), rdt, gdt, bdt);
	}

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
			TDEST &destVal = destLine[xCtr];

			dest.format.colorToRGB(destVal, r, g, b);

			// Check if dest pixel is transparent
			bool isDestPixelTrans = false;
			if (isDestTrans32) {
				dest.format.colorToRGB(destVal, r, g, b);
				if (rdt == r && gdt == g && bdt == b)
					isDestPixelTrans = true;
			} else if (dest.hasTransparentColor()) {
				isDestPixelTrans = destVal == dest.getTransparentColor();
			}

			if (isSrcTrans32 && !maskOnly) {
				src.format.colorToRGB(srcVal, r, g, b);
				if (rst == r && gst == g && bst == b)
					continue;

			} else if (srcVal == transColor && !maskOnly)
				continue;

			if (mask) {
				TSRC mskVal = mskLine[flipped ? src.w - scaleXCtr / SCALE_THRESHOLD - 1 : scaleXCtr / SCALE_THRESHOLD];
				if (!mskVal)
					continue;

				if (isDestPixelTrans)
					// Remove transparent color on dest so it isn't alpha blended
					destVal = 0;

				transBlitPixel<TSRC, TDEST>(srcVal, destVal, src.format, dest.format, overrideColor, mskVal, srcPalette, lookup);
			} else {
				if (isDestPixelTrans)
					// Remove transparent color on dest so it isn't alpha blended
					destVal = 0;

				transBlitPixel<TSRC, TDEST>(srcVal, destVal, src.format, dest.format, overrideColor, srcAlpha, srcPalette, lookup);
			}
		}
	}

	delete[] lookup;
}

#define HANDLE_BLIT(SRC_BYTES, DEST_BYTES, SRC_TYPE, DEST_TYPE) \
	if (src.format.bytesPerPixel == SRC_BYTES && format.bytesPerPixel == DEST_BYTES) \
		transBlit<SRC_TYPE, DEST_TYPE>(src, srcRect, *this, destRect, transColor, flipped, overrideColor, srcAlpha, srcPalette, dstPalette, mask, maskOnly); \
	else

void ManagedSurface::transBlitFromInner(const Surface &src, const Common::Rect &srcRect,
		const Common::Rect &destRect, uint32 transColor, bool flipped, uint32 overrideColor,
		uint32 srcAlpha, const byte *srcPalette, const byte *dstPalette, const Surface *mask, bool maskOnly) {
	if (src.w == 0 || src.h == 0 || destRect.width() == 0 || destRect.height() == 0)
		return;

	if (mask) {
		if (mask->w != src.w || mask->h != src.h)
			error("Surface::transBlitFrom: mask dimensions do not match src");
	}

	HANDLE_BLIT(1, 1, uint8,  uint8)
	HANDLE_BLIT(1, 2, uint8,  uint16)
	HANDLE_BLIT(1, 4, uint8,  uint32)
	HANDLE_BLIT(2, 1, uint16, uint8)
	HANDLE_BLIT(2, 2, uint16, uint16)
	HANDLE_BLIT(2, 4, uint16, uint32)
	HANDLE_BLIT(4, 1, uint32, uint8)
	HANDLE_BLIT(4, 2, uint32, uint16)
	HANDLE_BLIT(4, 4, uint32, uint32)
	error("Surface::transBlitFrom: bytesPerPixel must be 1, 2, or 4");

	// Mark the affected area
	addDirtyRect(destRect);
}

#undef HANDLE_BLIT

Common::Rect ManagedSurface::blendBlitTo(ManagedSurface &target,
										 const int posX, const int posY,
										 const int flipping,
										 const Common::Rect *srcRect,
										 const uint colorMod,
										 const int width, const int height,
										 const TSpriteBlendMode blend,
										 const AlphaType alphaType) {
	return blendBlitTo(*target.surfacePtr(), posX, posY, flipping, srcRect, colorMod, width, height, blend, alphaType);
}
Common::Rect ManagedSurface::blendBlitTo(Surface &target,
										 const int posX, const int posY,
										 const int flipping,
										 const Common::Rect *srcRect,
										 const uint colorMod,
										 const int width, const int height,
										 const TSpriteBlendMode blend,
										 const AlphaType alphaType) {
	Common::Rect srcArea = srcRect ? *srcRect : Common::Rect(0, 0, w, h);
	Common::Rect dstArea(posX, posY, posX + (width == -1 ? srcArea.width() : width), posY + (height == -1 ? srcArea.height() : height));
	
	if (!isBlendBlitPixelFormatSupported(format, target.format)) {
		warning("ManagedSurface::blendBlitTo only accepts RGBA32!");
		return Common::Rect(0, 0, 0, 0);
	}

	// Alpha is zero
	if ((colorMod & MS_ARGB(255, 0, 0, 0)) == 0) return Common::Rect(0, 0, 0, 0);

	const int scaleX = BlendBlit::getScaleFactor(srcArea.width(), dstArea.width());
	const int scaleY = BlendBlit::getScaleFactor(srcArea.height(), dstArea.height());
	int scaleXoff = 0, scaleYoff = 0;

	if (dstArea.left < 0) {
		scaleXoff = (-dstArea.left * scaleX) % BlendBlit::SCALE_THRESHOLD;
		srcArea.left += -dstArea.left * scaleX / BlendBlit::SCALE_THRESHOLD;
		dstArea.left = 0;
	}

	if (dstArea.top < 0) {
		scaleYoff = (-dstArea.top * scaleY) % BlendBlit::SCALE_THRESHOLD;
		srcArea.top += -dstArea.top * scaleY / BlendBlit::SCALE_THRESHOLD;
		dstArea.top = 0;
	}

	if (dstArea.right > target.w) {
		srcArea.right -= (dstArea.right - target.w) * scaleX / BlendBlit::SCALE_THRESHOLD;
		dstArea.right = target.w;
	}

	if (dstArea.bottom > target.h) {
		srcArea.bottom -= (dstArea.bottom - target.h) * scaleY / BlendBlit::SCALE_THRESHOLD;
		dstArea.bottom = target.h;
	}

	if (flipping & FLIP_H) {
		int tmp_w = srcArea.width();
		srcArea.left = w - srcArea.right;
		srcArea.right = srcArea.left + tmp_w;
		scaleXoff = (BlendBlit::SCALE_THRESHOLD - (scaleXoff + dstArea.width() * scaleX)) % BlendBlit::SCALE_THRESHOLD;
	}

	if (flipping & FLIP_V) {
		int tmp_h = srcArea.height();
		srcArea.top = h - srcArea.bottom;
		srcArea.bottom = srcArea.top + tmp_h;
		scaleYoff = (BlendBlit::SCALE_THRESHOLD - (scaleYoff + dstArea.height() * scaleY)) % BlendBlit::SCALE_THRESHOLD;
	}

	if (!dstArea.isEmpty() && !srcArea.isEmpty()) {
		BlendBlit::blit(
			(byte *)target.getBasePtr(0, 0),
			(const byte *)getBasePtr(srcArea.left, srcArea.top),
			target.pitch, pitch,
			dstArea.left, dstArea.top,
			dstArea.width(), dstArea.height(),
			scaleX, scaleY,
			scaleXoff, scaleYoff,
			colorMod, flipping,
			blend, alphaType);
	}

	if (dstArea.isEmpty()) return Common::Rect(0, 0, 0, 0);
	else return Common::Rect(0, 0, dstArea.width(), dstArea.height());
}

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

void ManagedSurface::clear(uint32 color) {
	if (!empty())
		fillRect(getBounds(), color);
}

void ManagedSurface::clearPalette() {
	if (_palette) {
		delete _palette;
		_palette = nullptr;
	}
}

bool ManagedSurface::hasPalette() const {
	return _palette && _palette->size > 0;
}

void ManagedSurface::grabPalette(byte *colors, uint start, uint num) const {
	_palette->grab(colors, start, num);
}

void ManagedSurface::grabPalette(Palette &palette, uint start, uint num) const {
	_palette->grab(palette, start, num);
}

void ManagedSurface::setPalette(const byte *colors, uint start, uint num) {
	if (!_palette)
		_palette = new Palette(256);
	_palette->set(colors, start, num);

	if (_owner)
		_owner->setPalette(colors, start, num);
}

void ManagedSurface::setPalette(const Palette &palette, uint start, uint num) {
	if (!_palette)
		_palette = new Palette(256);
	_palette->set(palette, start, num);

	if (_owner)
		_owner->setPalette(palette, start, num);
}

} // End of namespace Graphics
