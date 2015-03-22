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

#include "math/glmath.h"

namespace Math {

// function based on gluUnProject from Mesa 5.0 glu GPLv2+ licensed sources
bool gluMathUnProject(Vector3d win, const Matrix4 &mvpMatrix, const Common::Rect &viewport, Vector3d &obj) {
	Matrix4 A;
	Vector4d in, out;

	in.x() = (win.x() - viewport.left) * 2 / viewport.width() - 1.0;
	in.y() = (win.y() - viewport.top) * 2 / viewport.height() - 1.0;
	in.z() = 2 * win.z() - 1.0;
	in.w() = 1.0;

	A = mvpMatrix;
	A.inverse();

	out = A.transform(in);
	if (out.w() == 0.0)
		return false;

	obj.x() = out.x() / out.w();
	obj.y() = out.y() / out.w();
	obj.z() = out.z() / out.w();

	return true;
}

// function based on gluPerspective from Mesa 5.0 glu GPLv2+ licensed sources
Matrix4 makePerspectiveMatrix(double fovy, double aspect, double zNear, double zFar) {
	double xmin, xmax, ymin, ymax;

	ymax = zNear * tan(fovy * M_PI / 360.0);
	ymin = -ymax;
	xmin = ymin * aspect;
	xmax = ymax * aspect;

	return makeFrustumMatrix(xmin, xmax, ymin, ymax, zNear, zFar);
}

Matrix4 makeFrustumMatrix(double left, double right, double bottom, double top, double zNear, double zFar) {
	Matrix4 proj;
	proj(0, 0) = (2.0f * zNear) / (right - left);
	proj(1, 1) = (2.0f * zNear) / (top - bottom);
	proj(2, 0) = (right + left) / (right - left);
	proj(2, 1) = (top + bottom) / (top - bottom);
	proj(2, 2) = -(zFar + zNear) / (zFar - zNear);
	proj(2, 3) = -1.0f;
	proj(3, 2) = -(2.0f * zFar * zNear) / (zFar - zNear);
	proj(3, 3) = 0.0f;

	return proj;
}

Matrix4 makeLookAtMatrix(const Vector3d &eye, const Vector3d &center, const Vector3d &up) {
	Vector3d f = (center - eye).getNormalized();
	Vector3d u = up.getNormalized();
	Vector3d s = Vector3d::crossProduct(f, u).getNormalized();
	u = Vector3d::crossProduct(s, f);

	Math::Matrix4 look;
	look(0, 0) = s.x();
	look(1, 0) = s.y();
	look(2, 0) = s.z();
	look(0, 1) = u.x();
	look(1, 1) = u.y();
	look(2, 1) = u.z();
	look(0, 2) = -f.x();
	look(1, 2) = -f.y();
	look(2, 2) = -f.z();
	look(3, 3) = 1.0;

	return look;
}

}
