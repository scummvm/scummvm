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

#include "glk/zcode/processor.h"

namespace Glk {
namespace ZCode {

void Processor::z_copy_table() {
	zword addr;
	zword size = zargs[2];
	zbyte value;
	int i;

	if (zargs[1] == 0) {
		// zero table
		for (i = 0; i < size; i++)
			storeb((zword)(zargs[0] + i), 0);
	} else if ((short) size < 0 || zargs[0] > zargs[1])	{
		// copy forwards
		for (i = 0; i < (((short)size < 0) ? -(short)size : size); i++) {
			addr = zargs[0] + i;
			LOW_BYTE(addr, value);
			storeb((zword)(zargs[1] + i), value);
		}
	} else {
		// copy backwards
		for (i = size - 1; i >= 0; i--) {
			addr = zargs[0] + i;
			LOW_BYTE(addr, value);
			storeb((zword)(zargs[1] + i), value);
		}
	}
}

void Processor::z_loadb() {
	zword addr = zargs[0] + zargs[1];
	zbyte value;

	LOW_BYTE(addr, value);

	store(value);
}

void Processor::z_loadw() {
	zword addr = zargs[0] + 2 * zargs[1];
	zword value;

	LOW_WORD(addr, value);

	store(value);
}

void Processor::z_scan_table() {
	zword addr = zargs[1];
	int i;

	// Supply default arguments
	if (zargc < 4)
	zargs[3] = 0x82;

	// Scan byte or word array
	for (i = 0; i < zargs[2]; i++) {
		if (zargs[3] & 0x80) {
			// scan word array
			zword wvalue;

			LOW_WORD(addr, wvalue);

			if (wvalue == zargs[0])
				goto finished;
		} else {
			// scan byte array
			zbyte bvalue;

			LOW_BYTE(addr, bvalue);

			if (bvalue == zargs[0])
				goto finished;
		}

		addr += zargs[3] & 0x7f;
	}

	addr = 0;

finished:
	store(addr);
	branch(addr);
}

void Processor::z_storeb() {
	storeb((zword)(zargs[0] + zargs[1]), zargs[2]);
}

void Processor::z_storew() {
	storew((zword)(zargs[0] + 2 * zargs[1]), zargs[2]);
}

} // End of namespace ZCode
} // End of namespace Glk
