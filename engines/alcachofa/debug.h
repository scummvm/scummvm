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
		auto floor = g_engine->player().currentRoom()->activeFloor();
		auto renderer = dynamic_cast<IDebugRenderer *>(&g_engine->renderer());
		if (floor == nullptr || renderer == nullptr)
			return;

		Point target3D;
		if (_polygonI < 0 || (uint)_polygonI >= floor->polygonCount())
			target3D = floor->closestPointTo(mousePos3D);
		else
			target3D = floor->at((uint)_polygonI).closestPointTo(mousePos3D);
		renderer->debugPolyline(mousePos2D, g_engine->camera().transform3Dto2D(target3D));
	}
};

class FloorIntersectionsDebugHandler final : public IDebugHandler {
	int32 _polygonI;
	Point _fromPos3D;
public:
	FloorIntersectionsDebugHandler(int32 polygonI) : _polygonI(polygonI) {}

	virtual void update() final
	{
		auto floor = g_engine->player().currentRoom()->activeFloor();
		auto renderer = dynamic_cast<IDebugRenderer *>(&g_engine->renderer());
		if (floor == nullptr || renderer == nullptr)
			return;

		if (g_engine->input().debugInput().wasMouseLeftPressed())
			_fromPos3D = g_engine->input().debugInput().mousePos3D();
		renderer->debugPolyline(
			g_engine->camera().transform3Dto2D(_fromPos3D),
			g_engine->input().debugInput().mousePos2D(),
			kDebugRed);

		if (_polygonI >= 0 && (uint)_polygonI < floor->polygonCount())
			drawIntersectionsFor(floor->at((uint)_polygonI), renderer);
		else
		{
			for (uint i = 0; i < floor->polygonCount(); i++)
				drawIntersectionsFor(floor->at(i), renderer);
		}
	}

private:
	static constexpr float kMarkerLength = 16;

	void drawIntersectionsFor(const Polygon& polygon, IDebugRenderer* renderer)
	{
		auto &camera = g_engine->camera();
		auto mousePos2D = g_engine->input().debugInput().mousePos2D();
		auto mousePos3D = g_engine->input().debugInput().mousePos3D();
		for (uint i = 0; i < polygon._points.size(); i++)
		{
			if (!polygon.intersectsEdge(i, _fromPos3D, mousePos3D))
				continue;
			auto a = camera.transform3Dto2D(polygon._points[i]);
			auto b = camera.transform3Dto2D(polygon._points[(i + 1) % polygon._points.size()]);
			auto mid = (a + b) / 2;
			auto length = sqrtf(a.sqrDist(b));
			auto normal = a - b;
			normal = { normal.y, (int16)-normal.x};
			auto inner = mid + normal * (kMarkerLength / length);

			renderer->debugPolyline(a, b, kDebugGreen);
			renderer->debugPolyline(mid, inner, kDebugGreen);
		}
	}
};

}

#endif // DEBUG_H
