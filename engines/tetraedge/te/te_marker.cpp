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
	const TeVector3f32 transformLoc = camera->transformCoord(_loc);
	const TeVector3f32 btnSize = _button.size();
	if (transformLoc.z() < 0) {
		_button.setPosition(TeVector3f32(btnSize.x(), btnSize.y(), _someFloat));
	} else {
		TeVector3f32 transformLoc2(transformLoc.x() + btnSize.x() / 2, transformLoc.y() + btnSize.y() / 2, _someFloat);
		// TODO: device rotation (maybe?) is taken account of here
		// in original, should we do that?
		TeVector3f32 newScale(480.0f / camera->getViewportWidth(), 320.0f / camera->getViewportHeight(), 1.0);
		_button.setScale(newScale);
		_button.setPosition(TeVector3f32(newScale.x() * transformLoc2.x(), newScale.y() * transformLoc2.y(), newScale.z()));
	}
}

void TeMarker::visible(bool vis) {
	_visible = vis;
	bool buttonVis = (vis && _isActive);
	_button.setVisible(buttonVis);
}

} // end namespace Tetraedge
