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

// Define to use ScummVM's PNG decoder, instead of libpng
#define USE_INTERNAL_PNG_DECODER

#ifndef USE_INTERNAL_PNG_DECODER
// Disable symbol overrides so that we can use png.h
#define FORBIDDEN_SYMBOL_ALLOW_ALL
#endif

#include "common/memstream.h"
#include "sword25/gfx/image/image.h"
#include "sword25/gfx/image/pngloader.h"
#ifndef USE_INTERNAL_PNG_DECODER
#include <png.h>
#else
#include "graphics/pixelformat.h"
#include "graphics/png.h"
#endif

namespace Sword25 {

/**
 * Load a NULL-terminated string from the given stream.
 */
static Common::String loadString(Common::ReadStream &in, uint maxSize = 999) {
	Common::String result;

	while (!in.eos() && (result.size() < maxSize)) {
		char ch = (char)in.readByte();
		if (ch == '\0')
			break;

		result += ch;
	}

	return result;
}

/**
 * Check if the given data is a savegame, and if so, locate the
 * offset to the image data.
 * @return offset to image data if fileDataPtr contains a savegame; 0 otherwise
 */
static uint findEmbeddedPNG(const byte *fileDataPtr, uint fileSize) {
	if (fileSize < 100)
		return 0;
	if (memcmp(fileDataPtr, "BS25SAVEGAME", 12))
		return 0;

	// Read in the header
	Common::MemoryReadStream stream(fileDataPtr, fileSize);
	stream.seek(0, SEEK_SET);

	// Read header information of savegame
	uint compressedGamedataSize;
	loadString(stream);		// Marker
	loadString(stream);		// Version
	loadString(stream);		// Description
	Common::String gameSize = loadString(stream);
	compressedGamedataSize = atoi(gameSize.c_str());
	loadString(stream);

	// Return the offset of where the thumbnail starts
	return static_cast<uint>(stream.pos() + compressedGamedataSize);
}

#ifndef USE_INTERNAL_PNG_DECODER
static void png_user_read_data(png_structp png_ptr, png_bytep data, png_size_t length) {
	const byte **ref = (const byte **)png_get_io_ptr(png_ptr);
	memcpy(data, *ref, length);
	*ref += length;
}

static bool doIsCorrectImageFormat(const byte *fileDataPtr, uint fileSize) {
	return (fileSize > 8) && png_check_sig(const_cast<byte *>(fileDataPtr), 8);
}
#endif

bool PNGLoader::doDecodeImage(const byte *fileDataPtr, uint fileSize, byte *&uncompressedDataPtr, int &width, int &height, int &pitch) {
#ifndef USE_INTERNAL_PNG_DECODER
	png_structp png_ptr = NULL;
	png_infop   info_ptr = NULL;

	int         bitDepth;
	int         colorType;
	int         interlaceType;
	int         i;

	// Check for valid PNG signature
	if (!doIsCorrectImageFormat(fileDataPtr, fileSize)) {
		error("png_check_sig failed");
	}

	// Create both PNG structures
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		error("Could not create libpng read struct.");
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		error("Could not create libpng info struct.");
	}

	// Use alternative reading function
	const byte **ref = &fileDataPtr;
	png_set_read_fn(png_ptr, (void *)ref, png_user_read_data);

	// Read PNG header
	png_read_info(png_ptr, info_ptr);

	// Read out PNG informations

	png_uint_32 w, h;
	png_get_IHDR(png_ptr, info_ptr, &w, &h, &bitDepth, &colorType, &interlaceType, NULL, NULL);
	width = w;
	height = h;

	// Calculate pitch of output image
	pitch = GraphicEngine::calcPitch(GraphicEngine::CF_ARGB32, width);

	// Allocate memory for the final image data.
	// To keep memory framentation low this happens before allocating memory for temporary image data.
	uncompressedDataPtr = new byte[pitch * height];
	if (!uncompressedDataPtr) {
		error("Could not allocate memory for output image.");
	}

	// Images of all color formates will be transformed into ARGB images
	if (bitDepth == 16)
		png_set_strip_16(png_ptr);
	if (colorType == PNG_COLOR_TYPE_PALETTE)
		png_set_expand(png_ptr);
	if (bitDepth < 8)
		png_set_expand(png_ptr);
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		png_set_expand(png_ptr);
	if (colorType == PNG_COLOR_TYPE_GRAY ||
	        colorType == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png_ptr);

	png_set_bgr(png_ptr);

	if (colorType != PNG_COLOR_TYPE_RGB_ALPHA)
		png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);

	// After the transformations have been registered, the image data is read again.
	png_read_update_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &w, &h, &bitDepth, &colorType, NULL, NULL, NULL);
	width = w;
	height = h;

	if (interlaceType == PNG_INTERLACE_NONE) {
		// PNGs without interlacing can simply be read row by row.
		for (i = 0; i < height; i++) {
			png_read_row(png_ptr, uncompressedDataPtr + i * pitch, NULL);
		}
	} else {
		// PNGs with interlacing require us to allocate an auxillary
		// buffer with pointers to all row starts.

		// Allocate row pointer buffer
		png_bytep *pRowPtr = new png_bytep[height];
		if (!pRowPtr) {
			error("Could not allocate memory for row pointers.");
		}

		// Initialize row pointers
		for (i = 0; i < height; i++)
			pRowPtr[i] = uncompressedDataPtr + i * pitch;

		// Read image data
		png_read_image(png_ptr, pRowPtr);

		// Free row pointer buffer
		delete[] pRowPtr;
	}

	// Read additional data at the end.
	png_read_end(png_ptr, NULL);

	// Destroy libpng structures
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
#else
	Common::MemoryReadStream *fileStr = new Common::MemoryReadStream(fileDataPtr, fileSize, DisposeAfterUse::NO);
	Graphics::PNG *png = new Graphics::PNG();
	if (!png->read(fileStr))	// the fileStr pointer, and thus pFileData will be deleted after this is done
		error("Error while reading PNG image");

	Graphics::PixelFormat format = Graphics::PixelFormat(4, 8, 8, 8, 8, 16, 8, 0, 24);
	Graphics::Surface *pngSurface = png->getSurface(format);

	width = pngSurface->w;
	height = pngSurface->h;
	uncompressedDataPtr = new byte[pngSurface->pitch * pngSurface->h];
	memcpy(uncompressedDataPtr, (byte *)pngSurface->pixels, pngSurface->pitch * pngSurface->h);
	pngSurface->free();

	delete pngSurface;
	delete png;

#endif
	// Signal success
	return true;
}

bool PNGLoader::decodeImage(const byte *fileDataPtr, uint fileSize, byte *&uncompressedDataPtr, int &width, int &height, int &pitch) {
	uint pngOffset = findEmbeddedPNG(fileDataPtr, fileSize);
	return doDecodeImage(fileDataPtr + pngOffset, fileSize - pngOffset, uncompressedDataPtr, width, height, pitch);
}

bool PNGLoader::doImageProperties(const byte *fileDataPtr, uint fileSize, int &width, int &height) {
#ifndef USE_INTERNAL_PNG_DECODER
	// Check for valid PNG signature
	if (!doIsCorrectImageFormat(fileDataPtr, fileSize))
		return false;

	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;

	// Create both PNG structures
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		error("Could not create libpng read struct.");
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		error("Could not create libpng info struct.");
	}

	// Use alternative reading function
	const byte **ref = &fileDataPtr;
	png_set_read_fn(png_ptr, (void *)ref, png_user_read_data);

	// Read PNG Header
	png_read_info(png_ptr, info_ptr);

	// Read out PNG informations
	int bitDepth;
	int colorType;
	png_uint_32 w, h;
	png_get_IHDR(png_ptr, info_ptr, &w, &h, &bitDepth, &colorType, NULL, NULL, NULL);

	width = w;
	height = h;

	// Destroy libpng structures
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
#else
	// We don't need to read the image properties here...
#endif
	return true;

}

bool PNGLoader::imageProperties(const byte *fileDataPtr, uint fileSize, int &width, int &height) {
	uint pngOffset = findEmbeddedPNG(fileDataPtr, fileSize);
	return doImageProperties(fileDataPtr + pngOffset, fileSize - pngOffset, width, height);
}


} // End of namespace Sword25
