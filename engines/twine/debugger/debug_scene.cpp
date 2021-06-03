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
	if (!showingClips) {
		return;
	}
	_engine->_menu->drawBox(rect);
}

void DebugScene::projectBoundingBoxPoints(IVec3 *pPoint3d, IVec3 *pPoint3dProjected) {
	_engine->_renderer->projectPositionOnScreen(pPoint3d->x, pPoint3d->y, pPoint3d->z);

	pPoint3dProjected->x = _engine->_renderer->projPos.x;
	pPoint3dProjected->y = _engine->_renderer->projPos.y;
	pPoint3dProjected->z = _engine->_renderer->projPos.z;

	if (_engine->_redraw->renderRect.left > _engine->_renderer->projPos.x) {
		_engine->_redraw->renderRect.left = _engine->_renderer->projPos.x;
	}

	if (_engine->_redraw->renderRect.right < _engine->_renderer->projPos.x) {
		_engine->_redraw->renderRect.right = _engine->_renderer->projPos.x;
	}

	if (_engine->_redraw->renderRect.top > _engine->_renderer->projPos.y) {
		_engine->_redraw->renderRect.top = _engine->_renderer->projPos.y;
	}

	if (_engine->_redraw->renderRect.bottom < _engine->_renderer->projPos.y) {
		_engine->_redraw->renderRect.bottom = _engine->_renderer->projPos.y;
	}
}

bool DebugScene::checkZoneType(ZoneType type) const {
	switch (type) {
	case ZoneType::kCube:
		return (typeZones & 0x01) != 0;
	case ZoneType::kCamera:
		return (typeZones & 0x02) != 0;
	case ZoneType::kSceneric:
		return (typeZones & 0x04) != 0;
	case ZoneType::kGrid:
		return (typeZones & 0x08) != 0;
	case ZoneType::kObject:
		return (typeZones & 0x10) != 0;
	case ZoneType::kText:
		return (typeZones & 0x20) != 0;
	case ZoneType::kLadder:
		return (typeZones & 0x40) != 0;
	default:
		return true;
	}

	return false;
}

DebugScene::ScenePositionsProjected DebugScene::calculateBoxPositions(const IVec3 &mins, const IVec3 &maxs) {
	ScenePositionsProjected positions;
	// compute the points in 3D
	positions.frontBottomLeftPoint.x = mins.x - _engine->_grid->camera.x;
	positions.frontBottomLeftPoint.y = mins.y - _engine->_grid->camera.y;
	positions.frontBottomLeftPoint.z = maxs.z - _engine->_grid->camera.z;

	positions.frontBottomRightPoint.x = maxs.x - _engine->_grid->camera.x;
	positions.frontBottomRightPoint.y = mins.y - _engine->_grid->camera.y;
	positions.frontBottomRightPoint.z = maxs.z - _engine->_grid->camera.z;

	positions.frontTopLeftPoint.x = mins.x - _engine->_grid->camera.x;
	positions.frontTopLeftPoint.y = maxs.y - _engine->_grid->camera.y;
	positions.frontTopLeftPoint.z = maxs.z - _engine->_grid->camera.z;

	positions.frontTopRightPoint = maxs - _engine->_grid->camera;
	positions.backBottomLeftPoint = mins - _engine->_grid->camera;

	positions.backBottomRightPoint.x = maxs.x - _engine->_grid->camera.x;
	positions.backBottomRightPoint.y = mins.y - _engine->_grid->camera.y;
	positions.backBottomRightPoint.z = mins.z - _engine->_grid->camera.z;

	positions.backTopLeftPoint.x = mins.x - _engine->_grid->camera.x;
	positions.backTopLeftPoint.y = maxs.y - _engine->_grid->camera.y;
	positions.backTopLeftPoint.z = mins.z - _engine->_grid->camera.z;

	positions.backTopRightPoint.x = maxs.x - _engine->_grid->camera.x;
	positions.backTopRightPoint.y = maxs.y - _engine->_grid->camera.y;
	positions.backTopRightPoint.z = mins.z - _engine->_grid->camera.z;

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
	for (int i = 0; i < _engine->_scene->sceneNumActors; i++) {
		const ActorStruct *actorPtr = _engine->_scene->getActor(i);
		// TODO: redrawing doesn't work properly yet for moving actors
		if (!actorPtr->staticFlags.bIsSpriteActor) {
			continue;
		}
		const IVec3 &pos = actorPtr->pos;
		const BoundingBox &bbox = actorPtr->boudingBox;
		const ScenePositionsProjected &positions = calculateBoxPositions(pos + bbox.mins, pos + bbox.maxs);
		if (!drawBox(positions, COLOR_WHITE)) {
			continue;
		}
		const int boxwidth = 150;
		const int lineHeight = 14;
		const int boxheight = 2 * lineHeight;
		const Common::Rect filledRect(positions.frontTopRightPoint2D.x, positions.frontTopRightPoint2D.y, positions.frontTopRightPoint2D.x + boxwidth, positions.frontTopRightPoint2D.y + boxheight);
		_engine->_interface->drawFilledRect(filledRect, COLOR_WHITE);
		_engine->_menu->drawBox(filledRect);
		_engine->drawText(positions.frontTopRightPoint2D.x, positions.frontTopRightPoint2D.y, Common::String::format("Actor: %i", i), true, false, boxwidth);
		_engine->drawText(positions.frontTopRightPoint2D.x, positions.frontTopRightPoint2D.y + lineHeight, Common::String::format("pos: %i:%i:%i", positions.frontTopRightPoint.x, positions.frontTopRightPoint.y, positions.frontTopRightPoint.z), true, false, boxwidth);
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
	for (int i = 0; i < _engine->_scene->sceneNumZones; i++) {
		const ZoneStruct *zonePtr = &_engine->_scene->sceneZones[i];

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
		_engine->_menu->drawBox(filledRect);
		_engine->drawText(positions.frontTopRightPoint2D.x, positions.frontTopRightPoint2D.y, Common::String::format("Type: %i (%i)", (int)zonePtr->type, i), true, false, boxwidth);
		_engine->drawText(positions.frontTopRightPoint2D.x, positions.frontTopRightPoint2D.y + lineHeight, Common::String::format("pos: %i:%i:%i", positions.frontTopRightPoint.x, positions.frontTopRightPoint.y, positions.frontTopRightPoint.z), true, false, boxwidth);
		state = true;
	}
	return state;
}

void DebugScene::renderDebugView() {
	if (showingZones) {
		displayZones();
	}
	if (showingActors) {
		displayActors();
	}
	if (showingTracks) {
		displayTracks();
	}
}

} // namespace TwinE
