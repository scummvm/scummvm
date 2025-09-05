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
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/base/gfx/base_image.h"
#include "engines/wintermute/base/gfx/base_renderer.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/file/base_savefile_manager_file.h"

#include "graphics/surface.h"

#include "image/png.h"
#include "image/jpeg.h"
#include "image/bmp.h"
#include "image/tga.h"

#include "common/textconsole.h"
#include "common/stream.h"
#include "common/system.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////
BaseImage::BaseImage() {
	_fileManager = BaseFileManager::getEngineInstance();
	_palette = nullptr;
	_paletteCount = 0;
	_surface = nullptr;
	_decoder = nullptr;
	_deletableSurface = nullptr;
}


//////////////////////////////////////////////////////////////////////
BaseImage::~BaseImage() {
	delete _decoder;
	if (_deletableSurface) {
		_deletableSurface->free();
	}
	delete _deletableSurface;
}

bool BaseImage::loadFile(const Common::String &filename) {
	_filename = filename;
	_filename.toLowercase();
	if (filename.hasPrefix("savegame:") || _filename.hasSuffix(".bmp")) {
		_decoder = new Image::BitmapDecoder();
	} else if (_filename.hasSuffix(".png")) {
		_decoder = new Image::PNGDecoder();
	} else if (_filename.hasSuffix(".tga")) {
		_decoder = new Image::TGADecoder();
	} else if (_filename.hasSuffix(".jpg")) {
		Image::JPEGDecoder *jpeg = new Image::JPEGDecoder();
		jpeg->setOutputPixelFormat(BaseEngine::getRenderer()->getPixelFormat());
		_decoder = jpeg;
	} else {
		error("BaseImage::loadFile : Unsupported fileformat %s", filename.c_str());
	}
	_filename = filename;
	Common::SeekableReadStream *file = _fileManager->openFile(filename);
	if (!file) {
		return false;
	}

	_decoder->loadStream(*file);
	_surface = _decoder->getSurface();
	_palette = _decoder->getPalette().data();
	_paletteCount = _decoder->getPalette().size();
	_fileManager->closeFile(file);

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool BaseImage::saveBMPFile(const char *filename) const {
	Common::WriteStream *stream = openSfmFileForWrite(filename);
	if (stream) {
		bool ret = writeBMPToStream(stream);
		delete stream;
		return ret;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
bool BaseImage::writeBMPToStream(Common::WriteStream *stream) const {
	if (!stream || !_surface) {
		return false;
	}

	return Image::writeBMP(*stream, *_surface, _palette);
}


//////////////////////////////////////////////////////////////////////////
void BaseImage::copyFrom(const Graphics::Surface *surface, int newWidth, int newHeight, byte flip) {
	if (newWidth == 0)
		newWidth = surface->w;
	if (newHeight == 0)
		newHeight = surface->h;

	Graphics::Surface *temp;
	if (newWidth == surface->w && newHeight == surface->h && flip == 0) {
		temp = new Graphics::Surface();
		temp->copyFrom(*surface);
	} else {
		temp = surface->scale((uint16)newWidth, (uint16)newHeight, true, flip);
	}

	if (_deletableSurface) {
		_deletableSurface->free();
		delete _deletableSurface;
		_deletableSurface = nullptr;
	}
	_surface = _deletableSurface = temp;
}

} // End of namespace Wintermute
