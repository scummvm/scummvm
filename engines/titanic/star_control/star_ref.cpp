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

#include "titanic/star_control/star_ref.h"

namespace Titanic {

void CBaseStarRef::process(CSurfaceArea *surface, CStarControlSub12 *sub12) {
	// TODO
}

/*------------------------------------------------------------------------*/

bool CStarRef1::check(const Common::Point &pt, int index) {
	Common::Rect r(pt.x - 2, pt.y - 2, pt.x + 2, pt.y + 2);
	if (r.contains(_position)) {
		_index = index;
		return false;
	} else {
		return true;
	}
}

/*------------------------------------------------------------------------*/

bool CStarRef2::check(const Common::Point &pt, int index) {
	if (_index >= (int)_positions->size())
		return false;

	CStarPosition &sp = (*_positions)[index];
	sp._position = pt;
	sp._index1 = sp._index2 = index;
	return true;
}

/*------------------------------------------------------------------------*/

bool CStarRef3::check(const Common::Point &pt, int index) {
	++_index;
	return true;
}

} // End of namespace Titanic
