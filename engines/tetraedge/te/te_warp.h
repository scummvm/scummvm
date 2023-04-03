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

#ifndef TETRAEDGE_TE_TE_WARP_H
#define TETRAEDGE_TE_TE_WARP_H

#include "tetraedge/te/te_3d_object2.h"
#include "tetraedge/te/te_camera.h"
#include "tetraedge/te/te_quaternion.h"
#include "tetraedge/te/te_warp_marker.h"

namespace Tetraedge {

// Note: Only used in Amerzone
class TeWarp : Te3DObject2 {
public:
	class AnimData {
	};

	TeWarp();

	void update();
	void rotateCamera(const TeQuaternion &rot);
	void setMarkersOpacity(float opacity);
	void setMouseLeftUpForMakers();
	void setFov(float fov);

private:
	Common::String _warpPath;
	TeCamera _camera;
	bool _markersActive;

	Common::Array<TeWarpMarker *> _warpMarkers;

};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_WARP_H
