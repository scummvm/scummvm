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

#ifndef TITANIC_FPOSE_H
#define TITANIC_FPOSE_H

#include "titanic/star_control/fmatrix.h"

namespace Titanic {

class CMatrixTransform;

/*
 * This class combines a position and orientation in 3D space
 */
class FPose : public FMatrix {
public:
	FVector _vector;
public:
	FPose();
	FPose(Axis axis, float amount);
	FPose(const FPose &src);
	FPose(int mode, const FVector &src);
	/**
	 * This fpose is the fpose product of s1 (on the left) and s2 (on the right)
	 */
	FPose(const FPose &s1, const FPose &s2);

	/**
	 * Sets an identity matrix
	 */
	void identity();

	/**
	 * Sets a rotation matrix for the given axis for the given amount
	 */
	void setRotationMatrix(Axis axis, float val);

	/**
	 * Rotate this FPose about the Y axis
	 */
	void rotVectAxisY(double angleDeg);
	/**
	 * Copy from the specified source pose
	 */
	void copyFrom(const FPose &src);

	/**
	 * Copy from the specified source matrix
	 */
	void copyFrom(const FMatrix &src);

	/**
	 * Change this Daffine to have its first three columns be some mapping from src matrix
	 * and the 4rth column to be (three) zeros. The mapping is not as simple as replacing
	 * matching row/colmn indices
	 */
	void loadTransform(const CMatrixTransform &src);

	/**
	 * The inverse of rotation and the position vector
	 */
	FPose inverseTransform() const;

	/**
	 * Multiplication between this FPose (4x3) and a FMatrix (3x3)
	 * This is done by making the matrix be a FPose with a last row
	 * of zeros
	 */
	FPose compose(const FMatrix &m);

	/**
	 * Multiplication between this FPose (4x3) and another FPose
	 * This FPose is on the left and m is on the right.
	 * The last row of m is added to the output component wise
	 */
	FPose compose2(const FPose &m);
};

/**
 * Puts the fpose product between a and m in C, C = am
 * Caller must preallocate output matrix
 * Similar to matProd
 */
void fposeProd(const FPose &a, const FPose &m, FPose &C);

} // End of namespace Titanic

#endif /* TITANIC_FPOSE_H */
