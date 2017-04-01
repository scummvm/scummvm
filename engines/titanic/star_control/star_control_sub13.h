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
#include "titanic/star_control/base_star.h"
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
	FMatrix _matrix;
	CStarControlSub6 _sub1;
	CStarControlSub6 _sub2;
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
	CStarControlSub13();
	CStarControlSub13(CStarControlSub13 *src);

	/**
	 * Copys the data from another instance
	 */
	void copyFrom(const CStarControlSub13 *src);

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
	void setPosition(const CStarControlSub6 *sub6);

	/**
	 * Sets the matrix
	 */
	void setMatrix(const FMatrix &m);

	void fn11(const FVector &v);
	void fn12();
	void fn13(StarMode mode, double val);
	void reposition(double factor);
	void fn15(const FMatrix &matrix);
	CStarControlSub6 getSub1();
	CStarControlSub6 getSub2();
	void fn16(int index, const FVector &src, FVector &dest);
	FVector fn17(int index, const FVector &src);
	FVector fn18(int index, const FVector &src);
	void fn19(double *v1, double *v2, double *v3, double *v4);

	/**
	 * Returns the instance's matrix
	 */
	const FMatrix &getMatrix() const;

	void setC(double v);
	void set10(double v);
	void set14(double v);
	void set18(double v);
	void set1C(double v);
};

} // End of namespace Titanic

#endif /* TITANIC_STAR_CONTROL_SUB13_H */
