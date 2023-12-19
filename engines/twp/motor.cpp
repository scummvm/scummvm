
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

#include "twp/motor.h"
#include "twp/object.h"
#include "twp/scenegraph.h"

namespace Twp {

OffsetTo::~OffsetTo() {}

OffsetTo::OffsetTo(float duration, Object *obj, Math::Vector2d pos, InterpolationMethod im)
	: _obj(obj),
	  _tween(obj->_node->getOffset(), pos, duration, im){
}

void OffsetTo::update(float elapsed) {
  _tween.update(elapsed);
  _obj->_node->setOffset(_tween.current());
  if (!_tween.running())
    disable();
}

} // namespace Twp
