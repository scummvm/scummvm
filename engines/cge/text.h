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

#ifndef __TEXT__
#define __TEXT__

#include "cge/talk.h"
#include "cge/jbw.h"

namespace CGE {

#ifndef SYSTXT_MAX
#define SYSTXT_MAX      1000
#endif

#define SAY_EXT         ".SAY"

#define NOT_VGA_TEXT    90
#define BAD_CHIP_TEXT   91
#define BAD_DOS_TEXT    92
#define NO_CORE_TEXT    93
#define BAD_MIPS_TEXT   94
#define NO_MOUSE_TEXT   95
#define INF_NAME        101
#define SAY_NAME        102
#define INF_REF         301
#define SAY_REF         302


class TEXT {
	struct HAN {
		int Ref;
		char *Txt;
	} *Cache;
	int Size;
	char FileName[MAXPATH];
	char *Load(int idx, int ref);
	int Find(int ref);
public:
	TEXT(const char *fname, int size);
	~TEXT(void);
	void Clear(int from = 1, int upto = 0x7FFF);
	void Preload(int from = 1, int upto = 0x7FFF);
	char *getText(int ref);
};


extern TALK       *Talk;
extern TEXT       *Text;


void Say(const char *txt, SPRITE *spr);
void SayTime(SPRITE *spr);
void Inf(const char *txt);
void KillText(void);

} // End of namespace CGE

#endif
