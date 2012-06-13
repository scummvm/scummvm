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
#include "engines/wintermute/Base/BSurfaceSDL.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/BFileManager.h"
#include "engines/wintermute/graphics/transparentSurface.h"
#include "graphics/decoders/png.h"
#include "graphics/decoders/jpeg.h"
#include "graphics/decoders/bmp.h"
#include "graphics/surface.h"
#include "engines/wintermute/graphics/tga.h"
#include "common/textconsole.h"
#include "common/stream.h"
#include "common/system.h"
//#include "FreeImage.h"

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
}


//////////////////////////////////////////////////////////////////////
CBImage::~CBImage() {
	delete _bitmap;
	delete _decoder;
#if 0
	if (_bitmap) FreeImage_Unload(_bitmap);
#endif
}

HRESULT CBImage::loadFile(const Common::String &filename) {
	_filename = filename;
	
	if (filename.hasSuffix(".png")) {
		_decoder = new Graphics::PNGDecoder();
	} else if (filename.hasSuffix(".bmp")) {
		_decoder = new Graphics::BitmapDecoder();
	} else if (filename.hasSuffix(".tga")) {
		_decoder = new WinterMute::TGA();
	} else if (filename.hasSuffix(".jpg")) {
		_decoder = new Graphics::JPEGDecoder();
	} else {
		error("CBImage::loadFile : Unsupported fileformat %s", filename.c_str());
	}
	
	Common::SeekableReadStream *file = Game->_fileManager->OpenFile(filename.c_str());
	if (!file) return E_FAIL;
	
	_decoder->loadStream(*file);
	_surface = _decoder->getSurface();
	_palette = _decoder->getPalette();
	Game->_fileManager->CloseFile(file);
	
	return S_OK;
}

byte CBImage::getAlphaAt(int x, int y) {
	if (!_surface) return 0xFF;
	uint32 color = *(uint32*)_surface->getBasePtr(x, y);
	byte r, g, b, a;
	_surface->format.colorToARGB(color, a, r, g, b);
	return a;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBImage::SaveBMPFile(const char *Filename) {
#if 0
	if (!_bitmap) return E_FAIL;

	if (FreeImage_Save(FIF_BMP, _bitmap, Filename)) return S_OK;
	else return E_FAIL;
#endif
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBImage::Resize(int NewWidth, int NewHeight) {
#if 0
	if (!_bitmap) return E_FAIL;

	if (NewWidth == 0) NewWidth = FreeImage_GetWidth(_bitmap);
	if (NewHeight == 0) NewHeight = FreeImage_GetHeight(_bitmap);


	FIBITMAP *newImg = FreeImage_Rescale(_bitmap, NewWidth, NewHeight, FILTER_BILINEAR);
	if (newImg) {
		FreeImage_Unload(_bitmap);
		_bitmap = newImg;
		return S_OK;
	} else return E_FAIL;
#endif
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
byte *CBImage::CreateBMPBuffer(uint32 *BufferSize) {
	if (!_bitmap) return NULL;
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
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBImage::CopyFrom(CBImage *OrigImage, int NewWidth, int NewHeight) {
#if 0
	if (_bitmap) FreeImage_Unload(_bitmap);

	if (NewWidth == 0) NewWidth = FreeImage_GetWidth(OrigImage->GetBitmap());
	if (NewHeight == 0) NewHeight = FreeImage_GetHeight(OrigImage->GetBitmap());

	_bitmap = FreeImage_Rescale(OrigImage->GetBitmap(), NewWidth, NewHeight, FILTER_BILINEAR);
#endif
	return S_OK;
}

} // end of namespace WinterMute
