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
#include "twine/twine.h"

namespace TwinE {

DebugScene::DebugScene(TwinEEngine *engine) : _engine(engine) {}

void DebugScene::drawClip(const Common::Rect& rect) {
	if (!showingClips) {
		return;
	}
	_engine->_menu->drawBox(rect);
}

void DebugScene::drawBoundingBoxProjectPoints(ScenePoint *pPoint3d, ScenePoint *pPoint3dProjected) {
	_engine->_renderer->projectPositionOnScreen(pPoint3d->x, pPoint3d->y, pPoint3d->z);

	pPoint3dProjected->x = _engine->_renderer->projPosX;
	pPoint3dProjected->y = _engine->_renderer->projPosY;
	pPoint3dProjected->z = _engine->_renderer->projPosZ;

	if (_engine->_redraw->renderRect.left > _engine->_renderer->projPosX) {
		_engine->_redraw->renderRect.left = _engine->_renderer->projPosX;
	}

	if (_engine->_redraw->renderRect.right < _engine->_renderer->projPosX) {
		_engine->_redraw->renderRect.right = _engine->_renderer->projPosX;
	}

	if (_engine->_redraw->renderRect.top > _engine->_renderer->projPosY) {
		_engine->_redraw->renderRect.top = _engine->_renderer->projPosY;
	}

	if (_engine->_redraw->renderRect.bottom < _engine->_renderer->projPosY) {
		_engine->_redraw->renderRect.bottom = _engine->_renderer->projPosY;
	}
}

int32 DebugScene::checkZoneType(int32 type) {
	switch (type) {
	case 0:
		if (typeZones & 0x01)
			return 1;
		break;
	case 1:
		if (typeZones & 0x02)
			return 1;
		break;
	case 2:
		if (typeZones & 0x04)
			return 1;
		break;
	case 3:
		if (typeZones & 0x08)
			return 1;
		break;
	case 4:
		if (typeZones & 0x10)
			return 1;
		break;
	case 5:
		if (typeZones & 0x20)
			return 1;
		break;
	case 6:
		if (typeZones & 0x40)
			return 1;
		break;
	default:
		break;
	}

	return 0;
}

void DebugScene::displayZones() {
	if (!showingZones) {
		return;
	}
	for (int z = 0; z < _engine->_scene->sceneNumZones; z++) {
		const ZoneStruct *zonePtr = &_engine->_scene->sceneZones[z];

		if (!checkZoneType(zonePtr->type)) {
			continue;
		}
		ScenePoint frontBottomLeftPoint;
		ScenePoint frontBottomRightPoint;

		ScenePoint frontTopLeftPoint;
		ScenePoint frontTopRightPoint;

		ScenePoint backBottomLeftPoint;
		ScenePoint backBottomRightPoint;

		ScenePoint backTopLeftPoint;
		ScenePoint backTopRightPoint;

		ScenePoint frontBottomLeftPoint2D;
		ScenePoint frontBottomRightPoint2D;

		ScenePoint frontTopLeftPoint2D;
		ScenePoint frontTopRightPoint2D;

		ScenePoint backBottomLeftPoint2D;
		ScenePoint backBottomRightPoint2D;

		ScenePoint backTopLeftPoint2D;
		ScenePoint backTopRightPoint2D;

		// compute the points in 3D

		frontBottomLeftPoint.x = zonePtr->bottomLeft.x - _engine->_grid->cameraX;
		frontBottomLeftPoint.y = zonePtr->bottomLeft.y - _engine->_grid->cameraY;
		frontBottomLeftPoint.z = zonePtr->topRight.z - _engine->_grid->cameraZ;

		frontBottomRightPoint.x = zonePtr->topRight.x - _engine->_grid->cameraX;
		frontBottomRightPoint.y = zonePtr->bottomLeft.y - _engine->_grid->cameraY;
		frontBottomRightPoint.z = zonePtr->topRight.z - _engine->_grid->cameraZ;

		frontTopLeftPoint.x = zonePtr->bottomLeft.x - _engine->_grid->cameraX;
		frontTopLeftPoint.y = zonePtr->topRight.y - _engine->_grid->cameraY;
		frontTopLeftPoint.z = zonePtr->topRight.z - _engine->_grid->cameraZ;

		frontTopRightPoint.x = zonePtr->topRight.x - _engine->_grid->cameraX;
		frontTopRightPoint.y = zonePtr->topRight.y - _engine->_grid->cameraY;
		frontTopRightPoint.z = zonePtr->topRight.z - _engine->_grid->cameraZ;

		backBottomLeftPoint.x = zonePtr->bottomLeft.x - _engine->_grid->cameraX;
		backBottomLeftPoint.y = zonePtr->bottomLeft.y - _engine->_grid->cameraY;
		backBottomLeftPoint.z = zonePtr->bottomLeft.z - _engine->_grid->cameraZ;

		backBottomRightPoint.x = zonePtr->topRight.x - _engine->_grid->cameraX;
		backBottomRightPoint.y = zonePtr->bottomLeft.y - _engine->_grid->cameraY;
		backBottomRightPoint.z = zonePtr->bottomLeft.z - _engine->_grid->cameraZ;

		backTopLeftPoint.x = zonePtr->bottomLeft.x - _engine->_grid->cameraX;
		backTopLeftPoint.y = zonePtr->topRight.y - _engine->_grid->cameraY;
		backTopLeftPoint.z = zonePtr->bottomLeft.z - _engine->_grid->cameraZ;

		backTopRightPoint.x = zonePtr->topRight.x - _engine->_grid->cameraX;
		backTopRightPoint.y = zonePtr->topRight.y - _engine->_grid->cameraY;
		backTopRightPoint.z = zonePtr->bottomLeft.z - _engine->_grid->cameraZ;

		// project all points

		drawBoundingBoxProjectPoints(&frontBottomLeftPoint, &frontBottomLeftPoint2D);
		drawBoundingBoxProjectPoints(&frontBottomRightPoint, &frontBottomRightPoint2D);
		drawBoundingBoxProjectPoints(&frontTopLeftPoint, &frontTopLeftPoint2D);
		drawBoundingBoxProjectPoints(&frontTopRightPoint, &frontTopRightPoint2D);
		drawBoundingBoxProjectPoints(&backBottomLeftPoint, &backBottomLeftPoint2D);
		drawBoundingBoxProjectPoints(&backBottomRightPoint, &backBottomRightPoint2D);
		drawBoundingBoxProjectPoints(&backTopLeftPoint, &backTopLeftPoint2D);
		drawBoundingBoxProjectPoints(&backTopRightPoint, &backTopRightPoint2D);

		// draw all lines

		uint8 color = 15 * 3 + zonePtr->type * 16;

		// draw front part
		_engine->_interface->drawLine(frontBottomLeftPoint2D.x, frontBottomLeftPoint2D.y, frontTopLeftPoint2D.x, frontTopLeftPoint2D.y, color);
		_engine->_interface->drawLine(frontTopLeftPoint2D.x, frontTopLeftPoint2D.y, frontTopRightPoint2D.x, frontTopRightPoint2D.y, color);
		_engine->_interface->drawLine(frontTopRightPoint2D.x, frontTopRightPoint2D.y, frontBottomRightPoint2D.x, frontBottomRightPoint2D.y, color);
		_engine->_interface->drawLine(frontBottomRightPoint2D.x, frontBottomRightPoint2D.y, frontBottomLeftPoint2D.x, frontBottomLeftPoint2D.y, color);

		// draw top part
		_engine->_interface->drawLine(frontTopLeftPoint2D.x, frontTopLeftPoint2D.y, backTopLeftPoint2D.x, backTopLeftPoint2D.y, color);
		_engine->_interface->drawLine(backTopLeftPoint2D.x, backTopLeftPoint2D.y, backTopRightPoint2D.x, backTopRightPoint2D.y, color);
		_engine->_interface->drawLine(backTopRightPoint2D.x, backTopRightPoint2D.y, frontTopRightPoint2D.x, frontTopRightPoint2D.y, color);
		_engine->_interface->drawLine(frontTopRightPoint2D.x, frontTopRightPoint2D.y, frontTopLeftPoint2D.x, frontTopLeftPoint2D.y, color);

		// draw back part
		_engine->_interface->drawLine(backBottomLeftPoint2D.x, backBottomLeftPoint2D.y, backTopLeftPoint2D.x, backTopLeftPoint2D.y, color);
		_engine->_interface->drawLine(backTopLeftPoint2D.x, backTopLeftPoint2D.y, backTopRightPoint2D.x, backTopRightPoint2D.y, color);
		_engine->_interface->drawLine(backTopRightPoint2D.x, backTopRightPoint2D.y, backBottomRightPoint2D.x, backBottomRightPoint2D.y, color);
		_engine->_interface->drawLine(backBottomRightPoint2D.x, backBottomRightPoint2D.y, backBottomLeftPoint2D.x, backBottomLeftPoint2D.y, color);

		// draw bottom part
		_engine->_interface->drawLine(frontBottomLeftPoint2D.x, frontBottomLeftPoint2D.y, backBottomLeftPoint2D.x, backBottomLeftPoint2D.y, color);
		_engine->_interface->drawLine(backBottomLeftPoint2D.x, backBottomLeftPoint2D.y, backBottomRightPoint2D.x, backBottomRightPoint2D.y, color);
		_engine->_interface->drawLine(backBottomRightPoint2D.x, backBottomRightPoint2D.y, frontBottomRightPoint2D.x, frontBottomRightPoint2D.y, color);
		_engine->_interface->drawLine(frontBottomRightPoint2D.x, frontBottomRightPoint2D.y, frontBottomLeftPoint2D.x, frontBottomLeftPoint2D.y, color);
	}
	_engine->flip();
}

} // namespace TwinE
