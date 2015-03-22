/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#ifndef MATH_GLMATH_H
#define MATH_GLMATH_H

#include "common/rect.h"

#include "math/vector4d.h"
#include "math/matrix4.h"

namespace Math {

// function based on gluProject from Mesa 5.0 glu GPLv2+ licensed sources
template<typename T, typename S>
bool gluMathProject(Vector3d obj, const T model[16], const T proj[16], const S viewport[4], Vector3d &win) {
	Vector4d in, out;
	Matrix4 modelMatrix, projMatrix;

	in.set(obj.x(), obj.y(), obj.z(), 1.0);

	for (int i = 0; i < 4; i++) {
		modelMatrix(0, i) = model[0 * 4 + i];
		modelMatrix(1, i) = model[1 * 4 + i];
		modelMatrix(2, i) = model[2 * 4 + i];
		modelMatrix(3, i) = model[3 * 4 + i];
		projMatrix(0, i) = proj[0 * 4 + i];
		projMatrix(1, i) = proj[1 * 4 + i];
		projMatrix(2, i) = proj[2 * 4 + i];
		projMatrix(3, i) = proj[3 * 4 + i];
	}

	out = modelMatrix.transform(in);
	in = projMatrix.transform(out);
	if (in.w() == 0.0)
		return false;

	in.x() /= in.w();
	in.y() /= in.w();
	in.z() /= in.w();

	win.x() = viewport[0] + (1 + in.x()) * viewport[2] / 2;
	win.y() = viewport[1] + (1 + in.y()) * viewport[3] / 2;
	win.z() = (1 + in.z()) / 2;

	return true;
}

bool gluMathUnProject(Vector3d win, const Matrix4 &mvpMatrix, const Common::Rect &viewport, Vector3d &obj);

Matrix4 makePerspectiveMatrix(double fovy, double aspect, double zNear, double zFar);
Matrix4 makeFrustumMatrix(double left, double right, double bottom, double top, double zNear, double zFar);
Matrix4 makeLookAtMatrix(const Vector3d &eye, const Vector3d &center, const Vector3d &up);

}

#endif
