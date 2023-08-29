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
#include "common/util.h"
#include "common/rect.h"
#include "common/math.h"
#include "common/textconsole.h"
#include "graphics/blit.h"
#include "graphics/primitives.h"
#include "graphics/transparent_surface.h"
#include "graphics/transform_tools.h"

#include "graphics/managed_surface.h"

namespace Graphics {

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

Common::Rect TransparentSurface::blit(Graphics::Surface &target, int posX, int posY, int flipping, Common::Rect *pPartRect, uint color, int width, int height, TSpriteBlendMode blendMode) {
	Common::Rect retSize;
	retSize.top = 0;
	retSize.left = 0;
	retSize.setWidth(0);
	retSize.setHeight(0);
	// Check if we need to draw anything at all
	int ca = (color >> BlendBlit::kAModShift) & 0xff;

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

	int xOffset = 0, yOffset = 0, srcW = srcImage.w, srcH = srcImage.h;

	if (pPartRect) {
		xOffset = pPartRect->left;
		yOffset = pPartRect->top;
		srcW = pPartRect->width();
		srcH = pPartRect->height();
	}

	if (width == -1) {
		width = srcW;
	}
	if (height == -1) {
		height = srcH;
	}

	int scaleX = BlendBlit::getScaleFactor(srcW, width), scaleXoff = 0;
	int scaleY = BlendBlit::getScaleFactor(srcH, height), scaleYoff = 0;

#ifdef SCALING_TESTING
	// Hardcode scaling to 66% to test scaling
	width = width * 2 / 3;
	height = height * 2 / 3;
#endif

	// Handle off-screen clipping
	if (posY < 0) {
		height = MAX(0, (int)height + posY);
		scaleYoff += (-posY * scaleY) % BlendBlit::SCALE_THRESHOLD;
		yOffset += -posY * scaleY / BlendBlit::SCALE_THRESHOLD;
		srcH = MAX(0, srcH + posY * scaleY / BlendBlit::SCALE_THRESHOLD);
		posY = 0;
	}

	if (posX < 0) {
		width = MAX(0, (int)width + posX);
		scaleXoff += (-posX * scaleX) % BlendBlit::SCALE_THRESHOLD;
		xOffset += -posX * scaleX / BlendBlit::SCALE_THRESHOLD;
		srcW = MAX(0, srcW + posX * scaleX / BlendBlit::SCALE_THRESHOLD);
		posX = 0;
	}

	if (width + posX > target.w) {
		srcW -= ((width + posX) - target.w) * scaleX / BlendBlit::SCALE_THRESHOLD;
		width = target.w - posX;
	}

	if (height + posY > target.h) {
		srcH -= ((height + posY) - target.h) * scaleY / BlendBlit::SCALE_THRESHOLD;
		height = target.h - posY;
	}
	if (flipping & FLIP_H) {
		scaleXoff = (BlendBlit::SCALE_THRESHOLD - (scaleXoff + width * scaleX)) % BlendBlit::SCALE_THRESHOLD;
		xOffset = this->w - (xOffset + srcW);
	}

	if (flipping & FLIP_V) {
		scaleYoff = (BlendBlit::SCALE_THRESHOLD - (scaleYoff + height * scaleY)) % BlendBlit::SCALE_THRESHOLD;
		yOffset = this->h - (yOffset + srcH);
	}

	// Flip surface
	if ((width > 0) && (height > 0)) {
		BlendBlit::blit(
			(byte *)target.getBasePtr(0, 0),
			(byte *)srcImage.getBasePtr(xOffset, yOffset),
			target.pitch, srcImage.pitch,
			posX, posY, width, height, scaleX, scaleY,
			scaleXoff, scaleYoff,
			color, flipping,
			blendMode, _alphaMode);
	}

	retSize.setWidth((int16)width);
	retSize.setHeight((int16)height);

	return retSize;
}

Common::Rect TransparentSurface::blitClip(Graphics::Surface &target, Common::Rect clippingArea, int posX, int posY, int flipping, Common::Rect *pPartRect, uint color, int width, int height, TSpriteBlendMode blendMode) {
	Common::Rect retSize;
	retSize.top = 0;
	retSize.left = 0;
	retSize.setWidth(0);
	retSize.setHeight(0);
	// Check if we need to draw anything at all
	int ca = (color >> BlendBlit::kAModShift) & 0xff;

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

	int xOffset = 0, yOffset = 0, srcW = srcImage.w, srcH = srcImage.h;

	if (pPartRect) {
		xOffset = pPartRect->left;
		yOffset = pPartRect->top;
		srcW = pPartRect->width();
		srcH = pPartRect->height();
	}

	if (width == -1) {
		width = srcW;
	}
	if (height == -1) {
		height = srcH;
	}

	int scaleX = BlendBlit::getScaleFactor(srcW, width), scaleXoff = 0;;
	int scaleY = BlendBlit::getScaleFactor(srcH, height), scaleYoff = 0;;

#ifdef SCALING_TESTING
	// Hardcode scaling to 66% to test scaling
	width = width * 2 / 3;
	height = height * 2 / 3;
#endif

	// Handle off-screen clipping
	if (posY < clippingArea.top) {
		posY -= clippingArea.top;
		scaleYoff += (-posY * scaleY) % BlendBlit::SCALE_THRESHOLD;
		yOffset += -posY * scaleY / BlendBlit::SCALE_THRESHOLD;
		height = MAX(0, (int)height + posY);
		srcH = MAX(0, srcH + posY * scaleY / BlendBlit::SCALE_THRESHOLD);
		posY = clippingArea.top;
	}

	if (posX < clippingArea.left) {
		posX -= clippingArea.left;
		scaleXoff += (-posX * scaleX) % BlendBlit::SCALE_THRESHOLD;
		xOffset += -posX * scaleX / BlendBlit::SCALE_THRESHOLD;
		width = MAX(0, (int)width + posX);
		srcW = MAX(0, srcW + posX * scaleX / BlendBlit::SCALE_THRESHOLD);
		posX = clippingArea.left;
	}

	if (width + posX > clippingArea.right) {
		srcW -= ((width + posX) - clippingArea.right) * scaleX / BlendBlit::SCALE_THRESHOLD;
		width = clippingArea.right - posX;
	}

	if (height + posY > clippingArea.bottom) {
		srcH -= ((height + posY) - clippingArea.bottom) * scaleY / BlendBlit::SCALE_THRESHOLD;
		height = clippingArea.bottom - posY;
	}
	if (flipping & FLIP_H) {
		scaleXoff = (BlendBlit::SCALE_THRESHOLD - (scaleXoff + width * scaleX)) % BlendBlit::SCALE_THRESHOLD;
		xOffset = this->w - (xOffset + srcW);
	}

	if (flipping & FLIP_V) {
		scaleYoff = (BlendBlit::SCALE_THRESHOLD - (scaleYoff + height * scaleY)) % BlendBlit::SCALE_THRESHOLD;
		yOffset = this->h - (yOffset + srcH);
	}

	// Flip surface
	if ((width > 0) && (height > 0)) {
		BlendBlit::blit(
			(byte *)target.getBasePtr(0, 0),
			(byte *)srcImage.getBasePtr(xOffset, yOffset),
			target.pitch, srcImage.pitch,
			posX, posY, width, height, scaleX, scaleY,
			scaleXoff, scaleYoff,
			color, flipping,
			blendMode, _alphaMode);
	}

	retSize.setWidth(width);
	retSize.setHeight(height);

	return retSize;
}

AlphaType TransparentSurface::getAlphaMode() const {
	return _alphaMode;
}

void TransparentSurface::setAlphaMode(AlphaType mode) {
	_alphaMode = mode;
}

TransparentSurface *TransparentSurface::scale(int16 newWidth, int16 newHeight, bool filtering) const {

	TransparentSurface *target = new TransparentSurface();

	target->create(newWidth, newHeight, format);

	if (filtering) {
		scaleBlitBilinear((byte *)target->getPixels(), (const byte *)getPixels(), target->pitch, pitch, target->w, target->h, w, h, format);
	} else {
		scaleBlit((byte *)target->getPixels(), (const byte *)getPixels(), target->pitch, pitch, target->w, target->h, w, h, format);
	}

	return target;
}

TransparentSurface *TransparentSurface::rotoscale(const TransformStruct &transform, bool filtering) const {

	Common::Point newHotspot;
	Common::Rect rect = TransformTools::newRect(Common::Rect((int16)w, (int16)h), transform, &newHotspot);

	TransparentSurface *target = new TransparentSurface();

	target->create((uint16)rect.right - rect.left, (uint16)rect.bottom - rect.top, this->format);

	if (filtering) {
		rotoscaleBlitBilinear((byte *)target->getPixels(), (const byte *)getPixels(), target->pitch, pitch, target->w, target->h, w, h, format, transform, newHotspot);
	} else {
		rotoscaleBlit((byte *)target->getPixels(), (const byte *)getPixels(), target->pitch, pitch, target->w, target->h, w, h, format, transform, newHotspot);
	}

	return target;
}

TransparentSurface *TransparentSurface::convertTo(const PixelFormat &dstFormat, const byte *srcPalette, int srcPaletteCount, const byte *dstPalette, int dstPaletteCount, DitherMethod method) const {
	assert(pixels);

	TransparentSurface *surface = new TransparentSurface();

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

		convertPaletteToMap(map, srcPalette, 256, dstFormat);
		crossBlitMap(dst, src, surface->pitch, pitch, w, h, dstFormat.bytesPerPixel, map);
	} else {
		// Converting from high color to high color
		crossBlit(dst, src, surface->pitch, pitch, w, h, dstFormat, format);
	}

	return surface;
}

} // End of namespace Graphics
