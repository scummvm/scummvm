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

#ifndef DEBUG_H
#define DEBUG_H

#include "alcachofa.h"

using namespace Common;

namespace Alcachofa {

class IDebugHandler {
public:
	virtual ~IDebugHandler() = default;

	virtual void update() = 0;
};

class ClosestFloorPointDebugHandler final : public IDebugHandler {
	int32 _polygonI;
public:
	ClosestFloorPointDebugHandler(int32 polygonI) : _polygonI(polygonI) {}

	virtual void update() final
	{
		auto mousePos2D = g_engine->input().debugInput().mousePos2D();
		auto mousePos3D = g_engine->input().debugInput().mousePos3D();
		auto* floor = g_engine->player().currentRoom()->activeFloor();
		Point target3D;

		if (_polygonI < 0 || (uint)_polygonI >= floor->polygonCount())
			target3D = floor->getClosestPoint(mousePos3D);
		else
			target3D = floor->at((uint)_polygonI)._points[0];
		auto target2Dv = g_engine->camera().transform3Dto2D(
			{ (float)target3D.x, (float)target3D.y, kBaseScale });

		auto renderer = dynamic_cast<IDebugRenderer *>(&g_engine->renderer());
		renderer->debugPolyline(mousePos2D, { (int16)target2Dv.x(), (int16)target2Dv.y() });
	}
};

}

#endif // DEBUG_H
