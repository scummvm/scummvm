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

#ifndef TITANIC_MOTION_CONTROL_UNMARKED_H
#define TITANIC_MOTION_CONTROL_UNMARKED_H

#include "titanic/star_control/motion_control.h"
#include "titanic/star_control/flight_manager_unmarked.h"

namespace Titanic {

class FMatrix;
class FVector;

class CMotionControlUnmarked : public CMotionControl {
private:
	CFlightManagerUnmarked _autoMover;
public:
	CMotionControlUnmarked(const CNavigationInfo *src);
	~CMotionControlUnmarked() override {}

	/**
	 * Start a movement to a given specified destination
	 */
	void moveTo(const FVector &srcV, const FVector &destV, const FMatrix &orientation) override;

	void transitionBetweenOrientations(const FVector &v1, const FVector &v2, const FVector &v3, const FMatrix &m) override;

	/**
	 * Update the passed position and orientation matrix
	 */
	void updatePosition(CErrorCode &errorCode, FVector &pos, FMatrix &orientation) override;
};

} // End of namespace Titanic

#endif /* TITANIC_MOTION_CONTROL_UNMARKED_H */
