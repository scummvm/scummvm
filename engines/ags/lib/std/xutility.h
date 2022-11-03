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

#ifndef AGS_STD_XUTILITY_H
#define AGS_STD_XUTILITY_H

#include "common/algorithm.h"
#include "common/util.h"

namespace AGS3 {
namespace std {

template <class T>
void reverse(T *First, T *Last) {
	for (--Last; First < Last; ++First, --Last) {
		SWAP(*First, *Last);
	}
}

} // namespace std
} // namespace AGS3

#endif
