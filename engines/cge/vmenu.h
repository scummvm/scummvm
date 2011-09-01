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

#ifndef __VMENU__
#define __VMENU__

#include "cge/talk.h"

namespace CGE {

#define MB_VM       1
#define MB_HM       3


typedef struct  {
	char *Text;
	void (* Proc)(void);
} CHOICE;


class MENU_BAR : public TALK {
public:
	MENU_BAR(uint16 w);
};


class VMENU : public TALK {
	uint16 Items;
	CHOICE *Menu;
public:
	static VMENU *Addr;
	static int Recent;
	MENU_BAR *Bar;
	VMENU(CHOICE *list, int x, int y);
	~VMENU(void);
	void Touch(uint16 mask, int x, int y);
};

} // End of namespace CGE

#endif
