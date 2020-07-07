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

void Processor::z_dec() {
	zword value;

	if (zargs[0] == 0)
		(*_sp)--;
	else if (zargs[0] < 16)
		(*(_fp - zargs[0]))--;
	else {
		zword addr = h_globals + 2 * (zargs[0] - 16);
		LOW_WORD(addr, value);
		value--;
		SET_WORD(addr, value);
	}
}

void Processor::z_dec_chk() {
	zword value;

	if (zargs[0] == 0)
		value = --(*_sp);
	else if (zargs[0] < 16)
		value = --(*(_fp - zargs[0]));
	else {
		zword addr = h_globals + 2 * (zargs[0] - 16);
		LOW_WORD(addr, value);
		value--;
		SET_WORD(addr, value);
	}

	branch((short)value < (short)zargs[1]);
}

void Processor::z_inc() {
	zword value;

	if (zargs[0] == 0)
		(*_sp)++;
	else if (zargs[0] < 16)
		(*(_fp - zargs[0]))++;
	else {
		zword addr = h_globals + 2 * (zargs[0] - 16);
		LOW_WORD(addr, value);
		value++;
		SET_WORD(addr, value);
	}
}

void Processor::z_inc_chk() {
	zword value;

	if (zargs[0] == 0)
		value = ++(*_sp);
	else if (zargs[0] < 16)
		value = ++(*(_fp - zargs[0]));
	else {
		zword addr = h_globals + 2 * (zargs[0] - 16);
		LOW_WORD(addr, value);
		value++;
		SET_WORD(addr, value);
	}

	branch((short)value > (short)zargs[1]);
}

void Processor::z_load() {
	zword value;

	if (zargs[0] == 0)
		value = *_sp;
	else if (zargs[0] < 16)
		value = *(_fp - zargs[0]);
	else {
		zword addr = h_globals + 2 * (zargs[0] - 16);
		LOW_WORD(addr, value);
	}

	store(value);
}

void Processor::z_pop() {
	_sp++;
}

void Processor::z_pop_stack() {
	if (zargc == 2) {
		// it's a user stack
		zword size;
		zword addr = zargs[1];

		LOW_WORD(addr, size);

		size += zargs[0];
		storew(addr, size);
	} else {
		// it's the game stack
		_sp += zargs[0];
	}
}

void Processor::z_pull() {
	zword value;

	if (h_version != V6) {
		// not a V6 game, pop stack and write
		value = *_sp++;

		if (zargs[0] == 0)
			*_sp = value;
		else if (zargs[0] < 16)
			*(_fp - zargs[0]) = value;
		else {
			zword addr = h_globals + 2 * (zargs[0] - 16);
			SET_WORD(addr, value);
		}
	} else {
		// it's V6, but is there a user stack?
		if (zargc == 1) {
			// it's a user stack
			zword size;
			zword addr = zargs[0];

			LOW_WORD(addr, size);

			size++;
			storew(addr, size);

			addr += 2 * size;
			LOW_WORD(addr, value);
		} else {
			// it's the game stack
			value = *_sp++;
		}

		store(value);
	}
}

void Processor::z_push() {
	*--_sp = zargs[0];
}

void Processor::z_push_stack() {
	zword size;
	zword addr = zargs[1];

	LOW_WORD(addr, size);

	if (size != 0) {
		storew((zword)(addr + 2 * size), zargs[0]);

		size--;
		storew(addr, size);
	}

	branch(size);
}

void Processor::z_store() {
	zword value = zargs[1];

	if (zargs[0] == 0)
		*_sp = value;
	else if (zargs[0] < 16)
		*(_fp - zargs[0]) = value;
	else {
		zword addr = h_globals + 2 * (zargs[0] - 16);
		SET_WORD(addr, value);
	}
}

} // End of namespace ZCode
} // End of namespace Glk
