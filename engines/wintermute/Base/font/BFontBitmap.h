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

#ifndef WINTERMUTE_BFONTBITMAP_H
#define WINTERMUTE_BFONTBITMAP_H


#include "engines/wintermute/Base/font/BFont.h"

namespace WinterMute {
class CBSubFrame;
class CBFontBitmap : public CBFont {
public:
	DECLARE_PERSISTENT(CBFontBitmap, CBFont)
	bool loadBuffer(byte *Buffer);
	bool loadFile(const char *filename);
	virtual int getTextWidth(byte *text, int maxLength = -1);
	virtual int getTextHeight(byte *text, int width);
	virtual void drawText(byte *text, int x, int y, int width, TTextAlign align = TAL_LEFT, int max_height = -1, int maxLength = -1);
	virtual int getLetterHeight();

	CBFontBitmap(CBGame *inGame);
	virtual ~CBFontBitmap();


	bool getWidths();
	CBSprite *_sprite;
	int _widthsFrame;
	bool _fontextFix;
	int _numColumns;
	int _tileHeight;
	int _tileWidth;
	byte _widths[NUM_CHARACTERS];
	CBSubFrame *_subframe;
	bool _wholeCell;

private:
	int getCharWidth(byte index);
	void drawChar(byte  c, int x, int y);

	int textHeightDraw(byte *text, int x, int y, int width, TTextAlign align, bool draw, int max_height = -1, int MaxLength = -1);

};

} // end of namespace WinterMute

#endif
