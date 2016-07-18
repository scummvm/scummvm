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

#ifndef TITANIC_STAR_CONTROL_SUB13_H
#define TITANIC_STAR_CONTROL_SUB13_H

#include "titanic/support/simple_file.h"
#include "titanic/star_control/star_control_sub6.h"
#include "titanic/star_control/fmatrix.h"

namespace Titanic {

class CStarControlSub13 {
private:
	double _fieldC;
	double _field18;
	double _field1C;
	int _width;
	int _height;
	double _valArray[5];
	FMatrix _matrix;
	CStarControlSub6 _sub1;
	CStarControlSub6 _sub2;
	double _fieldC0;
	double _fieldC4;
	int _fieldC8;
	int _fieldCC;
	double _fieldD0;
	int _fieldD4;
private:
	void setup(void *ptr);

	void reset();
public:
	FVector _position;
	double _field10;
	double _field14;
	int _field24;
public:
	CStarControlSub13(void *ptr);
	CStarControlSub13(CStarControlSub13 *src);

	void copyFrom(const void *src);

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
	void setPosition(const CStarControlSub6 &sub6);

	/**
	 * Sets the matrix
	 */
	void setMatrix(const FMatrix &m);

	void fn11(const FVector &v);
	void fn12();
	void fn13(double v1, double v2);
	void fn14(double v);
	void fn15(FMatrix &matrix);
	CStarControlSub6 getSub1();
	CStarControlSub6 getSub2();
	FVector fn16(const FVector &v);
	FVector fn17(int index, const FVector &v);
	FVector fn18(int index, const FVector &v);
	void fn19(double *v1, double *v2, double *v3, double *v4);

	/**
	 * Makes a copy of the instance's matrix into the passed matrix
	 */
	void getMatrix(FMatrix *matrix);

	void setC(int v);
	void set10(int v);
	void set14(int v);
	void set18(int v);
	void set1C(int v);
};

} // End of namespace Titanic

#endif /* TITANIC_STAR_CONTROL_SUB13_H */
