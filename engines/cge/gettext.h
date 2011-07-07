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

#ifndef __CGE_GETTEXT__
#define __CGE_GETTEXT__

#include "cge/general.h"
#include "cge/talk.h"

namespace CGE {

#define GTMAX       24
#define GTBLINK      6
#define GTTIME       6

class GetText : public Talk {
	char   _buff[GTMAX + 2];
	char  *_text;
	uint16 _size;
	uint16 _len;
	uint16 _cntr;
	Sprite *_oldKeybClient;

public:
	static GetText *_ptr;
	GetText(CGEEngine *vm, const char *info, char *text, int size);
	~GetText();
	void touch(uint16 mask, int x, int y);
	void tick();

private:
	CGEEngine *_vm;
};

} // End of namespace CGE

#endif
