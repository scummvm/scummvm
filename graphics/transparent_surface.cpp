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

static const int kAModShift = 0;

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
		Graphics::blendBlitUnfiltered(
			(byte *)target.getBasePtr(0, 0),
			(byte *)img->getBasePtr(0, 0),
			target.pitch, img->pitch,
			posX, posY, img->w, img->h, BLEND_BLIT_SCALE_THRESHOLD, BLEND_BLIT_SCALE_THRESHOLD,
			color, flipping,
			blendMode, _alphaMode);
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
		Graphics::blendBlitUnfiltered(
			(byte *)target.getBasePtr(0, 0),
			(byte *)img->getBasePtr(0, 0),
			target.pitch, img->pitch,
			posX, posY, img->w, img->h, BLEND_BLIT_SCALE_THRESHOLD, BLEND_BLIT_SCALE_THRESHOLD,
			color, flipping,
			blendMode, _alphaMode);
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

} // End of namespace Graphics
