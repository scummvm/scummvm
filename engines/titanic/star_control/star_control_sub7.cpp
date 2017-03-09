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

#include "titanic/star_control/star_control_sub7.h"

namespace Titanic {

void CStarControlSub7::draw(CSurfaceArea *surfaceArea, CStarControlSub12 *sub12, CStarControlSub5 *sub5) {
	// TODO
}

bool CStarControlSub7::addStar(const CBaseStarEntry *entry) {
	// iterate through the existing stars
	for (uint idx = 0; idx < _data.size(); ++idx) {
		CBaseStarEntry &star = _data[idx];
		if (star._position == entry->_position) {
			// Found a matching star at the exact same position, so remove it instead
			_data.remove_at(idx);
			return true;
		}
	}

	// No existing match
	if (_data.size() == 32)
		// Out of space, so delete oldest star
		_data.remove_at(0);

	// Add new star
	_data.push_back(*entry);
	return true;
}

} // End of namespace Titanic
