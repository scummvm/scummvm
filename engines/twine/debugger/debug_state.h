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

#ifndef TWINE_DEBUG_SCENE_H
#define TWINE_DEBUG_SCENE_H

#include "common/rect.h"
#include "common/scummsys.h"
#include "twine/shared.h"
#include <cstdarg>

namespace TwinE {

class TwinEEngine;

class DebugState {
private:
	TwinEEngine *_engine;

	void projectBoundingBoxPoints(IVec3 *point3d, IVec3 *point3dProjected);
	bool checkZoneType(ZoneType type) const;
	bool displayZones();
	bool displayActors();
	bool displayTracks();

	struct ScenePositionsProjected {
		IVec3 frontBottomLeftPoint;
		IVec3 frontBottomRightPoint;

		IVec3 frontTopLeftPoint;
		IVec3 frontTopRightPoint;

		IVec3 backBottomLeftPoint;
		IVec3 backBottomRightPoint;

		IVec3 backTopLeftPoint;
		IVec3 backTopRightPoint;

		IVec3 frontBottomLeftPoint2D;
		IVec3 frontBottomRightPoint2D;

		IVec3 frontTopLeftPoint2D;
		IVec3 frontTopRightPoint2D;

		IVec3 backBottomLeftPoint2D;
		IVec3 backBottomRightPoint2D;

		IVec3 backTopLeftPoint2D;
		IVec3 backTopRightPoint2D;
	};

	ScenePositionsProjected calculateBoxPositions(const IVec3 &mins, const IVec3 &maxs);
	bool drawBox(const ScenePositionsProjected &positions, uint8 color);

	/** Change scenario camera positions */
	void changeGridCamera();

public:
	DebugState(TwinEEngine *engine);
	bool _showingZones = false;
	bool _showingActors = false;
	bool _showingTracks = false;
	bool _showingClips = false;
	bool _godMode = false;
	unsigned int _typeZones = 127; // all zones on as default
	int16 _onlyLoadActor = -1;
	const char *_openPopup = nullptr;
	bool _holomapFlagsWindow = false;
	bool _gameFlagsWindow = false;
	bool _menuTextWindow = false;
	bool _sceneDetailsWindow = false;
	bool _actorDetailsWindow = true;
	bool _sceneFlagsWindow = false;

	bool _useFreeCamera = false;
	bool _disableGridRendering = false;
	TextBankId _textBankId = TextBankId::Inventory_Intro_and_Holomap;

	void renderDebugView();
	void drawClip(const Common::Rect &rect);

	void update();
};

} // namespace TwinE

#endif
