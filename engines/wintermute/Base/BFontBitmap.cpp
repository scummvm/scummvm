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

#include "engines/wintermute/dcgf.h"
#include "engines/wintermute/Base/BFontBitmap.h"
#include "engines/wintermute/utils/StringUtil.h"
#include "engines/wintermute/Base/BParser.h"
#include "engines/wintermute/Base/BFrame.h"
#include "engines/wintermute/Base/BSurface.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/BSubFrame.h"
#include "engines/wintermute/Base/BFrame.h"
#include "engines/wintermute/Base/BSprite.h"
#include "engines/wintermute/Base/BFileManager.h"
#include "engines/wintermute/PlatformSDL.h"

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
void CBFontBitmap::drawText(byte *text, int x, int y, int width, TTextAlign align, int max_height, int maxLength) {
	textHeightDraw(text, x, y, width, align, true, max_height, maxLength);
}


//////////////////////////////////////////////////////////////////////
int CBFontBitmap::getTextHeight(byte *text, int width) {
	return textHeightDraw(text, 0, 0, width, TAL_LEFT, false);
}


//////////////////////////////////////////////////////////////////////
int CBFontBitmap::getTextWidth(byte *text, int maxLength) {
	AnsiString str;

	if (Game->_textEncoding == TEXT_UTF8) {
		WideString wstr = StringUtil::utf8ToWide(Utf8String((char *)text));
		str = StringUtil::wideToAnsi(wstr);
	} else {
		str = AnsiString((char *)text);
	}

	if (maxLength >= 0 && str.size() > maxLength)
		str = Common::String(str.c_str(), maxLength);
	//str.substr(0, maxLength); // TODO: Remove

	int textWidth = 0;
	for (size_t i = 0; i < str.size(); i++) {
		textWidth += getCharWidth(str[i]);
	}

	return textWidth;
}


//////////////////////////////////////////////////////////////////////
int CBFontBitmap::textHeightDraw(byte *text, int x, int y, int width, TTextAlign align, bool draw, int maxHeight, int maxLength) {
	if (maxLength == 0) return 0;

	if (text == NULL || text[0] == '\0') return _tileHeight;



	AnsiString str;

	if (Game->_textEncoding == TEXT_UTF8) {
		WideString wstr = StringUtil::utf8ToWide(Utf8String((char *)text));
		str = StringUtil::wideToAnsi(wstr);
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

	if (draw) Game->_renderer->startSpriteBatch();

	while (!done) {
		if (maxHeight > 0 && (NumLines + 1)*_tileHeight > maxHeight) {
			if (draw) Game->_renderer->endSpriteBatch();
			return NumLines * _tileHeight;
		}

		index++;

		if (str[index] == ' ' && (maxHeight < 0 || maxHeight / _tileHeight > 1)) {
			end = index - 1;
			RealLength = LineLength;
		}

		if (str[index] == '\n') {
			end = index - 1;
			RealLength = LineLength;
			new_line = true;
		}

		if (LineLength + getCharWidth(str[index]) > width && last_end == end) {
			end = index - 1;
			RealLength = LineLength;
			new_line = true;
			long_line = true;
		}

		if (str.size() == (index + 1) || (maxLength >= 0 && index == maxLength - 1)) {
			done = true;
			if (!new_line) {
				end = index;
				LineLength += getCharWidth(str[index]);
				RealLength = LineLength;
			}
		} else LineLength += getCharWidth(str[index]);

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
			default:
				error("CBFontBitmap::TextHeightDraw - Unhandled enum");
				break;
			}
			for (i = start; i < end + 1; i++) {
				if (draw) drawChar(str[i], StartX, y);
				StartX += getCharWidth(str[i]);
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

	if (draw) Game->_renderer->endSpriteBatch();

	return NumLines * _tileHeight;
}


//////////////////////////////////////////////////////////////////////
void CBFontBitmap::drawChar(byte c, int x, int y) {
	if (_fontextFix) c--;

	int row, col;

	row = c / _numColumns;
	col = c % _numColumns;

	RECT rect;
	/* l t r b */
	int tileWidth;
	if (_wholeCell) tileWidth = _tileWidth;
	else tileWidth = _widths[c];

	CBPlatform::SetRect(&rect, col * _tileWidth, row * _tileHeight, col * _tileWidth + tileWidth, (row + 1)*_tileHeight);
	bool handled = false;
	if (_sprite) {
		_sprite->GetCurrentFrame();
		if (_sprite->_currentFrame >= 0 && _sprite->_currentFrame < _sprite->_frames.GetSize() && _sprite->_frames[_sprite->_currentFrame]) {
			if (_sprite->_frames[_sprite->_currentFrame]->_subframes.GetSize() > 0) {
				_sprite->_frames[_sprite->_currentFrame]->_subframes[0]->_surface->displayTrans(x, y, rect);
			}
			handled = true;
		}
	}
	if (!handled && _subframe) _subframe->_surface->displayTrans(x, y, rect);
}


//////////////////////////////////////////////////////////////////////
HRESULT CBFontBitmap::loadFile(const char *filename) {
	byte *buffer = Game->_fileManager->readWholeFile(filename);
	if (buffer == NULL) {
		Game->LOG(0, "CBFontBitmap::LoadFile failed for file '%s'", filename);
		return E_FAIL;
	}

	HRESULT ret;

	_filename = new char [strlen(filename) + 1];
	strcpy(_filename, filename);

	if (FAILED(ret = loadBuffer(buffer))) Game->LOG(0, "Error parsing FONT file '%s'", filename);

	delete [] buffer;

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
HRESULT CBFontBitmap::loadBuffer(byte *buffer) {
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

	if (parser.GetCommand((char **)&buffer, commands, (char **)&params) != TOKEN_FONT) {
		Game->LOG(0, "'FONT' keyword expected.");
		return E_FAIL;
	}
	buffer = (byte *)params;

	int widths[300];
	int num = 0, default_width = 8;
	int lastWidth = 0;
	int i;
	int r = 255, g = 255, b = 255;
	bool custoTrans = false;
	char *surfaceFile = NULL;
	char *spriteFile = NULL;

	bool autoWidth = false;
	int spaceWidth = 0;
	int expandWidth = 0;

	while ((cmd = parser.GetCommand((char **)&buffer, commands, (char **)&params)) > 0) {

		switch (cmd) {
		case TOKEN_IMAGE:
			surfaceFile = (char *)params;
			break;

		case TOKEN_SPRITE:
			spriteFile = (char *)params;
			break;

		case TOKEN_TRANSPARENT:
			parser.ScanStr(params, "%d,%d,%d", &r, &g, &b);
			custoTrans = true;
			break;

		case TOKEN_WIDTHS:
			parser.ScanStr(params, "%D", widths, &num);
			for (i = 0; lastWidth < NUM_CHARACTERS, num > 0; lastWidth++, num--, i++) {
				_widths[lastWidth] = (byte)widths[i];
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
			parser.ScanStr(params, "%b", &autoWidth);
			break;

		case TOKEN_FONTEXT_FIX:
			parser.ScanStr(params, "%b", &_fontextFix);
			break;

		case TOKEN_PAINT_WHOLE_CELL:
			parser.ScanStr(params, "%b", &_wholeCell);
			break;

		case TOKEN_SPACE_WIDTH:
			parser.ScanStr(params, "%d", &spaceWidth);
			break;

		case TOKEN_EXPAND_WIDTH:
			parser.ScanStr(params, "%d", &expandWidth);
			break;

		case TOKEN_EDITOR_PROPERTY:
			parseEditorProperty((byte *)params, false);
			break;
		}

	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		Game->LOG(0, "Syntax error in FONT definition");
		return E_FAIL;
	}

	if (spriteFile != NULL) {
		delete _sprite;
		_sprite = new CBSprite(Game, this);
		if (!_sprite || FAILED(_sprite->loadFile(spriteFile))) {
			delete _sprite;
			_sprite = NULL;
		}
	}

	if (surfaceFile != NULL && !_sprite) {
		_subframe = new CBSubFrame(Game);
		if (custoTrans) _subframe->setSurface(surfaceFile, false, r, g, b);
		else _subframe->setSurface(surfaceFile);
	}


	if (((_subframe == NULL || _subframe->_surface == NULL) && _sprite == NULL) || _numColumns == 0 || _tileWidth == 0 || _tileHeight == 0) {
		Game->LOG(0, "Incomplete font definition");
		return E_FAIL;
	}

	if (autoWidth) {
		// calculate characters width
		getWidths();

		// do we need to modify widths?
		if (expandWidth != 0) {
			for (i = 0; i < NUM_CHARACTERS; i++) {
				int NewWidth = (int)_widths[i] + expandWidth;
				if (NewWidth < 0) NewWidth = 0;

				_widths[i] = (byte)NewWidth;
			}
		}

		// handle space character
		uint32 spaceChar = ' ';
		if (_fontextFix) spaceChar--;

		if (spaceWidth != 0) _widths[spaceChar] = spaceWidth;
		else {
			if (_widths[spaceChar] == expandWidth || _widths[spaceChar] == 0) {
				_widths[spaceChar] = (_widths['m'] + _widths['i']) / 2;
			}
		}
	} else {
		for (i = lastWidth; i < NUM_CHARACTERS; i++) _widths[i] = default_width;
	}


	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFontBitmap::persist(CBPersistMgr *persistMgr) {

	CBFont::persist(persistMgr);
	persistMgr->transfer(TMEMBER(_numColumns));

	persistMgr->transfer(TMEMBER(_subframe));
	persistMgr->transfer(TMEMBER(_tileHeight));
	persistMgr->transfer(TMEMBER(_tileWidth));
	persistMgr->transfer(TMEMBER(_sprite));
	persistMgr->transfer(TMEMBER(_widthsFrame));

	if (persistMgr->_saving)
		persistMgr->putBytes(_widths, sizeof(_widths));
	else
		persistMgr->getBytes(_widths, sizeof(_widths));


	persistMgr->transfer(TMEMBER(_fontextFix));
	persistMgr->transfer(TMEMBER(_wholeCell));


	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
int CBFontBitmap::getCharWidth(byte index) {
	if (_fontextFix) index--;
	return _widths[index];
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFontBitmap::getWidths() {
	CBSurface *surf = NULL;

	if (_sprite) {
		if (_widthsFrame >= 0 && _widthsFrame < _sprite->_frames.GetSize()) {
			if (_sprite->_frames[_widthsFrame] && _sprite->_frames[_widthsFrame]->_subframes.GetSize() > 0) {
				surf = _sprite->_frames[_widthsFrame]->_subframes[0]->_surface;
			}
		}
	}
	if (surf == NULL && _subframe) surf = _subframe->_surface;
	if (!surf || FAILED(surf->startPixelOp())) return E_FAIL;


	for (int i = 0; i < NUM_CHARACTERS; i++) {
		int xxx = (i % _numColumns) * _tileWidth;
		int yyy = (i / _numColumns) * _tileHeight;


		int minCol = -1;
		for (int row = 0; row < _tileHeight; row++) {
			for (int col = _tileWidth - 1; col >= minCol + 1; col--) {
				if (xxx + col < 0 || xxx + col >= surf->getWidth() || yyy + row < 0 || yyy + row >= surf->getHeight()) continue;
				if (!surf->isTransparentAtLite(xxx + col, yyy + row)) {
					//min_col = col;
					minCol = MAX(col, minCol);
					break;
				}
			}
			if (minCol == _tileWidth - 1) break;
		}

		_widths[i] = minCol + 1;
	}
	surf->endPixelOp();
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
int CBFontBitmap::getLetterHeight() {
	return _tileHeight;
}

} // end of namespace WinterMute
