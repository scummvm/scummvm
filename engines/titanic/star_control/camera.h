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

#ifndef TITANIC_CAMERA_H
#define TITANIC_CAMERA_H

#include "titanic/star_control/fmatrix.h"
#include "titanic/star_control/base_stars.h"
#include "titanic/star_control/viewport.h"

namespace Titanic {

class CMotionControl;
class CErrorCode;
struct CNavigationInfo;
class FPoint;
class SimpleFile;

enum StarLockLevel { ZERO_LOCKED=0, ONE_LOCKED=1, TWO_LOCKED=2, THREE_LOCKED=3 };

/**
 * Implements a reference point from which the starmap can be viewed
 */
class CCamera {
private:
	static FMatrix *_priorOrientation;
	static FMatrix *_newOrientation;
private:
	StarLockLevel _lockLevel;
	FMatrix _lockedStarsPos; // Each row represents the location of a locked star
	CMotionControl *_motion; // A marked or unmarked camera mover, contains an automover
	CViewport _viewport;
	bool _isMoved; // Used in CPetStarfield to determine if a star destination can be set
	bool _isInLockingProcess; // The mover/view is homing in on a new star
private:
	/**
	 * Creates a motion controller for the camera. This needs to be recreated
	 * when the number of locked stars changes. 
	 * @param src	Contains characteristics to set for the motion
	 */
	bool createMotionControl(const CNavigationInfo *src);

	/**
	 * Deletes the previous mover handle
	 */
	void deleteMotionController();

	/**
	 * Return whether the handler is locked
	 */
	bool isLocked();
public:
	static void init();
	static void deinit();
public:
	CCamera(const CNavigationInfo *data);
	CCamera(CViewport *src);
	virtual ~CCamera();

	/**
	 * Copy the state from a specified viewport
	 */
	virtual void setViewport(const CViewport *src);

	/**
	 * Set motion from the passed navigation info
	 */
	virtual void setMotion(const CNavigationInfo *src);

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

	/**
	 * Assigns a roll angle about the view direction
	 */
	virtual void setRoleAngle(double angle);

	/**
	 * Assign a near clip plane distance
	 */
	virtual void setFrontClip(double n);

	/**
	 * Assign a far clipping plane distance
	 */
	virtual void SetBackClip(double f);

	virtual void setCenterYAngle(int v);
	virtual void setCenterZAngle(int v);
	virtual void randomizeOrientation();
	virtual void setFields(StarMode mode, double val);

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
	virtual void accelerate();

	/**
	 * Increases movement speed in backward direction
	 */
	virtual void deccelerate();

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

	/**
	 * Get the front clipping distance
	 */
	virtual double getFrontClip() const;

	/**
	 * Get the back clipping distance
	 */
	virtual double getBackClip() const;


	virtual StarColor getStarColor() const;

	/**
	 * Return the passed vector relative to the center of the viewpoint
	 */
	virtual FVector getRelativePos(int index, const FVector &src);

	virtual FVector getRelativePosNoCentering(int index, const FVector &src);
	virtual FVector getRelativePosCentering(int index, const FVector &v);
	virtual FVector getRelativePosCenteringRaw(int index, const FVector &v);

	/**
	 * Sets the viewport position within the starfield
	 */
	virtual void setViewportAngle(const FPoint &angles);

	/**
	 * How many stars are currently locked onto
	 */
	virtual StarLockLevel getLockLevel() const { return _lockLevel; }

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

#endif /* TITANIC_CAMERA_H */
