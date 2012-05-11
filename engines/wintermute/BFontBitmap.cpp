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
	_subframe = NULL;
	_sprite = NULL;
	_widthsFrame = 0;
	memset(_widths, 0, NUM_CHARACTERS);
	_tileWidth = _tileHeight = _numColumns = 0;
	_fontextFix = false;
	_freezable = false;
	_wholeCell = false;
}


//////////////////////////////////////////////////////////////////////
CBFontBitmap::~CBFontBitmap() {
	delete _subframe;
	delete _sprite;
	_subframe = NULL;
	_sprite = NULL;
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

	if (Game->_textEncoding == TEXT_UTF8) {
		WideString wstr = StringUtil::Utf8ToWide(Utf8String((char *)text));
		str = StringUtil::WideToAnsi(wstr);
	} else {
		str = AnsiString((char *)text);
	}

	if (MaxLength >= 0 && str.size() > MaxLength) 
		str = Common::String(str.c_str(), MaxLength);
		//str.substr(0, MaxLength); // TODO: Remove

	int TextWidth = 0;
	for (size_t i = 0; i < str.size(); i++) {
		TextWidth += GetCharWidth(str[i]);
	}

	return TextWidth;
}


//////////////////////////////////////////////////////////////////////
int CBFontBitmap::TextHeightDraw(byte  *text, int x, int y, int width, TTextAlign align, bool draw, int max_height, int MaxLenght) {
	if (MaxLenght == 0) return 0;

	if (text == NULL || text[0] == '\0') return _tileHeight;



	AnsiString str;

	if (Game->_textEncoding == TEXT_UTF8) {
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

	if (draw) Game->_renderer->StartSpriteBatch();

	while (!done) {
		if (max_height > 0 && (NumLines + 1)*_tileHeight > max_height) {
			if (draw) Game->_renderer->EndSpriteBatch();
			return NumLines * _tileHeight;
		}

		index++;

		if (str[index] == ' ' && (max_height < 0 || max_height / _tileHeight > 1)) {
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

		if (str.size() == (index + 1) || (MaxLenght >= 0 && index == MaxLenght - 1)) {
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
			y += _tileHeight;
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

	if (draw) Game->_renderer->EndSpriteBatch();

	return NumLines * _tileHeight;
}


//////////////////////////////////////////////////////////////////////
void CBFontBitmap::DrawChar(byte  c, int x, int y) {
	if (_fontextFix) c--;

	int row, col;

	row = c / _numColumns;
	col = c % _numColumns;

	RECT rect;
	/* l t r b */
	int TileWidth;
	if (_wholeCell) TileWidth = _tileWidth;
	else TileWidth = _widths[c];

	CBPlatform::SetRect(&rect, col * _tileWidth, row * _tileHeight, col * _tileWidth + TileWidth, (row + 1)*_tileHeight);
	bool Handled = false;
	if (_sprite) {
		_sprite->GetCurrentFrame();
		if (_sprite->_currentFrame >= 0 && _sprite->_currentFrame < _sprite->_frames.GetSize() && _sprite->_frames[_sprite->_currentFrame]) {
			if (_sprite->_frames[_sprite->_currentFrame]->_subframes.GetSize() > 0) {
				_sprite->_frames[_sprite->_currentFrame]->_subframes[0]->_surface->DisplayTrans(x, y, rect);
			}
			Handled = true;
		}
	}
	if (!Handled && _subframe) _subframe->_surface->DisplayTrans(x, y, rect);
}


//////////////////////////////////////////////////////////////////////
HRESULT CBFontBitmap::LoadFile(const char *Filename) {
	byte *Buffer = Game->_fileManager->ReadWholeFile(Filename);
	if (Buffer == NULL) {
		Game->LOG(0, "CBFontBitmap::LoadFile failed for file '%s'", Filename);
		return E_FAIL;
	}

	HRESULT ret;

	_filename = new char [strlen(Filename) + 1];
	strcpy(_filename, Filename);

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
	bool custo_trans = false;
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
			custo_trans = true;
			break;

		case TOKEN_WIDTHS:
			parser.ScanStr(params, "%D", widths, &num);
			for (i = 0; last_width < NUM_CHARACTERS, num > 0; last_width++, num--, i++) {
				_widths[last_width] = (byte )widths[i];
			}
			break;

		case TOKEN_DEFAULT_WIDTH:
			parser.ScanStr(params, "%d", &default_width);
			break;

		case TOKEN_WIDTHS_FRAME:
			parser.ScanStr(params, "%d", &_widthsFrame);
			break;

		case TOKEN_COLUMNS:
			parser.ScanStr(params, "%d", &_numColumns);
			break;

		case TOKEN_TILE_WIDTH:
			parser.ScanStr(params, "%d", &_tileWidth);
			break;

		case TOKEN_TILE_HEIGHT:
			parser.ScanStr(params, "%d", &_tileHeight);
			break;

		case TOKEN_AUTO_WIDTH:
			parser.ScanStr(params, "%b", &AutoWidth);
			break;

		case TOKEN_FONTEXT_FIX:
			parser.ScanStr(params, "%b", &_fontextFix);
			break;

		case TOKEN_PAINT_WHOLE_CELL:
			parser.ScanStr(params, "%b", &_wholeCell);
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
		delete _sprite;
		_sprite = new CBSprite(Game, this);
		if (!_sprite || FAILED(_sprite->LoadFile(sprite_file))) SAFE_DELETE(_sprite);
	}

	if (surface_file != NULL && !_sprite) {
		_subframe = new CBSubFrame(Game);
		if (custo_trans) _subframe->SetSurface(surface_file, false, r, g, b);
		else _subframe->SetSurface(surface_file);
	}


	if (((_subframe == NULL || _subframe->_surface == NULL) && _sprite == NULL) || _numColumns == 0 || _tileWidth == 0 || _tileHeight == 0) {
		Game->LOG(0, "Incomplete font definition");
		return E_FAIL;
	}

	if (AutoWidth) {
		// calculate characters width
		GetWidths();

		// do we need to modify widths?
		if (ExpandWidth != 0) {
			for (i = 0; i < NUM_CHARACTERS; i++) {
				int NewWidth = (int)_widths[i] + ExpandWidth;
				if (NewWidth < 0) NewWidth = 0;

				_widths[i] = (byte )NewWidth;
			}
		}

		// handle space character
		char SpaceChar = ' ';
		if (_fontextFix) SpaceChar--;

		if (SpaceWidth != 0) _widths[SpaceChar] = SpaceWidth;
		else {
			if (_widths[SpaceChar] == ExpandWidth || _widths[SpaceChar] == 0) {
				_widths[SpaceChar] = (_widths['m'] + _widths['i']) / 2;
			}
		}
	} else {
		for (i = last_width; i < NUM_CHARACTERS; i++) _widths[i] = default_width;
	}


	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFontBitmap::Persist(CBPersistMgr *PersistMgr) {

	CBFont::Persist(PersistMgr);
	PersistMgr->Transfer(TMEMBER(_numColumns));

	PersistMgr->Transfer(TMEMBER(_subframe));
	PersistMgr->Transfer(TMEMBER(_tileHeight));
	PersistMgr->Transfer(TMEMBER(_tileWidth));
	PersistMgr->Transfer(TMEMBER(_sprite));
	PersistMgr->Transfer(TMEMBER(_widthsFrame));

	if (PersistMgr->_saving)
		PersistMgr->PutBytes(_widths, sizeof(_widths));
	else
		PersistMgr->GetBytes(_widths, sizeof(_widths));


	PersistMgr->Transfer(TMEMBER(_fontextFix));
	PersistMgr->Transfer(TMEMBER(_wholeCell));


	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
int CBFontBitmap::GetCharWidth(byte  Index) {
	if (_fontextFix) Index--;
	return _widths[Index];
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFontBitmap::GetWidths() {
	CBSurface *surf = NULL;

	if (_sprite) {
		if (_widthsFrame >= 0 && _widthsFrame < _sprite->_frames.GetSize()) {
			if (_sprite->_frames[_widthsFrame] && _sprite->_frames[_widthsFrame]->_subframes.GetSize() > 0) {
				surf = _sprite->_frames[_widthsFrame]->_subframes[0]->_surface;
			}
		}
	}
	if (surf == NULL && _subframe) surf = _subframe->_surface;
	if (!surf || FAILED(surf->StartPixelOp())) return E_FAIL;


	for (int i = 0; i < NUM_CHARACTERS; i++) {
		int xxx = (i % _numColumns) * _tileWidth;
		int yyy = (i / _numColumns) * _tileHeight;


		int min_col = -1;
		for (int row = 0; row < _tileHeight; row++) {
			for (int col = _tileWidth - 1; col >= min_col + 1; col--) {
				if (xxx + col < 0 || xxx + col >= surf->GetWidth() || yyy + row < 0 || yyy + row >= surf->GetHeight()) continue;
				if (!surf->IsTransparentAtLite(xxx + col, yyy + row)) {
					//min_col = col;
					min_col = std::max(col, min_col);
					break;
				}
			}
			if (min_col == _tileWidth - 1) break;
		}

		_widths[i] = min_col + 1;
	}
	surf->EndPixelOp();
	/*
	Game->LOG(0, "----- %s ------", _filename);
	for(int j=0; j<16; j++)
	{
	Game->LOG(0, "%02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d %02d", _widths[j*16+0], _widths[j*16+1], _widths[j*16+2], _widths[j*16+3], _widths[j*16+4], _widths[j*16+5], _widths[j*16+6], _widths[j*16+7], _widths[j*16+8], _widths[j*16+9], _widths[j*16+10], _widths[j*16+11], _widths[j*16+12], _widths[j*16+13], _widths[j*16+14], _widths[j*16+15]);
	}
	*/
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
int CBFontBitmap::GetLetterHeight() {
	return _tileHeight;
}

} // end of namespace WinterMute
