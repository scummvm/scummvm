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

#include <ft2build.h>
#include FT_FREETYPE_H
#include "dcgf.h"
#include "BFile.h"
#include "BFontTT.h"
#include "FontGlyphCache.h"
#include "PathUtil.h"
#include "StringUtil.h"
#include "MathUtil.h"
#include "BRenderSDL.h"
#include "BSurfaceSDL.h"
#include "BParser.h"
#include "BGame.h"
#include "BFileManager.h"
#include "utils.h"
#include "PlatformSDL.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CBFontTT, false)

//////////////////////////////////////////////////////////////////////////
CBFontTT::CBFontTT(CBGame *inGame): CBFont(inGame) {
	m_FontHeight = 12;
	m_IsBold = m_IsItalic = m_IsUnderline = m_IsStriked = false;

	m_FontFile = NULL;

	for (int i = 0; i < NUM_CACHED_TEXTS; i++) m_CachedTexts[i] = NULL;


	m_FTFace = NULL;
	m_FTStream = NULL;

	m_GlyphCache = NULL;

	m_Ascender = m_Descender = m_LineHeight = m_PointSize = m_UnderlinePos = 0;
	m_HorDpi = m_VertDpi = 0;
	m_MaxCharWidth = m_MaxCharHeight = 0;
}

//////////////////////////////////////////////////////////////////////////
CBFontTT::~CBFontTT(void) {
	ClearCache();

	for (int i = 0; i < m_Layers.GetSize(); i++) {
		delete m_Layers[i];
	}
	m_Layers.RemoveAll();

	delete[] m_FontFile;
	m_FontFile = NULL;

	delete m_GlyphCache;
	m_GlyphCache = NULL;

	if (m_FTFace) {
		FT_Done_Face(m_FTFace);
		m_FTFace = NULL;
	}
	delete[] m_FTStream;
	m_FTStream = NULL;

}


//////////////////////////////////////////////////////////////////////////
void CBFontTT::ClearCache() {
	for (int i = 0; i < NUM_CACHED_TEXTS; i++) {
		if (m_CachedTexts[i]) delete m_CachedTexts[i];
		m_CachedTexts[i] = NULL;
	}

}

//////////////////////////////////////////////////////////////////////////
void CBFontTT::InitLoop() {
	// we need more aggressive cache management on iOS not to waste too much memory on fonts
	if (Game->m_ConstrainedMemory) {
		// purge all cached images not used in the last frame
		for (int i = 0; i < NUM_CACHED_TEXTS; i++) {
			if (m_CachedTexts[i] == NULL) continue;

			if (!m_CachedTexts[i]->m_Marked) {
				delete m_CachedTexts[i];
				m_CachedTexts[i] = NULL;
			} else m_CachedTexts[i]->m_Marked = false;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
int CBFontTT::GetTextWidth(byte  *Text, int MaxLength) {
	WideString text;

	if (Game->m_TextEncoding == TEXT_UTF8) text = StringUtil::Utf8ToWide((char *)Text);
	else text = StringUtil::AnsiToWide((char *)Text);

	if (MaxLength >= 0 && text.size() > MaxLength) 
		text = Common::String(text.c_str(), MaxLength);
		//text = text.substr(0, MaxLength); // TODO: Remove

	int textWidth, textHeight;
	MeasureText(text, -1, -1, textWidth, textHeight);

	return textWidth;
}

//////////////////////////////////////////////////////////////////////////
int CBFontTT::GetTextHeight(byte  *Text, int Width) {
	WideString text;

	if (Game->m_TextEncoding == TEXT_UTF8) text = StringUtil::Utf8ToWide((char *)Text);
	else text = StringUtil::AnsiToWide((char *)Text);


	int textWidth, textHeight;
	MeasureText(text, Width, -1, textWidth, textHeight);

	return textHeight;
}


//////////////////////////////////////////////////////////////////////////
void CBFontTT::DrawText(byte  *Text, int X, int Y, int Width, TTextAlign Align, int MaxHeight, int MaxLength) {
	if (Text == NULL || strcmp((char *)Text, "") == 0) return;

	WideString text;

	if (Game->m_TextEncoding == TEXT_UTF8) text = StringUtil::Utf8ToWide((char *)Text);
	else text = StringUtil::AnsiToWide((char *)Text);

	if (MaxLength >= 0 && text.size() > MaxLength) 
		text = Common::String(text.c_str(), MaxLength);
		//text = text.substr(0, MaxLength); // TODO: Remove

	CBRenderSDL *m_Renderer = (CBRenderSDL *)Game->m_Renderer;

	// find cached surface, if exists
	int MinPriority = INT_MAX;
	int MinIndex = -1;
	CBSurface *Surface = NULL;
	int textOffset = 0;

	for (int i = 0; i < NUM_CACHED_TEXTS; i++) {
		if (m_CachedTexts[i] == NULL) {
			MinPriority = 0;
			MinIndex = i;
		} else {
			if (m_CachedTexts[i]->m_Text == text && m_CachedTexts[i]->m_Align == Align && m_CachedTexts[i]->m_Width == Width && m_CachedTexts[i]->m_MaxHeight == MaxHeight && m_CachedTexts[i]->m_MaxLength == MaxLength) {
				Surface = m_CachedTexts[i]->m_Surface;
				textOffset = m_CachedTexts[i]->m_TextOffset;
				m_CachedTexts[i]->m_Priority++;
				m_CachedTexts[i]->m_Marked = true;
				break;
			} else {
				if (m_CachedTexts[i]->m_Priority < MinPriority) {
					MinPriority = m_CachedTexts[i]->m_Priority;
					MinIndex = i;
				}
			}
		}
	}

	// not found, create one
	if (!Surface) {
		Surface = RenderTextToTexture(text, Width, Align, MaxHeight, textOffset);
		if (Surface) {
			// write surface to cache
			if (m_CachedTexts[MinIndex] != NULL) delete m_CachedTexts[MinIndex];
			m_CachedTexts[MinIndex] = new CBCachedTTFontText;

			m_CachedTexts[MinIndex]->m_Surface = Surface;
			m_CachedTexts[MinIndex]->m_Align = Align;
			m_CachedTexts[MinIndex]->m_Width = Width;
			m_CachedTexts[MinIndex]->m_MaxHeight = MaxHeight;
			m_CachedTexts[MinIndex]->m_MaxLength = MaxLength;
			m_CachedTexts[MinIndex]->m_Priority = 1;
			m_CachedTexts[MinIndex]->m_Text = text;
			m_CachedTexts[MinIndex]->m_TextOffset = textOffset;
			m_CachedTexts[MinIndex]->m_Marked = true;
		}
	}


	// and paint it
	if (Surface) {
		RECT rc;
		CBPlatform::SetRect(&rc, 0, 0, Surface->GetWidth(), Surface->GetHeight());
		for (int i = 0; i < m_Layers.GetSize(); i++) {
			uint32 Color = m_Layers[i]->m_Color;
			uint32 OrigForceAlpha = m_Renderer->m_ForceAlphaColor;
			if (m_Renderer->m_ForceAlphaColor != 0) {
				Color = DRGBA(D3DCOLGetR(Color), D3DCOLGetG(Color), D3DCOLGetB(Color), D3DCOLGetA(m_Renderer->m_ForceAlphaColor));
				m_Renderer->m_ForceAlphaColor = 0;
			}
			Surface->DisplayTransOffset(X, Y - textOffset, rc, Color, BLEND_NORMAL, false, false, m_Layers[i]->m_OffsetX, m_Layers[i]->m_OffsetY);

			m_Renderer->m_ForceAlphaColor = OrigForceAlpha;
		}
	}


}

//////////////////////////////////////////////////////////////////////////
CBSurface *CBFontTT::RenderTextToTexture(const WideString &text, int width, TTextAlign align, int maxHeight, int &textOffset) {
	TextLineList lines;
	WrapText(text, width, maxHeight, lines);


	TextLineList::iterator it;

	int textHeight = lines.size() * (m_MaxCharHeight + m_Ascender);
	SDL_Surface *surface = SDL_CreateRGBSurface(0, width, textHeight, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);

	SDL_LockSurface(surface);

	int posY = (int)GetLineHeight() - (int)m_Descender;

	for (it = lines.begin(); it != lines.end(); ++it) {
		TextLine *line = (*it);
		int posX = 0;

		switch (align) {
		case TAL_CENTER:
			posX += (width - line->GetWidth()) / 2;
			break;

		case TAL_RIGHT:
			posX += width - line->GetWidth();
			break;
		}


		textOffset = 0;
		for (size_t i = 0; i < line->GetText().size(); i++) {
			wchar_t ch = line->GetText()[i];

			GlyphInfo *glyph = m_GlyphCache->GetGlyph(ch);
			if (!glyph) continue;

			textOffset = std::max(textOffset, glyph->GetBearingY());
		}


		int origPosX = posX;

		wchar_t prevChar = L'\0';
		for (size_t i = 0; i < line->GetText().size(); i++) {
			wchar_t ch = line->GetText()[i];

			GlyphInfo *glyph = m_GlyphCache->GetGlyph(ch);
			if (!glyph) continue;

			float kerning = 0;
			if (prevChar != L'\0') kerning = GetKerning(prevChar, ch);
			posX += (int)kerning;


			if (glyph->GetBearingY() > 0) {
				int i = 10;
			}

			SDL_Rect rect;
			rect.x = posX + glyph->GetBearingX();
			rect.y = posY - glyph->GetBearingY() + textOffset;
			rect.w = glyph->GetImage()->w;
			rect.h = glyph->GetImage()->h;

			BlitSurface(glyph->GetImage(), surface, &rect);

			prevChar = ch;
			posX += (int)(glyph->GetAdvanceX());
			posY += (int)(glyph->GetAdvanceY());
		}

		if (m_IsUnderline) {
			for (int i = origPosX; i < origPosX + line->GetWidth(); i++) {
				Uint8 *buf = (Uint8 *)surface->pixels + (int)(m_UnderlinePos + m_Ascender) * surface->pitch;
				Uint32 *buf32 = (Uint32 *)buf;

				buf32[i] = SDL_MapRGBA(surface->format, 255, 255, 255, 255);
			}
		}

		SDL_UnlockSurface(surface);

		delete line;
		line = NULL;
		posY += GetLineHeight();
	}

	CBSurfaceSDL *wmeSurface = new CBSurfaceSDL(Game);
	if (SUCCEEDED(wmeSurface->CreateFromSDLSurface(surface))) {
		SDL_FreeSurface(surface);
		return wmeSurface;
	} else {
		SDL_FreeSurface(surface);
		delete wmeSurface;
		return NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
void CBFontTT::BlitSurface(SDL_Surface *src, SDL_Surface *target, SDL_Rect *targetRect) {
	//SDL_BlitSurface(src, NULL, target, targetRect);

	for (int y = 0; y < src->h; y++) {
		if (targetRect->y + y < 0 || targetRect->y + y >= target->h) continue;


		Uint8 *srcBuf = (Uint8 *)src->pixels + y * src->pitch;
		Uint8 *tgtBuf = (Uint8 *)target->pixels + (y + targetRect->y) * target->pitch;

		Uint32 *srcBuf32 = (Uint32 *)srcBuf;
		Uint32 *tgtBuf32 = (Uint32 *)tgtBuf;

		for (int x = 0; x < src->w; x++) {
			if (targetRect->x + x < 0 || targetRect->x + x >= target->w) continue;

			tgtBuf32[x + targetRect->x] = srcBuf32[x];
		}
	}

}

//////////////////////////////////////////////////////////////////////////
int CBFontTT::GetLetterHeight() {
	return GetLineHeight();
}


//////////////////////////////////////////////////////////////////////
HRESULT CBFontTT::LoadFile(char *Filename) {
	byte *Buffer = Game->m_FileManager->ReadWholeFile(Filename);
	if (Buffer == NULL) {
		Game->LOG(0, "CBFontTT::LoadFile failed for file '%s'", Filename);
		return E_FAIL;
	}

	HRESULT ret;

	m_Filename = new char [strlen(Filename) + 1];
	strcpy(m_Filename, Filename);

	if (FAILED(ret = LoadBuffer(Buffer))) Game->LOG(0, "Error parsing TTFONT file '%s'", Filename);

	delete [] Buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(TTFONT)
TOKEN_DEF(SIZE)
TOKEN_DEF(FACE)
TOKEN_DEF(FILENAME)
TOKEN_DEF(BOLD)
TOKEN_DEF(ITALIC)
TOKEN_DEF(UNDERLINE)
TOKEN_DEF(STRIKE)
TOKEN_DEF(CHARSET)
TOKEN_DEF(COLOR)
TOKEN_DEF(ALPHA)
TOKEN_DEF(LAYER)
TOKEN_DEF(OFFSET_X)
TOKEN_DEF(OFFSET_Y)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////
HRESULT CBFontTT::LoadBuffer(byte  *Buffer) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(TTFONT)
	TOKEN_TABLE(SIZE)
	TOKEN_TABLE(FACE)
	TOKEN_TABLE(FILENAME)
	TOKEN_TABLE(BOLD)
	TOKEN_TABLE(ITALIC)
	TOKEN_TABLE(UNDERLINE)
	TOKEN_TABLE(STRIKE)
	TOKEN_TABLE(CHARSET)
	TOKEN_TABLE(COLOR)
	TOKEN_TABLE(ALPHA)
	TOKEN_TABLE(LAYER)
	TOKEN_TABLE_END

	char *params;
	int cmd;
	CBParser parser(Game);

	if (parser.GetCommand((char **)&Buffer, commands, (char **)&params) != TOKEN_TTFONT) {
		Game->LOG(0, "'TTFONT' keyword expected.");
		return E_FAIL;
	}
	Buffer = (byte  *)params;

	uint32 BaseColor = 0x00000000;

	while ((cmd = parser.GetCommand((char **)&Buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_SIZE:
			parser.ScanStr(params, "%d", &m_FontHeight);
			break;

		case TOKEN_FACE:
			// we don't need this anymore
			break;

		case TOKEN_FILENAME:
			CBUtils::SetString(&m_FontFile, params);
			break;

		case TOKEN_BOLD:
			parser.ScanStr(params, "%b", &m_IsBold);
			break;

		case TOKEN_ITALIC:
			parser.ScanStr(params, "%b", &m_IsItalic);
			break;

		case TOKEN_UNDERLINE:
			parser.ScanStr(params, "%b", &m_IsUnderline);
			break;

		case TOKEN_STRIKE:
			parser.ScanStr(params, "%b", &m_IsStriked);
			break;

		case TOKEN_CHARSET:
			// we don't need this anymore
			break;

		case TOKEN_COLOR: {
			int r, g, b;
			parser.ScanStr(params, "%d,%d,%d", &r, &g, &b);
			BaseColor = DRGBA(r, g, b, D3DCOLGetA(BaseColor));
		}
		break;

		case TOKEN_ALPHA: {
			int a;
			parser.ScanStr(params, "%d", &a);
			BaseColor = DRGBA(D3DCOLGetR(BaseColor), D3DCOLGetG(BaseColor), D3DCOLGetB(BaseColor), a);
		}
		break;

		case TOKEN_LAYER: {
			CBTTFontLayer *Layer = new CBTTFontLayer;
			if (Layer && SUCCEEDED(ParseLayer(Layer, (byte  *)params))) m_Layers.Add(Layer);
			else {
				delete Layer;
				Layer = NULL;
				cmd = PARSERR_TOKENNOTFOUND;
			}
		}
		break;

		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		Game->LOG(0, "Syntax error in TTFONT definition");
		return E_FAIL;
	}

	// create at least one layer
	if (m_Layers.GetSize() == 0) {
		CBTTFontLayer *Layer = new CBTTFontLayer;
		Layer->m_Color = BaseColor;
		m_Layers.Add(Layer);
	}

	if (!m_FontFile) CBUtils::SetString(&m_FontFile, "arial.ttf");

	return InitFont();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFontTT::ParseLayer(CBTTFontLayer *Layer, byte *Buffer) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(OFFSET_X)
	TOKEN_TABLE(OFFSET_Y)
	TOKEN_TABLE(COLOR)
	TOKEN_TABLE(ALPHA)
	TOKEN_TABLE_END

	char *params;
	int cmd;
	CBParser parser(Game);

	while ((cmd = parser.GetCommand((char **)&Buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_OFFSET_X:
			parser.ScanStr(params, "%d", &Layer->m_OffsetX);
			break;

		case TOKEN_OFFSET_Y:
			parser.ScanStr(params, "%d", &Layer->m_OffsetY);
			break;

		case TOKEN_COLOR: {
			int r, g, b;
			parser.ScanStr(params, "%d,%d,%d", &r, &g, &b);
			Layer->m_Color = DRGBA(r, g, b, D3DCOLGetA(Layer->m_Color));
		}
		break;

		case TOKEN_ALPHA: {
			int a;
			parser.ScanStr(params, "%d", &a);
			Layer->m_Color = DRGBA(D3DCOLGetR(Layer->m_Color), D3DCOLGetG(Layer->m_Color), D3DCOLGetB(Layer->m_Color), a);
		}
		break;
		}
	}
	if (cmd != PARSERR_EOF) return E_FAIL;
	else return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFontTT::Persist(CBPersistMgr *PersistMgr) {
	CBFont::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(m_IsBold));
	PersistMgr->Transfer(TMEMBER(m_IsItalic));
	PersistMgr->Transfer(TMEMBER(m_IsUnderline));
	PersistMgr->Transfer(TMEMBER(m_IsStriked));
	PersistMgr->Transfer(TMEMBER(m_FontHeight));
	PersistMgr->Transfer(TMEMBER(m_FontFile));


	// persist layers
	int NumLayers;
	if (PersistMgr->m_Saving) {
		NumLayers = m_Layers.GetSize();
		PersistMgr->Transfer(TMEMBER(NumLayers));
		for (int i = 0; i < NumLayers; i++) m_Layers[i]->Persist(PersistMgr);
	} else {
		NumLayers = m_Layers.GetSize();
		PersistMgr->Transfer(TMEMBER(NumLayers));
		for (int i = 0; i < NumLayers; i++) {
			CBTTFontLayer *Layer = new CBTTFontLayer;
			Layer->Persist(PersistMgr);
			m_Layers.Add(Layer);
		}
	}

	if (!PersistMgr->m_Saving) {
		for (int i = 0; i < NUM_CACHED_TEXTS; i++) m_CachedTexts[i] = NULL;
		m_GlyphCache = NULL;
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CBFontTT::AfterLoad() {
	InitFont();
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBFontTT::InitFont() {
	if (!m_FontFile) return E_FAIL;

	CBFile *file = Game->m_FileManager->OpenFile(m_FontFile);
	if (!file) {
		// the requested font file is not in wme file space; try loading a system font
		AnsiString fontFileName = PathUtil::Combine(CBPlatform::GetSystemFontPath(), PathUtil::GetFileName(m_FontFile));
		file = Game->m_FileManager->OpenFile((char *)fontFileName.c_str(), false);

		if (!file) {
			Game->LOG(0, "Error loading TrueType font '%s'", m_FontFile);
			return E_FAIL;
		}
	}

	FT_Error error;

	float vertDpi = 96.0;
	float horDpi = 96.0;


	m_FTStream = (FT_Stream)new byte[sizeof(*m_FTStream)];
	memset(m_FTStream, 0, sizeof(*m_FTStream));

	m_FTStream->read = CBFontTT::FTReadSeekProc;
	m_FTStream->close = CBFontTT::FTCloseProc;
	m_FTStream->descriptor.pointer = file;
	m_FTStream->size = file->GetSize();

	FT_Open_Args args;
	args.flags = FT_OPEN_STREAM;
	args.stream = m_FTStream;

	error = FT_Open_Face(Game->m_FontStorage->GetFTLibrary(), &args, 0, &m_FTFace);
	if (error) {
		SAFE_DELETE_ARRAY(m_FTStream);
		Game->m_FileManager->CloseFile(file);
		return E_FAIL;
	}

	error = FT_Set_Char_Size(m_FTFace, 0, (FT_F26Dot6)(m_FontHeight * 64), (FT_UInt)horDpi, (FT_UInt)vertDpi);
	if (error) {
		FT_Done_Face(m_FTFace);
		m_FTFace = NULL;
		return E_FAIL;
	}

	// http://en.wikipedia.org/wiki/Em_(typography)
	float pixelsPerEm = (m_FontHeight / 72.f) * vertDpi; // Size in inches * dpi
	float EmsPerUnit = 1.0f / m_FTFace->units_per_EM;
	float pixelsPerUnit = pixelsPerEm * EmsPerUnit;

	// bounding box in pixels
	float xMin = m_FTFace->bbox.xMin * pixelsPerUnit;
	float xMax = m_FTFace->bbox.xMax * pixelsPerUnit;
	float yMin = m_FTFace->bbox.yMin * pixelsPerUnit;
	float yMax = m_FTFace->bbox.yMax * pixelsPerUnit;

	// metrics in pixels
	m_Ascender = m_FTFace->ascender * pixelsPerUnit;
	m_Descender = - m_FTFace->descender * pixelsPerUnit;
	m_LineHeight = MathUtil::RoundUp(m_FTFace->height * pixelsPerUnit) + 2;
	m_UnderlinePos = - m_FTFace->underline_position * pixelsPerUnit;

	// max character size (used for texture grid)
	m_MaxCharWidth  = (size_t)MathUtil::RoundUp(xMax - xMin);
	m_MaxCharHeight = (size_t)MathUtil::RoundUp(yMax - yMin);

	m_GlyphCache = new FontGlyphCache();
	m_GlyphCache->Initialize();


	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
// I/O bridge between FreeType and WME file system
//////////////////////////////////////////////////////////////////////////
unsigned long CBFontTT::FTReadSeekProc(FT_Stream stream, unsigned long offset,  unsigned char *buffer, unsigned long count) {
	CBFile *f = static_cast<CBFile *>(stream->descriptor.pointer);
	if (!f) return 0;

	f->Seek(offset, SEEK_TO_BEGIN);
	if (count) {
		uint32 oldPos = f->GetPos();
		f->Read(buffer, count);
		return f->GetPos() - oldPos;
	} else return 0;
}

//////////////////////////////////////////////////////////////////////////
void CBFontTT::FTCloseProc(FT_Stream stream) {
	CBFile *f = static_cast<CBFile *>(stream->descriptor.pointer);
	if (!f) return;

	CBGame *Game = f->Game;

	Game->m_FileManager->CloseFile(f);
	stream->descriptor.pointer = NULL;
}



//////////////////////////////////////////////////////////////////////////
void CBFontTT::WrapText(const WideString &text, int maxWidth, int maxHeight, TextLineList &lines) {
	int currWidth = 0;
	wchar_t prevChar = L'\0';
	int prevSpaceIndex = -1;
	int prevSpaceWidth = 0;
	int lineStartIndex = 0;

	PrepareGlyphs(text);

	for (size_t i = 0; i < text.size(); i++) {
		wchar_t ch = text[i];

		if (ch == L' ') {
			prevSpaceIndex = i;
			prevSpaceWidth = currWidth;
		}

		int charWidth = 0;

		if (ch != L'\n') {
			GlyphInfo *glyphInfo = GetGlyphCache()->GetGlyph(ch);
			if (!glyphInfo) continue;

			float kerning = 0;
			if (prevChar != L'\0') kerning = GetKerning(prevChar, ch);
			prevChar = ch;

			charWidth = (int)(glyphInfo->GetAdvanceX() + kerning);
		}

		bool lineTooLong = maxWidth >= 0 && currWidth + charWidth > maxWidth;
		bool breakOnSpace = false;

		// we can't fit even a single character
		if (lineTooLong && currWidth == 0) break;


		if (ch == L'\n' || i == text.size() - 1 || lineTooLong) {
			int breakPoint, breakWidth;

			if (prevSpaceIndex >= 0 && lineTooLong) {
				breakPoint = prevSpaceIndex;
				breakWidth = prevSpaceWidth;
				breakOnSpace = true;
			} else {
				breakPoint = i;
				breakWidth = currWidth;

				breakOnSpace = (ch == L'\n');

				// we're at the end
				if (i == text.size() - 1) {
					breakPoint++;
					breakWidth += charWidth;
				}
			}

			if (maxHeight >= 0 && (lines.size() + 1) * GetLineHeight() > maxHeight) break;

			//WideString line = text.substr(lineStartIndex, breakPoint - lineStartIndex); // TODO: Remove
			WideString line = Common::String(text.c_str() + lineStartIndex, breakPoint - lineStartIndex);
			lines.push_back(new TextLine(line, breakWidth));

			currWidth = 0;
			prevChar = L'\0';
			prevSpaceIndex = -1;

			if (breakOnSpace) breakPoint++;

			lineStartIndex = breakPoint;
			i = breakPoint - 1;

			continue;
		}

		//if (ch == L' ' && currLine.empty()) continue;
		currWidth += charWidth;
	}
}

//////////////////////////////////////////////////////////////////////////
void CBFontTT::MeasureText(const WideString &text, int maxWidth, int maxHeight, int &textWidth, int &textHeight) {
	TextLineList lines;
	WrapText(text, maxWidth, maxHeight, lines);

	textHeight = (int)(lines.size() * GetLineHeight());
	textWidth = 0;

	TextLineList::iterator it;
	for (it = lines.begin(); it != lines.end(); ++it) {
		TextLine *line = (*it);
		textWidth = std::max(textWidth, line->GetWidth());
		SAFE_DELETE(line);
	}
}


//////////////////////////////////////////////////////////////////////////
float CBFontTT::GetKerning(wchar_t leftChar, wchar_t rightChar) {
	GlyphInfo *infoLeft = m_GlyphCache->GetGlyph(leftChar);
	GlyphInfo *infoRight = m_GlyphCache->GetGlyph(rightChar);

	if (!infoLeft || !infoRight) return 0;

	FT_Vector delta;
	FT_Error error = FT_Get_Kerning(m_FTFace, infoLeft->GetGlyphIndex(), infoRight->GetGlyphIndex(), ft_kerning_unfitted, &delta);
	if (error) return 0;

	return delta.x * (1.0f / 64.0f);
}


//////////////////////////////////////////////////////////////////////////
void CBFontTT::PrepareGlyphs(const WideString &text) {
	// make sure we have all the glyphs we need
	for (size_t i = 0; i < text.size(); i++) {
		wchar_t ch = text[i];
		if (!m_GlyphCache->HasGlyph(ch)) CacheGlyph(ch);
	}
}

//////////////////////////////////////////////////////////////////////////
void CBFontTT::CacheGlyph(wchar_t ch) {
	FT_UInt glyphIndex = FT_Get_Char_Index(m_FTFace, ch);
	if (!glyphIndex) return;

	FT_Error error = FT_Load_Glyph(m_FTFace, glyphIndex, FT_LOAD_DEFAULT);
	if (error) return;

	error = FT_Render_Glyph(m_FTFace->glyph, FT_RENDER_MODE_NORMAL);
	if (error) return;

	byte *pixels = m_FTFace->glyph->bitmap.buffer;
	size_t stride = m_FTFace->glyph->bitmap.pitch;


	// convert from monochrome to grayscale if needed
	byte *tempBuffer = NULL;
	if (pixels != NULL && m_FTFace->glyph->bitmap.pixel_mode == FT_PIXEL_MODE_MONO) {
		tempBuffer = new byte[m_FTFace->glyph->bitmap.width * m_FTFace->glyph->bitmap.rows];
		for (int j = 0; j < m_FTFace->glyph->bitmap.rows; j++) {
			int rowOffset = stride * j;
			for (int i = 0; i < m_FTFace->glyph->bitmap.width; i++) {
				int byteOffset = i / 8;
				int bitOffset = 7 - (i % 8);
				byte bit = (pixels[rowOffset + byteOffset] & (1 << bitOffset)) >> bitOffset;
				tempBuffer[m_FTFace->glyph->bitmap.width * j + i] = 255 * bit;
			}
		}

		pixels = tempBuffer;
		stride = m_FTFace->glyph->bitmap.width;
	}

	// add glyph to cache
	m_GlyphCache->AddGlyph(ch, glyphIndex, m_FTFace->glyph, m_FTFace->glyph->bitmap.width, m_FTFace->glyph->bitmap.rows, pixels, stride);

	if (tempBuffer) delete [] tempBuffer;
}

} // end of namespace WinterMute
