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

#include "tetraedge/te/te_warp.h"

namespace Tetraedge {

TeWarp::TeWarp() {
}

void TeWarp::update() {
	error("TODO: Implement TeWarp::update");
}

void TeWarp::setMouseLeftUpForMakers() {
	// TODO:
	//for (auto &marker : _warpMarkers) {
	//	marker->marker()->sprite()->setEnable(true)
	//}
	error("TODO: Implement TeWarp::setMouseLeftUpForMakers");
}

void TeWarp::rotateCamera(const TeQuaternion &rot) {
	TeQuaternion normRot = rot;
	normRot.normalize();
	_camera.setRotation(normRot);
}

void TeWarp::setFov(float fov) {
	_camera.setFov(fov);
}

} // end namespace Tetraedge
