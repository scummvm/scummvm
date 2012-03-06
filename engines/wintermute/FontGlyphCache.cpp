/*
This file is part of WME Lite.
http://dead-code.org/redir.php?target=wmelite

Copyright (c) 2011 Jan Nedoma

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "FontGlyphCache.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////////
FontGlyphCache::FontGlyphCache() {
}

//////////////////////////////////////////////////////////////////////////
FontGlyphCache::~FontGlyphCache() {
	GlyphInfoMap::iterator it;

	for (it = m_Glyphs.begin(); it != m_Glyphs.end(); ++it) {
		delete it->second;
		it->second = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
bool FontGlyphCache::HasGlyph(wchar_t ch) {
	return (m_Glyphs.find(ch) != m_Glyphs.end());
}

//////////////////////////////////////////////////////////////////////////
void FontGlyphCache::Initialize() {
}

//////////////////////////////////////////////////////////////////////////
GlyphInfo *FontGlyphCache::GetGlyph(wchar_t ch) {
	GlyphInfoMap::const_iterator it;
	it = m_Glyphs.find(ch);
	if (it == m_Glyphs.end()) return NULL;

	return it->second;
}

//////////////////////////////////////////////////////////////////////////
void FontGlyphCache::AddGlyph(wchar_t ch, int glyphIndex, FT_GlyphSlot glyphSlot, size_t width, size_t height, byte *pixels, size_t stride) {
	if (stride == 0) stride = width;

	m_Glyphs[ch] = new GlyphInfo(glyphIndex);
	m_Glyphs[ch]->SetGlyphInfo(glyphSlot->advance.x / 64.f, glyphSlot->advance.y / 64.f, glyphSlot->bitmap_left, glyphSlot->bitmap_top);
	m_Glyphs[ch]->SetGlyphImage(width, height, stride, pixels);
}


//////////////////////////////////////////////////////////////////////////
void GlyphInfo::SetGlyphImage(size_t width, size_t height, size_t stride, byte *pixels) {
	if (m_Image) SDL_FreeSurface(m_Image);

	m_Image = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
	SDL_LockSurface(m_Image);

	Uint8 *buf = (Uint8 *)m_Image->pixels;

	for (int y = 0; y < height; y++) {
		Uint32 *buf32 = (Uint32 *)buf;

		for (int x = 0; x < width; x++) {
			byte alpha = pixels[y * stride + x];
			Uint32 color = SDL_MapRGBA(m_Image->format, 255, 255, 255, alpha);
			buf32[x] = color;
		}

		buf += m_Image->pitch;
	}

	SDL_UnlockSurface(m_Image);
}

} // end of namespace WinterMute
