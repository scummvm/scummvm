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
 * This code is based on original Sfinx source code
 * Copyright (c) 1994-1997 Janusz B. Wisniewski and L.K. Avalon
 */

#ifndef CGE2_TALK_H
#define CGE2_TALK_H

#include "cge2/general.h"
#include "cge2/vga13h.h"

namespace CGE2 {

#define kTextHMargin      (6&~1)                    // EVEN horizontal margins!
#define kTextVMargin       5                        // vertical margins
#define kTextLineSpace     2                        // line spacing
#define kTextRoundCorner   3                        // rounded corners
#define kWidSize           256
#define kPosSize           256
#define kMapSize          (256*8)
#define kFontHigh          8
#define kCaptionSide       24
#define kInfName           101
#define kSayName           102

class Font {
	void load();
	CGE2Engine *_vm;
public:
	uint8  *_widthArr;
	uint16 *_pos;
	uint8  *_map;
	uint8  _colorSet[kColorNum][4];
	Font(CGE2Engine *vm);
	~Font();
	uint16 width(const char *text);
};

enum TextBoxStyle { kTBPure, kTBRect, kTBRound };

class Talk : public Sprite {
protected:
	TextBoxStyle _mode;
	bool _created;
	uint8 *box(V2D siz);
	bool _wideSpace;
public:
	uint8 *_color;

	Talk(CGE2Engine *vm, const char *text, TextBoxStyle mode = kTBPure, ColorBank color = kCBStd, bool wideSpace = false);
	Talk(CGE2Engine *vm, ColorBank color = kCBStd);

	void update(const char *text);
private:
	CGE2Engine *_vm;
};

class InfoLine : public Talk {
	const char *_oldText, *_newText;
public:
	bool _realTime;
	InfoLine(CGE2Engine *vm, uint16 wid, ColorBank color = kCBStd);
	void update(const char *text);
	void update() { update(_newText); }
	void setText(const char *txt) { _newText = txt; }
private:
	CGE2Engine *_vm;
};

} // End of namespace CGE2

#endif // CGE2_TALK_H
