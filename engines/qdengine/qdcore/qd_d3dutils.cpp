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

// Note:  Must Define D3D_OVERLOADS to get C++ version of MATRIX3D
#include "qdengine/qdcore/qd_d3dutils.h"

//набор своих функций, потому что те, что представлены в xMath.h
//не удовлетворяют потребностям
//я порой такой требовательный, что самому страшно.
namespace QDEngine {

namespace vector_helpers {
inline VALUE3D
SquareMagnitude(const Vect3f &v) {
	return v.x * v.x + v.y * v.y + v.z * v.z;
}

inline VALUE3D
Magnitude(const Vect3f &v) {
	return (VALUE3D) sqrt(SquareMagnitude(v));
}


inline Vect3f
Normalize(const Vect3f &v) {
	return v / Magnitude(v);
}

inline VALUE3D DotProduct(const Vect3f &v1, const Vect3f &v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

inline Vect3f
CrossProduct(const Vect3f &v1, const Vect3f &v2) {
	Vect3f result;

	result[0] = v1[1] * v2[2] - v1[2] * v2[1];
	result[1] = v1[2] * v2[0] - v1[0] * v2[2];
	result[2] = v1[0] * v2[1] - v1[1] * v2[0];

	return result;
}

//угол между векторами лежащими в плоскости ХОУ
//иначе ее применять НЕЛЬЗЯ
float VectorAngle(const Vect3f &v1, const Vect3f &v2) {
	return float(atan2(v2.y, v2.x) - atan2(v1.y, v1.x));
}

}//vector_helpers

/*
**-----------------------------------------------------------------------------
**  Name:       ZeroMatrix
**  Purpose:    sets D3D matrix to all 0's
**-----------------------------------------------------------------------------
*/

MATRIX3D
ZeroMatrix() {
	MATRIX3D ret;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			ret(i, j) = 0.0f;
		}
	}
	return ret;
} // end ZeroMatrix

/*
**-----------------------------------------------------------------------------
**  Name:       IdentityMatrix
**  Purpose:    sets D3D matrix to Identiy (1's on diagonal, zero's elsewhere)
**-----------------------------------------------------------------------------
*/

MATRIX3D
IdentityMatrix() {
	MATRIX3D ret;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			ret(i, j) = 0.0f;
		}
		ret(i, i) = 1.0f;
	}
	return ret;
} // end IdentityMatrix

/*
**-----------------------------------------------------------------------------
**  Name:       ProjectionMatrix
**  Purpose:    sets Projection matrix from fov, near and far planes
**  Notes:
**      1. fov is in radians.
**      2. See Blinn, "A Trip Down the Graphics Pipeline" pg 188 for details.
**-----------------------------------------------------------------------------
*/

MATRIX3D
ProjectionMatrix(const float near_plane,
                 const float far_plane,
                 const float fov) {
	float c = (float) cos(fov * 0.5);
	float s = (float) sin(fov * 0.5);
	float Q = s / (1.0f - near_plane / far_plane);

	MATRIX3D ret = ZeroMatrix();
	ret(0, 0) = c;
	ret(1, 1) = c;
	ret(2, 2) = Q;
	ret(3, 2) = -Q * near_plane;
	ret(2, 3) = s;
	return ret;
}   // end ProjectionMatrix


/*
**-----------------------------------------------------------------------------
**  Name:       ViewMatrix
**  Purpose:    Controls where the camara is.
**  Notes:
**      1. Note the roll parameter is in radians and rools the viewpoint
**          around the viewing direction
**-----------------------------------------------------------------------------
*/

MATRIX3D ViewMatrixByDir(const Vect3f &from,
                         const Vect3f &view_dir,
                         const Vect3f &world_up,
                         const Vect3f &cam_up) {
	MATRIX3D view = IdentityMatrix();
	//view_dir - Это ось Z в системе координат камеры
	Vect3f zAxis = view_dir;

	//ось Х в правой системе координат
	Vect3f xAxis = vector_helpers::CrossProduct(zAxis, world_up);
	xAxis = vector_helpers::Normalize(xAxis);

	Vect3f yAxis = vector_helpers::CrossProduct(zAxis, xAxis);

	view(0, 0) = xAxis.x;
	view(1, 0) = xAxis.y;
	view(2, 0) = xAxis.z;

	view(0, 1) = yAxis.x;
	view(1, 1) = yAxis.y;
	view(2, 1) = yAxis.z;

	view(0, 2) = zAxis.x;
	view(1, 2) = zAxis.y;
	view(2, 2) = zAxis.z;

	view(3, 0) = -vector_helpers::DotProduct(xAxis, from);
	view(3, 1) = -vector_helpers::DotProduct(cam_up, from);
	view(3, 2) = -vector_helpers::DotProduct(zAxis, from);
	/*
	 *  после всех вычислений верх камеры имеет с верхом мира угол небольше 90 градусов,
	 *  что по сути своей не всегда правильно. Для того, чтобы вычислить правильно поворот
	 *  вокруг оси Z используем up и cam_up. Где cam_up - это верх камеры, который должен
	 *  быть у неё после поворота.
	 *  Перед тем как вычислить угол переводим оба вектора в координаты камеры, тогда они
	 *  будут лежать в плоскости ХОУ. вычисляем угол и поворачиваем.
	 */


	//переводим в координаты камеры, чтобы получить
	//плоскую картинку и пользоваться atan2
	float r = vector_helpers::VectorAngle(TransformVector(cam_up, view),
	                                      TransformVector(yAxis, view));

	view = MatrixMult(RotateZMatrix(-r), view);

	return view;
}

MATRIX3D
ViewMatrix(const Vect3f &from,
           const Vect3f &at,
           const Vect3f &world_up,
           const Vect3f &cam_up) {

	Vect3f view_dir = vector_helpers::Normalize(at - from);
	return ViewMatrixByDir(from, view_dir, world_up, cam_up);
} // end ViewMatrix

/*
**-----------------------------------------------------------------------------
**  Name:       RotateXMatrix
**  Purpose:    Rotate matrix about X axis
**-----------------------------------------------------------------------------
*/

MATRIX3D
RotateXMatrix(const float rads) {
	float cosine = (float) cos(rads);
	float sine = (float) sin(rads);
	MATRIX3D ret = IdentityMatrix();
	ret(1, 1) = cosine;
	ret(2, 2) = -cosine;
	ret(1, 2) = sine;
	ret(2, 1) = sine;
	return ret;
} // end RotateXMatrix

/*
**-----------------------------------------------------------------------------
**  Name:       RotateYMatrix
**  Purpose:    Rotate matrix about Y axis
**-----------------------------------------------------------------------------
*/

MATRIX3D
RotateYMatrix(const float rads) {
	float const cosine  = (float) cos(rads);
	float const sine    = (float) sin(rads);

	MATRIX3D ret = IdentityMatrix();
	ret(0, 0) = cosine;
	ret(2, 2) = -cosine;

	ret(0, 2) = sine;
	ret(2, 0) = sine;

	return ret;
} // end RotateY

/*
**-----------------------------------------------------------------------------
**  Name:       RotateZMatrix
**  Purpose:    Rotate matrix about Z axis
**-----------------------------------------------------------------------------
*/

MATRIX3D
RotateZMatrix(const float rads) {
	float const cosine = (float) cos(rads);
	float const sine = (float) sin(rads);
	MATRIX3D ret = IdentityMatrix();
	ret(0, 0) = cosine;
	ret(1, 1) = -cosine;

	ret(0, 1) = sine;
	ret(1, 0) = sine;
	return ret;
} // end RotateZMatrix

/*
**-----------------------------------------------------------------------------
**  Name:       TranslateMatrix
**  Purpose:    Returns matrix to translate by (dx, dy, dz)
**-----------------------------------------------------------------------------
*/

MATRIX3D
TranslateMatrix(const float dx, const float dy, const float dz) {
	MATRIX3D ret = IdentityMatrix();
	ret(3, 0) = dx;
	ret(3, 1) = dy;
	ret(3, 2) = dz;
	return ret;
} // end TranslateMatrix

/*
**-----------------------------------------------------------------------------
**  Name:       TranslateMatrix
**  Purpose:    Returns matrix to translate by v
**-----------------------------------------------------------------------------
*/

MATRIX3D
TranslateMatrix(const Vect3f &v) {
	MATRIX3D ret = IdentityMatrix();
	ret(3, 0) = v[0];
	ret(3, 1) = v[1];
	ret(3, 2) = v[2];
	return ret;
} // end TranslateMatrix

/*
**-----------------------------------------------------------------------------
**  Name:       ScaleMatrix
**  Purpose:    scale matrix (uniform)
**-----------------------------------------------------------------------------
*/

MATRIX3D
ScaleMatrix(const float size) {
	MATRIX3D ret = IdentityMatrix();
	ret(0, 0) = size;
	ret(1, 1) = size;
	ret(2, 2) = size;
	return ret;
} // end ScaleMatrix

/*
**-----------------------------------------------------------------------------
**  Name:       ScaleMatrix
**  Purpose:    scale matrix
**-----------------------------------------------------------------------------
*/

MATRIX3D
ScaleMatrix(const float a, const float b, const float c) {
	MATRIX3D ret = IdentityMatrix();
	ret(0, 0) = a;
	ret(1, 1) = b;
	ret(2, 2) = c;
	return ret;
} // end ScaleMatrix

/*
**-----------------------------------------------------------------------------
**  Name:       ScaleMatrix
**  Purpose:    scale matrix
**-----------------------------------------------------------------------------
*/

MATRIX3D
ScaleMatrix(const Vect3f &v) {
	MATRIX3D ret = IdentityMatrix();
	ret(0, 0) = v.x;
	ret(1, 1) = v.y;
	ret(2, 2) = v.z;
	return ret;
} // end ScaleMatrix

/*
**-----------------------------------------------------------------------------
**  Name:       MatrixMult
**  Purpose:    [C] = [A] * [B]
**-----------------------------------------------------------------------------
*/

MATRIX3D
MatrixMult(const MATRIX3D &a, const MATRIX3D &b) {
	MATRIX3D ret = ZeroMatrix();

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < 4; k++) {
				ret(i, j) += a(k, j) * b(i, k);
			}
		}
	}
	return ret;
} // end MatrixMult

/*
**-----------------------------------------------------------------------------
**  Name:       TransformVector
**  Purpose:    V' = V * [M]
**-----------------------------------------------------------------------------
*/

Vect3f
TransformVector(const Vect3f &v, const MATRIX3D &m) {
	float   hvec[4] = {0.f};
	for (int i = 0; i < 4; i++) {
		hvec[i] = 0.0f;
//		for (int j=0; j<3; j++) {
//				hvec[i] += v[j] * m(j, i);
//		}
//		hvec[i] += m(3, i);
		hvec[i] = v[0] * m(0, i)
		          + v[1] * m(1, i)
		          + v[2] * m(2, i)
		          +        m(3, i);
	}
	return Vect3f(hvec[0] / hvec[3], hvec[1] / hvec[3], hvec[2] / hvec[3]);
} // end TransformVector

/*
**-----------------------------------------------------------------------------
**  Name:       TransformNormal
**  Purpose:    N' = N * [M]
**-----------------------------------------------------------------------------
*/

Vect3f
TransformNormal(const Vect3f &v, const MATRIX3D &mat) {
	MATRIX3D    m;

	m = MatrixInverse(mat);
	m = MatrixTranspose(m);
	return TransformVector(v, m);
}  // end TransformNormal

/*
**-----------------------------------------------------------------------------
**  Name:       MatrixInverse
**  Purpose:    Creates the inverse of a 4x4 matrix
**-----------------------------------------------------------------------------
*/

static void lubksb(MATRIX3D &a, int *indx, float *b);
static void ludcmp(MATRIX3D &a, int *indx, float *d);

MATRIX3D
MatrixInverse(const MATRIX3D &m) {
	MATRIX3D    n, y;
	int         i, j, indx[4];
	float       d, col[4];

	n = m;
	ludcmp(n, indx, &d);

	for (j = 0; j < 4; j++) {
		for (i = 0; i < 4; i++) {
			col[i] = 0.0f;
		}
		col[j] = 1.0f;
		lubksb(n, indx, col);
		for (i = 0; i < 4; i++) {
			y(i, j) = col[i];
		}
	}
	return y;
} // end MatrixInverse

/*
**-----------------------------------------------------------------------------
**  Name:       lubksb
**  Purpose:    backward subsitution
**-----------------------------------------------------------------------------
*/

static void
lubksb(MATRIX3D &a, int *indx, float *b) {
	int     i, j, ii = -1, ip;
	float   sum;

	for (i = 0; i < 4; i++) {
		ip = indx[i];
		sum = b[ip];
		b[ip] = b[i];
		if (ii >= 0) {
			for (j = ii; j <= i - 1; j++) {
				sum -= a(i, j) * b[j];
			}
		} else if (sum != 0.0) {
			ii = i;
		}
		b[i] = sum;
	}
	for (i = 3; i >= 0; i--) {
		sum = b[i];
		for (j = i + 1; j < 4; j++) {
			sum -= a(i, j) * b[j];
		}
		b[i] = sum / a(i, i);
	}
} // end lubksb

/*
**-----------------------------------------------------------------------------
**  Name:       ludcmp
**  Purpose:    LU decomposition
**-----------------------------------------------------------------------------
*/

static void
ludcmp(MATRIX3D &a, int *indx, float *d) {
	float   vv[4];               /* implicit scale for each row */
	float   big, dum, sum, tmp;
	int     i, imax = 0, j, k;

	*d = 1.0f;
	for (i = 0; i < 4; i++) {
		big = 0.0f;
		for (j = 0; j < 4; j++) {
			if ((tmp = (float) fabs(a(i, j))) > big) {
				big = tmp;
			}
		}
		/*
		if (big == 0.0f) {
		    printf("ludcmp(): singular matrix found...\n");
		    exit(1);
		}
		*/
		vv[i] = 1.0f / big;
	}
	for (j = 0; j < 4; j++) {
		for (i = 0; i < j; i++) {
			sum = a(i, j);
			for (k = 0; k < i; k++) {
				sum -= a(i, k) * a(k, j);
			}
			a(i, j) = sum;
		}
		big = 0.0f;
		for (i = j; i < 4; i++) {
			sum = a(i, j);
			for (k = 0; k < j; k++) {
				sum -= a(i, k) * a(k, j);
			}
			a(i, j) = sum;
			if ((dum = vv[i] * (float)fabs(sum)) >= big) {
				big = dum;
				imax = i;
			}
		}
		if (j != imax) {
			for (k = 0; k < 4; k++) {
				dum = a(imax, k);
				a(imax, k) = a(j, k);
				a(j, k) = dum;
			}
			*d = -(*d);
			vv[imax] = vv[j];
		}
		indx[j] = imax;
		if (a(j, j) == 0.0f) {
			a(j, j) = 1.0e-20f;      /* can be 0.0 also... */
		}
		if (j != 3) {
			dum = 1.0f / a(j, j);
			for (i = j + 1; i < 4; i++) {
				a(i, j) *= dum;
			}
		}
	}
} // end ludcmp

/*
**-----------------------------------------------------------------------------
**  Name:       Matrix Transpose
**  Purpose:    [M] = [M]'
**-----------------------------------------------------------------------------
*/

MATRIX3D
MatrixTranspose(const MATRIX3D &m) {
	MATRIX3D    ret;
	int         i, j;

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			ret(i, j) = m(j, i);
		}
	}

	return ret;
} // end MatrixTranspose

/*
    Class Methods
*/

/*
**-----------------------------------------------------------------------------
**  end of File
**-----------------------------------------------------------------------------
*/


} // namespace QDEngine
