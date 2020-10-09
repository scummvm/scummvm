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

#ifndef MATH_SQUAREMATRIX_H
#define MATH_SQUAREMATRIX_H

#include "math/matrix.h"
#include "math/vector.h"

namespace Math {

/**
 * \class MatrixType<dim, dim>
 * This specialization of MatrixType defines some new methods for square
 * matrices.
 */
template<int dim>
class MatrixType<dim, dim> : public MatrixBase<dim, dim> {
public:
	inline void setToIdentity() { *this = 1.f; }
	inline void transformVector(Vector(dim) *vec) const {
		*vec = this->getThis() * *vec;
	}

	Matrix<dim, dim> &operator=(float i);

protected:
	MatrixType() : MatrixBase<dim, dim>() { setToIdentity(); }
	MatrixType(float *data) : MatrixBase<dim, dim>(data) { }
	MatrixType(const MatrixBase<dim, dim> &m) : MatrixBase<dim, dim>(m) { }
};

template<int dim>
Matrix<dim, dim> &MatrixType<dim, dim>::operator=(float i) {
	for (int row = 0; row < dim; ++row) {
		for (int col = 0; col < dim; ++col) {
			this->setValue(row, col, (row == col ? i : 0.f));
		}
	}

	return this->getThis();
}

}

#endif
