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

#ifndef TETRAEDGE_TE_TE_WARP_MARKER_H
#define TETRAEDGE_TE_TE_WARP_MARKER_H

#include "common/str.h"
#include "tetraedge/te/te_marker.h"
#include "tetraedge/te/te_signal.h"

namespace Tetraedge {

// Note: Only used in Amerzone
class TeWarpMarker {
public:
	TeWarpMarker();
	~TeWarpMarker();

	TeMarker *marker() { return _marker; }
	void marker(TeMarker *marker);
	bool onMarkerButtonValidated();
	TeSignal1Param<const Common::String &> &markerButtonSignal() { return _markerButtonSignal; };
	void setName(const Common::String newName) { _name = newName; }


private:
	TeMarker *_marker;
	Common::String _name;
	TeSignal1Param<const Common::String &> _markerButtonSignal;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_WARP_MARKER_H
