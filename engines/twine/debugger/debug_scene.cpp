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

#include "twine/debugger/debug_scene.h"
#include "twine/menu/interface.h"
#include "twine/menu/menu.h"
#include "twine/renderer/redraw.h"
#include "twine/renderer/renderer.h"
#include "twine/scene/grid.h"
#include "twine/scene/scene.h"
#include "twine/text.h"
#include "twine/twine.h"

namespace TwinE {

DebugScene::DebugScene(TwinEEngine *engine) : _engine(engine) {}

void DebugScene::drawClip(const Common::Rect &rect) {
	if (!_showingClips) {
		return;
	}
	_engine->_menu->drawRectBorders(rect);
}

void DebugScene::projectBoundingBoxPoints(IVec3 *pPoint3d, IVec3 *pPoint3dProjected) {
	*pPoint3dProjected = _engine->_renderer->projectPositionOnScreen(*pPoint3d);
}

bool DebugScene::checkZoneType(ZoneType type) const {
	switch (type) {
	case ZoneType::kCube:
		return (_typeZones & 0x01) != 0;
	case ZoneType::kCamera:
		return (_typeZones & 0x02) != 0;
	case ZoneType::kSceneric:
		return (_typeZones & 0x04) != 0;
	case ZoneType::kGrid:
		return (_typeZones & 0x08) != 0;
	case ZoneType::kObject:
		return (_typeZones & 0x10) != 0;
	case ZoneType::kText:
		return (_typeZones & 0x20) != 0;
	case ZoneType::kLadder:
		return (_typeZones & 0x40) != 0;
	default:
		return true;
	}

	return false;
}

DebugScene::ScenePositionsProjected DebugScene::calculateBoxPositions(const IVec3 &mins, const IVec3 &maxs) {
	ScenePositionsProjected positions;
	// compute the points in 3D
	positions.frontBottomLeftPoint.x = mins.x - _engine->_grid->_camera.x;
	positions.frontBottomLeftPoint.y = mins.y - _engine->_grid->_camera.y;
	positions.frontBottomLeftPoint.z = maxs.z - _engine->_grid->_camera.z;

	positions.frontBottomRightPoint.x = maxs.x - _engine->_grid->_camera.x;
	positions.frontBottomRightPoint.y = mins.y - _engine->_grid->_camera.y;
	positions.frontBottomRightPoint.z = maxs.z - _engine->_grid->_camera.z;

	positions.frontTopLeftPoint.x = mins.x - _engine->_grid->_camera.x;
	positions.frontTopLeftPoint.y = maxs.y - _engine->_grid->_camera.y;
	positions.frontTopLeftPoint.z = maxs.z - _engine->_grid->_camera.z;

	positions.frontTopRightPoint = maxs - _engine->_grid->_camera;
	positions.backBottomLeftPoint = mins - _engine->_grid->_camera;

	positions.backBottomRightPoint.x = maxs.x - _engine->_grid->_camera.x;
	positions.backBottomRightPoint.y = mins.y - _engine->_grid->_camera.y;
	positions.backBottomRightPoint.z = mins.z - _engine->_grid->_camera.z;

	positions.backTopLeftPoint.x = mins.x - _engine->_grid->_camera.x;
	positions.backTopLeftPoint.y = maxs.y - _engine->_grid->_camera.y;
	positions.backTopLeftPoint.z = mins.z - _engine->_grid->_camera.z;

	positions.backTopRightPoint.x = maxs.x - _engine->_grid->_camera.x;
	positions.backTopRightPoint.y = maxs.y - _engine->_grid->_camera.y;
	positions.backTopRightPoint.z = mins.z - _engine->_grid->_camera.z;

	// project all points

	projectBoundingBoxPoints(&positions.frontBottomLeftPoint, &positions.frontBottomLeftPoint2D);
	projectBoundingBoxPoints(&positions.frontBottomRightPoint, &positions.frontBottomRightPoint2D);
	projectBoundingBoxPoints(&positions.frontTopLeftPoint, &positions.frontTopLeftPoint2D);
	projectBoundingBoxPoints(&positions.frontTopRightPoint, &positions.frontTopRightPoint2D);
	projectBoundingBoxPoints(&positions.backBottomLeftPoint, &positions.backBottomLeftPoint2D);
	projectBoundingBoxPoints(&positions.backBottomRightPoint, &positions.backBottomRightPoint2D);
	projectBoundingBoxPoints(&positions.backTopLeftPoint, &positions.backTopLeftPoint2D);
	projectBoundingBoxPoints(&positions.backTopRightPoint, &positions.backTopRightPoint2D);

	return positions;
}

bool DebugScene::drawBox(const ScenePositionsProjected &positions, uint8 color) {
	bool state = false;
	// draw front part
	state |= _engine->_interface->drawLine(positions.frontBottomLeftPoint2D.x, positions.frontBottomLeftPoint2D.y, positions.frontTopLeftPoint2D.x, positions.frontTopLeftPoint2D.y, color);
	state |= _engine->_interface->drawLine(positions.frontTopLeftPoint2D.x, positions.frontTopLeftPoint2D.y, positions.frontTopRightPoint2D.x, positions.frontTopRightPoint2D.y, color);
	state |= _engine->_interface->drawLine(positions.frontTopRightPoint2D.x, positions.frontTopRightPoint2D.y, positions.frontBottomRightPoint2D.x, positions.frontBottomRightPoint2D.y, color);
	state |= _engine->_interface->drawLine(positions.frontBottomRightPoint2D.x, positions.frontBottomRightPoint2D.y, positions.frontBottomLeftPoint2D.x, positions.frontBottomLeftPoint2D.y, color);

	// draw top part
	state |= _engine->_interface->drawLine(positions.frontTopLeftPoint2D.x, positions.frontTopLeftPoint2D.y, positions.backTopLeftPoint2D.x, positions.backTopLeftPoint2D.y, color);
	state |= _engine->_interface->drawLine(positions.backTopLeftPoint2D.x, positions.backTopLeftPoint2D.y, positions.backTopRightPoint2D.x, positions.backTopRightPoint2D.y, color);
	state |= _engine->_interface->drawLine(positions.backTopRightPoint2D.x, positions.backTopRightPoint2D.y, positions.frontTopRightPoint2D.x, positions.frontTopRightPoint2D.y, color);
	state |= _engine->_interface->drawLine(positions.frontTopRightPoint2D.x, positions.frontTopRightPoint2D.y, positions.frontTopLeftPoint2D.x, positions.frontTopLeftPoint2D.y, color);

	// draw back part
	state |= _engine->_interface->drawLine(positions.backBottomLeftPoint2D.x, positions.backBottomLeftPoint2D.y, positions.backTopLeftPoint2D.x, positions.backTopLeftPoint2D.y, color);
	state |= _engine->_interface->drawLine(positions.backTopLeftPoint2D.x, positions.backTopLeftPoint2D.y, positions.backTopRightPoint2D.x, positions.backTopRightPoint2D.y, color);
	state |= _engine->_interface->drawLine(positions.backTopRightPoint2D.x, positions.backTopRightPoint2D.y, positions.backBottomRightPoint2D.x, positions.backBottomRightPoint2D.y, color);
	state |= _engine->_interface->drawLine(positions.backBottomRightPoint2D.x, positions.backBottomRightPoint2D.y, positions.backBottomLeftPoint2D.x, positions.backBottomLeftPoint2D.y, color);

	// draw bottom part
	state |= _engine->_interface->drawLine(positions.frontBottomLeftPoint2D.x, positions.frontBottomLeftPoint2D.y, positions.backBottomLeftPoint2D.x, positions.backBottomLeftPoint2D.y, color);
	state |= _engine->_interface->drawLine(positions.backBottomLeftPoint2D.x, positions.backBottomLeftPoint2D.y, positions.backBottomRightPoint2D.x, positions.backBottomRightPoint2D.y, color);
	state |= _engine->_interface->drawLine(positions.backBottomRightPoint2D.x, positions.backBottomRightPoint2D.y, positions.frontBottomRightPoint2D.x, positions.frontBottomRightPoint2D.y, color);
	state |= _engine->_interface->drawLine(positions.frontBottomRightPoint2D.x, positions.frontBottomRightPoint2D.y, positions.frontBottomLeftPoint2D.x, positions.frontBottomLeftPoint2D.y, color);

	return state;
}

bool DebugScene::displayActors() {
	bool state = false;
	for (int32 a = 0; a < _engine->_scene->_sceneNumActors; a++) {
		const ActorStruct *actorPtr = _engine->_scene->getActor(a);
		const IVec3 &pos = actorPtr->pos();
		const BoundingBox &bbox = actorPtr->_boundingBox;
		const ScenePositionsProjected &positions = calculateBoxPositions(pos + bbox.mins, pos + bbox.maxs);
		if (!drawBox(positions, COLOR_WHITE)) {
			continue;
		}
		const int boxwidth = 150;
		const int lineHeight = 14;
		const int boxheight = 2 * lineHeight;
		const Common::Rect filledRect(positions.frontTopRightPoint2D.x, positions.frontTopRightPoint2D.y, positions.frontTopRightPoint2D.x + boxwidth, positions.frontTopRightPoint2D.y + boxheight);
		_engine->_interface->drawFilledRect(filledRect, COLOR_WHITE);
		_engine->_menu->drawRectBorders(filledRect);
		_engine->drawText(positions.frontTopRightPoint2D.x, positions.frontTopRightPoint2D.y, Common::String::format("Actor: %i", a), true, true, boxwidth);
		_engine->drawText(positions.frontTopRightPoint2D.x, positions.frontTopRightPoint2D.y + lineHeight, Common::String::format("pos: %i:%i:%i", pos.x, pos.y, pos.z), true, true, boxwidth);
		const int16 rleft = positions.frontTopLeftPoint2D.x;
		const int16 rtop = positions.backTopLeftPoint2D.y;
		const int16 rright = positions.backTopRightPoint2D.x;
		const int16 rbottom = positions.frontBottomRightPoint2D.y;
		Common::Rect actorRect(rleft, rtop, rright, rbottom);
		actorRect.extend(filledRect);
		_engine->_redraw->addRedrawArea(actorRect);
		state = true;
	}
	return state;
}

// TODO: implement the rendering points of all tracks as a dot with the id
bool DebugScene::displayTracks() {
#if 0
	for (int i = 0; i < _engine->_scene->sceneNumTracks; i++) {
		const Vec3 *trackPoint = &_engine->_scene->sceneTracks[i];

	}
#endif
	return false;
}

bool DebugScene::displayZones() {
	bool state = false;
	for (int i = 0; i < _engine->_scene->_sceneNumZones; i++) {
		const ZoneStruct *zonePtr = &_engine->_scene->_sceneZones[i];

		if (!checkZoneType(zonePtr->type)) {
			continue;
		}

		const ScenePositionsProjected &positions = calculateBoxPositions(zonePtr->mins, zonePtr->maxs);
		const uint8 color = 15 * 3 + (int)zonePtr->type * 16;
		if (!drawBox(positions, color)) {
			continue;
		}

		const int boxwidth = 150;
		const int lineHeight = 14;
		const int boxheight = 2 * lineHeight;
		const Common::Rect filledRect(positions.frontTopRightPoint2D.x, positions.frontTopRightPoint2D.y, positions.frontTopRightPoint2D.x + boxwidth, positions.frontTopRightPoint2D.y + boxheight);
		_engine->_interface->drawFilledRect(filledRect, COLOR_WHITE);
		_engine->_menu->drawRectBorders(filledRect);
		_engine->drawText(positions.frontTopRightPoint2D.x, positions.frontTopRightPoint2D.y, Common::String::format("Type: %i (%i)", (int)zonePtr->type, i), true, false, boxwidth);
		_engine->drawText(positions.frontTopRightPoint2D.x, positions.frontTopRightPoint2D.y + lineHeight, Common::String::format("pos: %i:%i:%i", positions.frontTopRightPoint.x, positions.frontTopRightPoint.y, positions.frontTopRightPoint.z), true, false, boxwidth);
		state = true;
	}
	return state;
}

void DebugScene::renderDebugView() {
	if (_showingZones) {
		displayZones();
	}
	if (_showingActors) {
		displayActors();
	}
	if (_showingTracks) {
		displayTracks();
	}
}

} // namespace TwinE
