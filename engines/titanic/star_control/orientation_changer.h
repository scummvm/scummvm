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

#ifndef TITANIC_ORIENTATION_CHANGER_H
#define TITANIC_ORIENTATION_CHANGER_H

#include "titanic/star_control/fmatrix.h"
#include "titanic/star_control/matrix_transform.h"

namespace Titanic {

class COrientationChanger {
public:
	FMatrix _minOrient;
	FMatrix _maxOrient;
	CMatrixTransform _sub1;
	CMatrixTransform _sub2;
public:
	/**
	 * Loads the constraints for the minimum and maximum orientation
	 */
	void load(const FMatrix &minOrient, const FMatrix &maxOrient);

	/**
	 * Returns the orientation for a given percentage between the two
	 * extremes
	 * @param percent	Percentage transition 0.0 to 1.0
	 * @returns		New orientation for the given percentage between the two
	 */
	FMatrix getOrientation(double percent);
};

} // End of namespace Titanic

#endif /* TITANIC_ORIENTATION_CHANGER_H */
