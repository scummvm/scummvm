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

#include "titanic/star_control/star_control_sub2.h"
#include "titanic/star_control/star_camera.h"
#include "titanic/titanic.h"

namespace Titanic {

bool CStarControlSub2::setup() {
	loadData("STARFIELD/132");
	return true;
}

bool CStarControlSub2::loadYale(int v1) {
	clear();
	error("Original loadYale not supported");
	return true;
}

bool CStarControlSub2::selectStar(CSurfaceArea *surfaceArea,
		CStarCamera *camera, const Common::Point &pt, void *handler) {
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

bool CStarControlSub2::loadStar() {
	error("loadStar not supported");
}

} // End of namespace Titanic
