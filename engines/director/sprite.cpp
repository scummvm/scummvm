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

#include "director/director.h"
#include "director/cast.h"
#include "director/score.h"
#include "director/sprite.h"

namespace Director {

Sprite::Sprite() {
	_enabled = false;
	_trails = 0;
	_width = 0;
	_ink = kInkTypeCopy;
	_flags = 0;
	_height = 0;
	_castId = 0;
	_constraint = 0;
	_moveable = 0;
	_castId = 0;
	_backColor = 0;
	_foreColor = 0;
	_left = 0;
	_right = 0;
	_top = 0;
	_bottom = 0;
	_visible = false;
	_movieRate = 0;
	_movieTime = 0;
	_startTime = 0;
	_stopTime = 0;
	_volume = 0;
	_stretch = 0;
	_type = kInactiveSprite;

	_cast = nullptr;
	_blend = 0;
	_lineSize = 1;
}

Sprite::Sprite(const Sprite &sprite) {
	_enabled = sprite._enabled;
	_castId = sprite._castId;
	_flags = sprite._flags;
	_trails = sprite._trails;
	_ink = sprite._ink;
	_width = sprite._width;
	_height = sprite._height;
	_startPoint.x = sprite._startPoint.x;
	_startPoint.y = sprite._startPoint.y;
	_backColor = sprite._backColor;
	_foreColor = sprite._foreColor;
	_left = sprite._left;
	_right = sprite._right;
	_top = sprite._top;
	_bottom = sprite._bottom;
	_visible = sprite._visible;
	_movieRate = sprite._movieRate;
	_movieTime = sprite._movieTime;
	_stopTime = sprite._stopTime;
	_volume = sprite._volume;
	_stretch = sprite._stretch;
	_type = sprite._type;

	_cast = sprite._cast;
	_constraint = sprite._constraint;
	_moveable = sprite._moveable;
	_blend = sprite._blend;
	_startTime = sprite._startTime;
	_lineSize = sprite._lineSize;
}

Sprite::~Sprite() {
	delete _cast;
}

} // End of namespace Director
