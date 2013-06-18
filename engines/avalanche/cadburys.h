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

#ifndef __cadburys_h__
#define __cadburys_h__

namespace Avalanche {

const integer arraysize = 12000;

const array<1, 44, char> chunkheader =
    string("Chunk-type AVD file, for an Avvy game.") + '\32' + '\261' + '\x30' + '\x1' + '\x75' + '\261';


enum flavourtype {ch_ega, ch_bgi, last_flavourtype};

struct chunkblocktype {
	flavourtype flavour;
	integer x, y;
	integer xl, yl;
	longint size;
	boolean natural;

	boolean memorise; /* Hold it in memory? */
};


#ifdef __cadburys_implementation__
#undef EXTERN
#define EXTERN
#endif

EXTERN array<0, arraysize, byte> aa;
EXTERN byte num_chunks, this_chunk;
EXTERN integer gd, gm;
EXTERN array<1, 30, longint> offsets;
EXTERN untyped_file f;
#undef EXTERN
#define EXTERN extern


void grab(integer x1, integer y1, integer x2, integer y2, integer realx, integer realy, flavourtype flav,
          boolean mem, boolean nat);

} // End of namespace Avalanche.

#endif
