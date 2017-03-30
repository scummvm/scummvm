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

#include "bladerunner/mouse.h"

#include "bladerunner/bladerunner.h"
#include "bladerunner/scene.h"
#include "bladerunner/scene_objects.h"
#include "bladerunner/shape.h"
#include "bladerunner/zbuffer.h"

#include "graphics/surface.h"

namespace BladeRunner {

Mouse::Mouse(BladeRunnerEngine *vm) : _vm(vm) {
	_cursor = 0;
	_frame = 3;
	_hotspotX = 0;
	_hotspotY = 0;
	_x = 0;
	_y = 0;
	_disabledCounter = 0;
	_lastFrameTime = 0;
}

Mouse::~Mouse() {
}

void Mouse::setCursor(int cursor) {
	assert(cursor >= 0 && cursor <= 16);
	if (cursor == _cursor) {
		return;
	}

	_cursor = cursor;

	switch (_cursor) {
	case 0:
		_frame = 3;
		_hotspotX = 0;
		_hotspotY = 0;
		break;
	case 1:
		_frame = 4;
		_hotspotX = 0;
		_hotspotY = 0;
		break;
	case 2:
		_frame = 12;
		_hotspotX = 12;
		_hotspotY = 0;
		break;
	case 3:
		_frame = 15;
		_hotspotX = 23;
		_hotspotY = 12;
		break;
	case 4:
		_frame = 13;
		_hotspotX = 12;
		_hotspotY = 23;
		break;
	case 5:
		_frame = 14;
		_hotspotX = 0;
		_hotspotY = 12;
		break;
	case 6:
		_frame = 16;
		_hotspotX = 19;
		_hotspotY = 19;
		break;
	case 7:
		_frame = 17;
		_hotspotX = 19;
		_hotspotY = 19;
		break;
	case 8:
		_frame = 25;
		_hotspotX = 19;
		_hotspotY = 19;
		break;
	case 9:
		_frame = 26;
		_hotspotX = 19;
		_hotspotY = 19;
		break;
	case 10:
		_frame = 34;
		_hotspotX = 19;
		_hotspotY = 19;
		break;
	case 11:
		_frame = 35;
		_hotspotX = 19;
		_hotspotY = 19;
		break;
	case 12:
		_frame = 12;
		_hotspotX = 12;
		_hotspotY = 0;
		_animCounter = 0;
		break;
	case 13:
		_frame = 15;
		_hotspotX = 23;
		_hotspotY = 12;
		_animCounter = 0;
		break;
	case 14:
		_frame = 13;
		_hotspotX = 12;
		_hotspotY = 23;
		_animCounter = 0;
		break;
	case 15:
		_frame = 14;
		_hotspotX = 0;
		_hotspotY = 12;
		_animCounter = 0;
		break;
	case 16:
		_frame = 0;
		_hotspotX = 11;
		_hotspotY = 11;
	}
}

void Mouse::disable() {
	++_disabledCounter;
}

void Mouse::enable() {
	if (--_disabledCounter <= 0) {
		_disabledCounter = 0;
	}
}

bool Mouse::isDisabled() {
	return _disabledCounter > 0;
}

void Mouse::draw(Graphics::Surface &surface, int x, int y) {
	if (_disabledCounter) {
		return;
	}

	_x = CLIP(x, 0, surface.w - 1);
	_y = CLIP(y, 0, surface.h - 1);

	if (_cursor < 0 || (uint)_cursor >= _vm->_shapes.size()) {
		return;
	}

	Shape *cursorShape = _vm->_shapes[_frame];

	cursorShape->draw(surface, _x - _hotspotX, _y - _hotspotY);

	updateCursorFrame();
}

void Mouse::updateCursorFrame() {
	uint32 now = _vm->getTotalPlayTime();
	const int offset[4] = { 0, 6, 12, 6 };

	if (now - _lastFrameTime < 66) {
		return;
	}
	_lastFrameTime = now;

	switch (_cursor) {
	case 0:
		break;
	case 1:
		if (++_frame > 11)
			_frame = 4;
		break;
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
		break;
	case 7:
		if (++_frame > 24)
			_frame = 17;
		break;
	case 8:
		break;
	case 9:
		if (++_frame > 33)
			_frame = 26;
		break;
	case 10:
		break;
	case 11:
		if (++_frame > 42)
			_frame = 35;
		break;
	case 12:
		if (++_animCounter >= 4) {
			_animCounter = 0;
		}
		_hotspotY = -offset[_animCounter];
		break;
	case 13:
		if (++_animCounter >= 4) {
			_animCounter = 0;
		}
		_hotspotX = 23 + offset[_animCounter];
		break;
	case 14:
		if (++_animCounter >= 4) {
			_animCounter = 0;
		}
		_hotspotY = 23 + offset[_animCounter];
		break;
	case 15:
		if (++_animCounter >= 4) {
			_animCounter = 0;
		}
		_hotspotX = -offset[_animCounter];
		break;
	case 16:
		if (++_frame > 2)
			_frame = 0;
	}
}

void Mouse::tick(int x, int y) {
	if (!_vm->playerHasControl() || isDisabled())
		return;

	Vector3 mousePosition = getXYZ(x, y);
	int cursorId = 0;

	int isClickable = 0;
	int isObstacle  = 0;
	int isTarget    = 0;

	int sceneObjectId = _vm->_sceneObjects->findByXYZ(&isClickable, &isObstacle, &isTarget, mousePosition.x, mousePosition.y, mousePosition.z, 1, 0, 1);
	int exitType = _vm->_scene->_exits->getTypeAtXY(x, y);

	if (sceneObjectId >= 0 && sceneObjectId <= 74) {
		exitType = -1;
	}

	if (exitType != -1) {
		switch (exitType) {
			case 1:
				cursorId = 13;
				break;
			case 2:
				cursorId = 14;
				break;
			case 3:
				cursorId = 15;
				break;
			default:
				cursorId = 12;
		}
		setCursor(cursorId);
		return;
	}

	if (true /* not in combat */) {
		if (sceneObjectId == 0
		 || (sceneObjectId >= 0 && isClickable)
		 || _vm->_scene->_regions->getRegionAtXY(x, y) >= 0) {
			cursorId = 1;
		}
		setCursor(cursorId);
		return;
	}

	setCursor(cursorId);
}

// TEST: RC01 after intro: [290, 216] -> [-204.589249 51.450668 7.659241]
Vector3 Mouse::getXYZ(int x, int y) {
	if (_vm->_scene->getSetId() == -1)
		return Vector3();

	int screenRight = 640 - x;
	int screenDown  = 480 - y;

	float zcoef = 1.0f / tan(_vm->_view->_fovX / 2.0f);

	float x3d = (2.0f / 640.0f * screenRight - 1.0f);
	float y3d = (2.0f / 480.0f * screenDown  - 1.0f) * 0.75f;

	uint16 zbufval = _vm->_zbuffer->getZValue(x, y);

	Vector3 pos;
	pos.z = zbufval / 25.5f;
	pos.x = pos.z / zcoef * x3d;
	pos.y = pos.z / zcoef * y3d;

	Matrix4x3 matrix = _vm->_view->_frameViewMatrix;

	matrix.unknown();

	return matrix * pos;
}

} // End of namespace BladeRunner
