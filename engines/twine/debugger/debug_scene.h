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

#ifndef TWINE_DEBUG_SCENE_H
#define TWINE_DEBUG_SCENE_H

#include "common/rect.h"
#include "common/scummsys.h"
#include "twine/shared.h"

namespace TwinE {

class TwinEEngine;

class DebugScene {
private:
	TwinEEngine *_engine;

	void drawBoundingBoxProjectPoints(Vec3 *point3d, Vec3 *point3dProjected);
	int32 checkZoneType(int32 type) const;
	bool displayZones();
	bool displayActors();
	bool displayTracks();

	struct ScenePositionsProjected {
		Vec3 frontBottomLeftPoint;
		Vec3 frontBottomRightPoint;

		Vec3 frontTopLeftPoint;
		Vec3 frontTopRightPoint;

		Vec3 backBottomLeftPoint;
		Vec3 backBottomRightPoint;

		Vec3 backTopLeftPoint;
		Vec3 backTopRightPoint;

		Vec3 frontBottomLeftPoint2D;
		Vec3 frontBottomRightPoint2D;

		Vec3 frontTopLeftPoint2D;
		Vec3 frontTopRightPoint2D;

		Vec3 backBottomLeftPoint2D;
		Vec3 backBottomRightPoint2D;

		Vec3 backTopLeftPoint2D;
		Vec3 backTopRightPoint2D;
	};

	ScenePositionsProjected calculateBoxPositions(const Vec3 &bottomLeft, const Vec3 &topRight);
	bool drawBox(const ScenePositionsProjected &positions, uint8 color);
public:
	DebugScene(TwinEEngine *engine);
	bool showingZones = false;
	bool showingActors = false;
	bool showingTracks = false;
	bool showingClips = false;
	bool useScenePatches = false;
	int32 typeZones = 127; // all zones on as default
	int16 onlyLoadActor = -1;

	void renderDebugView();

	void drawClip(const Common::Rect &rect);
};

} // namespace TwinE

#endif
