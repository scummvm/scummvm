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
	 * Copy from the specified source pose
	 */
	void copyFrom(const FPose &src);

	/**
	 * Copy from the specified source matrix
	 */
	void copyFrom(const FMatrix &src);

	FPose fn4() const;
};

} // End of namespace Titanic

#endif /* TITANIC_FPOSE_H */
