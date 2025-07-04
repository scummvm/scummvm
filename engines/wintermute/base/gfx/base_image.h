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

#ifndef WINTERMUTE_BASE_IMAGE_H
#define WINTERMUTE_BASE_IMAGE_H

#include "graphics/surface.h"
#include "graphics/pixelformat.h"

#include "common/endian.h"
#include "common/str.h"
#include "common/stream.h"

namespace Image {
class ImageDecoder;
}

namespace Wintermute {
class BaseSurface;
class BaseFileManager;
class BaseImage {

public:
	BaseImage();
	~BaseImage();

	bool loadFile(const Common::String &filename);
	const Graphics::Surface *getSurface() const {
		return _surface;
	};
	const byte *getPalette() const {
		return _palette;
	}
	uint16 getPaletteCount() const {
		return _paletteCount;
	}
	bool writeBMPToStream(Common::WriteStream *stream) const;
	bool saveBMPFile(const Common::String &filename) const;
	void copyFrom(const Graphics::Surface *surface, int newWidth = 0, int newHeight = 0, byte flip = 0);
private:
	Common::String _filename;
	Image::ImageDecoder *_decoder;
	const Graphics::Surface *_surface;
	Graphics::Surface *_deletableSurface;
	const byte *_palette;
	uint16 _paletteCount;
	BaseFileManager *_fileManager;
};

} // End of namespace Wintermute

#endif
