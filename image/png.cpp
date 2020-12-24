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

// Since we need to work with libpng here, we need to allow all symbols
// to avoid compilation issues.
#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/scummsys.h"

#ifdef USE_PNG
#include <png.h>
#endif

#include "image/png.h"

#include "graphics/pixelformat.h"
#include "graphics/surface.h"

#include "common/array.h"
#include "common/stream.h"

namespace Image {

PNGDecoder::PNGDecoder() :
        _outputSurface(0),
        _palette(0),
        _paletteColorCount(0),
        _skipSignature(false),
		_keepTransparencyPaletted(false),
		_transparentColor(-1) {
}

PNGDecoder::~PNGDecoder() {
	destroy();
}

void PNGDecoder::destroy() {
	if (_outputSurface) {
		_outputSurface->free();
		delete _outputSurface;
		_outputSurface = 0;
	}
	delete[] _palette;
	_palette = NULL;
}

Graphics::PixelFormat PNGDecoder::getByteOrderRgbaPixelFormat() const {
#ifdef SCUMM_BIG_ENDIAN
	return Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
#else
	return Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24);
#endif
}

#ifdef USE_PNG
// libpng-error-handling:
void pngError(png_structp pngptr, png_const_charp errorMsg) {
	error("%s", errorMsg);
}

void pngWarning(png_structp pngptr, png_const_charp warningMsg) {
	warning("%s", warningMsg);
}

// libpng-I/O-helpers:
void pngReadFromStream(png_structp pngPtr, png_bytep data, png_size_t length) {
	void *readIOptr = png_get_io_ptr(pngPtr);
	Common::SeekableReadStream *stream = (Common::SeekableReadStream *)readIOptr;
	stream->read(data, length);
}

void pngWriteToStream(png_structp pngPtr, png_bytep data, png_size_t length) {
	void *writeIOptr = png_get_io_ptr(pngPtr);
	Common::WriteStream *stream = (Common::WriteStream *)writeIOptr;
	stream->write(data, length);
}

void pngFlushStream(png_structp pngPtr) {
	void *writeIOptr = png_get_io_ptr(pngPtr);
	Common::WriteStream *stream = (Common::WriteStream *)writeIOptr;
	stream->flush();
}
#endif

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

bool PNGDecoder::loadStream(Common::SeekableReadStream &stream) {
#ifdef USE_PNG
	destroy();

	// First, check the PNG signature (if not set to skip it)
	if (!_skipSignature) {
		if (stream.readUint32BE() != MKTAG(0x89, 'P', 'N', 'G')) {
			return false;
		}
		if (stream.readUint32BE() != MKTAG(0x0d, 0x0a, 0x1a, 0x0a)) {
			return false;
		}
	}

	// The following is based on the guide provided in:
	//http://www.libpng.org/pub/png/libpng-1.2.5-manual.html#section-3
	//http://www.libpng.org/pub/png/libpng-1.4.0-manual.pdf
	// along with the png-loading code used in the sword25-engine.
	png_structp pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!pngPtr) {
		return false;
	}
	png_infop infoPtr = png_create_info_struct(pngPtr);
	if (!infoPtr) {
		png_destroy_read_struct(&pngPtr, NULL, NULL);
		return false;
	}

	png_set_error_fn(pngPtr, NULL, pngError, pngWarning);
	// TODO: The manual says errors should be handled via setjmp

	png_set_read_fn(pngPtr, &stream, pngReadFromStream);
	png_set_crc_action(pngPtr, PNG_CRC_DEFAULT, PNG_CRC_WARN_USE);
	// We already verified the PNG-header
	png_set_sig_bytes(pngPtr, 8);

	// Read PNG header
	png_read_info(pngPtr, infoPtr);

	// No handling for unknown chunks yet.
	int bitDepth, colorType, width, height, interlaceType;
	png_uint_32 w, h;
	uint32 rgbaPalette[256];
	bool hasRgbaPalette = false;

	png_get_IHDR(pngPtr, infoPtr, &w, &h, &bitDepth, &colorType, &interlaceType, NULL, NULL);
	width = w;
	height = h;

	// Allocate memory for the final image data.
	// To keep memory framentation low this happens before allocating memory for temporary image data.
	_outputSurface = new Graphics::Surface();

	// Images of all color formats except PNG_COLOR_TYPE_PALETTE
	// will be transformed into ARGB images
	if (colorType == PNG_COLOR_TYPE_PALETTE && (_keepTransparencyPaletted || !png_get_valid(pngPtr, infoPtr, PNG_INFO_tRNS))) {
		int numPalette = 0;
		png_colorp palette = NULL;
		png_bytep trans = nullptr;
		int numTrans = 0;

		uint32 success = png_get_PLTE(pngPtr, infoPtr, &palette, &numPalette);
		if (success != PNG_INFO_PLTE) {
			png_destroy_read_struct(&pngPtr, &infoPtr, NULL);
			return false;
		}
		_paletteColorCount = numPalette;
		_palette = new byte[_paletteColorCount * 3];
		for (int i = 0; i < _paletteColorCount; i++) {
			_palette[(i * 3)] = palette[i].red;
			_palette[(i * 3) + 1] = palette[i].green;
			_palette[(i * 3) + 2] = palette[i].blue;
		}

		if (png_get_valid(pngPtr, infoPtr, PNG_INFO_tRNS)) {
			png_color_16p transColor;
			png_get_tRNS(pngPtr, infoPtr, &trans, &numTrans, &transColor);

			if (numTrans == 1) {
				// For a single transparency color, the alpha should be fully transparent
				assert(*trans == 0);
				_transparentColor = 0;
			} else {
				// Multiple alphas are being specified for the palette, so we can't use
				// _transparentColor, and will instead need to build an RGBA surface
				assert(numTrans > 1);
				hasRgbaPalette = true;
			}
		}

		_outputSurface->create(width, height,
			hasRgbaPalette ? getByteOrderRgbaPixelFormat() : Graphics::PixelFormat::createFormatCLUT8());
		png_set_packing(pngPtr);

		if (hasRgbaPalette) {
			// Build up the RGBA palette using the transparency alphas
			Common::fill(&rgbaPalette[0], &rgbaPalette[256], 0);
			for (int i = 0; i < _paletteColorCount; ++i) {
				byte a = (i < numTrans) ? trans[i] : 0xff;
				rgbaPalette[i] = _outputSurface->format.ARGBToColor(
					a, palette[i].red, palette[i].green, palette[i].blue);
			}

			// We won't be needing a separate palette
			_paletteColorCount = 0;
			delete[] _palette;
			_palette = nullptr;
		}
	} else {
		if (png_get_valid(pngPtr, infoPtr, PNG_INFO_tRNS)) {
			png_set_expand(pngPtr);
		}

		_outputSurface->create(width, height, getByteOrderRgbaPixelFormat());
		if (!_outputSurface->getPixels()) {
			error("Could not allocate memory for output image.");
		}
		if (bitDepth == 16)
			png_set_strip_16(pngPtr);
		if (bitDepth < 8)
			png_set_expand(pngPtr);
		if (colorType == PNG_COLOR_TYPE_GRAY ||
			colorType == PNG_COLOR_TYPE_GRAY_ALPHA)
			png_set_gray_to_rgb(pngPtr);

		if (colorType != PNG_COLOR_TYPE_RGB_ALPHA)
			png_set_filler(pngPtr, 0xff, PNG_FILLER_AFTER);
	}

	// After the transformations have been registered, the image data is read again.
	png_set_interlace_handling(pngPtr);
	png_read_update_info(pngPtr, infoPtr);
	png_get_IHDR(pngPtr, infoPtr, &w, &h, &bitDepth, &colorType, NULL, NULL, NULL);
	width = w;
	height = h;

	if (hasRgbaPalette) {
		// Build up the RGBA surface from paletted rows
		png_bytep rowPtr = new byte[width];
		if (!rowPtr)
			error("Could not allocate memory for row.");

		for (int yp = 0; yp < height; ++yp) {
			png_read_row(pngPtr, rowPtr, nullptr);
			uint32 *destRowP = (uint32 *)_outputSurface->getBasePtr(0, yp);

			for (int xp = 0; xp < width; ++xp)
				destRowP[xp] = rgbaPalette[rowPtr[xp]];
		}

		delete[] rowPtr;
	} else  if (interlaceType == PNG_INTERLACE_NONE) {
		// PNGs without interlacing can simply be read row by row.
		for (int i = 0; i < height; i++) {
			png_read_row(pngPtr, (png_bytep)_outputSurface->getBasePtr(0, i), NULL);
		}
	} else {
		// PNGs with interlacing require us to allocate an auxillary
		// buffer with pointers to all row starts.

		// Allocate row pointer buffer
		png_bytep *rowPtr = new png_bytep[height];
		if (!rowPtr) {
			error("Could not allocate memory for row pointers.");
		}

		// Initialize row pointers
		for (int i = 0; i < height; i++)
			rowPtr[i] = (png_bytep)_outputSurface->getBasePtr(0, i);

		// Read image data
		png_read_image(pngPtr, rowPtr);

		// Free row pointer buffer
		delete[] rowPtr;
	}

	// Read additional data at the end.
	png_read_end(pngPtr, NULL);

	// Destroy libpng structures
	png_destroy_read_struct(&pngPtr, &infoPtr, NULL);

	return true;
#else
	return false;
#endif
}

bool writePNG(Common::WriteStream &out, const Graphics::Surface &input) {
#ifdef USE_PNG
#ifdef SCUMM_LITTLE_ENDIAN
	const Graphics::PixelFormat requiredFormat_3byte(3, 8, 8, 8, 0, 0, 8, 16, 0);
	const Graphics::PixelFormat requiredFormat_4byte(4, 8, 8, 8, 8, 0, 8, 16, 24);
#else
	const Graphics::PixelFormat requiredFormat_3byte(3, 8, 8, 8, 0, 16, 8, 0, 0);
	const Graphics::PixelFormat requiredFormat_4byte(4, 8, 8, 8, 8, 24, 16, 8, 0);
#endif

	int colorType;
	Graphics::Surface *tmp = NULL;
	const Graphics::Surface *surface;

	if (input.format == requiredFormat_3byte) {
		surface = &input;
		colorType = PNG_COLOR_TYPE_RGB;
	} else {
		if (input.format == requiredFormat_4byte) {
			surface = &input;
		} else {
			surface = tmp = input.convertTo(requiredFormat_4byte);
		}
		colorType = PNG_COLOR_TYPE_RGB_ALPHA;
	}

	png_structp pngPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!pngPtr) {
		if (tmp) {
			tmp->free();
			delete tmp;
		}
		return false;
	}
	png_infop infoPtr = png_create_info_struct(pngPtr);
	if (!infoPtr) {
		png_destroy_write_struct(&pngPtr, NULL);
		if (tmp) {
			tmp->free();
			delete tmp;
		}
		return false;
	}

	png_set_error_fn(pngPtr, NULL, pngError, pngWarning);
	// TODO: The manual says errors should be handled via setjmp

	png_set_write_fn(pngPtr, &out, pngWriteToStream, pngFlushStream);

	png_set_IHDR(pngPtr, infoPtr, surface->w, surface->h, 8, colorType, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	Common::Array<const uint8 *> rows;
	rows.reserve(surface->h);
	for (uint y = 0; y < surface->h; ++y) {
		rows.push_back((const uint8 *)surface->getBasePtr(0, y));
	}

	png_set_rows(pngPtr, infoPtr, const_cast<uint8 **>(&rows.front()));
	png_write_png(pngPtr, infoPtr, 0, NULL);
	png_destroy_write_struct(&pngPtr, &infoPtr);

	// free tmp surface
	if (tmp) {
		tmp->free();
		delete tmp;
	}

	return true;
#else
	return false;
#endif
}

} // End of namespace Image
