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
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/dcgf.h"
#include "engines/wintermute/Base/BImage.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/BFileManager.h"
#include "engines/wintermute/graphics/transparentSurface.h"
#include "engines/wintermute/utils/StringUtil.h"
#include "graphics/decoders/png.h"
#include "graphics/decoders/jpeg.h"
#include "graphics/decoders/bmp.h"
#include "graphics/surface.h"
#include "engines/wintermute/graphics/tga.h"
#include "common/textconsole.h"
#include "common/stream.h"
#include "common/system.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////
CBImage::CBImage(CBGame *inGame, FIBITMAP *bitmap): CBBase(inGame) {
#if 0
	_bitmap = bitmap;
#endif
	_bitmap = NULL;
	_palette = NULL;
	_surface = NULL;
	_decoder = NULL;
	_deletableSurface = NULL;
}


//////////////////////////////////////////////////////////////////////
CBImage::~CBImage() {
	/*  delete _bitmap; */
	delete _decoder;
	if (_deletableSurface) {
		_deletableSurface->free();
	}
	delete _deletableSurface;
#if 0
	if (_bitmap) FreeImage_Unload(_bitmap);
#endif
}

ERRORCODE CBImage::loadFile(const Common::String &filename) {
	_filename = filename;
	_filename.toLowercase();
	if (StringUtil::startsWith(filename, "savegame:", true)) {
		_decoder = new Graphics::BitmapDecoder();
	} else if (_filename.hasSuffix(".png")) {
		_decoder = new Graphics::PNGDecoder();
	} else if (_filename.hasSuffix(".bmp")) {
		_decoder = new Graphics::BitmapDecoder();
	} else if (_filename.hasSuffix(".tga")) {
		_decoder = new WinterMute::TGA();
	} else if (_filename.hasSuffix(".jpg")) {
		_decoder = new Graphics::JPEGDecoder();
	} else {
		error("CBImage::loadFile : Unsupported fileformat %s", filename.c_str());
	}
	_filename = filename;
	Common::SeekableReadStream *file = Game->_fileManager->openFile(filename.c_str());
	if (!file) return STATUS_FAILED;

	_decoder->loadStream(*file);
	_surface = _decoder->getSurface();
	_palette = _decoder->getPalette();
	Game->_fileManager->closeFile(file);

	return STATUS_OK;
}

byte CBImage::getAlphaAt(int x, int y) {
	if (!_surface) return 0xFF;
	uint32 color = *(uint32 *)_surface->getBasePtr(x, y);
	byte r, g, b, a;
	_surface->format.colorToARGB(color, a, r, g, b);
	return a;
}

void CBImage::copyFrom(Graphics::Surface *surface) {
	_surface = _deletableSurface = new Graphics::Surface();
	_deletableSurface->copyFrom(*surface);
}

//////////////////////////////////////////////////////////////////////////
ERRORCODE CBImage::saveBMPFile(const char *filename) {
#if 0
	if (!_bitmap) return STATUS_FAILED;

	if (FreeImage_Save(FIF_BMP, _bitmap, filename)) return STATUS_OK;
	else return STATUS_FAILED;
#endif
	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CBImage::resize(int newWidth, int newHeight) {
#if 0
	if (!_bitmap) return STATUS_FAILED;

	if (newWidth == 0) NewWidth = FreeImage_GetWidth(_bitmap);
	if (newHeight == 0) NewHeight = FreeImage_GetHeight(_bitmap);


	FIBITMAP *newImg = FreeImage_Rescale(_bitmap, NewWidth, NewHeight, FILTER_BILINEAR);
	if (newImg) {
		FreeImage_Unload(_bitmap);
		_bitmap = newImg;
		return STATUS_OK;
	} else return STATUS_FAILED;
#endif
	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
bool CBImage::writeBMPToStream(Common::WriteStream *stream) {
	if (!_surface) return false;

	/* The following is just copied over and inverted to write-ops from the BMP-decoder */
	stream->writeByte('B');
	stream->writeByte('M');

	/* Since we don't care during reads, we don't care during writes: */
	/* uint32 fileSize = */
	stream->writeUint32LE(54 + _surface->h * _surface->pitch);
	/* uint16 res1 = */
	stream->writeUint16LE(0);
	/* uint16 res2 = */
	stream->writeUint16LE(0);
	const uint32 imageOffset = 54;
	stream->writeUint32LE(imageOffset);

	const uint32 infoSize = 40; /* Windows v3 BMP */
	stream->writeUint32LE(infoSize);

	uint32 width = _surface->w;
	int32 height = _surface->h;
	stream->writeUint32LE(width);
	stream->writeUint32LE((uint32)height);

	if (width == 0 || height == 0)
		return false;

	if (height < 0) {
		warning("Right-side up bitmaps not supported");
		return false;
	}

	/* uint16 planes = */ stream->writeUint16LE(1);
	const uint16 bitsPerPixel = 24;
	stream->writeUint16LE(bitsPerPixel);

	const uint32 compression = 0;
	stream->writeUint32LE(compression);

	/* uint32 imageSize = */
	stream->writeUint32LE(_surface->h * _surface->pitch);
	/* uint32 pixelsPerMeterX = */
	stream->writeUint32LE(0);
	/* uint32 pixelsPerMeterY = */
	stream->writeUint32LE(0);
	const uint32 paletteColorCount = 0;
	stream->writeUint32LE(paletteColorCount);
	/* uint32 colorsImportant = */
	stream->writeUint32LE(0);

	// Start us at the beginning of the image (54 bytes in)
	Graphics::PixelFormat format = Graphics::PixelFormat::createFormatCLUT8();

	// BGRA for 24bpp
	if (bitsPerPixel == 24)
		format = Graphics::PixelFormat(4, 8, 8, 8, 8, 8, 16, 24, 0);

	Graphics::Surface *surface = _surface->convertTo(format);

	int srcPitch = width * (bitsPerPixel >> 3);
	const int extraDataLength = (srcPitch % 4) ? 4 - (srcPitch % 4) : 0;

	for (int32 i = height - 1; i >= 0; i--) {
		for (uint32 j = 0; j < width; j++) {
			byte b, g, r;
			uint32 color = *(uint32 *)surface->getBasePtr(j, i);
			surface->format.colorToRGB(color, r, g, b);
			stream->writeByte(b);
			stream->writeByte(g);
			stream->writeByte(r);
		}

		for (int k = 0; k < extraDataLength; k++) {
			stream->writeByte(0);
		}
	}
	surface->free();
	delete surface;
	return true;

	//*BufferSize = 0;
#if 0
	FIMEMORY *fiMem = FreeImage_OpenMemory();
	FreeImage_SaveToMemory(FIF_PNG, _bitmap, fiMem);
	uint32 size;
	byte *data;
	FreeImage_AcquireMemory(fiMem, &data, &size);


	byte *Buffer = new byte[size];
	memcpy(Buffer, data, size);

	FreeImage_CloseMemory(fiMem);

	if (BufferSize) *BufferSize = size;

	return Buffer;
#endif
	return false;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CBImage::copyFrom(CBImage *origImage, int newWidth, int newHeight) {
#if 0
	if (_bitmap) FreeImage_Unload(_bitmap);

	if (NewWidth == 0) NewWidth = FreeImage_GetWidth(OrigImage->GetBitmap());
	if (NewHeight == 0) NewHeight = FreeImage_GetHeight(OrigImage->GetBitmap());

	_bitmap = FreeImage_Rescale(OrigImage->GetBitmap(), NewWidth, NewHeight, FILTER_BILINEAR);
#endif
	TransparentSurface temp(*origImage->_surface, false);
	if (_deletableSurface) {
		_deletableSurface->free();
		delete _deletableSurface;
		_deletableSurface = NULL;
	}
	_surface = _deletableSurface = temp.scale(newWidth, newHeight);
	return STATUS_OK;
}

} // end of namespace WinterMute
