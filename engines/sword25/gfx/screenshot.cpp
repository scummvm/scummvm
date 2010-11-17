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

// Disable symbol overrides so that we can use png.h
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#define BS_LOG_PREFIX "SCREENSHOT"

#include "common/system.h"
#include "common/savefile.h"
#include "sword25/gfx/screenshot.h"
#include "sword25/kernel/filesystemutil.h"
#include <png.h>

namespace Sword25 {

#include "common/pack-start.h"
struct RGB_PIXEL {
	byte red;
	byte green;
	byte blue;
} PACKED_STRUCT;
#include "common/pack-end.h"

void userWriteFn(png_structp png_ptr, png_bytep data, png_size_t length) {
	static_cast<Common::WriteStream *>(png_get_io_ptr(png_ptr))->write(data, length);
}

void userFlushFn(png_structp png_ptr) {
}

bool Screenshot::saveToFile(Graphics::Surface *data, Common::WriteStream *stream) {
	// Reserve buffer space
	RGB_PIXEL *pixelBuffer = new RGB_PIXEL[data->w * data->h];

	// Convert the RGBA data to RGB
	const byte *pSrc = (const byte *)data->getBasePtr(0, 0);
	RGB_PIXEL *pDest = pixelBuffer;

	for (uint y = 0; y < data->h; y++) {
		for (uint x = 0; x < data->w; x++) {
			uint32 srcPixel = READ_LE_UINT32(pSrc);
			pSrc += sizeof(uint32);
			pDest->red = (srcPixel >> 16) & 0xff;
			pDest->green = (srcPixel >> 8) & 0xff;
			pDest->blue = srcPixel & 0xff;
			++pDest;
		}
	}

	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
		error("Could not create PNG write-struct.");

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
		error("Could not create PNG info-struct.");

	// The compression buffer must be large enough to the entire image.
	// This ensures that only an IDAT chunk is created.
	// When buffer size is used 110% of the raw data size to be sure.
	png_set_compression_buffer_size(png_ptr, (data->w * data->h * 3 * 110) / 100);

	// Initialise PNG-Info structure
	png_set_IHDR(png_ptr, info_ptr,
	             data->w,                        // Width
	             data->h,                        // Height
	             8,                              // Bits depth
	             PNG_COLOR_TYPE_RGB,             // Color type
	             PNG_INTERLACE_NONE,             // No interlacing
	             PNG_COMPRESSION_TYPE_DEFAULT,   // Compression type
	             PNG_FILTER_TYPE_DEFAULT);       // Filter Type

	// Rowpointer erstellen
	png_bytep *rowPointers = new png_bytep[data->h];
	for (uint i = 0; i < data->h; i++) {
		rowPointers[i] = (png_bytep)&pixelBuffer[data->w * i];
	}
	png_set_rows(png_ptr, info_ptr, &rowPointers[0]);

	// Write out the png data to the file
	png_set_write_fn(png_ptr, (void *)stream, userWriteFn, userFlushFn);
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	png_destroy_write_struct(&png_ptr, &info_ptr);

	delete[] pixelBuffer;
	delete[] rowPointers;

	return true;
}

// -----------------------------------------------------------------------------

Common::MemoryReadStream *Screenshot::createThumbnail(Graphics::Surface *data) {
	// This method takes a screen image with a dimension of 800x600, and creates a screenshot with a dimension of 200x125.
	// First 50 pixels are cut off the top and bottom (the interface boards in the game). The remaining image of 800x500 
	// will be on a 16th of its size, reduced by being handed out in 4x4 pixel blocks and the average of each block 
	// generates a pixel of the target image. Finally, the result as a PNG file is stored as a file.

	// The source image must be 800x600.
	if (data->w != 800 || data->h != 600 || data->bytesPerPixel != 4) {
		BS_LOG_ERRORLN("The sreenshot dimensions have to be 800x600 in order to be saved as a thumbnail.");
		return false;
	}

	// Buffer for the output thumbnail
	Graphics::Surface thumbnail;
	thumbnail.create(200, 125, 4);

	// Über das Zielbild iterieren und einen Pixel zur Zeit berechnen.
	uint x, y;
	x = y = 0;
	
	for (byte *pDest = (byte *)thumbnail.pixels; pDest < ((byte *)thumbnail.pixels + thumbnail.pitch * thumbnail.h); ) {
		// Get an average over a 4x4 pixel block in the source image
		int alpha, red, green, blue;
		alpha = red = green = blue = 0;
		for (int j = 0; j < 4; ++j) {
			const uint32 *srcP = (const uint32 *)data->getBasePtr(x * 4, y * 4 + j + 50);
			for (int i = 0; i < 4; ++i) {
				uint32 pixel = READ_LE_UINT32(srcP + i);
				alpha += (pixel >> 24);
				red += (pixel >> 16) & 0xff;
				green += (pixel >> 8) & 0xff;
				blue += pixel & 0xff;
			}
		}

		// Write target pixel
		*pDest++ = blue / 16;
		*pDest++ = green / 16;
		*pDest++ = red / 16;
		*pDest++ = alpha / 16;

		// Move to next block
		++x;
		if (x == thumbnail.w) {
			x = 0;
			++y;
		}
	}

	// Create a PNG representation of the thumbnail data
	Common::MemoryWriteStreamDynamic *stream = new Common::MemoryWriteStreamDynamic();
	saveToFile(&thumbnail, stream);

	// Output a MemoryReadStream that encompasses the written data
	Common::MemoryReadStream *result = new Common::MemoryReadStream(stream->getData(), stream->size(),
		DisposeAfterUse::YES);
	return result;
}

} // End of namespace Sword25
