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

#ifndef TITANIC_DAFFINE_H
#define TITANIC_DAFFINE_H

#include "titanic/star_control/dvector.h"
#include "titanic/star_control/fvector.h"

namespace Titanic {

class FMatrix;
class CMatrixTransform;

/**
 * Affine transformation.
 *
 * Handles transformation functions between affine spaces,
 * which preserves points, straight lines and planes
 */
class DAffine {
private:
	static DAffine *_static;
public:
	DVector _col1;
	DVector _col2;
	DVector _col3;
	DVector _col4;
public:
	DAffine();
	// TODO: consider making mode an enum since that is more helpful when it is used in code
	DAffine(int mode, const DVector &src);
	DAffine(Axis axis, double angleDeg);
	DAffine(const FMatrix &src);

	/**
	 * Sets all elements to zero
	 */
	void clear();

	/**
	 * Sets up an affine matrix for rotating on a given axis by an amount in degrees
	 */
	void setRotationMatrix(Axis axis, double angleDeg);

	/**
	 * Return the Inverse of this Daffine
	 */
	DAffine inverseTransform() const;

	/**
	 * Change this Daffine to have its first three columns be some mapping from src matrix
	 * and the 4rth column to be (three) zeros. The mapping is not as simple as replacing
	 * matching row/colmn indices
	 */
	void loadTransform(const CMatrixTransform &src);

	/**
	 * Do the affine product between this Daffine on the right
	 * and the m Daffine matrix on the left. This product is NOT the same
	 * as multiplying two matrices of dimensions 3x4.
	 */
	DAffine compose(const DAffine &m);
};

} // End of namespace Titanic

#endif /* TITANIC_DAFFINE_H */
