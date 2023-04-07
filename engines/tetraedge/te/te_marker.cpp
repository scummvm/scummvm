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

#include "tetraedge/te/te_marker.h"

namespace Tetraedge {

TeMarker::TeMarker() : _visible(true), _isActive(false) {
}

void TeMarker::active(bool val) {
	_isActive = val;
	_button.setVisible(!_visible && val);
}

void TeMarker::update(TeCamera *camera) {
	if (!_visible)
		return;
	TeVector3f32 transformLoc = camera->transformCoord(_loc);
	if (transformLoc.z() < 0) {
		error("TODO: Finish TeMarker::update");
	} else {
		error("TODO: Finish TeMarker::update");
	}
}

void TeMarker::visible(bool vis) {
	_visible = vis;
	bool buttonVis = (vis && _isActive);
	_button.setVisible(buttonVis);
}

} // end namespace Tetraedge
