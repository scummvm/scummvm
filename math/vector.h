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

#ifndef MATH_VECTOR_H
#define MATH_VECTOR_H

#include "common/stream.h"

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
	float getSquareMagnitude() const;
	float getDistanceTo(const Vector(dim) &point) const;
	float dotProduct(const Vector(dim) &v) const;

	inline void setValue(int i, float val) { value(i) = val; }
	inline float getValue(int i) const { return value(i); }

	template<int d>
	inline static float dotProduct(const Vector(d) &v1, const Vector(d) &v2) {
		return v1.dotProduct(v2);
	}

	/**
	 * Reads <i>dim</i> floats from the passed stream, and uses them
	 * as value 0...dim in chronological order.
	 */
	void readFromStream(Common::ReadStream *stream);

protected:
	MatrixType() : MatrixBase<dim, 1>() { }
	MatrixType(const float *data) : MatrixBase<dim, 1>(data) { }
	MatrixType(const MatrixBase<dim, 1> &m) : MatrixBase<dim, 1>(m) { }

	inline float &value(int i) { return this->operator()(i, 0); }
	inline float value(int i) const { return this->operator()(i, 0); }
};


template<int dim>
void MatrixType<dim, 1>::normalize() {
	float mag = getMagnitude();
	if (mag > 0.f) {
		for (int i = 0; i < dim; ++i) {
			this->operator()(i, 0) /= mag;
		}
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
	return sqrt(getSquareMagnitude());
}

template<int dim>
float MatrixType<dim, 1>::getSquareMagnitude() const {
	float mag = 0;
	for (int i = 0; i < dim; ++i) {
		mag += square(getValue(i));
	}
	return mag;
}

template<int dim>
float MatrixType<dim, 1>::getDistanceTo(const Vector(dim) &point) const {
	float result = 0;
	for (int i = 0; i < dim; ++i) {
		result += square(getValue(i) - point.getValue(i));
	}
	return sqrt(result);
}

template<int dim>
float MatrixType<dim, 1>::dotProduct(const Vector(dim) &v) const {
	float result = 0;
	for (int i = 0; i < dim; ++i) {
		result += value(i) * v.value(i);
	}
	return result;
}

template<int dim>
void MatrixType<dim, 1>::readFromStream(Common::ReadStream *stream) {
	for (int i = 0; i < dim; ++i) {
		setValue(i, stream->readFloatLE());
	}
}


template<int dim>
Common::StreamDebug &operator<<(Common::StreamDebug dbg, const Math::Matrix<dim, 1> &v) {
	dbg.nospace() << "Vector<" << dim << ">(" << v.getValue(0);
	for (int i = 1; i < dim; ++i) {
		dbg << ", " << v.getValue(i);
	}
	dbg << ")";

	return dbg.space();
}

}

#endif
