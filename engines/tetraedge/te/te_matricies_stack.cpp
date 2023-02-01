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

#include "tetraedge/te/te_matricies_stack.h"

namespace Tetraedge {

TeMatriciesStack::TeMatriciesStack() {
	_stack.push(TeMatrix4x4());
}

const TeMatrix4x4 &TeMatriciesStack::currentMatrix() const {
	return _stack.top();
}

bool TeMatriciesStack::isEmpty() const {
	return _stack.empty();
}

void TeMatriciesStack::loadIdentity() {
	_stack.top() = TeMatrix4x4();
}

void TeMatriciesStack::loadMatrix(const TeMatrix4x4 &matrix) {
	_stack.top() = matrix;
}

void TeMatriciesStack::multiplyMatrix(const TeMatrix4x4 &matrix) {
	_stack.top() = _stack.top() * matrix;
}

void TeMatriciesStack::popMatrix() {
	if (_stack.size() > 1)
		_stack.pop();
}

void TeMatriciesStack::pushMatrix() {
	_stack.push(_stack.top());
}

void TeMatriciesStack::rotate(const TeQuaternion &rot) {
	_stack.top() = _stack.top() * rot.toTeMatrix();
}

void TeMatriciesStack::rotate(float angle, const TeVector3f32 &axis) {
	rotate(TeQuaternion::fromAxisAndAngle(axis, angle));
}

void TeMatriciesStack::scale(const TeVector3f32 &scale) {
	_stack.top().scale(scale);
}

uint TeMatriciesStack::size() {
	return _stack.size();
}

void TeMatriciesStack::translate(const TeVector3f32 &trans) {
	_stack.top().translate(trans);
}

} // end namespace Tetraedge
