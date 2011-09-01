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
 * This code is based on original Soltys source code
 * Copyright (c) 1994-1995 Janus B. Wisniewski and L.K. Avalon
 */

#ifndef __CGE_TALK__
#define __CGE_TALK__

#include "cge/vga13h.h"
#include "cge/general.h"
#include "cge/jbw.h"

namespace CGE {

#define kTextColFG         DARK                     // foreground color
#define kTextColBG         GRAY                     // background color
#define kTextHMargin      (6&~1)                    // EVEN horizontal margins!
#define kTextVMargin       5                        // vertical margins
#define kTextLineSpace     2                        // line spacing
#define kTextRoundCorner   3                        // rounded corners
#define kWidSize           256
#define kPosSize           256
#define kMapSize          (256*8)

#define kFontHigh   8
#define kFontExt    ".CFT"
#define kPathMax           128

enum TextBoxStyle { kTBPure, kTBRect, kTBRound };

class Font {
	char _path[kPathMax];
	void load();
public:
	uint8  *_wid;
	uint16 *_pos;
	uint8  *_map;
	Font(const char *name);
	~Font();
	uint16 width(const char *text);
	void save();
};

class Talk : public Sprite {
protected:
	TextBoxStyle _mode;
	BitmapPtr *_ts;
	Bitmap *box(uint16 w, uint16 h);
public:
	Talk(CGEEngine *vm, const char *text, TextBoxStyle mode);
	Talk(CGEEngine *vm);
	//~TALK();

	static Font *_font;
	static void init();
	static void deinit();

	virtual void update(const char *text);
	virtual void update() {}
	void putLine(int line, const char *text);
private:
	CGEEngine *_vm;
};

class InfoLine : public Talk {
	const char *_oldText;
public:
	InfoLine(CGEEngine *vm, uint16 wid);
	void update(const char *text);
private:
	CGEEngine *_vm;
};

} // End of namespace CGE

#endif
