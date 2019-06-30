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

#include "bladerunner/savefile.h"
#include "bladerunner/slice_renderer.h"
#include "bladerunner/zbuffer.h"

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
	_isTarget = false;
	_isSpinning = false;
	_facingChange = 0;
	_isVisible = true;
	_isPoliceMazeEnemy = false;
	_screenRectangle.bottom = -1;
	_screenRectangle.right = -1;
	_screenRectangle.top = -1;
	_screenRectangle.left = -1;
}

void Item::getXYZ(float *x, float *y, float *z) const {
	*x = _position.x;
	*y = _position.y;
	*z = _position.z;
}

void Item::getWidthHeight(int *width, int *height) const {
	*width = _width;
	*height = _height;
}

void Item::getAnimationId(int *animationId) const {
	*animationId = _animationId;
}

void Item::setFacing(int facing) {
	_facing = facing;
	_angle = _facing * (M_PI / 512.0f);
}

bool Item::tick(Common::Rect *screenRect, bool special) {
	if (!_isVisible) {
		*screenRect = Common::Rect();
		return false;
	}

	bool isVisibleFlag = false;

	Vector3 position(_position.x, -_position.z, _position.y);
	int animationId = _animationId + (special ? 1 : 0);
	_vm->_sliceRenderer->drawInWorld(animationId, 0, position, M_PI - _angle, 1.0f, _vm->_surfaceFront, _vm->_zbuffer->getData());
	_vm->_sliceRenderer->getScreenRectangle(&_screenRectangle, animationId, 0, position, M_PI - _angle, 1.0f);

	if (!_screenRectangle.isEmpty()) {
		*screenRect = _screenRectangle;
		isVisibleFlag = true;
	} else {
		*screenRect = Common::Rect();
	}

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

	return isVisibleFlag;
}

// setXYZ() recalculates the item's bounding box,
// but in addition to the item's (Vector3) position
// it takes into account the item's width and height
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

void Item::setup(int itemId, int setId, int animationId, Vector3 position, int facing, int height, int width, bool isTargetFlag, bool isVisibleFlag, bool isPoliceMazeEnemyFlag) {
	_itemId = itemId;
	_setId = setId;
	_animationId = animationId;
	_facing = facing;
	_angle = facing * (M_PI / 512.0f);
	_width = width;
	_height = height;
	_isTarget = isTargetFlag;
	_isVisible = isVisibleFlag;
	_isPoliceMazeEnemy = isPoliceMazeEnemyFlag;
	setXYZ(position);
	_screenRectangle.bottom = -1;
	_screenRectangle.right = -1;
	_screenRectangle.top = -1;
	_screenRectangle.left = -1;
}

void Item::spinInWorld() {
	_isSpinning = true;
	if (_vm->_rnd.getRandomNumberRng(1, 2) == 1) {
		_facingChange = -340;
	} else {
		_facingChange = 340;
	}
}

bool Item::isUnderMouse(int mouseX, int mouseY) const {
	return _isVisible
	    && mouseX >= _screenRectangle.left   - 10
	    && mouseX <= _screenRectangle.right  + 10
	    && mouseY >= _screenRectangle.top    - 10
	    && mouseY <= _screenRectangle.bottom + 10;
}

void Item::save(SaveFileWriteStream &f) {
	f.writeInt(_setId);
	f.writeInt(_itemId);
	f.writeBoundingBox(_boundingBox, false);
	f.writeRect(_screenRectangle);
	f.writeInt(_animationId);
	f.writeVector3(_position);
	f.writeInt(_facing);
	f.writeFloat(_angle);
	f.writeInt(_width);
	f.writeInt(_height);
	f.writeInt(_screenX);
	f.writeInt(_screenY);
	f.writeFloat(_depth);
	f.writeBool(_isTarget);
	f.writeBool(_isSpinning);
	f.writeInt(_facingChange);
	f.writeFloat(0.0f); // _viewAngle
	f.writeBool(_isVisible);
	f.writeBool(_isPoliceMazeEnemy);
}

void Item::load(SaveFileReadStream &f) {
	_setId = f.readInt();
	_itemId = f.readInt();
	_boundingBox = f.readBoundingBox(false);
	_screenRectangle = f.readRect();
	_animationId = f.readInt();
	_position = f.readVector3();
	_facing  = f.readInt();
	_angle = f.readFloat();
	_width = f.readInt();
	_height = f.readInt();
	_screenX = f.readInt();
	_screenY = f.readInt();
	_depth = f.readFloat();
	_isTarget = f.readBool();
	_isSpinning = f.readBool();
	_facingChange = f.readInt();
	f.skip(4);
	_isVisible = f.readBool();
	_isPoliceMazeEnemy = f.readBool();
}

} // End of namespace BladeRunner
