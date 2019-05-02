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

#ifndef TITANIC_CAMERA_AUTO_MOVER_H
#define TITANIC_CAMERA_AUTO_MOVER_H

#include "titanic/star_control/fvector.h"
#include "titanic/star_control/orientation_changer.h"
#include "common/array.h"

namespace Titanic {

class CErrorCode;
class FMatrix;
const int nMoverTransitions = 32; // The number of vector transitions when doing a mover change is fixed
enum MoverState { NOT_ACTIVE = 0, MOVING = 1, DONE_MOVING = 2 };

/**
 * Base class for automatic movement of the starview camera
 */
class CCameraAutoMover {
protected:
	int _field4;
	bool _active;
	FVector _srcPos, _destPos;
	double _distance;
	FVector _posDelta;
	bool _field34;
	double _field38;
	double _field3C;
	int _field40;
	int _field44;
	int _field48;
	int _field4C;
	double _speeds[nMoverTransitions];
	int _field54;
	double _transitionPercent;
	double _transitionPercentInc;
	COrientationChanger _orientationChanger;
public:
	CCameraAutoMover();
	virtual ~CCameraAutoMover() {}

	/**
	 * Clear src and dest orientation and set some default values for other fields
	 */
	void clear();

	/**
	 * Setup a transition to from one position to another
	 */
	void setPath(const FVector &srcV, const FVector &destV);

	/**
	 * Applys speeds to the mover. More than one application is usually done for several transitions
	 */
	virtual MoverState move(CErrorCode &errorCode, FVector &pos, FMatrix &orientation) { return DONE_MOVING; }
	/**
	 * Given a distance to cover, determines a bunch of speeds for a gradual transition
	 * from one position to another (the mover). The speeds go from fast to slow
	 */
	virtual void calcSpeeds(int val1, int val2, float distance);

	bool isActive() const { return _active; }
};

} // End of namespace Titanic

#endif /* TITANIC_CAMERA_AUTO_MOVER_H */
