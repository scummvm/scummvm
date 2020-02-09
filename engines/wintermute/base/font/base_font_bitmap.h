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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
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

#ifndef WINTERMUTE_BASE_FONTBITMAP_H
#define WINTERMUTE_BASE_FONTBITMAP_H


#include "engines/wintermute/base/font/base_font.h"

namespace Wintermute {
class BaseSubFrame;
class BaseFontBitmap : public BaseFont {
public:
	DECLARE_PERSISTENT(BaseFontBitmap, BaseFont)
	bool loadBuffer(char *buffer);
	bool loadFile(const Common::String &filename);
	int getTextWidth(const byte *text, int maxLength = -1) override;
	int getTextHeight(const byte *text, int width) override;
	void drawText(const byte *text, int x, int y, int width, TTextAlign align = TAL_LEFT, int max_height = -1, int maxLength = -1) override;
	int getLetterHeight() override;

	BaseFontBitmap(BaseGame *inGame);
	~BaseFontBitmap() override;

private:
	bool getWidths();
	BaseSprite *_sprite;
	int32 _widthsFrame;
	bool _fontextFix;
	int32 _numColumns;
	int32 _tileHeight;
	int32 _tileWidth;
	byte _widths[NUM_CHARACTERS];
	BaseSubFrame *_subframe;
	bool _wholeCell;

	int getCharWidth(byte index);
	void drawChar(byte c, int x, int y);

	int textHeightDraw(const byte *text, int x, int y, int width, TTextAlign align, bool draw, int max_height = -1, int maxLength = -1);

};

} // End of namespace Wintermute

#endif
