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

void Processor::z_add() {
	store((zword)((short)zargs[0] + (short)zargs[1]));
}

void Processor::z_and() {
	store((zword)(zargs[0] & zargs[1]));
}

void Processor::z_art_shift() {
	if ((short)zargs[1] > 0)
		store((zword)((short)zargs[0] << (short)zargs[1]));
	else
		store((zword)((short)zargs[0] >> - (short)zargs[1]));
}

void Processor::z_div() {
	if (zargs[1] == 0)
		runtimeError(ERR_DIV_ZERO);

	store((zword)((short)zargs[0] / (short)zargs[1]));
}

void Processor::z_je() {
	branch(
		zargc > 1 && (zargs[0] == zargs[1] || (
		zargc > 2 && (zargs[0] == zargs[2] || (
		zargc > 3 && (zargs[0] == zargs[3])))))
	);
}

void Processor::z_jg() {
	branch((short)zargs[0] > (short)zargs[1]);
}

void Processor::z_jl() {
	branch((short)zargs[0] < (short)zargs[1]);
}

void Processor::z_jz() {
	branch((short)zargs[0] == 0);
}

void Processor::z_log_shift() {
	if ((short)zargs[1] > 0)
		store((zword)(zargs[0] << (short)zargs[1]));
	else
		store((zword)(zargs[0] >> - (short)zargs[1]));
}

void Processor::z_mod() {
	if (zargs[1] == 0)
		runtimeError(ERR_DIV_ZERO);

	store((zword)((short)zargs[0] % (short)zargs[1]));
}

void Processor::z_mul() {
	store((zword)((short)zargs[0] * (short)zargs[1]));
}

void Processor::z_not() {
	store((zword)~zargs[0]);
}

void Processor::z_or() {
	store((zword)(zargs[0] | zargs[1]));
}

void Processor::z_sub() {
	store((zword)((short)zargs[0] - (short)zargs[1]));
}

void Processor::z_test() {
	branch((zargs[0] & zargs[1]) == zargs[1]);
}

} // End of namespace ZCode
} // End of namespace Glk
