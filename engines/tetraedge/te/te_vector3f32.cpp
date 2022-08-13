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

#include "tetraedge/tetraedge.h"
#include "common/str-array.h"
#include "tetraedge/te/te_vector3f32.h"

namespace Tetraedge {

bool TeVector3f32::parse(const Common::String &val) {
	const Common::StringArray parts = TetraedgeEngine::splitString(val, ',');
	if (parts.size() != 3)
		return false;
	x() = atof(parts[0].c_str());
	y() = atof(parts[1].c_str());
	z() = atof(parts[2].c_str());
	return true;
}


} // end namespace Tetraedge
