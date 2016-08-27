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

#include "common/algorithm.h"
#include "titanic/star_control/star_control_sub4.h"

namespace Titanic {

CStarControlSub4::CStarControlSub4() {
}

void CStarControlSub4::initialize() {
	_min._v1 = _min._v2 = _min._v3 = 9.9999994e27;
	_max._v1 = _max._v2 = _max._v3 = -9.9999994e27;
}

void CStarControlSub4::checkEntry(const CBaseStarVal &val) {
	_min._v1 = MIN(_min._v1, val._v1);
	_min._v2 = MIN(_min._v2, val._v2);
	_min._v3 = MIN(_min._v3, val._v3);
	_max._v1 = MAX(_max._v1, val._v1);
	_max._v2 = MAX(_max._v2, val._v2);
	_max._v3 = MAX(_max._v3, val._v3);
}

} // End of namespace Titanic
