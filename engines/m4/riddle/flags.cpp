/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "m4/riddle/flags.h"
#include "m4/riddle/vars.h"
#include "m4/core/term.h"
#include "m4/adv_r/adv.h"
#include "m4/adv_r/adv_inv.h"

namespace M4 {
namespace Riddle {

int32 Flags::_flags[FLAGS_COUNT];

Flags::Flags() {
	reset();
}

void Flags::sync(Common::Serializer &s) {
	size_t count = size();
	for (uint i = 0; i < count; ++i)
		s.syncAsSint32LE(_flags[i]);
}

void Flags::reset() {
	Common::fill(_flags, _flags + FLAGS_COUNT, 0);
	_flags[V071] = 2;
	_flags[V088] = 1;
	_flags[V086] = 2;
	_flags[V014] = 1;
	_flags[V270] = 824;
	_flags[V282] = 1;
	_flags[V292] = 1;
}

} // namespace Riddle
} // namespace M4
