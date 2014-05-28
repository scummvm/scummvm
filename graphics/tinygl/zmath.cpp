
#include "common/scummsys.h"

#include "graphics/tinygl/zmath.h"

namespace TinyGL {

// Inversion of a general nxn matrix.
// Note : m is destroyed

int Matrix_Inv(float *r, float *m, int n) {
	int k;
	float max, tmp, t;

	// identitée dans r
	for (int i = 0; i < n * n; i++)
		r[i] = 0;
	for (int i = 0; i < n; i++)
		r[i * n + i] = 1;

	for (int j = 0; j < n; j++) {
		max = m[j * n + j];
		k = j;
		for (int i = j + 1; i < n; i++) {
			if (fabs(m[i * n + j]) > fabs(max)) {
				k = i;
				max = m[i * n + j];
			}
		}
		// non intersible matrix
		if (max == 0)
			return 1;

		if (k != j) {
			for (int i = 0; i < n; i++) {
				tmp = m[j * n + i];
				m[j * n + i] = m[k * n + i];
				m[k * n + i] = tmp;

				tmp = r[j * n + i];
				r[j * n + i] = r[k * n + i];
				r[k * n + i] = tmp;
			}
		}

		max = 1 / max;
		for (int i = 0; i < n; i++) {
			m[j * n + i] *= max;
			r[j * n + i] *= max;
		}


		for (int l = 0; l < n; l++) {
			if (l != j) {
				t = m[l * n + j];
				for (int i = 0; i < n; i++) {
					m[l * n + i] -= m[j * n + i] * t;
					r[l * n + i] -= r[j * n + i] * t;
				}
			}
		}
	}

	return 0;
}

Vector3::Vector3() {
	// Empty constructor, no overhead
}

Vector3::Vector3(const Vector3 &other) {
	memcpy(_v,other._v,sizeof(_v));
}

Vector3::Vector3(float x, float y, float z) {
	_v[0] = x;
	_v[1] = y;
	_v[2] = z;
}

void Vector3::normalize() {
	float n;
	n = sqrt(_v[0] * _v[0] + _v[1] * _v[1] + _v[2] * _v[2]);
	if (n != 0) {
		_v[0] /= n;
		_v[1] /= n;
		_v[2] /= n;
	}
}

Vector4::Vector4() {
	// Empty constructor, no overhead
}


Vector4::Vector4(float x, float y, float z, float w) {
	_v[0] = x;
	_v[1] = y;
	_v[2] = z;
	_v[3] = w;
}

Vector4::Vector4(const Vector3 &vec, float w) {
	_v[0] = vec.getX();
	_v[1] = vec.getY();
	_v[2] = vec.getZ();
	_v[3] = w;
}

Matrix4::Matrix4() {
	// Empty constructor, no overhead
}

Matrix4::Matrix4(const Matrix4 &other) {
	memcpy(_m,other._m,sizeof(_m));
}

TinyGL::Matrix4 Matrix4::identity() {
	Matrix4 a;
	a.fill(0);
	a.set(0,0,1.0f);
	a.set(1,1,1.0f);
	a.set(2,2,1.0f);
	a.set(3,3,1.0f);
	return a;
}

TinyGL::Matrix4 Matrix4::transpose() const {
	Matrix4 a;

	a.set(0,0, this->get(0,0));
	a.set(0,1, this->get(1,0));
	a.set(0,2, this->get(2,0));
	a.set(0,3, this->get(3,0));

	a.set(1,0, this->get(0,1));
	a.set(1,1, this->get(1,1));
	a.set(1,2, this->get(2,1));
	a.set(1,3, this->get(3,1));

	a.set(2,0, this->get(0,2));
	a.set(2,1, this->get(1,2));
	a.set(2,2, this->get(2,2));
	a.set(2,3, this->get(3,2));

	a.set(3,0, this->get(0,3));
	a.set(3,1, this->get(1,3));
	a.set(3,2, this->get(2,3));
	a.set(3,3, this->get(3,3));

	return a;
}

Matrix4 Matrix4::inverseOrtho() const {
	Matrix4 a;

	int i, j;
	float s;
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++)
			a.set(i,j, this->get(j,i));
		a.set(3,0, 0.0f);
		a.set(3,1, 0.0f);
		a.set(3,2, 0.0f);
		a.set(3,3, 1.0f);
		for (i = 0; i < 3; i++) {
			s = 0;
			for (j = 0; j < 3; j++)
				s -= this->get(j,i) * this->get(j,3);
			a.set(i,3,s);
		}
	}

	return a;
}

Matrix4 Matrix4::inverse() const {
	Matrix4 source(*this);
	int k;
	float max, tmp, t;

	// identitée dans r
	Matrix4 result = identity();

	for (int j = 0; j < 4; j++) {
		max = source.get(j,j);
		k = j;
		for (int i = j + 1; i < 4; i++) {
			if (fabs(source.get(i,j)) > fabs(max)) {
				k = i;
				max = source.get(i,j);
			}
		}
		// non intersible matrix
		if (max == 0)
			return result;

		if (k != j) {
			for (int i = 0; i < 4; i++) {
				tmp = source.get(j,i);
				source.set(j,i, source.get(k,i));
				source.set(k,i, tmp);

				tmp = result.get(j,i);
				result.set(j,i, source.get(k,i));
				result.set(k,i,tmp);
			}
		}

		max = 1 / max;
		for (int i = 0; i < 4; i++) {
			source.set(j,i, max * source.get(j,i));
			result.set(j,i, max * result.get(j,i));
		}

		for (int l = 0; l < 4; l++) {
			if (l != j) {
				t = source.get(l,j);
				for (int i = 0; i < 4; i++) {
					source.set(l,i, source.get(l,i) - t * source.get(j,i));
					result.set(l,i, result.get(l,i) - t * result.get(j,i));
				}
			}
		}
	}

	return result;
}

Matrix4 Matrix4::rotation(float t, int u) {
	Matrix4 a = identity();
	float s, c;
	int v, w;

	if ((v = u + 1) > 2)
		v = 0;
	if ((w = v + 1) > 2)
		w = 0;
	s = sin(t);
	c = cos(t);
	a.set(v,v,c);
	a.set(v,w,-s);
	a.set(w,v,s);
	a.set(w,w,c);

	return a;
}

Vector3 Matrix4::transform(const Vector3 &vector) const {
	return Vector3(
	           vector.getX() * get(0,0) + vector.getY() * get(0,1) + vector.getZ() * get(0,2) + get(0,3),
	           vector.getX() * get(1,0) + vector.getY() * get(1,1) + vector.getZ() * get(1,2) + get(1,3),
	           vector.getX() * get(2,0) + vector.getY() * get(2,1) + vector.getZ() * get(2,2) + get(2,3));
}

Vector3 Matrix4::transform3x3(const Vector3 &vector) const {
	return Vector3(
	           vector.getX() * get(0,0) + vector.getY() * get(0,1) + vector.getZ() * get(0,2),
	           vector.getX() * get(1,0) + vector.getY() * get(1,1) + vector.getZ() * get(1,2),
	           vector.getX() * get(2,0) + vector.getY() * get(2,1) + vector.getZ() * get(2,2));
}

Vector4 Matrix4::transform(const Vector4 &vector) const {
	return Vector4(
	           vector.getX() * get(0,0) + vector.getY() * get(0,1) + vector.getZ() * get(0,2) + vector.getW() * get(0,3),
	           vector.getX() * get(1,0) + vector.getY() * get(1,1) + vector.getZ() * get(1,2) + vector.getW() * get(1,3),
	           vector.getX() * get(2,0) + vector.getY() * get(2,1) + vector.getZ() * get(2,2) + vector.getW() * get(2,3),
	           vector.getX() * get(3,0) + vector.getY() * get(3,1) + vector.getZ() * get(3,2) + vector.getW() * get(3,3));
}

bool Matrix4::IsIdentity() const {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (i == j) {
				if (_m[i][j] != 1.0)
					return false;
			} else if (_m[i][j] != 0.0)
				return false;
		}
	}
	return true;
}

} // end of namespace TinyGL
