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

#ifndef SCOOBY_CONTRACTS_H
#define SCOOBY_CONTRACTS_H

#include "common/error.h"

// This project is built with -fno-exceptions, so the C# source's
// "throw new InvalidOperationException(...)" in unreachable/invariant-
// violation branches (an exhaustive switch's default case, a corrupt-state
// check) becomes a fail-fast abort instead. These macros are intentionally
// NOT compiled out in release builds (unlike assert()/NDEBUG): an
// unreachable branch that is actually reached means the port has a bug,
// and silently continuing would corrupt game state.

#define SDM_UNREACHABLE(message)                                                                 \
	do {                                                                                         \
		error("Unreachable state at %s:%d: %s\n", __FILE__, __LINE__, (message)); \
	} while (false)

#define SDM_ASSERT(condition, message)                                                              \
	do {                                                                                            \
		if (!(condition)) {                                                                         \
			error("Assertion failed at %s:%d: %s\n", __FILE__, __LINE__, (message)); \
		}                                                                                           \
	} while (false)

#endif // SCOOBY_CONTRACTS_H
