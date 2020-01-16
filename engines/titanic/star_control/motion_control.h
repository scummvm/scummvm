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

#ifndef TITANIC_MOTION_CONTROL_H
#define TITANIC_MOTION_CONTROL_H

#include "titanic/star_control/fmatrix.h"

namespace Titanic {

class CCamera;
class CErrorCode;
class FMatrix;
class FVector;
class SimpleFile;

struct CNavigationInfo {
	double _initialVelocity;
	double _minVelocity;
	double _maxVelocity;
	double _velocity;
	double _acceleration;
	double _rotationX;
	double _rotationY;
	double _rotationZ;
};

/**
 * Handles doing a callback to the starfield to lock in a star after movement finishes
 */
class CCallbackHandler {
private:
	CCamera *_owner;
	FVector _vector;
public:
	CCallbackHandler(CCamera *owner, const FVector &v) : _owner(owner), _vector(v) {
	}

	/**
	 * Locks in the star at the given position
	 */
	void apply();
};

class CMotionControl {
protected:
	double _currVelocity;
	double _incVelocity;
	double _incAcceleration;
	double _minVelocity;
	double _maxVelocity;
	double _rotationX;
	double _rotationY;
	double _rotationZ;
public:
	int _lockCounter;
	CCallbackHandler *_callback;
public:
	CMotionControl(const CNavigationInfo *src);
	virtual ~CMotionControl();

	virtual void setMotion(const CNavigationInfo *src);
	virtual void getMotion(CNavigationInfo *dest);

	/**
	 * delete _starVector
	 */
	virtual void clear();
	/**
	 * Set default values for CNavigationInfo
	 */
	virtual void reset();

	/**
	 * Sets this CCallbackHandler
	 */
	virtual void setCallback(CCallbackHandler *callback);
	/**
	 * Increases movement speed in forward direction
	 */
	virtual void accelerate();

	/**
	 * Decreases movement speed in backward direction
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

	/**
	 * Move the mover from an old position and orientation to a new
	 * position and orientation
	 */
	virtual void transitionBetweenPosOrients(const FVector &oldPos, const FVector &newPos,
		const FMatrix &oldOrientation, const FMatrix &newOrientation) {}

	/**
	 * Start a movement to a given specified destination
	 */
	virtual void moveTo(const FVector &srcV, const FVector &destV, const FMatrix &orientation) {}

	/**
	 * First two vectors are used to form a new orientation that gets transitioned to from the old
	 * orientation m.
	 */
	virtual void transitionBetweenOrientations(const FVector &v1, const FVector &v2, const FVector &v3, const FMatrix &m) {}

	/**
	 * Update the passed position and orientation matrix
	 */
	virtual void updatePosition(CErrorCode &errorCode, FVector &pos, FMatrix &orientation) {}

	/**
	 * Load the class
	 */
	virtual void load(SimpleFile *file, int version = 0);

	/**
	 * Save the class
	 */
	virtual void save(SimpleFile *file, int indent);

	/**
	 * Increment tthe lock counter
	 */
	void incLockCount();

	/**
	 * Decrement the lock counter
	 */
	void decLockCount();

	/**
	 * Returns true if the lock counter is non-zero
	 */
	bool isLocked() const { return _lockCounter > 0; }
};

} // End of namespace Titanic

#endif /* TITANIC_MOTION_CONTROL_H */
