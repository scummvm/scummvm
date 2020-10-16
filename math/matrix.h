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

#ifndef MATH_MATRIX_H
#define MATH_MATRIX_H

#include <string.h>
#include <assert.h>

#include "common/streamdebug.h"

/**
 * \namespace Math.
 * This namespace contains some useful classes dealing with math and geometry.
 *
 * The most important classes are Matrix and its base classes.
 * MatrixBase is a template class which is the base of all the matrices with
 *            many convenient functions.
 * MatrixType is an intermediate class that, using template specialization,
 *            is able to create different kinds of matrices, like vectors or
 *            square matrices.
 * Matrix     is the actual matrix class and it is derived from MatrixType.
 *
 * MatrixBase and MatrixType have their constructors protected, so they can't
 * be instantiated. But while MatrixBase is just a backend class, MatrixType
 * can be used to create new kinds of matrices:
 * \code
 template<int dim>
 class MatrixType<1, dim> : public MatrixBase<1, dim> {
 ...
 };
 * \endcode
 * Given that declaration, every Matrix<1, dim>, with "dim" whatever positive
 * number, will have the methods and members defined in MatrixType<1, dim>.
 *
 * This design allows us to have the equality of, say, the class "three-dimensional
 * vector" and Matrix<3, 1>. Vector3d is not <b>a</b> Matrix<3, 1>, it <b>is</b> Matrix<3, 1>.
 * Every method in MatrixBase and MatrixType returning a matrix returns a Matrix<\r, c>,
 * and not a MatrixBase<\r, c>. This reduces code duplication, since otherwise many
 * functions declared for Matrix would need to be declared for MatrixBase too,
 * like many operators.
 */
namespace Math {

template<int rows, int cols> class Matrix;

/**
 * \class MatrixBase
 * The base class for all the matrices.
 */
template<int rows, int cols>
class MatrixBase {
public:
	/**
	 * Convenient class for feeding a matrix.
	 */
	class Row {
	public:
		Row &operator=(const Row &r);
		Row &operator<<(float value);

	private:
		Row(MatrixBase<rows, cols> *m, int row);

		MatrixBase<rows, cols> *_matrix;
		int _row;
		int _col;

		friend class MatrixBase<rows, cols>;
	};

	/**
	 * Returns true if this matrix's values are all 0.
	 */
	bool isZero() const;
	Matrix<rows, cols> getNegative() const;

	/**
	 * Returns an instance of Row for a particular row of this matrix.
	 * Row is a convenient class for feeding a matrix.
	 * \code
	 Matrix<3, 3> m;
	 m.getRow(0) << 0 << 0   << 0;
	 m.getRow(1) << 1 << 2   << 0;
	 m.getRow(2) << 0 << 0.5 << 1;
	 * \endcode
	 *
	 * \param row The row to be feeded.
	 */
	Row getRow(int row);

	/**
	 * Returns a pointer to the internal data of this matrix.
	 */
	inline float *getData();
	/**
	 * Returns a pointer to the internal data of this matrix.
	 */
	inline const float *getData() const;
	/**
	 * Sets the internal data of this matrix.
	 */
	void setData(const float *data);
	inline float getValue(int row, int col) const;
	inline void setValue(int row, int col, float value);

	inline float &operator()(int row, int col);
	inline float operator()(int row, int col) const;

	inline operator const Matrix<rows, cols>&() const { return getThis(); }
	inline operator Matrix<rows, cols>&() { return getThis(); }

	static Matrix<rows, cols> sum(const Matrix<rows, cols> &m1, const Matrix<rows, cols> &m2);
	static Matrix<rows, cols> difference(const Matrix<rows, cols> &m1, const Matrix<rows, cols> &m2);
	static Matrix<rows, cols> product(const Matrix<rows, cols> &m1, float factor);
	static Matrix<rows, cols> quotient(const Matrix<rows, cols> &m1, float factor);

	Matrix<rows, cols> &operator=(const Matrix<rows, cols> &m);
	Matrix<rows, cols> &operator+=(const Matrix<rows, cols> &m);
	Matrix<rows, cols> &operator-=(const Matrix<rows, cols> &m);
	Matrix<rows, cols> &operator*=(float factor);
	Matrix<rows, cols> &operator/=(float factor);

protected:
	MatrixBase();
	MatrixBase(const float *data);
	MatrixBase(const MatrixBase<rows, cols> &m);

	inline const Matrix<rows, cols> &getThis() const {
		return *static_cast<const Matrix<rows, cols> *>(this); }
	inline Matrix<rows, cols> &getThis() {
		return *static_cast<Matrix<rows, cols> *>(this); }

private:
	float _values[rows * cols];
};

/**
 * \class MatrixType
 * MatrixType is a class used to create different kinds of matrices.
 */
template<int r, int c>
class MatrixType : public MatrixBase<r, c> {
protected:
	MatrixType() : MatrixBase<r, c>() { }
	MatrixType(const float *data) : MatrixBase<r, c>(data) { }
	MatrixType(const MatrixBase<r, c> &m) : MatrixBase<r, c>(m) { }
};

#define Vector(dim) Matrix<dim, 1>

/**
 * \class Matrix The actual Matrix class.
 * This template class must be instantiated passing it the number of the rows
 * and the number of the columns.
 */
template<int r, int c>
class Matrix : public MatrixType<r, c> {
public:
	Matrix() : MatrixType<r, c>() { }
	Matrix(const float *data) : MatrixType<r, c>(data) { }
	Matrix(const MatrixBase<r, c> &m) : MatrixType<r, c>(m) { }
};


template <int m, int n, int p>
Matrix<m, n> operator*(const Matrix<m, p> &m1, const Matrix<p, n> &m2);

template <int r, int c>
inline Matrix<r, c> operator+(const Matrix<r, c> &m1, const Matrix<r, c> &m2);

template <int r, int c>
inline Matrix<r, c> operator-(const Matrix<r, c> &m1, const Matrix<r, c> &m2);

template <int r, int c>
inline Matrix<r, c> operator*(const Matrix<r, c> &m1, float factor);

template <int r, int c>
inline Matrix<r, c> operator/(const Matrix<r, c> &m1, float factor);

template <int r, int c>
Matrix<r, c> operator*(float factor, const Matrix<r, c> &m1);

template <int r, int c>
Matrix<r, c> operator-(const Matrix<r, c> &m);

template <int r, int c>
bool operator==(const Matrix<r, c> &m1, const Matrix<r, c> &m2);

template <int r, int c>
bool operator!=(const Matrix<r, c> &m1, const Matrix<r, c> &m2);


// Constructors
template<int rows, int cols>
MatrixBase<rows, cols>::MatrixBase() {
	for (int i = 0; i < rows * cols; ++i) {
		_values[i] = 0.f;
	}
}

template<int rows, int cols>
MatrixBase<rows, cols>::MatrixBase(const float *data) {
	setData(data);
}

template<int rows, int cols>
MatrixBase<rows, cols>::MatrixBase(const MatrixBase<rows, cols> &m) {
	setData(m._values);
}




// Data management
template<int rows, int cols>
float *MatrixBase<rows, cols>::getData() {
	return _values;
}

template<int rows, int cols>
const float *MatrixBase<rows, cols>::getData() const {
	return _values;
}

template<int rows, int cols>
void MatrixBase<rows, cols>::setData(const float *data) {
	::memcpy(_values, data, rows * cols * sizeof(float));
}

template<int rows, int cols>
float MatrixBase<rows, cols>::getValue(int row, int col) const {
	assert(rows > row && cols > col && row >= 0 && col >= 0);
	return _values[row * cols + col];
}

template<int rows, int cols>
void MatrixBase<rows, cols>::setValue(int row, int col, float v) {
	operator()(row, col) = v;
}



// Operations helpers
template<int rows, int cols>
bool MatrixBase<rows, cols>::isZero() const {
	for (int i = 0; i < rows * cols; ++i) {
		if (_values[i] != 0.f) {
			return false;
		}
	}
	return true;
}

template <int r, int c>
Matrix<r, c> MatrixBase<r, c>::getNegative() const {
	Matrix<r, c> result;
	for (int i = 0; i < r * c; ++i) {
		result._values[i] = -_values[i];
	}
	return result;
}

template <int r, int c>
Matrix<r, c> MatrixBase<r, c>::sum(const Matrix<r, c> &m1, const Matrix<r, c> &m2) {
	Matrix<r, c> result;
	for (int i = 0; i < r * c; ++i) {
		result._values[i] = m1._values[i] + m2._values[i];
	}
	return result;
}

template <int r, int c>
Matrix<r, c> MatrixBase<r, c>::difference(const Matrix<r, c> &m1, const Matrix<r, c> &m2) {
	Matrix<r, c> result;
	for (int i = 0; i < r * c; ++i) {
		result._values[i] = m1._values[i] - m2._values[i];
	}
	return result;
}

template <int r, int c>
Matrix<r, c> MatrixBase<r, c>::product(const Matrix<r, c> &m1, float factor) {
	Matrix<r, c> result;
	for (int i = 0; i < r * c; ++i) {
		result._values[i] = m1._values[i] * factor;
	}
	return result;
}

template <int r, int c>
Matrix<r, c> MatrixBase<r, c>::quotient(const Matrix<r, c> &m1, float factor) {
	Matrix<r, c> result;
	for (int i = 0; i < r * c; ++i) {
		result._values[i] = m1._values[i] / factor;
	}
	return result;
}




// Member operators
template<int rows, int cols>
float &MatrixBase<rows, cols>::operator()(int row, int col) {
	assert(rows > row && cols > col && row >= 0 && col >= 0);
	return _values[row * cols + col];
}

template<int rows, int cols>
float MatrixBase<rows, cols>::operator()(int row, int col) const {
	return getValue(row, col);
}

template<int rows, int cols>
Matrix<rows, cols> &MatrixBase<rows, cols>::operator=(const Matrix<rows, cols> &m) {
	setData(m._values);

	return getThis();
}

template<int rows, int cols>
Matrix<rows, cols> &MatrixBase<rows, cols>::operator+=(const Matrix<rows, cols> &m) {
	for (int i = 0; i < rows * cols; ++i) {
		_values[i] += m._values[i];
	}

	return getThis();
}

template<int rows, int cols>
Matrix<rows, cols> &MatrixBase<rows, cols>::operator-=(const Matrix<rows, cols> &m) {
	for (int i = 0; i < rows * cols; ++i) {
		_values[i] -= m._values[i];
	}

	return getThis();
}

template<int rows, int cols>
Matrix<rows, cols> &MatrixBase<rows, cols>::operator*=(float factor) {
	for (int i = 0; i < rows * cols; ++i) {
		_values[i] *= factor;
	}

	return getThis();
}

template<int rows, int cols>
Matrix<rows, cols> &MatrixBase<rows, cols>::operator/=(float factor) {
	for (int i = 0; i < rows * cols; ++i) {
		_values[i] /= factor;
	}

	return getThis();
}



// Row
template<int rows, int cols>
typename MatrixBase<rows, cols>::Row MatrixBase<rows, cols>::getRow(int row) {
	return Row(this, row);
}

template<int rows, int cols>
MatrixBase<rows, cols>::Row::Row(MatrixBase<rows, cols> *m, int row) :
	_matrix(m), _row(row), _col(0) {

}

template<int rows, int cols>
typename MatrixBase<rows, cols>::Row &MatrixBase<rows, cols>::Row::operator=(const Row &r) {
	_col = r._col;
	_row = r._row;
	_matrix = r._matrix;

	return *this;
}

template<int rows, int cols>
typename MatrixBase<rows, cols>::Row &MatrixBase<rows, cols>::Row::operator<<(float value) {
	assert(_col < cols);
	_matrix->setValue(_row, _col++, value);
	return *this;
}



// Global operators
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

template <int r, int c>
inline Matrix<r, c> operator+(const Matrix<r, c> &m1, const Matrix<r, c> &m2) {
	return Matrix<r, c>::sum(m1, m2);
}

template <int r, int c>
inline Matrix<r, c> operator-(const Matrix<r, c> &m1, const Matrix<r, c> &m2) {
	return Matrix<r, c>::difference(m1, m2);
}

template <int r, int c>
inline Matrix<r, c> operator*(const Matrix<r, c> &m1, float factor) {
	return Matrix<r, c>::product(m1, factor);
}

template <int r, int c>
inline Matrix<r, c> operator/(const Matrix<r, c> &m1, float factor) {
	return Matrix<r, c>::quotient(m1, factor);
}

template <int r, int c>
Matrix<r, c> operator*(float factor, const Matrix<r, c> &m1) {
	return Matrix<r, c>::product(m1, factor);
}

template <int r, int c>
Matrix<r, c> operator-(const Matrix<r, c> &m) {
	return m.getNegative();
}

template <int r, int c>
bool operator==(const Matrix<r, c> &m1, const Matrix<r, c> &m2) {
	for (int row = 0; row < r; ++row) {
		for (int col = 0; col < c; ++col) {
			if (m1(row, col) != m2(row, col)) {
				return false;
			}
		}
	}
	return true;
}

template <int r, int c>
bool operator!=(const Matrix<r, c> &m1, const Matrix<r, c> &m2) {
	return !(m1 == m2);
}

template<int r, int c>
Common::StreamDebug &operator<<(Common::StreamDebug dbg, const Math::Matrix<r, c> &m) {
	dbg.nospace() << "Matrix<" << r << ", " << c << ">(";
	for (int col = 0; col < c; ++col) {
		dbg << m(0, col) << ", ";
	}
	for (int row = 1; row < r; ++row) {
		dbg << "\n            ";
		for (int col = 0; col < c; ++col) {
			dbg << m(row, col) << ", ";
		}
	}
	dbg << ')';

	return dbg.space();
}

}

#endif

