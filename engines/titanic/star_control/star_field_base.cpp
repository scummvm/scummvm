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

#include "titanic/star_control/star_field_base.h"
#include "titanic/debugger.h"
#include "titanic/star_control/camera.h"
#include "titanic/support/simple_file.h"

namespace Titanic {

bool CStarFieldBase::setup() {
	loadData("STARFIELD/132");
	return true;
}

bool CStarFieldBase::loadYale(int v1) {
	clear();
	error("Original loadYale not supported");
	return true;
}

bool CStarFieldBase::selectStar(CSurfaceArea *surfaceArea,
		CCamera *camera, const Common::Point &pt, void *handler) {
		int index = findStar(surfaceArea, camera, pt);
	if (index == -1) {
		return false;
	} else if (!handler) {
		debugC(DEBUG_BASIC, kDebugStarfield, "Select star %d", index);
		camera->setDestination(_data[index]._position);
		return true;
	} else {
		error("no handler ever passed in original");
	}
}

bool CStarFieldBase::loadStar() {
	error("loadStar not supported");
}

} // End of namespace Titanic
