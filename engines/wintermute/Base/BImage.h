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

#ifndef WINTERMUTE_BIMAGE_H
#define WINTERMUTE_BIMAGE_H

#include "engines/wintermute/Base/BBase.h"
#include "graphics/surface.h"
#include "graphics/pixelformat.h"
#include "graphics/decoders/image_decoder.h"
#include "common/endian.h"
#include "common/str.h"
#include "common/stream.h"

struct FIBITMAP;

namespace WinterMute {
class CBSurface;
class CBImage: CBBase {

public:
	CBImage(CBGame *inGame, FIBITMAP *bitmap = NULL);
	~CBImage();

	HRESULT loadFile(const Common::String &filename);
	const Graphics::Surface *getSurface() const {
		return _surface;
	};
	const byte *getPalette() const {
		return _palette;
	}
	byte getAlphaAt(int x, int y);
	bool writeBMPToStream(Common::WriteStream *stream);
	HRESULT Resize(int NewWidth, int NewHeight);
	HRESULT SaveBMPFile(const char *Filename);
	HRESULT CopyFrom(CBImage *OrigImage, int NewWidth = 0, int NewHeight = 0);
	void copyFrom(Graphics::Surface *surface);
private:
	Common::String _filename;
	Graphics::ImageDecoder *_decoder;
	FIBITMAP *_bitmap;
	const Graphics::Surface *_surface;
	Graphics::Surface *_deletableSurface;
	const byte *_palette;
};

} // end of namespace WinterMute

#endif
