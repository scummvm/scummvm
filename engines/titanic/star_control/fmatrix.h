/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
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

#ifndef TITANIC_FMATRIX_H
#define TITANIC_FMATRIX_H

#include "titanic/star_control/fvector.h"

namespace Titanic {

class SimpleFile;

/**
 * Floating point matrix class.

 */
class FMatrix {
public:
	FVector _row1;
	FVector _row2;
	FVector _row3;
public:
	FMatrix();
	FMatrix(const FVector &, const FVector &, const FVector &);
	FMatrix(const FMatrix &src);

	/**
	 * Load the data for the class from file
	 */
	void load(SimpleFile *file, int param);

	/**
	 * Save the data for the class to file
	 */
	void save(SimpleFile *file, int indent);

	/**
	 * Clears the matrix
	 */
	void clear();

	/**
	 * Sets up an identity matrix
	 */
	void identity();

	/**
	 * Sets the data for the matrix
	 */
	void set(const FMatrix &m);

	/**
	 * Sets the data for the matrix
	 */
	void set(const FVector &row1, const FVector &row2, const FVector &row3);

	/**
	 * Sets the data for the matrix from a vector
	 */
	void set(const FVector &v);

	/**
	 * Changes this matrix, A, to be C, where C=Am. Matrix m multiplies this matrix (A) on its Right.
	 * m is said to premultiply A (the previous this matrix).
	 */
	void matRProd(const FMatrix &m);

	/**
	 * Changes this matrix, A, to be C, where C=mA. Matrix m multiplies this matrix (A) on its Left.
	 * m is said to postmultiply A (the previous this matrix).
	 */
	void matLProd(const FMatrix &m);

	/**
	 * Returns true if the passed matrix equals this one
	 */
	bool operator==(const FMatrix &src) {
		return _row1 == src._row1 && _row2 == src._row2 && _row3 == src._row3;
	}

	/**
	 * Returns true if the passed matrix does not equal this one
	 */
	bool operator!=(const FMatrix &src) {
		return !operator==(src);
	}

	/**
	 * Allows accessing rows as an array
	 */
	FVector &operator[](int idx) {
		assert(idx >= 0 && idx <= 2);
		FVector *rows[3] = { &_row1, &_row2, &_row3 };
		return *rows[idx];
	}
};

/**
 * Puts the matrix product between a and m in C, C = am
 * Called by MatLProd and MatLProd
 * Caller must preallocate output matrix
 */
void matProd(const FMatrix &a, const FMatrix &m, FMatrix &C);

} // End of namespace Titanic

#endif /* TITANIC_FMATRIX_H */
