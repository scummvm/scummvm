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

#ifndef TITANIC_STAR_CONTROL_SUB6_H
#define TITANIC_STAR_CONTROL_SUB6_H

#include "titanic/star_control/fmatrix.h"

namespace Titanic {

class CStarControlSub6 : public FMatrix {
private:
	static CStarControlSub6 *_static;
public:
	static void init();
	static void deinit();
public:
	FVector _vector;
public:
	CStarControlSub6();
	CStarControlSub6(Axis axis, double amount);
	CStarControlSub6(const CStarControlSub6 *src);

	/**
	 * Sets an identity matrix
	 */
	void identity();

	/**
	 * Sets up a passed instance from the specified two other ones
	 */
	static CStarControlSub6 *setup(CStarControlSub6 *dest, const CStarControlSub6 *s2, const CStarControlSub6 *s3);

	/**
	 * Sets a rotation matrix for the given axis for the given amount
	 */
	void setRotationMatrix(Axis axis, double val);

	void copyFrom(const CStarControlSub6 *src);

	/**
	 * Copy from the specified matrix
	 */
	void copyFrom(const FMatrix &src);

	void fn1(CStarControlSub6 *sub6);
};

} // End of namespace Titanic

#endif /* TITANIC_STAR_CONTROL_SUB6_H */
