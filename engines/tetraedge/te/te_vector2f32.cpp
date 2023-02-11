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

#include "tetraedge/te/te_vector2f32.h"
#include "tetraedge/te/te_vector2s32.h"
#include "tetraedge/te/te_vector3f32.h"
#include "tetraedge/tetraedge.h"

namespace Tetraedge {

TeVector2f32::TeVector2f32(const TeVector2s32 &other) {
	setX(other._x);
	setY(other._y);
}

bool TeVector2f32::parse(const Common::String &val) {
	const Common::StringArray parts = TetraedgeEngine::splitString(val, ',');
	if (parts.size() != 2)
		return false;
	setX(atof(parts[0].c_str()));
	setY(atof(parts[1].c_str()));
	return true;
}


} // end namespace Tetraedge
