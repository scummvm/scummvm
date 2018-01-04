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

#ifndef TITANIC_MATRIX_TRANSFORM_H
#define TITANIC_MATRIX_TRANSFORM_H

#include "titanic/star_control/fvector.h"

namespace Titanic {

class FMatrix;

class CMatrixTransform {
private:
	double fn2(const CMatrixTransform &src);
	CMatrixTransform resize(double factor) const;
public:
	double _field0;
	FVector _vector;
public:
	CMatrixTransform() : _field0(1.0) {}

	/**
	 * Sets the field values
	 */
	void setup(double val1, double x, double y, double z);

	/**
	 * Copies from another instance
	 */
	void copyFrom(const CMatrixTransform &src);

	double fn1() const;
	void fn4(const FMatrix &m);
	CMatrixTransform fn5(double percent, const CMatrixTransform &src);
};

} // End of namespace Titanic

#endif /* TITANIC_MATRIX_TRANSFORM_H */
