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

#ifndef ALCACHOFA_DEBUG_H
#define ALCACHOFA_DEBUG_H

#include "alcachofa/alcachofa.h"

using namespace Common;

namespace Alcachofa {

class IDebugHandler {
public:
	virtual ~IDebugHandler() {}

	virtual void update() = 0;
};

class ClosestFloorPointDebugHandler final : public IDebugHandler {
	int32 _polygonI;
public:
	ClosestFloorPointDebugHandler(int32 polygonI) : _polygonI(polygonI) {}

	void update() override {
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

	void update() override {
		auto floor = g_engine->player().currentRoom()->activeFloor();
		auto renderer = dynamic_cast<IDebugRenderer *>(&g_engine->renderer());
		if (floor == nullptr || renderer == nullptr) {
			g_engine->console().attach("Either the room has no floor or the renderer is not a debug renderer");
			g_engine->setDebugMode(DebugMode::None, 0);
			return;
		}

		if (g_engine->input().debugInput().wasMouseLeftPressed())
			_fromPos3D = g_engine->input().debugInput().mousePos3D();
		renderer->debugPolyline(
			g_engine->camera().transform3Dto2D(_fromPos3D),
			g_engine->input().debugInput().mousePos2D(),
			kDebugRed);

		if (_polygonI >= 0 && (uint)_polygonI < floor->polygonCount())
			drawIntersectionsFor(floor->at((uint)_polygonI), renderer);
		else {
			for (uint i = 0; i < floor->polygonCount(); i++)
				drawIntersectionsFor(floor->at(i), renderer);
		}
	}

private:
	static constexpr float kMarkerLength = 16;

	void drawIntersectionsFor(const Polygon &polygon, IDebugRenderer *renderer) {
		auto &camera = g_engine->camera();
		auto mousePos3D = g_engine->input().debugInput().mousePos3D();
		for (uint i = 0; i < polygon._points.size(); i++) {
			if (!polygon.intersectsEdge(i, _fromPos3D, mousePos3D))
				continue;
			auto a = camera.transform3Dto2D(polygon._points[i]);
			auto b = camera.transform3Dto2D(polygon._points[(i + 1) % polygon._points.size()]);
			auto mid = (a + b) / 2;
			auto length = sqrtf(a.sqrDist(b));
			auto normal = a - b;
			normal = { normal.y, (int16)-normal.x };
			auto inner = mid + normal * (kMarkerLength / length);

			renderer->debugPolyline(a, b, kDebugGreen);
			renderer->debugPolyline(mid, inner, kDebugGreen);
		}
	}
};

class TeleportCharacterDebugHandler final : public IDebugHandler {
	MainCharacterKind _kind;
public:
	TeleportCharacterDebugHandler(int32 kindI) : _kind((MainCharacterKind)kindI) {}

	void update() override {
		g_engine->drawQueue().clear();
		g_engine->player().drawCursor(true);
		g_engine->drawQueue().draw();

		auto &input = g_engine->input().debugInput();
		if (input.wasMouseRightPressed()) {
			g_engine->setDebugMode(DebugMode::None, 0);
			return;
		}

		if (!input.wasMouseLeftPressed())
			return;
		auto floor = g_engine->player().currentRoom()->activeFloor();
		if (floor == nullptr || !floor->contains(input.mousePos3D()))
			return;

		if (_kind == MainCharacterKind::Filemon)
			teleport(g_engine->world().filemon(), input.mousePos3D());
		else if (_kind == MainCharacterKind::Mortadelo)
			teleport(g_engine->world().mortadelo(), input.mousePos3D());
		else {
			teleport(g_engine->world().filemon(), input.mousePos3D());
			teleport(g_engine->world().mortadelo(), input.mousePos3D());
		}
		g_engine->setDebugMode(DebugMode::None, 0);
	}

private:
	void teleport(MainCharacter &character, Point position) {
		auto currentRoom = g_engine->player().currentRoom();
		if (character.room() != currentRoom) {
			character.resetTalking();
			character.room() = currentRoom;
		}
		character.setPosition(position);
	}
};

class FloorColorDebugHandler final : public IDebugHandler {
	const FloorColorShape &_shape;
	const bool _useColor;
	Color _curColor = kDebugGreen;
	bool _isOnFloor = false;
	static constexpr size_t kBufferSize = 64;
	char _buffer[kBufferSize];

	FloorColorDebugHandler(const FloorColorShape &shape, bool useColor)
		: _shape(shape)
		, _useColor(useColor) {}
public:
	static FloorColorDebugHandler *create(int32 objectI, bool useColor) {
		const Room *room = g_engine->player().currentRoom();
		uint floorCount = 0;
		for (auto itObject = room->beginObjects(); itObject != room->endObjects(); ++itObject) {
			FloorColor *floor = dynamic_cast<FloorColor *>(*itObject);
			if (floor == nullptr)
				continue;
			if (objectI <= 0)
				// dynamic_cast is not possible due to Shape not having virtual methods
				return new FloorColorDebugHandler(*(FloorColorShape *)(floor->shape()), useColor);
			floorCount++;
			objectI--;
		}

		g_engine->console().debugPrintf("Invalid floor color index, there are %u floors in this room\n", floorCount);
		return nullptr;
	}

	void update() override {
		auto &input = g_engine->input().debugInput();
		if (input.wasMouseRightPressed()) {
			g_engine->setDebugMode(DebugMode::None, 0);
			return;
		}

		if (input.isMouseLeftDown()) {
			auto optColor = _shape.colorAt(input.mousePos3D());
			_isOnFloor = optColor.first;
			if (!_isOnFloor) {
				uint8 roomAlpha = (uint)(g_engine->player().currentRoom()->characterAlphaTint() * 255 / 100);
				optColor.second = Color { 255, 255, 255, roomAlpha };
			}

			_curColor = _useColor
				? Color { optColor.second.r, optColor.second.g, optColor.second.b, 255 }
			: Color { optColor.second.a, optColor.second.a, optColor.second.a, 255 };
			g_engine->world().mortadelo().color() =
				g_engine->world().filemon().color() =
				_useColor ? optColor.second : Color { 255, 255, 255, optColor.second.a };
		}

		snprintf(_buffer, kBufferSize, "r:%3d g:%3d b:%3d a:%3d",
			(int)_curColor.r, (int)_curColor.g, (int)_curColor.b, (int)_curColor.a);

		auto *debugRenderer = dynamic_cast<IDebugRenderer *>(&g_engine->renderer());
		g_engine->drawQueue().clear();
		g_engine->player().drawCursor(true);
		g_engine->renderer().setTexture(nullptr);
		g_engine->renderer().quad({ 0, 0 }, { 50, 50 }, _isOnFloor ? _curColor : kDebugGreen);
		g_engine->drawQueue().add<TextDrawRequest>(
			g_engine->globalUI().dialogFont(), _buffer, Point { 70, 20 }, 500, false, kWhite, -kForegroundOrderCount + 1);
		if (!_isOnFloor)
			g_engine->renderer().quad({ 5, 5 }, { 40, 40 }, _curColor);
		if (debugRenderer != nullptr)
			debugRenderer->debugShape(_shape, kDebugBlue);
		g_engine->drawQueue().draw();
	}
};

}

#endif // ALCACHOFA_DEBUG_H
