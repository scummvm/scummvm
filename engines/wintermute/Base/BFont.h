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

#ifndef WINTERMUTE_BFONT_H
#define WINTERMUTE_BFONT_H

#include "engines/wintermute/Base/BObject.h"

#define NUM_CHARACTERS 256

namespace WinterMute {

class CBFont: public CBObject {
public:
	DECLARE_PERSISTENT(CBFont, CBObject)
	virtual int getTextWidth(byte  *text, int MaxLenght = -1);
	virtual int getTextHeight(byte  *text, int width);
	virtual void drawText(byte  *text, int x, int y, int width, TTextAlign align = TAL_LEFT, int max_height = -1, int MaxLenght = -1);
	virtual int getLetterHeight();

	virtual void initLoop() {};
	virtual void afterLoad() {};
	CBFont(CBGame *inGame);
	virtual ~CBFont();

	static CBFont *createFromFile(CBGame *Game, const char *filename);

private:
	//HRESULT loadBuffer(byte * Buffer);
	//HRESULT loadFile(const char* Filename);
	static bool isTrueType(CBGame *Game, const char *filename);
};

} // end of namespace WinterMute

#endif
