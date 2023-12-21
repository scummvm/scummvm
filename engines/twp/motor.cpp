
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
	  _tween(obj->_node->getOffset(), pos, duration, im) {
}

void OffsetTo::update(float elapsed) {
	_tween.update(elapsed);
	_obj->_node->setOffset(_tween.current());
	if (!_tween.running())
		disable();
}

MoveTo::~MoveTo() {}

MoveTo::MoveTo(float duration, Object *obj, Math::Vector2d pos, InterpolationMethod im)
	: _obj(obj),
	  _tween(obj->_node->getPos(), pos, duration, im) {
}

void MoveTo::update(float elapsed) {
	_tween.update(elapsed);
	_obj->_node->setPos(_tween.current());
	if (!_tween.running())
		disable();
}

AlphaTo::~AlphaTo() {}

AlphaTo::AlphaTo(float duration, Object *obj, float to, InterpolationMethod im)
	: _obj(obj),
	  _tween(obj->_node->getAlpha(), to, duration, im) {
}

void AlphaTo::update(float elapsed) {
	_tween.update(elapsed);
	float alpha = _tween.current();
	_obj->_node->setAlpha(alpha);
	if (!_tween.running())
		disable();
}

RotateTo::~RotateTo() {}

RotateTo::RotateTo(float duration, Node *node, float to, InterpolationMethod im)
	: _node(node),
	  _tween(node->getRotation(), to, duration, im) {
}

void RotateTo::update(float elapsed) {
	_tween.update(elapsed);
	_node->setRotation(_tween.current());
	if (!_tween.running())
		disable();
}

ScaleTo::~ScaleTo() {}

ScaleTo::ScaleTo(float duration, Node *node, float to, InterpolationMethod im)
	: _node(node),
	  _tween(node->getScale().getX(), to, duration, im) {
}

void ScaleTo::update(float elapsed) {
	_tween.update(elapsed);
	float x = _tween.current();
	_node->setScale(Math::Vector2d(x, x));
	if (!_tween.running())
		disable();
}

Shake::~Shake() {}

Shake::Shake(Node *node, float amount)
	: _node(node),
	  _amount(amount) {
}

void Shake::update(float elapsed) {
	_shakeTime += 40.f * elapsed;
	_elapsed += elapsed;
	// TODO: check if it's necessary to create a _shakeOffset in a node
	_node->setOffset(Math::Vector2d(_amount * cos(_shakeTime + 0.3f), _amount * sin(_shakeTime)));
}

} // namespace Twp
