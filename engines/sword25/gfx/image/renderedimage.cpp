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

#include "common/savefile.h"
#include "sword25/package/packagemanager.h"
#include "sword25/gfx/image/imgloader.h"
#include "sword25/gfx/image/renderedimage.h"

#include "sword25/gfx/renderobjectmanager.h"

#include "common/system.h"
#include "graphics/thumbnail.h"

namespace Sword25 {

// -----------------------------------------------------------------------------
// CONSTRUCTION / DESTRUCTION
// -----------------------------------------------------------------------------

/**
 * Load a NULL-terminated string from the given stream.
 */
static Common::String loadString(Common::SeekableReadStream &in, uint maxSize = 999) {
	Common::String result;

	while (!in.eos() && (result.size() < maxSize)) {
		char ch = (char)in.readByte();
		if (ch == '\0')
			break;

		result += ch;
	}

	return result;
}

static byte *readSavegameThumbnail(const Common::String &filename, uint &fileSize, bool &isPNG) {
	byte *pFileData;
	Common::SaveFileManager *sfm = g_system->getSavefileManager();
	Common::InSaveFile *file = sfm->openForLoading(lastPathComponent(filename, '/'));
	if (!file)
		error("Save file \"%s\" could not be loaded.", filename.c_str());

	// Seek to the actual PNG image
	loadString(*file);		// Marker (BS25SAVEGAME)
	Common::String storedVersionID = loadString(*file);		// Version
	if (storedVersionID != "SCUMMVM1")
		loadString(*file);

	loadString(*file);		// Description
	uint32 compressedGamedataSize = atoi(loadString(*file).c_str());
	loadString(*file);		// Uncompressed game data size
	file->skip(compressedGamedataSize);	// Skip the game data and move to the thumbnail itself
	uint32 thumbnailStart = file->pos();

	fileSize = file->size() - thumbnailStart;

	// Check if the thumbnail is in our own format, or a PNG file.
	uint32 header = file->readUint32BE();
	isPNG = (header != MKTAG('S','C','R','N'));
	file->seek(-4, SEEK_CUR);

	pFileData = new byte[fileSize];
	file->read(pFileData, fileSize);
	delete file;

	return pFileData;
}

RenderedImage::RenderedImage(const Common::String &filename, bool &result) :
	_data(0),
	_width(0),
	_height(0),
	_isTransparent(true) {
	result = false;

	PackageManager *pPackage = Kernel::getInstance()->getPackage();
	assert(pPackage);

	_backSurface = Kernel::getInstance()->getGfx()->getSurface();

	// Load file
	byte *pFileData;
	uint fileSize;

	bool isPNG = true;

	if (filename.hasPrefix("/saves")) {
		pFileData = readSavegameThumbnail(filename, fileSize, isPNG);
	} else {
		pFileData = pPackage->getFile(filename, &fileSize);
	}

	if (!pFileData) {
		error("File \"%s\" could not be loaded.", filename.c_str());
		return;
	}

	// Uncompress the image
	int pitch;
	if (isPNG)
		result = ImgLoader::decodePNGImage(pFileData, fileSize, _data, _width, _height, pitch);
	else
		result = ImgLoader::decodeThumbnailImage(pFileData, fileSize, _data, _width, _height, pitch);

	if (!result) {
		error("Could not decode image.");
		delete[] pFileData;
		return;
	}

	// Cleanup FileData
	delete[] pFileData;

	_doCleanup = true;

#if defined(SCUMM_LITTLE_ENDIAN)
	// Makes sense for LE only at the moment
	checkForTransparency();
#endif

	return;
}

// -----------------------------------------------------------------------------

RenderedImage::RenderedImage(uint width, uint height, bool &result) :
	_width(width),
	_height(height),
	_isTransparent(true) {

	_data = new byte[width * height * 4];
	Common::fill(_data, &_data[width * height * 4], 0);

	_backSurface = Kernel::getInstance()->getGfx()->getSurface();

	_doCleanup = true;

	result = true;
	return;
}

RenderedImage::RenderedImage() : _width(0), _height(0), _data(0), _isTransparent(true) {
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

bool RenderedImage::blit(int posX, int posY, int flipping, Common::Rect *pPartRect, uint color, int width, int height, RectangleList *updateRects) {
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
	// TODO: Is the data really in the screen format?
	srcImage.format = g_system->getScreenFormat();
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

	for (RectangleList::iterator it = updateRects->begin(); it != updateRects->end(); ++it) {
		const Common::Rect &clipRect = *it;
	
		int skipLeft = 0, skipTop = 0;
		int drawX = posX, drawY = posY;
		int drawWidth = img->w;
		int drawHeight = img->h;

		// Handle clipping
		if (drawX < clipRect.left) {
			skipLeft = clipRect.left - drawX;
			drawWidth -= skipLeft;
			drawX = clipRect.left;
		}
		
		if (drawY < clipRect.top) {
			skipTop = clipRect.top - drawY;
			drawHeight -= skipTop;
			drawY = clipRect.top;
		}

		if (drawX + drawWidth >= clipRect.right)
			drawWidth = clipRect.right - drawX;
	
		if (drawY + drawHeight >= clipRect.bottom)
			drawHeight = clipRect.bottom - drawY;
			
		if ((drawWidth > 0) && (drawHeight > 0)) {
			int xp = 0, yp = 0;
	
			int inStep = 4;
			int inoStep = img->pitch;
			if (flipping & Image::FLIP_V) {
				inStep = -inStep;
				xp = img->w - 1 - skipLeft;
			} else {
				xp = skipLeft;
			}
	
			if (flipping & Image::FLIP_H) {
				inoStep = -inoStep;
				yp = img->h - 1 - skipTop;
			} else {
				yp = skipTop;
			}
	
			byte *ino = (byte *)img->getBasePtr(xp, yp);
			byte *outo = (byte *)_backSurface->getBasePtr(drawX, drawY);

#if defined(SCUMM_LITTLE_ENDIAN)
			// Simple memcpy if the source bitmap doesn't have transparent pixels and the drawing transparency is 255
			// NOTE Only possible with LE-machines at the moment, maybe it would be feasible to convert the bitmap pixels at loading time?
			if (!_isTransparent && ca == 255) {
				for (int i = 0; i < drawHeight; i++) {
					memcpy(outo, ino, drawWidth * 4);
					outo += _backSurface->pitch;
					ino += inoStep;
				}
			} else
#endif			 
			{
				byte *in, *out;
				for (int i = 0; i < drawHeight; i++) {
					out = outo;
					in = ino;
					for (int j = 0; j < drawWidth; j++) {
						uint32 pix = *(uint32 *)in;
						int a = (pix >> 24) & 0xff;
						in += inStep;
						
						if (ca != 255) {
							a = a * ca >> 8;
						}

						if (a == 0) {
							// Full transparency
							out += 4;
							continue;
						}
						
						int b = (pix >> 0) & 0xff;
						int g = (pix >> 8) & 0xff;
						int r = (pix >> 16) & 0xff;
		
						if (a == 255) {
#if defined(SCUMM_LITTLE_ENDIAN)
							if (cb != 255)
								b = (b * cb) >> 8;
							if (cg != 255)
								g = (g * cg) >> 8;
							if (cr != 255)
								r = (r * cr) >> 8;
							*(uint32 *)out = (255 << 24) | (r << 16) | (g << 8) | b;
							out += 4;
#else
							*out++ = a;
							if (cr != 255)
								*out++ = (r * cr) >> 8;
							else
								*out++ = r;
							if (cg != 255)
								*out++ = (g * cg) >> 8;
							else
								*out++ = g;
							if (cb != 255)
								*out++ = (b * cb) >> 8;
							else
								*out++ = b;
#endif
						} else {
#if defined(SCUMM_LITTLE_ENDIAN)
							pix = *(uint32 *)out;
							int outb = (pix >> 0) & 0xff;
							int outg = (pix >> 8) & 0xff;
							int outr = (pix >> 16) & 0xff;
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
							*(uint32 *)out = (255 << 24) | (outr << 16) | (outg << 8) | outb;
							out += 4;
#else
							*out = 255;
							out++;
							if (cr == 0)
								*out = 0;
							else if (cr != 255)
								*out += ((r - *out) * a * cr) >> 16;
							else
								*out += ((r - *out) * a) >> 8;
							out++;
							if (cg == 0)
								*out = 0;
							else if (cg != 255)
								*out += ((g - *out) * a * cg) >> 16;
							else
								*out += ((g - *out) * a) >> 8;
							out++;
							if (cb == 0)
								*out = 0;
							else if (cb != 255)
								*out += ((b - *out) * a * cb) >> 16;
							else
								*out += ((b - *out) * a) >> 8;
							out++;
#endif
						}
					}
					outo += _backSurface->pitch;
					ino += inoStep;
				}
			}

		}
		
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

void RenderedImage::checkForTransparency() {
	// Check if the source bitmap has any transparent pixels at all
	_isTransparent = false;
	byte *data = _data;
	for (int i = 0; i < _height; i++) {
		for (int j = 0; j < _width; j++) {
			_isTransparent = data[3] != 0xff;
			if (_isTransparent)
				return;
			data += 4;
		}
	}
}

} // End of namespace Sword25
