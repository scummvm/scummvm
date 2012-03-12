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

#include "dcgf.h"
#include "BFontBitmap.h"
#include "StringUtil.h"
#include "BParser.h"
#include "BFrame.h"
#include "BSurface.h"
#include "BGame.h"
#include "BSubFrame.h"
#include "BFrame.h"
#include "BSprite.h"
#include "BFileManager.h"
#include "PlatformSDL.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_PERSISTENT(CBFontBitmap, false)

//////////////////////////////////////////////////////////////////////
CBFontBitmap::CBFontBitmap(CBGame *inGame): CBFont(inGame) {
	m_Subframe = NULL;
	m_Sprite = NULL;
	m_WidthsFrame = 0;
	memset(m_Widths, 0, NUM_CHARACTERS);
	m_TileWidth = m_TileHeight = m_NumColumns = 0;
	m_FontextFix = false;
	m_Freezable = false;
	m_WholeCell = false;
}


//////////////////////////////////////////////////////////////////////
CBFontBitmap::~CBFontBitmap() {
	delete m_Subframe;
	delete m_Sprite;
	m_Subframe = NULL;
	m_Sprite = NULL;
}


//////////////////////////////////////////////////////////////////////
void CBFontBitmap::DrawText(byte  *text, int x, int y, int width, TTextAlign align, int max_height, int MaxLenght) {
	TextHeightDraw(text, x, y, width, align, true, max_height, MaxLenght);
}


//////////////////////////////////////////////////////////////////////
int CBFontBitmap::GetTextHeight(byte  *text, int width) {
	return TextHeightDraw(text, 0, 0, width, TAL_LEFT, false);
}


//////////////////////////////////////////////////////////////////////
int CBFontBitmap::GetTextWidth(byte  *text, int MaxLength) {
	AnsiString str;

	if (Game->m_TextEncoding == TEXT_UTF8) {
		WideString wstr = StringUtil::Utf8ToWide(Utf8String((char *)text));
		str = StringUtil::WideToAnsi(wstr);
	} else {
		str = AnsiString((char *)text);
	}

	if (MaxLength >= 0 && str.length() > MaxLength) str = str.substr(0, MaxLength);

	int TextWidth = 0;
	for (size_t i = 0; i < str.length(); i++) {
		TextWidth += GetCharWidth(str[i]);
	}

	return TextWidth;
}


//////////////////////////////////////////////////////////////////////
int CBFontBitmap::TextHeightDraw(byte  *text, int x, int y, int width, TTextAlign align, bool draw, int max_height, int MaxLenght) {
	if (MaxLenght == 0) return 0;

	if (text == NULL || text[0] == '\0') return m_TileHeight;



	AnsiString str;

	if (Game->m_TextEncoding == TEXT_UTF8) {
		WideString wstr = StringUtil::Utf8ToWide(Utf8String((char *)text));
		str = StringUtil::WideToAnsi(wstr);
	} else {
		str = AnsiString((char *)text);
	}
	if (str.empty()) return 0;

	int LineLength = 0;
	int RealLength = 0;
	int NumLines = 0;

	int i;

	int index = -1;
	int start = 0;
	int end = 0;
	int last_end = 0;

	bool done = false;
	bool new_line = false;
	bool long_line = false;

	if (draw) Game->m_Renderer->StartSpriteBatch();

	while (!done) {
		if (max_height > 0 && (NumLines + 1)*m_TileHeight > max_height) {
			if (draw) Game->m_Renderer->EndSpriteBatch();
			return NumLines * m_TileHeight;
		}

		index++;

		if (str[index] == ' ' && (max_height < 0 || max_height / m_TileHeight > 1)) {
			end = index - 1;
			RealLength = LineLength;
		}

		if (str[index] == '\n') {
			end = index - 1;
			RealLength = LineLength;
			new_line = true;
		}

		if (LineLength + GetCharWidth(str[index]) > width && last_end == end) {
			end = index - 1;
			RealLength = LineLength;
			new_line = true;
			long_line = true;
		}

		if (str[index + 1] == '\0' || (MaxLenght >= 0 && index == MaxLenght - 1)) {
			done = true;
			if (!new_line) {
				end = index;
				LineLength += GetCharWidth(str[index]);
				RealLength = LineLength;
			}
		} else LineLength += GetCharWidth(str[index]);

		if ((LineLength > width) || done || new_line) {
			if (end < 0) done = true;
			int StartX;
			switch (align) {
			case TAL_CENTER:
				StartX = x + (width - RealLength) / 2;
				break;
			case TAL_RIGHT:
				StartX = x + width - RealLength;
				break;
			case TAL_LEFT:
				StartX = x;
				break;
			}
			for (i = start; i < end + 1; i++) {
				if (draw) DrawChar(str[i], StartX, y);
				StartX += GetCharWidth(str[i]);
			}
			y += m_TileHeight;
			last_end = end;
			if (long_line) end--;
			start = end + 2;
			index = end + 1;
			LineLength = 0;
			new_line = false;
			long_line = false;
			NumLines++;
		}
	}

	if (draw) Game->m_Renderer->EndSpriteBatch();

	return NumLines * m_TileHeight;
}


//////////////////////////////////////////////////////////////////////
void CBFontBitmap::DrawChar(byte  c, int x, int y) {
	if (m_FontextFix) c--;

	int row, col;

	row = c / m_NumColumns;
	col = c % m_NumColumns;

	RECT rect;
	/* l t r b */
	int TileWidth;
	if (m_WholeCell) TileWidth = m_TileWidth;
	else TileWidth = m_Widths[c];

	CBPlatform::SetRect(&rect, col * m_TileWidth, row * m_TileHeight, col * m_TileWidth + TileWidth, (row + 1)*m_TileHeight);
	bool Handled = false;
	if (m_Sprite) {
		m_Sprite->GetCurrentFrame();
		if (m_Sprite->m_CurrentFrame >= 0 && m_Sprite->m_CurrentFrame < m_Sprite->m_Frames.GetSize() && m_Sprite->m_Frames[m_Sprite->m_CurrentFrame]) {
			if (m_Sprite->m_Frames[m_Sprite->m_CurrentFrame]->m_Subframes.GetSize() > 0) {
				m_Sprite->m_Frames[m_Sprite->m_CurrentFrame]->m_Subframes[0]->m_Surface->DisplayTrans(x, y, rect);
			}
			Handled = true;
		}
	}
	if (!Handled && m_Subframe) m_Subframe->m_Surface->DisplayTrans(x, y, rect);
}


//////////////////////////////////////////////////////////////////////
HRESULT CBFontBitmap::LoadFile(char *Filename) {
	byte *Buffer = Game->m_FileManager->ReadWholeFile(Filename);
	if (Buffer == NULL) {
		Game->LOG(0, "CBFontBitmap::LoadFile failed for file '%s'", Filename);
		return E_FAIL;
	}

	HRESULT ret;

	m_Filename = new char [strlen(Filename) + 1];
	strcpy(m_Filename, Filename);

	if (FAILED(ret = LoadBuffer(Buffer))) Game->LOG(0, "Error parsing FONT file '%s'", Filename);

	delete [] Buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(FONTEXT_FIX)
TOKEN_DEF(FONT)
TOKEN_DEF(IMAGE)
TOKEN_DEF(TRANSPARENT)
TOKEN_DEF(COLUMNS)
TOKEN_DEF(TILE_WIDTH)
TOKEN_DEF(TILE_HEIGHT)
TOKEN_DEF(DEFAULT_WIDTH)
TOKEN_DEF(WIDTHS)
TOKEN_DEF(AUTO_WIDTH)
TOKEN_DEF(SPACE_WIDTH)
TOKEN_DEF(EXPAND_WIDTH)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF(SPRITE)
TOKEN_DEF(WIDTHS_FRAME)
TOKEN_DEF(PAINT_WHOLE_CELL)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////
HRESULT CBFontBitmap::LoadBuffer(byte  *Buffer) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(FONTEXT_FIX)
	TOKEN_TABLE(FONT)
	TOKEN_TABLE(IMAGE)
	TOKEN_TABLE(TRANSPARENT)
	TOKEN_TABLE(COLUMNS)
	TOKEN_TABLE(TILE_WIDTH)
	TOKEN_TABLE(TILE_HEIGHT)
	TOKEN_TABLE(DEFAULT_WIDTH)
	TOKEN_TABLE(WIDTHS)
	TOKEN_TABLE(AUTO_WIDTH)
	TOKEN_TABLE(SPACE_WIDTH)
	TOKEN_TABLE(EXPAND_WIDTH)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE(SPRITE)
	TOKEN_TABLE(WIDTHS_FRAME)
	TOKEN_TABLE(PAINT_WHOLE_CELL)
	TOKEN_TABLE_END

	char *params;
	int cmd;
	CBParser parser(Game);

	if (parser.GetCommand((char **)&Buffer, commands, (char **)&params) != TOKEN_FONT) {
		Game->LOG(0, "'FONT' keyword expected.");
		return E_FAIL;
	}
	Buffer = (byte  *)params;

	int widths[300];
	int num = 0, default_width = 8;
	int last_width = 0;
	int i;
	int r = 255, g = 255, b = 255;
	bool custom_trans = false;
	char *surface_file = NULL;
	char *sprite_file = NULL;

	bool AutoWidth = false;
	int SpaceWidth = 0;
	int ExpandWidth = 0;

	while ((cmd = parser.GetCommand((char **)&Buffer, commands, (char **)&params)) > 0) {

		switch (cmd) {
		case TOKEN_IMAGE:
			surface_file = (char *)params;
			break;

		case TOKEN_SPRITE:
			sprite_file = (char *)params;
			break;

		case TOKEN_TRANSPARENT:
			parser.ScanStr(params, "%d,%d,%d", &r, &g, &b);
			custom_trans = true;
			break;

		case TOKEN_WIDTHS:
			parser.ScanStr(params, "%D", widths, &num);
			for (i = 0; last_width < NUM_CHARACTERS, num > 0; last_width++, num--, i++) {
				m_Widths[last_width] = (byte )widths[i];
			}
			break;

		case TOKEN_DEFAULT_WIDTH:
			parser.ScanStr(params, "%d", &default_width);
			break;

		case TOKEN_WIDTHS_FRAME:
			parser.ScanStr(params, "%d", &m_WidthsFrame);
			break;

		case TOKEN_COLUMNS:
			parser.ScanStr(params, "%d", &m_NumColumns);
			break;

		case TOKEN_TILE_WIDTH:
			parser.ScanStr(params, "%d", &m_TileWidth);
			break;

		case TOKEN_TILE_HEIGHT:
			parser.ScanStr(params, "%d", &m_TileHeight);
			break;

		case TOKEN_AUTO_WIDTH:
			parser.ScanStr(params, "%b", &AutoWidth);
			break;

		case TOKEN_FONTEXT_FIX:
			parser.ScanStr(params, "%b", &m_FontextFix);
			break;

		case TOKEN_PAINT_WHOLE_CELL:
			parser.ScanStr(params, "%b", &m_WholeCell);
			break;

		case TOKEN_SPACE_WIDTH:
			parser.ScanStr(params, "%d", &SpaceWidth);
			break;

		case TOKEN_EXPAND_WIDTH:
			parser.ScanStr(params, "%d", &ExpandWidth);
			break;

		case TOKEN_EDITOR_PROPERTY:
			ParseEditorProperty((byte  *)params, false);
			break;
		}

	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		Game->LOG(0, "Syntax error in FONT definition");
		return E_FAIL;
	}

	if (sprite_file != NULL) {
		delete m_Sprite;
		m_Sprite = new CBSprite(Game, this);
		if (!m_Sprite || FAILED(m_Sprite->LoadFile(sprite_file))) SAFE_DELETE(m_Sprite);
	}

	if (surface_file != NULL && !m_Sprite) {
		m_Subframe = new CBSubFrame(Game);
		if (custom_trans) m_Subframe->SetSurface(surface_file, false, r, g, b);
		else m_Subframe->SetSurface(surface_file);
	}


	if (((m_Subframe == NULL || m_Subframe->m_Surface == NULL) && m_Sprite == NULL) || m_NumColumns == 0 || m_TileWidth == 0 || m_TileHeight == 0) {
		Game->LOG(0, "Incomplete font definition");
		return E_FAIL;
	}

	if (AutoWidth) {
		// calculate characters width
		GetWidths();

		// do we need to modify widths?
		if (ExpandWidth != 0) {
			for (i = 0; i < NUM_CHARACTERS; i++) {
				int NewWidth = (int)m_Widths[i] + ExpandWidth;
				if (NewWidth < 0) NewWidth = 0;

				m_Widths[i] = (byte )NewWidth;
			}
		}

		// handle space character
		char SpaceChar = ' ';
		if (m_FontextFix) SpaceChar--;

		if (SpaceWidth != 0) m_Widths[SpaceChar] = SpaceWidth;
		else {
			if (m_Widths[SpaceChar] == ExpandWidth || m_Widths[SpaceChar] == 0) {
				m_Widths[SpaceChar] = (m_Widths['m'] + m_Widths['i']) / 2;
			}
		}
	} else {
		for (i = last_width; i < NUM_CHARACTERS; i++) m_Widths[i] = default_width;
	}


	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFontBitmap::Persist(CBPersistMgr *PersistMgr) {

	CBFont::Persist(PersistMgr);
	PersistMgr->Transfer(TMEMBER(m_NumColumns));

	PersistMgr->Transfer(TMEMBER(m_Subframe));
	PersistMgr->Transfer(TMEMBER(m_TileHeight));
	PersistMgr->Transfer(TMEMBER(m_TileWidth));
	PersistMgr->Transfer(TMEMBER(m_Sprite));
	PersistMgr->Transfer(TMEMBER(m_WidthsFrame));

	if (PersistMgr->m_Saving)
		PersistMgr->PutBytes(m_Widths, sizeof(m_Widths));
	else
		PersistMgr->GetBytes(m_Widths, sizeof(m_Widths));


	PersistMgr->Transfer(TMEMBER(m_FontextFix));
	PersistMgr->Transfer(TMEMBER(m_WholeCell));


	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
int CBFontBitmap::GetCharWidth(byte  Index) {
	if (m_FontextFix) Index--;
	return m_Widths[Index];
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFontBitmap::GetWidths() {
	CBSurface *surf = NULL;

	if (m_Sprite) {
		if (m_WidthsFrame >= 0 && m_WidthsFrame < m_Sprite->m_Frames.GetSize()) {
			if (m_Sprite->m_Frames[m_WidthsFrame] && m_Sprite->m_Frames[m_WidthsFrame]->m_Subframes.GetSize() > 0) {
				surf = m_Sprite->m_Frames[m_WidthsFrame]->m_Subframes[0]->m_Surface;
			}
		}
	}
	if (surf == NULL && m_Subframe) surf = m_Subframe->m_Surface;
	if (!surf || FAILED(surf->StartPixelOp())) return E_FAIL;


	for (int i = 0; i < NUM_CHARACTERS; i++) {
		int xxx = (i % m_NumColumns) * m_TileWidth;
		int yyy = (i / m_NumColumns) * m_TileHeight;


		int min_col = -1;
		for (int row = 0; row < m_TileHeight; row++) {
			for (int col = m_TileWidth - 1; col >= min_col + 1; col--) {
				if (xxx + col < 0 || xxx + col >= surf->GetWidth() || yyy + row < 0 || yyy + row >= surf->GetHeight()) continue;
				if (!surf->IsTransparentAtLite(xxx + col, yyy + row)) {
					//min_col = col;
					min_col = std::max(col, min_col);
					break;
				}
			}
			if (min_col == m_TileWidth - 1) break;
		}

		m_Widths[i] = min_col + 1;
	}
	surf->EndPixelOp();
	/*
	Game->LOG(0, "----- %s ------", m_Filename);
	for(int j=0; j<16; j++)
	{
	Game->LOG(0, "%02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d", m_Widths[j*16+0], m_Widths[j*16+1], m_Widths[j*16+2], m_Widths[j*16+3], m_Widths[j*16+4], m_Widths[j*16+5], m_Widths[j*16+6], m_Widths[j*16+7], m_Widths[j*16+8], m_Widths[j*16+9], m_Widths[j*16+10], m_Widths[j*16+11], m_Widths[j*16+12], m_Widths[j*16+13], m_Widths[j*16+14], m_Widths[j*16+15]);
	}
	*/
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
int CBFontBitmap::GetLetterHeight() {
	return m_TileHeight;
}

} // end of namespace WinterMute
