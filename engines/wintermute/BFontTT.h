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

#include "BFontStorage.h"
#include "BFont.h"
#include "BSurface.h"

#define NUM_CACHED_TEXTS 30
class SDL_Surface;
class SDL_Rect;
namespace WinterMute {

class FontGlyphCache;

class CBFontTT : public CBFont {
private:
	//////////////////////////////////////////////////////////////////////////
	class CBCachedTTFontText {
	public:
		WideString m_Text;
		int m_Width;
		TTextAlign m_Align;
		int m_MaxHeight;
		int m_MaxLength;
		CBSurface *m_Surface;
		int m_Priority;
		int m_TextOffset;
		bool m_Marked;

		CBCachedTTFontText() {
			//m_Text = L"";
			m_Text = "";
			m_Width = m_MaxHeight = m_MaxLength = -1;
			m_Align = TAL_LEFT;
			m_Surface = NULL;
			m_Priority = -1;
			m_TextOffset = 0;
			m_Marked = false;
		}

		virtual ~CBCachedTTFontText() {
			if (m_Surface) delete m_Surface;
		}
	};

public:
	//////////////////////////////////////////////////////////////////////////
	class CBTTFontLayer {
	public:
		CBTTFontLayer() {
			m_OffsetX = m_OffsetY = 0;
			m_Color = 0x00000000;
		}

		HRESULT Persist(CBPersistMgr *PersistMgr) {
			PersistMgr->Transfer(TMEMBER(m_OffsetX));
			PersistMgr->Transfer(TMEMBER(m_OffsetY));
			PersistMgr->Transfer(TMEMBER(m_Color));
			return S_OK;
		}

		int m_OffsetX;
		int m_OffsetY;
		uint32 m_Color;
	};

	//////////////////////////////////////////////////////////////////////////
	class TextLine {
	public:
		TextLine(const WideString &text, int width) {
			m_Text = text;
			m_Width = width;
		}

		const WideString &GetText() const {
			return m_Text;
		}
		int GetWidth() const {
			return m_Width;
		}
	private:
		WideString m_Text;
		int m_Width;
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
	HRESULT LoadFile(char *Filename);

	static unsigned long FTReadSeekProc(FT_Stream stream, unsigned long offset, unsigned char *buffer, unsigned long count);
	static void FTCloseProc(FT_Stream stream);

	FontGlyphCache *GetGlyphCache() {
		return m_GlyphCache;
	}

	float GetLineHeight() const {
		return m_LineHeight;
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
	void BlitSurface(SDL_Surface *src, SDL_Surface *target, SDL_Rect *targetRect);


	CBCachedTTFontText *m_CachedTexts[NUM_CACHED_TEXTS];

	HRESULT InitFont();
	FT_Stream m_FTStream;
	FT_Face m_FTFace;

	FontGlyphCache *m_GlyphCache;

	float m_Ascender;
	float m_Descender;
	float m_LineHeight;
	float m_UnderlinePos;
	float m_PointSize;
	float m_VertDpi;
	float m_HorDpi;

	size_t m_MaxCharWidth;
	size_t m_MaxCharHeight;

public:
	bool m_IsBold;
	bool m_IsItalic;
	bool m_IsUnderline;
	bool m_IsStriked;
	int m_FontHeight;
	char *m_FontFile;

	CBArray<CBTTFontLayer *, CBTTFontLayer *> m_Layers;
	void ClearCache();

};

} // end of namespace WinterMute

#endif
