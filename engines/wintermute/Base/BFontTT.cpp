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

//#include <ft2build.h>
//#include FT_FREETYPE_H
#include "engines/wintermute/dcgf.h"
#include "engines/wintermute/Base/file/BFile.h"
#include "engines/wintermute/Base/BFontTT.h"
#include "engines/wintermute/FontGlyphCache.h"
#include "engines/wintermute/utils/PathUtil.h"
#include "engines/wintermute/utils/StringUtil.h"
#include "engines/wintermute/math/MathUtil.h"
#include "engines/wintermute/Base/BRenderSDL.h"
#include "engines/wintermute/Base/BSurfaceSDL.h"
#include "engines/wintermute/Base/BParser.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/BFileManager.h"
#include "engines/wintermute/utils/utils.h"
#include "engines/wintermute/PlatformSDL.h"
#include "graphics/fonts/ttf.h"
#include "graphics/fontman.h"
#include <limits.h>

namespace WinterMute {

IMPLEMENT_PERSISTENT(CBFontTT, false)

//////////////////////////////////////////////////////////////////////////
CBFontTT::CBFontTT(CBGame *inGame): CBFont(inGame) {
	_fontHeight = 12;
	_isBold = _isItalic = _isUnderline = _isStriked = false;

	_fontFile = NULL;
	_font = NULL;
	_fallbackFont = NULL;
	_deletableFont = NULL;

	for (int i = 0; i < NUM_CACHED_TEXTS; i++) _cachedTexts[i] = NULL;

#if 0
	_fTFace = NULL;
	_fTStream = NULL;
#endif
	_glyphCache = NULL;

	_ascender = _descender = _lineHeight = _pointSize = _underlinePos = 0;
	_horDpi = _vertDpi = 0;
	_maxCharWidth = _maxCharHeight = 0;
}

//////////////////////////////////////////////////////////////////////////
CBFontTT::~CBFontTT(void) {
	ClearCache();

	for (int i = 0; i < _layers.GetSize(); i++) {
		delete _layers[i];
	}
	_layers.RemoveAll();

	delete[] _fontFile;
	_fontFile = NULL;

	delete _deletableFont;
	_font = NULL;

	delete _glyphCache;
	_glyphCache = NULL;
#if 0
	if (_fTFace) {
		FT_Done_Face(_fTFace);
		_fTFace = NULL;
	}
	delete[] _fTStream;
	_fTStream = NULL;
#endif
}


//////////////////////////////////////////////////////////////////////////
void CBFontTT::ClearCache() {
	for (int i = 0; i < NUM_CACHED_TEXTS; i++) {
		if (_cachedTexts[i]) delete _cachedTexts[i];
		_cachedTexts[i] = NULL;
	}

}

//////////////////////////////////////////////////////////////////////////
void CBFontTT::InitLoop() {
	// we need more aggressive cache management on iOS not to waste too much memory on fonts
	if (Game->_constrainedMemory) {
		// purge all cached images not used in the last frame
		for (int i = 0; i < NUM_CACHED_TEXTS; i++) {
			if (_cachedTexts[i] == NULL) continue;

			if (!_cachedTexts[i]->_marked) {
				delete _cachedTexts[i];
				_cachedTexts[i] = NULL;
			} else _cachedTexts[i]->_marked = false;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
int CBFontTT::GetTextWidth(byte  *Text, int MaxLength) {
	WideString text;

	if (Game->_textEncoding == TEXT_UTF8) text = StringUtil::Utf8ToWide((char *)Text);
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

	if (Game->_textEncoding == TEXT_UTF8) text = StringUtil::Utf8ToWide((char *)Text);
	    else text = StringUtil::AnsiToWide((char *)Text);


	int textWidth, textHeight;
	MeasureText(text, Width, -1, textWidth, textHeight);

	return textHeight;
}


//////////////////////////////////////////////////////////////////////////
void CBFontTT::DrawText(byte  *Text, int X, int Y, int Width, TTextAlign Align, int MaxHeight, int MaxLength) {
	if (Text == NULL || strcmp((char *)Text, "") == 0) return;

	WideString text = (char*)Text;

	// TODO: Why do we still insist on Widestrings everywhere?
/*	if (Game->_textEncoding == TEXT_UTF8) text = StringUtil::Utf8ToWide((char *)Text);
	    else text = StringUtil::AnsiToWide((char *)Text);*/

	if (MaxLength >= 0 && text.size() > MaxLength)
		text = Common::String(text.c_str(), MaxLength);
	//text = text.substr(0, MaxLength); // TODO: Remove

	CBRenderSDL *_renderer = (CBRenderSDL *)Game->_renderer;

	// find cached surface, if exists
	int MinPriority = INT_MAX;
	int MinIndex = -1;
	CBSurface *Surface = NULL;
	int textOffset = 0;

	for (int i = 0; i < NUM_CACHED_TEXTS; i++) {
		if (_cachedTexts[i] == NULL) {
			MinPriority = 0;
			MinIndex = i;
		} else {
			if (_cachedTexts[i]->_text == text && _cachedTexts[i]->_align == Align && _cachedTexts[i]->_width == Width && _cachedTexts[i]->_maxHeight == MaxHeight && _cachedTexts[i]->_maxLength == MaxLength) {
				Surface = _cachedTexts[i]->_surface;
				textOffset = _cachedTexts[i]->_textOffset;
				_cachedTexts[i]->_priority++;
				_cachedTexts[i]->_marked = true;
				break;
			} else {
				if (_cachedTexts[i]->_priority < MinPriority) {
					MinPriority = _cachedTexts[i]->_priority;
					MinIndex = i;
				}
			}
		}
	}

	// not found, create one
	if (!Surface) {
		warning("Draw text: %s", Text);
		Surface = RenderTextToTexture(text, Width, Align, MaxHeight, textOffset);
		if (Surface) {
			// write surface to cache
			if (_cachedTexts[MinIndex] != NULL) delete _cachedTexts[MinIndex];
			_cachedTexts[MinIndex] = new CBCachedTTFontText;

			_cachedTexts[MinIndex]->_surface = Surface;
			_cachedTexts[MinIndex]->_align = Align;
			_cachedTexts[MinIndex]->_width = Width;
			_cachedTexts[MinIndex]->_maxHeight = MaxHeight;
			_cachedTexts[MinIndex]->_maxLength = MaxLength;
			_cachedTexts[MinIndex]->_priority = 1;
			_cachedTexts[MinIndex]->_text = text;
			_cachedTexts[MinIndex]->_textOffset = textOffset;
			_cachedTexts[MinIndex]->_marked = true;
		}
	}


	// and paint it
	if (Surface) {
		RECT rc;
		CBPlatform::SetRect(&rc, 0, 0, Surface->getWidth(), Surface->getHeight());
		for (int i = 0; i < _layers.GetSize(); i++) {
			uint32 Color = _layers[i]->_color;
			uint32 OrigForceAlpha = _renderer->_forceAlphaColor;
			if (_renderer->_forceAlphaColor != 0) {
				Color = DRGBA(D3DCOLGetR(Color), D3DCOLGetG(Color), D3DCOLGetB(Color), D3DCOLGetA(_renderer->_forceAlphaColor));
				_renderer->_forceAlphaColor = 0;
			}
			Surface->displayTransOffset(X, Y - textOffset, rc, Color, BLEND_NORMAL, false, false, _layers[i]->_offsetX, _layers[i]->_offsetY);

			_renderer->_forceAlphaColor = OrigForceAlpha;
		}
	}


}

//////////////////////////////////////////////////////////////////////////
CBSurface *CBFontTT::RenderTextToTexture(const WideString &text, int width, TTextAlign align, int maxHeight, int &textOffset) {
	//TextLineList lines;
	// TODO
	//WrapText(text, width, maxHeight, lines);
	Common::Array<Common::String> lines;
	_font->wordWrapText(text, width, lines);

	Graphics::TextAlign alignment = Graphics::kTextAlignInvalid;
	if (align == TAL_LEFT) {
		alignment = Graphics::kTextAlignLeft;
	} else if (align == TAL_CENTER) {
		alignment = Graphics::kTextAlignCenter;
	} else if (align == TAL_RIGHT) {
		alignment = Graphics::kTextAlignRight;
	}
	// TODO: This function gets called a lot, so warnings like these drown out the usefull information
	static bool hasWarned = false;
	if (!hasWarned) {
		hasWarned = true;
		warning("CBFontTT::RenderTextToTexture - Not fully ported yet");
	}
	warning("%s %d %d %d %d", text.c_str(), D3DCOLGetR(_layers[0]->_color), D3DCOLGetG(_layers[0]->_color),D3DCOLGetB(_layers[0]->_color),D3DCOLGetA(_layers[0]->_color));
//	void drawString(Surface *dst, const Common::String &str, int x, int y, int w, uint32 color, TextAlign align = kTextAlignLeft, int deltax = 0, bool useEllipsis = true) const;
	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(width, _fontHeight * lines.size(), Graphics::PixelFormat(2, 5, 5, 5, 1, 10, 5, 0, 15));

	uint16 useColor = 0xffff;
	Common::Array<Common::String>::iterator it;
	int heightOffset = 0;
	for (it = lines.begin(); it != lines.end(); it++) {
		_font->drawString(surface, *it, 0, heightOffset, width, useColor, alignment);	
		heightOffset += _lineHeight;
	}

	CBSurfaceSDL *retSurface = new CBSurfaceSDL(Game);
	retSurface->putSurface(*surface->convertTo(Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8 ,0)));
	delete surface;
	return retSurface;
#if 0 //TODO
	int textHeight = lines.size() * (_maxCharHeight + _ascender);
	SDL_Surface *surface = SDL_CreateRGBSurface(0, width, textHeight, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);

	SDL_LockSurface(surface);

	int posY = (int)GetLineHeight() - (int)_descender;

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

			GlyphInfo *glyph = _glyphCache->GetGlyph(ch);
			if (!glyph) continue;

			textOffset = MAX(textOffset, glyph->GetBearingY());
		}


		int origPosX = posX;

		wchar_t prevChar = L'\0';
		for (size_t i = 0; i < line->GetText().size(); i++) {
			wchar_t ch = line->GetText()[i];

			GlyphInfo *glyph = _glyphCache->GetGlyph(ch);
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

		if (_isUnderline) {
			for (int i = origPosX; i < origPosX + line->GetWidth(); i++) {
				Uint8 *buf = (Uint8 *)surface->pixels + (int)(_underlinePos + _ascender) * surface->pitch;
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
#endif
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
void CBFontTT::BlitSurface(Graphics::Surface *src, Graphics::Surface *target, Common::Rect *targetRect) {
	//SDL_BlitSurface(src, NULL, target, targetRect);
	warning("CBFontTT::BlitSurface - not ported yet");
#if 0
	for (int y = 0; y < src->h; y++) {
		if (targetRect->y + y < 0 || targetRect->y + y >= target->h) continue;


		uint8 *srcBuf = (uint8 *)src->pixels + y * src->pitch;
		uint8 *tgtBuf = (uint8 *)target->pixels + (y + targetRect->y) * target->pitch;

		uint32 *srcBuf32 = (uint32 *)srcBuf;
		uint32 *tgtBuf32 = (uint32 *)tgtBuf;

		for (int x = 0; x < src->w; x++) {
			if (targetRect->x + x < 0 || targetRect->x + x >= target->w) continue;

			tgtBuf32[x + targetRect->x] = srcBuf32[x];
		}
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
int CBFontTT::GetLetterHeight() {
	return GetLineHeight();
}


//////////////////////////////////////////////////////////////////////
HRESULT CBFontTT::LoadFile(const char *Filename) {
	byte *Buffer = Game->_fileManager->ReadWholeFile(Filename);
	if (Buffer == NULL) {
		Game->LOG(0, "CBFontTT::LoadFile failed for file '%s'", Filename);
		return E_FAIL;
	}

	HRESULT ret;

	_filename = new char [strlen(Filename) + 1];
	strcpy(_filename, Filename);

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
	Buffer = (byte *)params;

	uint32 BaseColor = 0x00000000;

	while ((cmd = parser.GetCommand((char **)&Buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_SIZE:
			parser.ScanStr(params, "%d", &_fontHeight);
			break;

		case TOKEN_FACE:
			// we don't need this anymore
			break;

		case TOKEN_FILENAME:
			CBUtils::SetString(&_fontFile, params);
			break;

		case TOKEN_BOLD:
			parser.ScanStr(params, "%b", &_isBold);
			break;

		case TOKEN_ITALIC:
			parser.ScanStr(params, "%b", &_isItalic);
			break;

		case TOKEN_UNDERLINE:
			parser.ScanStr(params, "%b", &_isUnderline);
			break;

		case TOKEN_STRIKE:
			parser.ScanStr(params, "%b", &_isStriked);
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
			if (Layer && SUCCEEDED(ParseLayer(Layer, (byte *)params))) _layers.Add(Layer);
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
	if (_layers.GetSize() == 0) {
		CBTTFontLayer *Layer = new CBTTFontLayer;
		Layer->_color = BaseColor;
		_layers.Add(Layer);
	}

	if (!_fontFile) CBUtils::SetString(&_fontFile, "arial.ttf");

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
			parser.ScanStr(params, "%d", &Layer->_offsetX);
			break;

		case TOKEN_OFFSET_Y:
			parser.ScanStr(params, "%d", &Layer->_offsetY);
			break;

		case TOKEN_COLOR: {
			int r, g, b;
			parser.ScanStr(params, "%d,%d,%d", &r, &g, &b);
			Layer->_color = DRGBA(r, g, b, D3DCOLGetA(Layer->_color));
		}
		break;

		case TOKEN_ALPHA: {
			int a;
			parser.ScanStr(params, "%d", &a);
			Layer->_color = DRGBA(D3DCOLGetR(Layer->_color), D3DCOLGetG(Layer->_color), D3DCOLGetB(Layer->_color), a);
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

	PersistMgr->Transfer(TMEMBER(_isBold));
	PersistMgr->Transfer(TMEMBER(_isItalic));
	PersistMgr->Transfer(TMEMBER(_isUnderline));
	PersistMgr->Transfer(TMEMBER(_isStriked));
	PersistMgr->Transfer(TMEMBER(_fontHeight));
	PersistMgr->Transfer(TMEMBER(_fontFile));


	// persist layers
	int NumLayers;
	if (PersistMgr->_saving) {
		NumLayers = _layers.GetSize();
		PersistMgr->Transfer(TMEMBER(NumLayers));
		for (int i = 0; i < NumLayers; i++) _layers[i]->Persist(PersistMgr);
	} else {
		NumLayers = _layers.GetSize();
		PersistMgr->Transfer(TMEMBER(NumLayers));
		for (int i = 0; i < NumLayers; i++) {
			CBTTFontLayer *Layer = new CBTTFontLayer;
			Layer->Persist(PersistMgr);
			_layers.Add(Layer);
		}
	}

	if (!PersistMgr->_saving) {
		for (int i = 0; i < NUM_CACHED_TEXTS; i++) _cachedTexts[i] = NULL;
		_glyphCache = NULL;
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CBFontTT::AfterLoad() {
	InitFont();
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBFontTT::InitFont() {
	if (!_fontFile) return E_FAIL;

	Common::SeekableReadStream *file = Game->_fileManager->OpenFile(_fontFile);
	if (!file) {
		// the requested font file is not in wme file space; try loading a system font
		AnsiString fontFileName = PathUtil::Combine(CBPlatform::GetSystemFontPath(), PathUtil::GetFileName(_fontFile));
		file = Game->_fileManager->OpenFile((char *)fontFileName.c_str(), false);
		if (!file) {
			Game->LOG(0, "Error loading TrueType font '%s'", _fontFile);
			//return E_FAIL;
		}
	}

	if (file) {
#ifdef USE_FREETYPE2
		_deletableFont = Graphics::loadTTFFont(*file, _fontHeight);
		_font = _deletableFont;
#endif
	} 
	if (!_font) {
		_font = _fallbackFont = FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
		warning("BFontTT::InitFont - Couldn't load %s", _fontFile);
	}
	_lineHeight = _font->getFontHeight();
	return S_OK;
#if 0
	FT_Error error;

	float vertDpi = 96.0;
	float horDpi = 96.0;


	_fTStream = (FT_Stream)new byte[sizeof(*_fTStream)];
	memset(_fTStream, 0, sizeof(*_fTStream));

	_fTStream->read = CBFontTT::FTReadSeekProc;
	_fTStream->close = CBFontTT::FTCloseProc;
	_fTStream->descriptor.pointer = file;
	_fTStream->size = file->GetSize();

	FT_Open_Args args;
	args.flags = FT_OPEN_STREAM;
	args.stream = _fTStream;

	error = FT_Open_Face(Game->_fontStorage->GetFTLibrary(), &args, 0, &_fTFace);
	if (error) {
		SAFE_DELETE_ARRAY(_fTStream);
		Game->_fileManager->CloseFile(file);
		return E_FAIL;
	}

	error = FT_Set_Char_Size(_fTFace, 0, (FT_F26Dot6)(_fontHeight * 64), (FT_UInt)horDpi, (FT_UInt)vertDpi);
	if (error) {
		FT_Done_Face(_fTFace);
		_fTFace = NULL;
		return E_FAIL;
	}

	// http://en.wikipedia.org/wiki/E_(typography)
	float pixelsPerEm = (_fontHeight / 72.f) * vertDpi; // Size in inches * dpi
	float EmsPerUnit = 1.0f / _fTFace->units_per_EM;
	float pixelsPerUnit = pixelsPerEm * EmsPerUnit;

	// bounding box in pixels
	float xMin = _fTFace->bbox.xMin * pixelsPerUnit;
	float xMax = _fTFace->bbox.xMax * pixelsPerUnit;
	float yMin = _fTFace->bbox.yMin * pixelsPerUnit;
	float yMax = _fTFace->bbox.yMax * pixelsPerUnit;

	// metrics in pixels
	_ascender = _fTFace->ascender * pixelsPerUnit;
	_descender = - _fTFace->descender * pixelsPerUnit;
	_lineHeight = MathUtil::RoundUp(_fTFace->height * pixelsPerUnit) + 2;
	_underlinePos = - _fTFace->underline_position * pixelsPerUnit;

	// max character size (used for texture grid)
	_maxCharWidth  = (size_t)MathUtil::RoundUp(xMax - xMin);
	_maxCharHeight = (size_t)MathUtil::RoundUp(yMax - yMin);

	_glyphCache = new FontGlyphCache();
	_glyphCache->Initialize();

#endif
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
// I/O bridge between FreeType and WME file system
//////////////////////////////////////////////////////////////////////////
/*
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

    Game->_fileManager->CloseFile(f);
    stream->descriptor.pointer = NULL;
}*/



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
	//TextLineList lines;
	// TODO: This function gets called a lot, so warnings like these drown out the usefull information
	static bool hasWarned = false;
	if (!hasWarned) {
		hasWarned = true;
		warning("Todo: Test Mesuretext");
	}
	if (maxWidth >= 0) {
		Common::Array<Common::String> lines;
		_font->wordWrapText(text, maxWidth, lines);
		Common::Array<Common::String>::iterator it;
		textWidth = 0;
		for (it = lines.begin(); it != lines.end(); it++) {
			textWidth = MAX(textWidth, _font->getStringWidth(*it));
		}
	
		//WrapText(text, maxWidth, maxHeight, lines);

		textHeight = (int)(lines.size() * GetLineHeight());
	} else {
		textWidth = _font->getStringWidth(text);
		textHeight = _fontHeight;
	}
/*
	TextLineList::iterator it;
	for (it = lines.begin(); it != lines.end(); ++it) {
		TextLine *line = (*it);
		textWidth = MAX(textWidth, line->GetWidth());
		delete line;
		line = NULL;
	}*/
}


//////////////////////////////////////////////////////////////////////////
float CBFontTT::GetKerning(wchar_t leftChar, wchar_t rightChar) {
	GlyphInfo *infoLeft = _glyphCache->GetGlyph(leftChar);
	GlyphInfo *infoRight = _glyphCache->GetGlyph(rightChar);
#if 0
	if (!infoLeft || !infoRight) return 0;

	FT_Vector delta;
	FT_Error error = FT_Get_Kerning(_fTFace, infoLeft->GetGlyphIndex(), infoRight->GetGlyphIndex(), ft_kerning_unfitted, &delta);
	if (error) return 0;

	return delta.x * (1.0f / 64.0f);
#endif
	return 0;
}


//////////////////////////////////////////////////////////////////////////
void CBFontTT::PrepareGlyphs(const WideString &text) {
	// make sure we have all the glyphs we need
	for (size_t i = 0; i < text.size(); i++) {
		wchar_t ch = text[i];
		if (!_glyphCache->HasGlyph(ch)) CacheGlyph(ch);
	}
}

//////////////////////////////////////////////////////////////////////////
void CBFontTT::CacheGlyph(wchar_t ch) {
#if 0
	FT_UInt glyphIndex = FT_Get_Char_Index(_fTFace, ch);
	if (!glyphIndex) return;

	FT_Error error = FT_Load_Glyph(_fTFace, glyphIndex, FT_LOAD_DEFAULT);
	if (error) return;

	error = FT_Render_Glyph(_fTFace->glyph, FT_RENDER_MODE_NORMAL);
	if (error) return;

	byte *pixels = _fTFace->glyph->bitmap.buffer;
	size_t stride = _fTFace->glyph->bitmap.pitch;


	// convert from monochrome to grayscale if needed
	byte *tempBuffer = NULL;
	if (pixels != NULL && _fTFace->glyph->bitmap.pixel_mode == FT_PIXEL_MODE_MONO) {
		tempBuffer = new byte[_fTFace->glyph->bitmap.width * _fTFace->glyph->bitmap.rows];
		for (int j = 0; j < _fTFace->glyph->bitmap.rows; j++) {
			int rowOffset = stride * j;
			for (int i = 0; i < _fTFace->glyph->bitmap.width; i++) {
				int byteOffset = i / 8;
				int bitOffset = 7 - (i % 8);
				byte bit = (pixels[rowOffset + byteOffset] & (1 << bitOffset)) >> bitOffset;
				tempBuffer[_fTFace->glyph->bitmap.width * j + i] = 255 * bit;
			}
		}

		pixels = tempBuffer;
		stride = _fTFace->glyph->bitmap.width;
	}

	// add glyph to cache
	_glyphCache->AddGlyph(ch, glyphIndex, _fTFace->glyph, _fTFace->glyph->bitmap.width, _fTFace->glyph->bitmap.rows, pixels, stride);

	if (tempBuffer) delete [] tempBuffer;
#endif
}

} // end of namespace WinterMute
