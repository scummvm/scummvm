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

#include "titanic/support/simple_file.h"
#include "titanic/star_control/fvector.h"

namespace Titanic {

class DMatrix;

class FMatrix {
private:
	FVector _row1;
	FVector _row2;
	FVector _row3;
private:
	/**
	 * Copys data from a given source
	 */
	void copyFrom(const DMatrix *src);
public:
	FMatrix();
	FMatrix(DMatrix *src);

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
	 * Sets the data for the matrix
	 */
	void set(FVector *row1, FVector *row2, FVector *row3);
};

} // End of namespace Titanic

#endif /* TITANIC_MATRIX3_H */
