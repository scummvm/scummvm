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

#ifndef TETRAEDGE_TE_TE_MATRICIES_STACK_H
#define TETRAEDGE_TE_TE_MATRICIES_STACK_H

#include "common/stack.h"

#include "tetraedge/te/te_matrix4x4.h"
#include "tetraedge/te/te_quaternion.h"

namespace Tetraedge {

class TeMatriciesStack {
public:
	TeMatriciesStack();

	const TeMatrix4x4 &currentMatrix() const;
	bool isEmpty() const;
	void loadIdentity();
	void loadMatrix(const TeMatrix4x4 &matrix);
	void multiplyMatrix(const TeMatrix4x4 &matrix);
	void popMatrix();
	void pushMatrix();
	void rotate(const TeQuaternion &rot);
	void rotate(float f, const TeVector3f32 &rot);
	void scale(const TeVector3f32 &scale);
	uint size();
	void translate(const TeVector3f32 &trans);

private:
	Common::Stack<TeMatrix4x4> _stack;

};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_MATRICIES_STACK_H
