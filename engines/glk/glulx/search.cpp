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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/glulx/glulx.h"

namespace Glk {
namespace Glulx {

enum serop {
	serop_KeyIndirect       = 0x01,
	serop_ZeroKeyTerminates = 0x02,
	serop_ReturnIndex       = 0x04
};

uint Glulx::linear_search(uint key, uint keysize,  uint start, uint structsize, uint numstructs,
                           uint keyoffset, uint options) {
	unsigned char keybuf[4];
	uint count;
	uint ix;
	int retindex = ((options & serop_ReturnIndex) != 0);
	int zeroterm = ((options & serop_ZeroKeyTerminates) != 0);

	fetchkey(keybuf, key, keysize, options);

	for (count = 0; count < numstructs; count++, start += structsize) {
		int match = true;
		if (keysize <= 4) {
			for (ix = 0; match && ix < keysize; ix++) {
				if (Mem1(start + keyoffset + ix) != keybuf[ix])
					match = false;
			}
		} else {
			for (ix = 0; match && ix < keysize; ix++) {
				if (Mem1(start + keyoffset + ix) != Mem1(key + ix))
					match = false;
			}
		}

		if (match) {
			if (retindex)
				return count;
			else
				return start;
		}

		if (zeroterm) {
			match = true;
			for (ix = 0; match && ix < keysize; ix++) {
				if (Mem1(start + keyoffset + ix) != 0)
					match = false;
			}
			if (match) {
				break;
			}
		}
	}

	if (retindex)
		return (uint) - 1;
	else
		return 0;
}


uint Glulx::binary_search(uint key, uint keysize,  uint start, uint structsize, uint numstructs,
                           uint keyoffset, uint options) {
	byte keybuf[4];
	byte byte1, byte2;
	uint top, bot, val, addr;
	uint ix;
	int retindex = ((options & serop_ReturnIndex) != 0);

	fetchkey(keybuf, key, keysize, options);

	bot = 0;
	top = numstructs;
	while (bot < top) {
		int cmp = 0;
		val = (top + bot) / 2;
		addr = start + val * structsize;

		if (keysize <= 4) {
			for (ix = 0; (!cmp) && ix < keysize; ix++) {
				byte1 = Mem1(addr + keyoffset + ix);
				byte2 = keybuf[ix];
				if (byte1 < byte2)
					cmp = -1;
				else if (byte1 > byte2)
					cmp = 1;
			}
		} else {
			for (ix = 0; (!cmp) && ix < keysize; ix++) {
				byte1 = Mem1(addr + keyoffset + ix);
				byte2 = Mem1(key + ix);
				if (byte1 < byte2)
					cmp = -1;
				else if (byte1 > byte2)
					cmp = 1;
			}
		}

		if (!cmp) {
			if (retindex)
				return val;
			else
				return addr;
		}

		if (cmp < 0) {
			bot = val + 1;
		} else {
			top = val;
		}
	}

	if (retindex)
		return (uint) - 1;
	else
		return 0;
}

uint Glulx::linked_search(uint key, uint keysize,  uint start, uint keyoffset, uint nextoffset, uint options) {
	unsigned char keybuf[4];
	uint ix;
	uint val;
	int zeroterm = ((options & serop_ZeroKeyTerminates) != 0);

	fetchkey(keybuf, key, keysize, options);

	while (start != 0) {
		int match = true;
		if (keysize <= 4) {
			for (ix = 0; match && ix < keysize; ix++) {
				if (Mem1(start + keyoffset + ix) != keybuf[ix])
					match = false;
			}
		} else {
			for (ix = 0; match && ix < keysize; ix++) {
				if (Mem1(start + keyoffset + ix) != Mem1(key + ix))
					match = false;
			}
		}

		if (match) {
			return start;
		}

		if (zeroterm) {
			match = true;
			for (ix = 0; match && ix < keysize; ix++) {
				if (Mem1(start + keyoffset + ix) != 0)
					match = false;
			}
			if (match) {
				break;
			}
		}

		val = start + nextoffset;
		start = Mem4(val);
	}

	return 0;
}

void Glulx::fetchkey(unsigned char *keybuf, uint key, uint keysize,  uint options) {
	uint ix;

	if (options & serop_KeyIndirect) {
		if (keysize <= 4) {
			for (ix = 0; ix < keysize; ix++)
				keybuf[ix] = Mem1(key + ix);
		}
	} else {
		switch (keysize) {
		case 4:
			Write4(keybuf, key);
			break;
		case 2:
			Write2(keybuf, key);
			break;
		case 1:
			Write1(keybuf, key);
			break;
		default:
			fatal_error("Direct search key must hold one, two, or four bytes.");
		}
	}
}

} // End of namespace Glulx
} // End of namespace Glk
