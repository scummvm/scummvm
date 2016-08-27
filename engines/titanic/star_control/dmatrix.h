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

#ifndef TITANIC_DMATRIX_H
#define TITANIC_DMATRIX_H

#include "titanic/star_control/dvector.h"
#include "titanic/star_control/fvector.h"

namespace Titanic {

class FMatrix;
class CStarControlSub26;

/**
 * Double based matrix class.
 * @remarks		TODO: See if it can be merged with FMatrix
 */
class DMatrix {
private:
	static DMatrix *_static;
public:
	DVector _row1;
	DVector _row2;
	DVector _row3;
	FVector _frow1;
	FVector _frow2;
public:
	static void init();
	static void deinit();
public:
	DMatrix();
	DMatrix(int mode, const FMatrix *src);
	DMatrix(int mode, double val);

	/**
	 * Sets up data for the matrix
	 */
	void set(int mode, double amount);

	void fn3(CStarControlSub26 *sub26);
};

} // End of namespace Titanic

#endif /* TITANIC_DMATRIX_H */
