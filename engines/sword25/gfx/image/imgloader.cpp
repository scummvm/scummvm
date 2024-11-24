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

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#include "common/memstream.h"
#include "sword25/gfx/image/image.h"
#include "sword25/gfx/image/imgloader.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"
#include "image/png.h"

namespace Sword25 {

bool ImgLoader::decodePNGImage(const byte *fileDataPtr, uint fileSize, Graphics::Surface *dest) {
	assert(dest);
	Common::MemoryReadStream *fileStr = new Common::MemoryReadStream(fileDataPtr, fileSize, DisposeAfterUse::NO);

	::Image::PNGDecoder png;
	if (!png.loadStream(*fileStr)) // the fileStr pointer, and thus pFileData will be deleted after this is done
		error("Error while reading PNG image");

	const Graphics::Surface *sourceSurface = png.getSurface();
	Graphics::Surface *pngSurface = sourceSurface->convertTo(Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0), png.getPalette(), png.getPaletteColorCount());

	dest->copyFrom(*pngSurface);

	pngSurface->free();
	delete pngSurface;
	delete fileStr;

	// Signal success
	return true;
}

bool ImgLoader::decodeThumbnailImage(const byte *pFileData, uint fileSize, Graphics::Surface *dest) {
	assert(dest);
	const byte *src = pFileData + 4;	// skip header
	uint width = READ_LE_UINT16(src); src += 2;
	uint height = READ_LE_UINT16(src); src += 2;
	src++;	// version, ignored for now

	dest->create(width, height, Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0));
	uint32 *dst = (uint32 *)dest->getBasePtr(0, 0); // treat as uint32, for pixelformat output
	byte r, g, b;

	for (uint32 i = 0; i < width * height; i++) {
		r = *src++;
		g = *src++;
		b = *src++;
		*dst++ = dest->format.RGBToColor(r, g, b);
	}

	return true;
}

} // End of namespace Sword25
