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

#include "titanic/star_control/frange.h"
#include "common/algorithm.h"

namespace Titanic {

FRange::FRange() {
}

void FRange::reset() {
	_min._x = _min._y = _min._z = 9.9999994e27F;
	_max._x = _max._y = _max._z = -9.9999994e27F;
}

void FRange::expand(const FVector &v) {
	_min._x = MIN(_min._x, v._x);
	_min._y = MIN(_min._y, v._y);
	_min._z = MIN(_min._z, v._z);
	_max._x = MAX(_max._x, v._x);
	_max._y = MAX(_max._y, v._y);
	_max._z = MAX(_max._z, v._z);
}

} // End of namespace Titanic
