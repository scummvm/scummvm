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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ZVISION_RENDER_MANAGER_H
#define ZVISION_RENDER_MANAGER_H

#include "common/types.h"

#include "zvision/dense_2d_array.h"

class OSystem;

namespace Common {
class String;
class Point;
}

namespace ZVision {

class RenderManager {
public:
	RenderManager(OSystem *system, const int width, const int height);

public:
	enum RenderState {
		PANORAMA,
		TILT,
		FLAT
	};

private:
	OSystem *_system;
	const int _width;
	const int _height;
	RenderState _renderState;

	struct {
		uint16 fieldOfView;
		uint16 linearScale;
	} _panoramaOptions;

	// TODO: See if tilt and panorama need to have separate options
	struct {
		uint16 fieldOfView;
		uint16 linearScale;
	} _tiltOptions;

	Dense2DArray<Common::Point> _renderTable;

	bool _needsScreenUpdate;

public:
	void renderImageToScreen(const Common::String &fileName, uint32 x, uint32 y);
	void generatePanoramaLookupTable();
};

} // End of namespace ZVision

#endif
