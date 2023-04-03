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

#include "tetraedge/te/te_warp_marker.h"

namespace Tetraedge {

TeWarpMarker::TeWarpMarker() : _marker(nullptr) {
}

TeWarpMarker::~TeWarpMarker() {
	if (_marker)
		_marker->button().onMouseClickValidated().remove(this, &TeWarpMarker::onMarkerButtonValidated);
}

void TeWarpMarker::marker(TeMarker *marker) {
	if (_marker)
		_marker->button().onMouseClickValidated().remove(this, &TeWarpMarker::onMarkerButtonValidated);
	_marker = marker;
	if (_marker)
		_marker->button().onMouseClickValidated().add(this, &TeWarpMarker::onMarkerButtonValidated);
}

bool TeWarpMarker::onMarkerButtonValidated() {
	_markerButtonSignal.call(_name);
	return false;
}

} // end namespace Tetraedge
