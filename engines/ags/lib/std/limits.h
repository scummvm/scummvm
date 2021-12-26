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

#include "ags/lib/std/math.h"
#include "ags/shared/core/types.h"

#ifndef AGS_STD_LIMITS_H
#define AGS_STD_LIMITS_H

namespace AGS3 {
namespace std {

class _Num_base {
};

template <class _Ty>
class numeric_limits : public _Num_base {
public:
	static constexpr _Ty(min)() {
		return _Ty();
	}

	static constexpr _Ty(max)() {
		return _Ty();
	}
};

template <>
class numeric_limits<float> {
public:
	static constexpr float quiet_undefined() {
		return FLOAT_UNASSIGNED;
	}
};

template <>
class numeric_limits<uint16_t> {
public:
	static constexpr uint16_t quiet_undefined() {
		return 0;
	}

	static constexpr uint16_t min() {
		return 0;
	}
	static constexpr uint16_t max() {
		return UINT16_MAX;
	}
};

} // namespace std
} // namespace AGS3

#endif
