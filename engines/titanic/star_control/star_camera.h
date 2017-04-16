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

#ifndef TITANIC_STAR_CAMERA_H
#define TITANIC_STAR_CAMERA_H

#include "titanic/support/simple_file.h"
#include "titanic/star_control/fmatrix.h"
#include "titanic/star_control/fpoint.h"
#include "titanic/star_control/base_stars.h"
#include "titanic/star_control/star_control_sub13.h"
#include "titanic/star_control/star_control_sub20.h"
#include "titanic/star_control/error_code.h"

namespace Titanic {

class CStarCamera {
private:
	static FMatrix *_matrix1;
	static FMatrix *_matrix2;
private:
	int _matrixRow;
	FMatrix _matrix;
	CStarControlSub20 *_handlerP;
	CStarControlSub13 _sub13;
	int _field108;
private:
	/**
	 * Set up a handler
	 */
	bool setupHandler(const CStar20Data *src);

	/**
	 * Deletes any previous handler
	 */
	void deleteHandler();

	/**
	 * Return whether the handler is locked
	 */
	bool isLocked() { return _handlerP->isLocked(); }
public:
	static void init();
	static void deinit();
public:
	CStarCamera(const CStar20Data *data);
	CStarCamera(CStarControlSub13 *src);
	virtual ~CStarCamera();

	virtual void proc2(const CStarControlSub13 *src);
	virtual void proc3(const CStar20Data *src);
	virtual void setPosition(const FVector &v);
	virtual void proc5(const FVector &v);
	virtual void proc6(int v);
	virtual void proc7(int v);
	virtual void proc8(int v);
	virtual void proc9(int v);
	virtual void proc10(int v);
	virtual void proc11();
	virtual void proc12(StarMode mode, double v2);
	virtual void proc13(CStarControlSub13 *dest);

	/**
	 * Sets the destination to move the camera to
	 */
	virtual void setDestination(const FVector &v);

	virtual void proc15(CErrorCode *errorCode);
	virtual void proc16();
	virtual void proc17();
	virtual void proc18();
	virtual void proc19();
	virtual void proc20(double factor);
	virtual void proc21(const FPose &pose);
	virtual void proc22(FMatrix &m);

	/**
	 * Get the position and direction of the camera
	 */
	virtual FPose getPose();

	virtual FPose getPose2();
	virtual double getThreshold() const;

	virtual double proc26() const;
	virtual int proc27() const;
	virtual FVector proc28(int index, const FVector &src);
	virtual FVector proc29(int index, const FVector &src);
	virtual FVector proc30(int index, const FVector &v);
	virtual FVector proc31(int index, const FVector &v);

	/**
	 * Sets the viewport position within the starfield
	 */
	virtual void setViewportPosition(const FPoint &angles);

	virtual int getMatrixRow() const { return _matrixRow; }
	virtual bool addMatrixRow(const FVector &v);
	virtual bool removeMatrixRow();
	virtual void proc36(double *v1, double *v2, double *v3, double *v4);

	/**
	 * Load the data for the class from file
	 */
	virtual void load(SimpleFile *file, int param);

	/**
	 * Save the data for the class to file
	 */
	virtual void save(SimpleFile *file, int indent);


	bool is108() const { return _field108; }
	void set108() { _field108 = true; }
	void reset108() { _field108 = false; }

	void fn1(CStarControlSub13 *sub13, const FVector &v);
	void fn2(FVector v1, FVector v2, FVector v3);
	void fn3(CStarControlSub13 *sub13, const FVector &v);
};

} // End of namespace Titanic

#endif /* TITANIC_STAR_CAMERA_H */
