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

/* CELER	The unit for updating the screen pics. */

#ifndef CELER2_H
#define CELER2_H

#include "common/scummsys.h"
#include "common/file.h"
#include "common/str.h"

namespace Avalanche {
class AvalancheEngine;

class Celer {
public:
	enum flavourtype {ch_ega, ch_bgi, ch_natural_image};

	struct chunkblocktype {
		flavourtype flavour;
		int16 x, y;
		int16 xl, yl;
		int32 size;
		bool natural;

		bool memorise; /* Hold it in memory? */
	};

	struct memotype {
		int16 x, y;
		int16 xl, yl;
		flavourtype flavour;
		uint16 size;
	};

	int32 offsets[40];
	byte num_chunks;
	memotype memos[40];
	byte *memory[40];
	bytefield r;

	

	Celer(AvalancheEngine *vm);

	void pics_link();

	void load_chunks(Common::String xx);

	void forget_chunks();

	void show_one(byte which);

	void show_one_at(byte which, int16 xxx, int16 yyy);

private:
	AvalancheEngine *_vm;

	Common::String filename;

	Common::File f;

	static const int16 on_disk; /* Value of memos[fv].x when it's not in memory. */

	void mdrop(int16 x, int16 y, int16 xl, int16 yl, void *p);

	void display_it(int16 x, int16 y, int16 xl, int16 yl, flavourtype flavour, byte *p);

	void display_it_at(int16 xl, int16 yl, flavourtype flavour, void *p, int16 &xxx, int16 &yyy);
};

} // End of namespace Avalanche.

#endif // CELER2_H
