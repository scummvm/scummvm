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

#include "titanic/star_control/fmatrix.h"
#include "titanic/star_control/base_stars.h"
#include "titanic/star_control/viewport.h"

namespace Titanic {

class CCameraMover;
class CErrorCode;
struct CNavigationInfo;
class FPoint;
class SimpleFile;

enum StarLockState { ZERO_LOCKED=0, ONE_LOCKED=1, TWO_LOCKED=2, THREE_LOCKED=3 };

/**
 * Implements a reference point from which the starmap can be viewed
 */
class CStarCamera {
private:
	static FMatrix *_priorOrientation;
	static FMatrix *_newOrientation;
private:
	StarLockState _starLockState;
	FMatrix _lockedStarsPos; // Each row represents the location of a locked star
	CCameraMover *_mover; // A marked or unmarked camera mover, contains an automover
	CViewport _viewport;
	bool _isMoved; // Used in CPetStarfield to determine if a star destination can be set
	bool _isInLockingProcess; // The mover/view is homing in on a new star
private:
	/**
	 * Set Mover type to be unmarked or marked camera mover based on 
	 * the number of stars currently locked (_starLockState)
	 * The CNavigationInfo data is used to initialize the mover
	 */
	bool setMoverType(const CNavigationInfo *src);

	/**
	 * Deletes the previous mover handle
	 */
	void removeMover();

	/**
	 * Return whether the handler is locked
	 */
	bool isLocked();
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
	 * The mover/view is not currently homing in on a new star
	 * This can mean it is unmarked, or that it is fully locked 
	 * onto one star or more (but not in the process of doing so)
	 */
	bool isNotInLockingProcess();

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
	virtual void setCenterYAngle(int v);
	virtual void setCenterZAngle(int v);
	virtual void randomizeOrientation();
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
	 * Increases movement speed in forward direction
	 */
	virtual void increaseForwardSpeed();

	/**
	 * Increases movement speed in backward direction
	 */
	virtual void increaseBackwardSpeed();

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

	virtual void changeOrientation(FMatrix &m);

	/**
	 * Get the position and direction of the camera
	 */
	virtual FPose getPose();

	virtual FPose getRawPose();
	virtual double getThreshold() const;

	virtual double proc26() const;
	virtual StarColor getStarColor() const;

	/**
	 * Return the passed vector relative to the center of the viewpoint
	 */
	virtual FVector getRelativePos(int index, const FVector &src);

	virtual FVector getRelativePosNoCentering(int index, const FVector &src);
	virtual FVector proc30(int index, const FVector &v);
	virtual FVector proc31(int index, const FVector &v);

	/**
	 * Sets the viewport position within the starfield
	 */
	virtual void setViewportAngle(const FPoint &angles);

	/**
	 * How many stars are currently locked onto
	 */
	virtual StarLockState getStarLockState() const { return _starLockState; }

	/**
	 * Adds the row for a locked in marker/star
	 * @remarks		This can't be a pass-by-reference, since adding
	 * the vector for the star destroys the calling star vector
	 */
	virtual bool addLockedStar(const FVector v);

	/**
	 * Removes the most recent locked in marker/star
	 * @remarks		This can't be a pass-by-reference, since adding
	 * the vector for the star destroys the calling star vector
	 */
	virtual bool removeLockedStar();

	/**
	 * All arguments are return values
	 * First is the x center coordinate relative to y
	 * Second is the x center coordinate relative to z
	 * Third is the first x center pixel offset
	 * Fourth is the second x center pixel offset
	 */	
	virtual void getRelativeXCenterPixels(double *v1, double *v2, double *v3, double *v4);

	/**
	 * Load the data for the class from file
	 */
	virtual void load(SimpleFile *file, int param);

	/**
	 * Save the data for the class to file
	 */
	virtual void save(SimpleFile *file, int indent);

	/**
	 * Calculates the angle of rotation of y that achieves
	 * the minimum distance to x. 
	 * The angle is in degrees.
	 * Also returns the minimum distance calculated
	 */
	float calcAngleForMinDist(FVector &x, FVector &y, float &minDistance);

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
	bool lockMarker1(FVector v1, FVector v2, FVector v3);

	/**
	 * Lock in the second matched star marker
	 */
	bool lockMarker2(CViewport *viewport, const FVector &v);

	/**
	 * Lock in the third and final matched star marker
	 */
	bool lockMarker3(CViewport *viewport, const FVector &v);
};

} // End of namespace Titanic

#endif /* TITANIC_STAR_CAMERA_H */
