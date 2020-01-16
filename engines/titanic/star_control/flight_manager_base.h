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

#ifndef TITANIC_FLIGHT_MANAGER_BASE_H
#define TITANIC_FLIGHT_MANAGER_BASE_H

#include "titanic/star_control/fvector.h"
#include "titanic/star_control/orientation_changer.h"
#include "common/array.h"

namespace Titanic {

#define GAMMA_TABLE_SIZE 32

class CErrorCode;
class FMatrix;
enum MoverState { NOT_ACTIVE = 0, MOVING = 1, DONE_MOVING = 2 };

/**
 * Base class for flight manager handling automated movement
 */
class CFlightManagerBase {
protected:
	bool _active;
	FVector _srcPos, _destPos;
	double _distance;
	FVector _direction;
	bool _flight;
	double _step;
	double _step1;
	int _accCount;
	int _traCount;
	int _decCount;
	int _totCount;
	double _gammaTable[GAMMA_TABLE_SIZE];
	double _currentSpin;
	double _spinStep;
	COrientationChanger _orientationChanger;
public:
	CFlightManagerBase();
	virtual ~CFlightManagerBase() {}

	/**
	 * Clear src and dest orientation and set some default values for other fields
	 */
	void clear();

	/**
	 * Setup a transition to from one position to another
	 */
	void setPath(const FVector &from, const FVector &to);

	/**
	 * Applys speeds to the mover. More than one application is usually done for several transitions
	 */
	virtual MoverState move(CErrorCode &errorCode, FVector &pos, FMatrix &orientation) { return DONE_MOVING; }

	/**
	 * Given a distance to cover, builds an acceleration table for the journey
	 */
	virtual void buildMotionTable(int sustain, int decay, float distance);

	bool isActive() const { return _active; }
};

} // End of namespace Titanic

#endif /* TITANIC_FLIGHT_MANAGER_BASE_H */
