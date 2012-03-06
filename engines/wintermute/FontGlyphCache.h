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
		m_GlyphIndex = glyphIndex;
		m_AdvanceX = m_AdvanceY = 0;
		m_BearingX = m_BearingY = 0;

		m_Width = m_Height = 0;

		m_Image = NULL;
	}

	~GlyphInfo() {
		if (m_Image) SDL_FreeSurface(m_Image);
	}

	void SetGlyphInfo(float AdvanceX, float AdvanceY, int BearingX, int BearingY) {
		m_AdvanceX = AdvanceX;
		m_AdvanceY = AdvanceY;
		m_BearingX = BearingX;
		m_BearingY = BearingY;
	}

	void SetGlyphImage(size_t width, size_t height, size_t stride, byte *pixels);

	int GetGlyphIndex() {
		return m_GlyphIndex;
	}
	int GetWidth() {
		return m_Width;
	}
	int GetHeight() {
		return m_Height;
	}
	float GetAdvanceX() {
		return m_AdvanceX;
	}
	float GetAdvanceY() {
		return m_AdvanceY;
	}
	int GetBearingX() {
		return m_BearingX;
	}
	int GetBearingY() {
		return m_BearingY;
	}
	SDL_Surface *GetImage() {
		return m_Image;
	}

private:
	int m_GlyphIndex;

	float m_AdvanceX;
	float m_AdvanceY;
	int m_BearingX;
	int m_BearingY;

	int m_Width;
	int m_Height;

	SDL_Surface *m_Image;
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
	typedef std::map<wchar_t, GlyphInfo *> GlyphInfoMap;
	GlyphInfoMap m_Glyphs;
};

} // end of namespace WinterMute

#endif // __WmeFontGlyphCache_H__