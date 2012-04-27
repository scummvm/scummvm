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

#ifndef __WmeFontGlyphCache_H__
#define __WmeFontGlyphCache_H__


#include "SDL.h"
#include "BFontStorage.h"

namespace WinterMute {

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
		if (_image) SDL_FreeSurface(_image);
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
	SDL_Surface *GetImage() {
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

	SDL_Surface *_image;
};



//////////////////////////////////////////////////////////////////////////
class FontGlyphCache {
public:
	FontGlyphCache();
	virtual ~FontGlyphCache();

	void Initialize();
	bool HasGlyph(wchar_t ch);
	GlyphInfo *GetGlyph(wchar_t ch);
	void AddGlyph(wchar_t ch, int glyphIndex, FT_GlyphSlot glyphSlot, size_t width, size_t height, byte *pixels, size_t stride = 0);

private:
	//typedef Common::HashMap<wchar_t, GlyphInfo *> GlyphInfoMap;
	typedef Common::HashMap<char, GlyphInfo *> GlyphInfoMap; // TODO
	GlyphInfoMap _glyphs;
};

} // end of namespace WinterMute

#endif // __WmeFontGlyphCache_H__