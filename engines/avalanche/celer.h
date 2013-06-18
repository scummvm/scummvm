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
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

#ifndef __celer_h__
#define __celer_h__


#include "closing.h"
#include "incline.h"
#include "gyro.h"

namespace Avalanche {

enum flavourtype {ch_ega, ch_bgi, last_flavourtype};

struct chunkblocktype {
	flavourtype flavour;
	integer x, y;
	integer xl, yl;
	longint size;
	boolean natural;

	boolean memorise; /* Hold it in memory? */
};

struct memotype {
	integer x, y;
	integer xl, yl;
	flavourtype flavour;
	word size;
};


#ifdef __celer_implementation__
#undef EXTERN
#define EXTERN
#endif

EXTERN array<1, 40, longint> offsets;
EXTERN byte num_chunks;
EXTERN array<1, 40, memotype> memos;
EXTERN array<1, 40, pointer> memory;
#undef EXTERN
#define EXTERN extern


void pics_link();

void load_chunks(string xx);

void forget_chunks();

void show_one(byte which);

void show_one_at(byte which, integer xxx, integer yyy);

} // End of namespace Avalanche.

#endif
