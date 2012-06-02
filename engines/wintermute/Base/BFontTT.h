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

#ifndef WINTERMUTE_BFONTTT_H
#define WINTERMUTE_BFONTTT_H

#include "engines/wintermute/Base/BFontStorage.h"
#include "engines/wintermute/Base/BFont.h"
#include "engines/wintermute/Base/BSurface.h"
#include "common/rect.h"
#include "graphics/surface.h"
#include "graphics/font.h"

#define NUM_CACHED_TEXTS 30

namespace WinterMute {

class FontGlyphCache;

class CBFontTT : public CBFont {
private:
	//////////////////////////////////////////////////////////////////////////
	class CBCachedTTFontText {
	public:
		WideString _text;
		int _width;
		TTextAlign _align;
		int _maxHeight;
		int _maxLength;
		CBSurface *_surface;
		int _priority;
		int _textOffset;
		bool _marked;

		CBCachedTTFontText() {
			//_text = L"";
			_text = "";
			_width = _maxHeight = _maxLength = -1;
			_align = TAL_LEFT;
			_surface = NULL;
			_priority = -1;
			_textOffset = 0;
			_marked = false;
		}

		virtual ~CBCachedTTFontText() {
			if (_surface) delete _surface;
		}
	};

public:
	//////////////////////////////////////////////////////////////////////////
	class CBTTFontLayer {
	public:
		CBTTFontLayer() {
			_offsetX = _offsetY = 0;
			_color = 0x00000000;
		}

		HRESULT Persist(CBPersistMgr *PersistMgr) {
			PersistMgr->Transfer(TMEMBER(_offsetX));
			PersistMgr->Transfer(TMEMBER(_offsetY));
			PersistMgr->Transfer(TMEMBER(_color));
			return S_OK;
		}

		int _offsetX;
		int _offsetY;
		uint32 _color;
	};

	//////////////////////////////////////////////////////////////////////////
	class TextLine {
	public:
		TextLine(const WideString &text, int width) {
			_text = text;
			_width = width;
		}

		const WideString &GetText() const {
			return _text;
		}
		int GetWidth() const {
			return _width;
		}
	private:
		WideString _text;
		int _width;
	};
	typedef Common::List<TextLine *> TextLineList;


public:
	DECLARE_PERSISTENT(CBFontTT, CBFont)
	CBFontTT(CBGame *inGame);
	virtual ~CBFontTT(void);

	virtual int GetTextWidth(byte  *text, int MaxLenght = -1);
	virtual int GetTextHeight(byte  *text, int width);
	virtual void DrawText(byte  *text, int x, int y, int width, TTextAlign align = TAL_LEFT, int max_height = -1, int MaxLenght = -1);
	virtual int GetLetterHeight();

	HRESULT LoadBuffer(byte  *Buffer);
	HRESULT LoadFile(const char *Filename);

	/*  static unsigned long FTReadSeekProc(FT_Stream stream, unsigned long offset, unsigned char *buffer, unsigned long count);
	    static void FTCloseProc(FT_Stream stream);*/

	FontGlyphCache *GetGlyphCache() {
		return _glyphCache;
	}

	float GetLineHeight() const {
		return _lineHeight;
	}

	void AfterLoad();
	void InitLoop();

private:
	HRESULT ParseLayer(CBTTFontLayer *Layer, byte *Buffer);

	void WrapText(const WideString &text, int maxWidth, int maxHeight, TextLineList &lines);
	void MeasureText(const WideString &text, int maxWidth, int maxHeight, int &textWidth, int &textHeight);
	float GetKerning(wchar_t leftChar, wchar_t rightChar);
	void PrepareGlyphs(const WideString &text);
	void CacheGlyph(wchar_t ch);

	CBSurface *RenderTextToTexture(const WideString &text, int width, TTextAlign align, int maxHeight, int &textOffset);
	void BlitSurface(Graphics::Surface *src, Graphics::Surface *target, Common::Rect *targetRect);


	CBCachedTTFontText *_cachedTexts[NUM_CACHED_TEXTS];

	HRESULT InitFont();
	//FT_Stream _fTStream;
	//FT_Face _fTFace;
	Graphics::Font *_font;
	const Graphics::Font *_fallbackFont;
	FontGlyphCache *_glyphCache;

	float _ascender;
	float _descender;
	float _lineHeight;
	float _underlinePos;
	float _pointSize;
	float _vertDpi;
	float _horDpi;

	size_t _maxCharWidth;
	size_t _maxCharHeight;

public:
	bool _isBold;
	bool _isItalic;
	bool _isUnderline;
	bool _isStriked;
	int _fontHeight;
	char *_fontFile;

	CBArray<CBTTFontLayer *, CBTTFontLayer *> _layers;
	void ClearCache();

};

} // end of namespace WinterMute

#endif
