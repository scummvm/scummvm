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

#ifndef SCOOBY_MAKE_UNIQUE_H
#define SCOOBY_MAKE_UNIQUE_H

#include "common/ptr.h"
#include "common/util.h"

// std::make_unique is C++14; this project targets C++11. Herb Sutter's
// well-known polyfill, scoped to our namespace so it can't collide with a
// standard library that already provides one under a newer -std flag.

namespace Scooby {
template<typename T, typename... Args>
Common::ScopedPtr<T> makeUnique(Args &&...args) {
	return Common::ScopedPtr<T>(new T(Common::forward<Args>(args)...));
}
} // namespace Scooby

#endif // SCOOBY_MAKE_UNIQUE_H
