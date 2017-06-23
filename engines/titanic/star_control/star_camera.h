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
#include "titanic/star_control/viewport.h"
#include "titanic/star_control/camera_mover.h"
#include "titanic/star_control/error_code.h"

namespace Titanic {

/**
 * Implements a reference point from which the starmap can be viewed
 */
class CStarCamera {
private:
	static FMatrix *_priorOrientation;
	static FMatrix *_newOrientation;
private:
	int _matrixRow;
	FMatrix _matrix;
	CCameraMover *_mover;
	CViewport _viewport;
	bool _isMoved;
private:
	/**
	 * Set up a handler
	 */
	bool setupHandler(const CNavigationInfo *src);

	/**
	 * Deletes any previous handler
	 */
	void deleteHandler();

	/**
	 * Return whether the handler is locked
	 */
	bool isLocked() { return _mover->isLocked(); }
public:
	static void init();
	static void deinit();
public:
	CStarCamera(const CNavigationInfo *data);
	CStarCamera(CViewport *src);
	virtual ~CStarCamera();

	virtual void proc2(const CViewport *src);
	virtual void proc3(const CNavigationInfo *src);

	/**
	 * Set the camera position
	 */
	virtual void setPosition(const FVector &v);

	/**
	 * Sets the camera orientation
	 */
	virtual void setOrientation(const FVector &v);

	virtual void proc6(int v);
	virtual void proc7(int v);
	virtual void proc8(int v);
	virtual void proc9(int v);
	virtual void proc10(int v);
	virtual void proc11();
	virtual void proc12(StarMode mode, double v2);
	virtual void proc13(CViewport *dest);

	/**
	 * Sets the destination to move the camera to
	 */
	virtual void setDestination(const FVector &v);

	/**
	 * Updates the camera position
	 */
	virtual void updatePosition(CErrorCode *errorCode);

	/**
	 * Increases movement speed
	 */
	virtual void increaseSpeed();

	/**
	 * Decreases movement speed
	 */
	virtual void decreaseSpeed();

	/**
	 * Increase to full speed
	 */
	virtual void fullSpeed();

	/**
	 * Completely stop
	 */
	virtual void stop();

	virtual void reposition(double factor);

	/**
	 * Set the camera position
	 */
	virtual void setPosition(const FPose &pose);

	virtual void proc22(FMatrix &m);

	/**
	 * Get the position and direction of the camera
	 */
	virtual FPose getPose();

	virtual FPose getRawPose();
	virtual double getThreshold() const;

	virtual double proc26() const;
	virtual int proc27() const;

	/**
	 * Return the passed vector relative to the center of the viewpoint
	 */
	virtual FVector getRelativePos(int index, const FVector &src);

	virtual FVector proc29(int index, const FVector &src);
	virtual FVector proc30(int index, const FVector &v);
	virtual FVector proc31(int index, const FVector &v);

	/**
	 * Sets the viewport position within the starfield
	 */
	virtual void setViewportAngle(const FPoint &angles);

	virtual int getMatrixRow() const { return _matrixRow; }

	/**
	 * Adds the row for a locked in marker
	 * @remarks		This can't be a pass-by-reference, since adding
	 * the vector for the star destroys the calling star vector
	 */
	virtual bool addMatrixRow(const FVector v);

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

	/**
	 * Returns true for whether the camera has been moved
	 */
	bool isMoved() const { return _isMoved; }

	/**
	 * Sets the camera as having been moved
	 */
	void setIsMoved() { _isMoved = true; }

	/**
	 * Resets the flag for whether the camera has moved
	 */
	void clearIsMoved() { _isMoved = false; }

	/**
	 * Lock in the first matched star marker
	 */
	void lockMarker1(FVector v1, FVector v2, FVector v3);
	
	/**
	 * Lock in the second matched star marker
	 */
	void lockMarker2(CViewport *viewport, const FVector &v);

	/**
	 * Lock in the third and final matched star marker
	 */
	void lockMarker3(CViewport *viewport, const FVector &v);
};

} // End of namespace Titanic

#endif /* TITANIC_STAR_CAMERA_H */
