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

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 */

#ifndef GRAPHICS_MATRIX_H
#define GRAPHICS_MATRIX_H

#include <assert.h>

namespace Graphics {

template<int rows, int cols>
class Matrix {
public:
	class Row {
	public:
		Row(Matrix<rows, cols> *m, int row);

		Row &operator=(const Row &r);
		Row &operator<<(float value);

	private:
		Matrix<rows, cols> *_matrix;
		int _row;
		int _col;
	};

	Matrix();
	Matrix(float *data);

	void setToIdentity();

	Row getRow(int row);

	float *getData() const;
	void setData(float *data);
	float getValue(int row, int col) const;
	void setValue(int row, int col, float value);

	float &operator()(int row, int col);
	float operator()(int row, int col) const;

	Matrix<rows, cols> &operator=(const Matrix<rows, cols> &m);
	Matrix<rows, cols> &operator*=(const Matrix<rows, cols> &m);

private:
	float _values[rows][cols];
};


template <int m, int n, int p>
Matrix<m, n> operator*(const Matrix<m, p> &m1, const Matrix<p, n> &m2);




template<int rows, int cols>
Matrix<rows, cols>::Matrix() {
	setToIdentity();
}

template<int rows, int cols>
Matrix<rows, cols>::Matrix(float *data) {
	setData(data);
}

template<int rows, int cols>
void Matrix<rows, cols>::setToIdentity() {
	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < cols; ++j) {
			setValue(i, j, (i == j ? 1.f : 0.f));
		}
	}
}

template<int rows, int cols>
typename Matrix<rows, cols>::Row Matrix<rows, cols>::getRow(int row) {
	return Row(this, row);
}

template<int rows, int cols>
float *Matrix<rows, cols>::getData() const {
	return _values[0];
}

template<int rows, int cols>
void Matrix<rows, cols>::setData(float *data) {
	for (int row = 0; row < rows; ++row) {
		for (int col = 0; col < cols; ++col) {
			setValue(row, col, data[row * rows + col]);
		}
	}
}

template<int rows, int cols>
float Matrix<rows, cols>::getValue(int row, int col) const {
	return _values[row][col];
}

template<int rows, int cols>
void Matrix<rows, cols>::setValue(int row, int col, float v) {
	operator()(row, col) = v;
}

template<int rows, int cols>
float &Matrix<rows, cols>::operator()(int row, int col) {
	return _values[row][col];
}

template<int rows, int cols>
float Matrix<rows, cols>::operator()(int row, int col) const {
	return getValue(row, col);
}

template<int rows, int cols>
Matrix<rows, cols> &Matrix<rows, cols>::operator=(const Matrix<rows, cols> &m) {
	for (int row = 0; row < rows; ++row) {
		for (int col = 0; col < cols; ++col) {
			setValue(row, col, m(row, col));
		}
	}

	return *this;
}

template<int rows, int cols>
Matrix<rows, cols> &Matrix<rows, cols>::operator*=(const Matrix<rows, cols> &m) {
	*this = *this * m;

	return *this;
}

template<int rows, int cols>
Matrix<rows, cols>::Row::Row(Matrix<rows, cols> *m, int row) :
_matrix(m), _row(row), _col(0) {

}

template<int rows, int cols>
typename Matrix<rows, cols>::Row &Matrix<rows, cols>::Row::operator=(const Row &r) {
	_col = r._col;
	_row = r._row;
	_matrix = r._matrix;

	return *this;
}

template<int rows, int cols>
typename Matrix<rows, cols>::Row &Matrix<rows, cols>::Row::operator<<(float value) {
	assert(_col < cols);
	_matrix->setValue(_row, _col++, value);
	return *this;
}


template <int m, int n, int p>
Matrix<m, n> operator*(const Matrix<m, p> &m1, const Matrix<p, n> &m2) {
	Matrix<m, n> result;
	for (int row = 0; row < m; ++row) {
		for (int col = 0; col < n; ++col) {
			float sum(0.0f);
			for (int j = 0; j < p; ++j)
				sum += m1(row, j) * m2(j, col);
			result(row, col) = sum;
		}
	}
	return result;
}

}

#endif

