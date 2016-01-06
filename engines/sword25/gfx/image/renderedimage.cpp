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
	if (isPNG)
		result = ImgLoader::decodePNGImage(pFileData, fileSize, &_surface);
	else
		result = ImgLoader::decodeThumbnailImage(pFileData, fileSize, &_surface);

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
	_isTransparent(true) {

	_surface.create(width, height, Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0));

	_backSurface = Kernel::getInstance()->getGfx()->getSurface();

	_doCleanup = true;

	result = true;
	return;
}

RenderedImage::RenderedImage() : _isTransparent(true) {
	_backSurface = Kernel::getInstance()->getGfx()->getSurface();

	_surface.format = Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);

	_doCleanup = false;

	return;
}

// -----------------------------------------------------------------------------

RenderedImage::~RenderedImage() {
	if (_doCleanup) {
		_surface.free();
	}
}

// -----------------------------------------------------------------------------

bool RenderedImage::fill(const Common::Rect *pFillRect, uint color) {
	error("Fill() is not supported.");
	return false;
}

// -----------------------------------------------------------------------------

bool RenderedImage::setContent(const byte *pixeldata, uint size, uint offset, uint stride) {
	// Check if PixelData contains enough pixel to create an image with image size equals width * height
	if (size < static_cast<uint>(_surface.w * _surface.h * 4)) {
		error("PixelData vector is too small to define a 32 bit %dx%d image.", _surface.w, _surface.h);
		return false;
	}

	const byte *in = &pixeldata[offset];
	byte *out = (byte *)_surface.getPixels();

	for (int i = 0; i < _surface.h; i++) {
		memcpy(out, in, _surface.w * 4);
		out += _surface.w * 4;
		in += stride;
	}

	return true;
}

void RenderedImage::replaceContent(byte *pixeldata, int width, int height) {
	_surface.w = width;
	_surface.h = height;
	_surface.pitch = width * 4;
	_surface.setPixels(pixeldata);
}
// -----------------------------------------------------------------------------

uint RenderedImage::getPixel(int x, int y) {
	error("GetPixel() is not supported. Returning black.");
	return 0;
}

// -----------------------------------------------------------------------------

bool RenderedImage::blit(int posX, int posY, int flipping, Common::Rect *pPartRect, uint color, int width, int height, RectangleList *updateRects) {
	_surface.blit(*_backSurface, posX, posY, (((flipping & 1) ? Graphics::FLIP_V : 0) | ((flipping & 2) ? Graphics::FLIP_H : 0)), pPartRect, color, width, height);

	return true;
}

void RenderedImage::copyDirectly(int posX, int posY) {
	byte *data = (byte *)_surface.getPixels();
	int w = _surface.w;
	int h = _surface.h;

	// Handle off-screen clipping
	if (posY < 0) {
		h = MAX(0, (int)_surface.h - -posY);
		data = (byte *)_surface.getPixels() + _surface.w * -posY;
		posY = 0;
	}

	if (posX < 0) {
		w = MAX(0, (int)_surface.h - -posX);
		data = (byte *)_surface.getPixels() + (-posX * 4);
		posX = 0;
	}

	w = CLIP((int)w, 0, (int)MAX((int)_backSurface->w - posX, 0));
	h = CLIP((int)h, 0, (int)MAX((int)_backSurface->h - posY, 0));

	g_system->copyRectToScreen(data, _backSurface->pitch, posX, posY, w, h);
}

void RenderedImage::checkForTransparency() {
	// Check if the source bitmap has any transparent pixels at all
	_isTransparent = false;
	byte *data = (byte *)_surface.getPixels();
	for (int i = 0; i < _surface.h; i++) {
		for (int j = 0; j < _surface.w; j++) {
			_isTransparent = data[3] != 0xff;
			if (_isTransparent)
				return;
			data += 4;
		}
	}
}

} // End of namespace Sword25
