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

#include "mtropolis/coroutines.h"
#include "common/debug.h"

namespace MTropolis {

struct MyCoroutine {
	CORO_DEFINE_RETURN_TYPE(void);
	CORO_DEFINE_PARAMS_0();
};

CORO_BEGIN_DEFINITION(MyCoroutine)
	struct Locals {
	};

	CORO_BEGIN_FUNCTION
		debug(1, "Hello");
	CORO_END_FUNCTION
CORO_END_DEFINITION

} // namespace MTropolis
