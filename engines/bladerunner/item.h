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

#ifndef BLADERUNNER_ITEM_H
#define BLADERUNNER_ITEM_H

#include "bladerunner/boundingbox.h"
#include "bladerunner/vector.h"

#include "common/rect.h"

namespace BladeRunner {

class BladeRunnerEngine;
class Items;
class SaveFileReadStream;
class SaveFileWriteStream;

class Item {
	friend class Items;

	BladeRunnerEngine *_vm;

	int          _itemId;
	int          _setId;

	BoundingBox  _boundingBox;
	Common::Rect _screenRectangle;
	int          _animationId;
	Vector3      _position;
	int          _facing;
	float        _angle;
	int          _width;
	int          _height;
	int          _screenX;
	int          _screenY;
	float        _depth;
	bool         _isTarget;
	bool         _isSpinning;
	int          _facingChange;
	bool         _isVisible;
	bool         _isPoliceMazeEnemy;

public:
	Item(BladeRunnerEngine *vm);

	void getXYZ(float *x, float *y, float *z) const;
	void setXYZ(Vector3 position);
	void getWidthHeight(int *width, int *height) const;
	void getAnimationId(int *animationId) const;

	const BoundingBox &getBoundingBox() { return _boundingBox; }
	const Common::Rect &getScreenRectangle() { return _screenRectangle; }

	int getFacing() const { return _facing; }
	void setFacing(int facing);

	bool isTarget() const { return _isTarget; }
	void setIsTarget(bool val) { _isTarget = val; }

	bool isSpinning() const { return _isSpinning; }
	void spinInWorld();

	bool isVisible() const { return _isVisible; }
	void setVisible(bool val) { _isVisible = val; }

	bool isPoliceMazeEnemy() const { return _isPoliceMazeEnemy; }
	void setPoliceMazeEnemy(bool val) { _isPoliceMazeEnemy = val; }

	bool tick(Common::Rect *screenRect, bool special);

	void setup(int itemId, int setId, int animationId, Vector3 position, int facing, int height, int width, bool isTargetFlag, bool isVisibleFlag, bool isPoliceMazeEnemyFlag);

	bool isUnderMouse(int mouseX, int mouseY) const;

	void save(SaveFileWriteStream &f);
	void load(SaveFileReadStream &f);
};

}

#endif
