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

#include "bladerunner/item.h"

#include "bladerunner/bladerunner.h"

#include "bladerunner/slice_renderer.h"

namespace BladeRunner {

Item::Item(BladeRunnerEngine *vm) {
	_vm = vm;

	_itemId = -1;
	_setId = -1;

	_animationId = -1;
	_position.x = 0;
	_position.y = 0;
	_position.z = 0;
	_facing = 0;
	_angle = 0.0f;
	_width = 0;
	_height = 0;
	_boundingBox.setXYZ(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	_screenX = 0;
	_screenY = 0;
	_depth = 0.0f;
	_isTargetable = false;
	_isSpinning = false;
	_facingChange = 0;
	_isVisible = true;
	_isPoliceMazeEnemy = true;
	_screenRectangle.bottom = -1;
	_screenRectangle.right = -1;
	_screenRectangle.top = -1;
	_screenRectangle.left = -1;
}

Item::~Item() {
}

void Item::getXYZ(float *x, float *y, float *z) {
	*x = _position.x;
	*y = _position.y;
	*z = _position.z;
}

void Item::getWidthHeight(int *width, int *height) {
	*width = _width;
	*height = _height;
}

bool Item::isTargetable() {
	return _isTargetable;
}

void Item::tick(bool special) {
	if (_isVisible) {
		Vector3 postition(_position.x, -_position.z, _position.y);
		int animationId = _animationId + (special ? 1 : 0);
		_vm->_sliceRenderer->drawInWorld(animationId, 0, postition, M_PI - _angle, 1.0f, _vm->_surface2, _vm->_zBuffer2);
		_vm->_sliceRenderer->getScreenRectangle(&_screenRectangle, animationId, 0, postition, M_PI - _angle, 1.0f);

		if (_isSpinning) {
			_facing += _facingChange;

			if (_facing >= 1024) {
				_facing -= 1024;
			} else if (_facing < 0) {
				_facing += 1024;
			}
			_angle = _facing * (M_PI / 512.0f);

			if (_facingChange > 0) {
				_facingChange = _facingChange - 20;
				if (_facingChange < 0) {
					_facingChange = 0;
					_isSpinning = false;
				}
			} else if (_facingChange < 0) {
				_facingChange = _facingChange + 20;
				if (_facingChange > 0) {
					_facingChange = 0;
					_isSpinning = false;
				}
			} else {
				_isSpinning = false;
			}
		}
	}
}

void Item::setXYZ(Vector3 position) {
	_position = position;
	int halfWidth = _width / 2;
	_boundingBox.setXYZ(_position.x - halfWidth, _position.y, _position.z - halfWidth,
	                    _position.x + halfWidth, _position.y + _height, _position.z + halfWidth);
	Vector3 screenPosition = _vm->_view->calculateScreenPosition(_position);
	_screenX = screenPosition.x;
	_screenY = screenPosition.y;
	_depth = screenPosition.z * 25.5f;
}

void Item::setup(int itemId, int setId, int animationId, Vector3 position, int facing, int height, int width, bool isTargetable, bool isVisible, bool isPoliceMazeEnemy) {
	_itemId = itemId;
	_setId = setId;
	_animationId = animationId;
	_facing = facing;
	_angle = facing * (M_PI / 512.0f);
	_width = width;
	_height = height;
	_isTargetable = isTargetable;
	_isVisible = isVisible;
	_isPoliceMazeEnemy = isPoliceMazeEnemy;
	setXYZ(position);
	_screenRectangle.bottom = -1;
	_screenRectangle.right = -1;
	_screenRectangle.top = -1;
	_screenRectangle.left = -1;
}

} // End of namespace BladeRunner
