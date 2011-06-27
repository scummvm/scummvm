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

#ifndef __GETTEXT__
#define __GETTEXT__

#include "cge/general.h"
#include "cge/talk.h"

namespace CGE {

#define GTMAX       24
#define GTBLINK      6
#define GTTIME       6

class GET_TEXT : public TALK {
	char Buff[GTMAX + 2], * Text;
	uint16 Size, Len;
	uint16 Cntr;
	SPRITE *OldKeybClient;
	void (*Click)();
public:
	static GET_TEXT *Ptr;
	GET_TEXT(CGEEngine *vm, const char *info, char *text, int size, void (*click)(void) = NULL);
	~GET_TEXT();
	void Touch(uint16 mask, int x, int y);
	void Tick();

private:
	CGEEngine *_vm;
};

} // End of namespace CGE

#endif
