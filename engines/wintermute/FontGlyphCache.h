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

#ifndef WINTERMUTE_FONTGLYPHCACHE_H
#define WINTERMUTE_FONTGLYPHCACHE_H


#include "engines/wintermute/Base/BFontStorage.h"
#include "graphics/surface.h"

namespace WinterMute {
#if 0
//////////////////////////////////////////////////////////////////////////
class GlyphInfo {
public:
	GlyphInfo(int glyphIndex) {
		_glyphIndex = glyphIndex;
		_advanceX = _advanceY = 0;
		_bearingX = _bearingY = 0;

		_width = _height = 0;

		_image = NULL;
	}

	~GlyphInfo() {
		// TODO
		//if (_image) SDL_FreeSurface(_image);
	}

	void SetGlyphInfo(float AdvanceX, float AdvanceY, int BearingX, int BearingY) {
		_advanceX = AdvanceX;
		_advanceY = AdvanceY;
		_bearingX = BearingX;
		_bearingY = BearingY;
	}

	void SetGlyphImage(size_t width, size_t height, size_t stride, byte *pixels);

	int GetGlyphIndex() {
		return _glyphIndex;
	}
	int GetWidth() {
		return _width;
	}
	int GetHeight() {
		return _height;
	}
	float GetAdvanceX() {
		return _advanceX;
	}
	float GetAdvanceY() {
		return _advanceY;
	}
	int GetBearingX() {
		return _bearingX;
	}
	int GetBearingY() {
		return _bearingY;
	}
	Graphics::Surface *GetImage() {
		return _image;
	}

private:
	int _glyphIndex;

	float _advanceX;
	float _advanceY;
	int _bearingX;
	int _bearingY;

	int _width;
	int _height;

	Graphics::Surface *_image;
};



//////////////////////////////////////////////////////////////////////////
class FontGlyphCache {
public:
	FontGlyphCache();
	virtual ~FontGlyphCache();

	void Initialize();
	bool HasGlyph(wchar_t ch);
	GlyphInfo *GetGlyph(wchar_t ch);
	//void AddGlyph(wchar_t ch, int glyphIndex, FT_GlyphSlot glyphSlot, size_t width, size_t height, byte *pixels, size_t stride = 0);

private:
	//typedef Common::HashMap<wchar_t, GlyphInfo *> GlyphInfoMap;
	typedef Common::HashMap<char, GlyphInfo *> GlyphInfoMap; // TODO
	GlyphInfoMap _glyphs;
};
#endif
} // end of namespace WinterMute

#endif // WINTERMUTE_FONTGLYPHCACHE_H
