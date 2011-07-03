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

#define TEXT_FG     DARK    // foreground color
#define TEXT_BG     GRAY    // background color
#define TEXT_HM     (6&~1)  // EVEN horizontal margins!
#define TEXT_VM     5   // vertical margins
#define TEXT_LS     2   // line spacing
#define TEXT_RD     3   // rounded corners

#define FONT_HIG    8
#define FONT_EXT    ".CFT"



#define MAXPATH  128

class Font {
	char _path[MAXPATH];
	void load();
public:
//  static uint8 _wid[256];
//  static uint16 _pos[256];
//  static uint8 _map[256*8];
	uint8  *_wid;
	uint16 *_pos;
	uint8  *_map;
	Font(const char *name);
	~Font();
	uint16 width(const char *text);
	void save();
};


enum    TBOX_STYLE  { PURE, RECT, ROUND };


class Talk : public Sprite {
protected:
	TBOX_STYLE _mode;
	Bitmap *_ts[2];
	Bitmap *box(uint16 w, uint16 h);
public:
	Talk(CGEEngine *vm, const char *tx, TBOX_STYLE mode);
	Talk(CGEEngine *vm);
	//~TALK();

	static Font *_font;
	static void init();
	static void deinit();

	virtual void update(const char *tx);
	virtual void update() {}
	void putLine(int line, const char *text);
private:
	CGEEngine *_vm;
};


class InfoLine : public Talk {
	const char *_oldTxt;
public:
	InfoLine(CGEEngine *vm, uint16 wid);
	void update(const char *tx);
private:
	CGEEngine *_vm;
};

} // End of namespace CGE

#endif
