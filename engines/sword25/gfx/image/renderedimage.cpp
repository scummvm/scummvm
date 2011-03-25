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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

// -----------------------------------------------------------------------------
// INCLUDES
// -----------------------------------------------------------------------------

#include "sword25/package/packagemanager.h"
#include "sword25/gfx/image/pngloader.h"
#include "sword25/gfx/image/renderedimage.h"

#include "common/system.h"

namespace Sword25 {

// -----------------------------------------------------------------------------
// CONSTRUCTION / DESTRUCTION
// -----------------------------------------------------------------------------

RenderedImage::RenderedImage(const Common::String &filename, bool &result) :
	_data(0),
	_width(0),
	_height(0) {
	result = false;

	PackageManager *pPackage = Kernel::getInstance()->getPackage();
	assert(pPackage);

	_backSurface = Kernel::getInstance()->getGfx()->getSurface();

	// Load file
	byte *pFileData;
	uint fileSize;
	pFileData = pPackage->getFile(filename, &fileSize);
	if (!pFileData) {
		error("File \"%s\" could not be loaded.", filename.c_str());
		return;
	}

	// Determine image properties
	int pitch;
	if (!PNGLoader::imageProperties(pFileData, fileSize, _width, _height)) {
		error("Could not read image properties.");
		delete[] pFileData;
		return;
	}

	// Uncompress the image
	if (!PNGLoader::decodeImage(pFileData, fileSize, _data, _width, _height, pitch)) {
		error("Could not decode image.");
		delete[] pFileData;
		return;
	}

	// Cleanup FileData
	delete[] pFileData;

	_doCleanup = true;

	result = true;
	return;
}

// -----------------------------------------------------------------------------

RenderedImage::RenderedImage(uint width, uint height, bool &result) :
	_width(width),
	_height(height) {

	_data = new byte[width * height * 4];
	Common::set_to(_data, &_data[width * height * 4], 0);

	_backSurface = Kernel::getInstance()->getGfx()->getSurface();

	_doCleanup = true;

	result = true;
	return;
}

RenderedImage::RenderedImage() : _width(0), _height(0), _data(0) {
	_backSurface = Kernel::getInstance()->getGfx()->getSurface();

	_doCleanup = false;

	return;
}

// -----------------------------------------------------------------------------

RenderedImage::~RenderedImage() {
	if (_doCleanup)
		delete[] _data;
}

// -----------------------------------------------------------------------------

bool RenderedImage::fill(const Common::Rect *pFillRect, uint color) {
	error("Fill() is not supported.");
	return false;
}

// -----------------------------------------------------------------------------

bool RenderedImage::setContent(const byte *pixeldata, uint size, uint offset, uint stride) {
	// Check if PixelData contains enough pixel to create an image with image size equals width * height
	if (size < static_cast<uint>(_width * _height * 4)) {
		error("PixelData vector is too small to define a 32 bit %dx%d image.", _width, _height);
		return false;
	}

	const byte *in = &pixeldata[offset];
	byte *out = _data;

	for (int i = 0; i < _height; i++) {
		memcpy(out, in, _width * 4);
		out += _width * 4;
		in += stride;
	}

	return true;
}

void RenderedImage::replaceContent(byte *pixeldata, int width, int height) {
	_width = width;
	_height = height;
	_data = pixeldata;
}
// -----------------------------------------------------------------------------

uint RenderedImage::getPixel(int x, int y) {
	error("GetPixel() is not supported. Returning black.");
	return 0;
}

// -----------------------------------------------------------------------------

bool RenderedImage::blit(int posX, int posY, int flipping, Common::Rect *pPartRect, uint color, int width, int height) {
	int ca = (color >> 24) & 0xff;

	// Check if we need to draw anything at all
	if (ca == 0)
		return true;

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
	srcImage.bytesPerPixel = 4;
	srcImage.pitch = _width * 4;
	srcImage.w = _width;
	srcImage.h = _height;
	srcImage.pixels = _data;

	if (pPartRect) {
		srcImage.pixels = &_data[pPartRect->top * srcImage.pitch + pPartRect->left * 4];
		srcImage.w = pPartRect->right - pPartRect->left;
		srcImage.h = pPartRect->bottom - pPartRect->top;

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
		img = imgScaled = scale(srcImage, width, height);
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

	img->w = CLIP((int)img->w, 0, (int)MAX((int)_backSurface->w - posX, 0));
	img->h = CLIP((int)img->h, 0, (int)MAX((int)_backSurface->h - posY, 0));

	if ((img->w > 0) && (img->h > 0)) {
		int xp = 0, yp = 0;

		int inStep = 4;
		int inoStep = img->pitch;
		if (flipping & Image::FLIP_V) {
			inStep = -inStep;
			xp = img->w - 1;
		}

		if (flipping & Image::FLIP_H) {
			inoStep = -inoStep;
			yp = img->h - 1;
		}

		byte *ino = (byte *)img->getBasePtr(xp, yp);
		byte *outo = (byte *)_backSurface->getBasePtr(posX, posY);
		byte *in, *out;

		for (int i = 0; i < img->h; i++) {
			out = outo;
			in = ino;
			for (int j = 0; j < img->w; j++) {
				int b = in[0];
				int g = in[1];
				int r = in[2];
				int a = in[3];
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
						*out++ = (b * cb) >> 8;
					else
						*out++ = b;

					if (cg != 255)
						*out++ = (g * cg) >> 8;
					else
						*out++ = g;

					if (cr != 255)
						*out++ = (r * cr) >> 8;
					else
						*out++ = r;

					*out++ = a;
					break;

				default: // alpha blending
					if (cb != 255)
						*out += ((b - *out) * a * cb) >> 16;
					else
						*out += ((b - *out) * a) >> 8;
					out++;
					if (cg != 255)
						*out += ((g - *out) * a * cg) >> 16;
					else
						*out += ((g - *out) * a) >> 8;
					out++;
					if (cr != 255)
						*out += ((r - *out) * a * cr) >> 16;
					else
						*out += ((r - *out) * a) >> 8;
					out++;
					*out = 255;
					out++;
				}
			}
			outo += _backSurface->pitch;
			ino += inoStep;
		}

		g_system->copyRectToScreen((byte *)_backSurface->getBasePtr(posX, posY), _backSurface->pitch, posX, posY,
			img->w, img->h);
	}

	if (imgScaled) {
		imgScaled->pixels = savedPixels;
		imgScaled->free();
		delete imgScaled;
	}

	return true;
}

void RenderedImage::copyDirectly(int posX, int posY) {
	byte *data = _data;
	int w = _width;
	int h = _height;

	// Handle off-screen clipping
	if (posY < 0) {
		h = MAX(0, (int)_height - -posY);
		data = (byte *)_data + _width * -posY;
		posY = 0;
	}

	if (posX < 0) {
		w = MAX(0, (int)_width - -posX);
		data = (byte *)_data + (-posX * 4);
		posX = 0;
	}

	w = CLIP((int)w, 0, (int)MAX((int)_backSurface->w - posX, 0));
	h = CLIP((int)h, 0, (int)MAX((int)_backSurface->h - posY, 0));

	g_system->copyRectToScreen(data, _backSurface->pitch, posX, posY, w, h);
}

/**
 * Scales a passed surface, creating a new surface with the result
 * @param srcImage		Source image to scale
 * @param scaleFactor	Scale amount. Must be between 0 and 1.0 (but not zero)
 * @remarks Caller is responsible for freeing the returned surface
 */
Graphics::Surface *RenderedImage::scale(const Graphics::Surface &srcImage, int xSize, int ySize) {
	Graphics::Surface *s = new Graphics::Surface();
	s->create(xSize, ySize, srcImage.bytesPerPixel);

	int *horizUsage = scaleLine(xSize, srcImage.w);
	int *vertUsage = scaleLine(ySize, srcImage.h);

	// Loop to create scaled version
	for (int yp = 0; yp < ySize; ++yp) {
		const byte *srcP = (const byte *)srcImage.getBasePtr(0, vertUsage[yp]);
		byte *destP = (byte *)s->getBasePtr(0, yp);

		for (int xp = 0; xp < xSize; ++xp) {
			const byte *tempSrcP = srcP + (horizUsage[xp] * srcImage.bytesPerPixel);
			for (int byteCtr = 0; byteCtr < srcImage.bytesPerPixel; ++byteCtr) {
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
 * Returns an array indicating which pixels of a source image horizontally or vertically get
 * included in a scaled image
 */
int *RenderedImage::scaleLine(int size, int srcSize) {
	int scale = 100 * size / srcSize;
	assert(scale > 0);
	int *v = new int[size];
	Common::set_to(v, &v[size], 0);

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

} // End of namespace Sword25
