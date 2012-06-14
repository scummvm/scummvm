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

#include "FontGlyphCache.h"

namespace WinterMute {
#if 0
//////////////////////////////////////////////////////////////////////////
FontGlyphCache::FontGlyphCache() {
}

//////////////////////////////////////////////////////////////////////////
FontGlyphCache::~FontGlyphCache() {
	GlyphInfoMap::iterator it;

	for (it = _glyphs.begin(); it != _glyphs.end(); ++it) {
		delete it->_value;
		it->_value = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
bool FontGlyphCache::HasGlyph(wchar_t ch) {
	return (_glyphs.find(ch) != _glyphs.end());
}

//////////////////////////////////////////////////////////////////////////
void FontGlyphCache::Initialize() {
}

//////////////////////////////////////////////////////////////////////////
GlyphInfo *FontGlyphCache::GetGlyph(wchar_t ch) {
	GlyphInfoMap::const_iterator it;
	it = _glyphs.find(ch);
	if (it == _glyphs.end()) return NULL;

	return it->_value;
}
/*
//////////////////////////////////////////////////////////////////////////
void FontGlyphCache::AddGlyph(wchar_t ch, int glyphIndex, FT_GlyphSlot glyphSlot, size_t width, size_t height, byte *pixels, size_t stride) {
    if (stride == 0) stride = width;

    _glyphs[ch] = new GlyphInfo(glyphIndex);
    _glyphs[ch]->SetGlyphInfo(glyphSlot->advance.x / 64.f, glyphSlot->advance.y / 64.f, glyphSlot->bitmap_left, glyphSlot->bitmap_top);
    _glyphs[ch]->SetGlyphImage(width, height, stride, pixels);
}
*/

//////////////////////////////////////////////////////////////////////////
void GlyphInfo::SetGlyphImage(size_t width, size_t height, size_t stride, byte *pixels) {
	warning("GlyphInfo::SetGlyphImage - Not ported yet");
#if 0
	if (_image) SDL_FreeSurface(_image);

	_image = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
	SDL_LockSurface(_image);

	Uint8 *buf = (Uint8 *)_image->pixels;

	for (int y = 0; y < height; y++) {
		Uint32 *buf32 = (Uint32 *)buf;

		for (int x = 0; x < width; x++) {
			byte alpha = pixels[y * stride + x];
			Uint32 color = SDL_MapRGBA(_image->format, 255, 255, 255, alpha);
			buf32[x] = color;
		}

		buf += _image->pitch;
	}

	SDL_UnlockSurface(_image);
#endif
}
#endif
} // end of namespace WinterMute
