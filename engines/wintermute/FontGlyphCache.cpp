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

} // end of namespace WinterMute
