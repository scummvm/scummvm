/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 */

#ifndef MATH_VECTOR_H
#define MATH_VECTOR_H

#include "math/matrix.h"
#include "math/utils.h"

namespace Math {

/**
 * \class MatrixType<dim, 1>
 * This MatrixType specialization defines new methods for the vectors.
 */
template<int dim>
class MatrixType<dim, 1> : public MatrixBase<dim, 1> {
public:
	void normalize();
	Vector(dim) getNormalized() const;
	float getMagnitude() const;
	float getDistanceTo(const Vector(dim) &point) const;
	float getDotProduct(const Vector(dim) &v) const;

	inline void setValue(int i, float val) { value(i) = val; }
	inline float getValue(int i) const { return value(i); }

protected:
	MatrixType() : MatrixBase<dim, 1>() { }
	MatrixType(float *data) : MatrixBase<dim, 1>(data) { }
	MatrixType(const MatrixBase<dim, 1> &m) : MatrixBase<dim, 1>(m) { }

	inline float &value(int i) { return this->operator()(i, 0); }
	inline float value(int i) const { return this->operator()(i, 0); }
};


template<int dim>
void MatrixType<dim, 1>::normalize() {
	float mag = getMagnitude();
	for (int i = 0; i < dim; ++i) {
		this->operator()(i, 0) /= mag;
	}
}

template<int dim>
Vector(dim) MatrixType<dim, 1>::getNormalized() const {
	Vector(dim) v(*this);
	v.normalize();
	return v;
}

template<int dim>
float MatrixType<dim, 1>::getMagnitude() const {
	float mag = 0;
	for (int i = 0; i < dim; ++i) {
		mag += square(getValue(i));
	}
	return sqrt(mag);
}

template<int dim>
float MatrixType<dim, 1>::getDistanceTo(const Vector(dim) &point) const {
	float sum = 0;
	for (int i = 0; i < dim; ++i) {
		sum += square(getValue(i) - point.getValue(i));
	}
	return sqrt(sum);
}

template<int dim>
float MatrixType<dim, 1>::getDotProduct(const Vector(dim) &v) const {
	float result = 0;
	for (int i = 0; i < dim; ++i) {
		result += value(i) * v.value(i);
	}
	return result;
}

template<int dim>
inline float dot(const Vector(dim) &v1, const Vector(dim) &v2) {
	return v1.getDotProduct(v2);
}

}

template<int dim>
Common::Debug &operator<<(Common::Debug dbg, const Math::Matrix<dim, 1> &v) {
	dbg.nospace() << "Vector<" << dim << ">(" << v.getValue(0);
	for (int i = 1; i < dim; ++i) {
		dbg << ", " << v.getValue(i);
	}
	dbg << ")";

	return dbg.space();
}

#endif
