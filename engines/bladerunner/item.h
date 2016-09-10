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

#include "bladerunner/bladerunner.h"
#include "bladerunner/boundingbox.h"

#include "common/array.h"
#include "common/rect.h"

namespace BladeRunner {
	class Items;

	class Item {
		friend class Items;
	private:
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
		//int field_1C8;
		int          _target;
		//float field_1D0;
		int          _targetable;
		int          _spinning;
		int          _angleChange;
		float        _cameraAngle;
		int          _obstacle;
		//int field_1E8;
	public:
		Item();
		~Item();

		void getXYZ(float *x, float *y, float *z);
	};

}

#endif
