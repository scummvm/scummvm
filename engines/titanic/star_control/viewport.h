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

#ifndef TITANIC_VIEWPORT_H
#define TITANIC_VIEWPORT_H

#include "titanic/support/simple_file.h"
#include "titanic/star_control/base_stars.h"
#include "titanic/star_control/fpose.h"
#include "titanic/star_control/fmatrix.h"

namespace Titanic {

/**
 * Implements the viewport functionality for viewing the star field in
 * a given position and orientation
 */
class CViewport {
private:
	double _fieldC;
	double _field18;
	double _field1C;
	int _width;
	int _height;
	FMatrix _orientation;
	FPose _currentPose;
	FPose _rawPose;
	FPoint _center;
	bool _flag;
private:
	void reset();
public:
	FVector _position;
	double _field10;
	double _field14;
	int _field24;
	double _valArray[5];
	FVector _centerVector;
public:
	CViewport();
	CViewport(CViewport *src);

	/**
	 * Copys the data from another instance
	 */
	void copyFrom(const CViewport *src);

	/**
	 * Load the data for the class from file
	 */
	void load(SimpleFile *file, int param);

	/**
	 * Save the data for the class to file
	 */
	void save(SimpleFile *file, int indent);

	/**
	 * Sets the position
	 */
	void setPosition(const FVector &v);

	/**
	 * Sets the position
	 */
	void setPosition(const FPose &pose);

	/**
	 * Sets the orientation from a passed matrix
	 */
	void setOrientation(const FMatrix &m);

	/**
	 * Sets the orientation from a passed vector
	 */
	void setOrientation(const FVector &v);

	void fn12();
	void fn13(StarMode mode, double val);
	void reposition(double factor);
	void fn15(const FMatrix &matrix);
	FPose getPose();
	FPose getRawPose();
	FVector fn16(int index, const FVector &src);
	FVector fn17(int index, const FVector &src);
	FVector fn18(int index, const FVector &src);
	void fn19(double *v1, double *v2, double *v3, double *v4);

	/**
	 * Returns the viewport's orientation
	 */
	const FMatrix &getOrientation() const;

	void setC(double v);
	void set10(double v);
	void set14(double v);
	void set18(double v);
	void set1C(double v);
};

} // End of namespace Titanic

#endif /* TITANIC_VIEWPORT_H */
