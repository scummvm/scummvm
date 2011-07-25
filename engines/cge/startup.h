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

#ifndef __CGE_STARTUP__
#define __CGE_STARTUP__


#include "cge/general.h"

namespace CGE {

#define GAME_ID         45
#define CDINI_FNAME     46

#define NOT_VGA_TEXT    90
#define BAD_CHIP_TEXT   91
#define BAD_DOS_TEXT    92
#define NO_CORE_TEXT    93
#define BAD_MIPS_TEXT   94
#define NO_MOUSE_TEXT   95
#define BAD_ARG_TEXT    96
#define BADCD_TEXT      97

#define CFG_EXT         ".CFG"

#if defined(DEMO)
#define MINI_EMM_SIZE   0x00004000L
#define CORE_HIG        400
#else
#define MINI_EMM_SIZE   0x00010000L
#define CORE_HIG        450
#endif

#define CORE_MID        (CORE_HIG - 20)


class Startup {
	static bool getParms();
public:
	static int _mode;
	static int _core;
	static int _soundOk;
	static uint16 _summa;
	Startup();
};


const char *usrPath(const char *nam);

} // End of namespace CGE

#endif
