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
#include "tetraedge/game/application.h"
#include "tetraedge/te/te_warp.h"
#include "tetraedge/te/te_input_mgr.h"

namespace Tetraedge {

/*static*/
bool TeWarp::debug = false;

TeWarp::TeWarp() : _visible1(false) {
}

void TeWarp::activeMarkers(bool active) {
	_markersActive = active;
	for (auto &warpMarker : _warpMarkers)
		warpMarker->marker()->active(active);
}

void TeWarp::init() {
	// This mostly sets up the camera.. maybe nothing to do?
	warning("TODO: Implement TeWarp::init");
}

bool TeWarp::onMouseLeftDown(const Common::Point &pt) {
	error("TODO: Implement TeWarp::onMouseLeftDown");
}

void TeWarp::update() {
	if (!_visible1 || !_file.isOpen())
		return;
	Application *app = g_engine->getApplication();
	_frustum.update(app->mainWindowCamera());
	error("TODO: Implement TeWarp::update");
}

void TeWarp::setMouseLeftUpForMakers() {
	// TODO:
	//for (auto &marker : _warpMarkers) {
	//	marker->marker()->sprite()->setEnable(true)
	//}
	error("TODO: Implement TeWarp::setMouseLeftUpForMakers");
}

void TeWarp::setVisible(bool v1, bool v2) {
	if (_visible1 == v1)
		return;

	_visible1 = v1;
	TeInputMgr *inputMgr = g_engine->getInputMgr();
	if (v1) {
		inputMgr->_mouseLDownSignal.add(this, &TeWarp::onMouseLeftDown);
	} else {
		if (v2) {
			error("TODO: Implement TeWarp::setVisible for v2==true");
		}
		inputMgr->_mouseLDownSignal.remove(this, &TeWarp::onMouseLeftDown);
	}
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
