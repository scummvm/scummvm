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

#ifndef TITANIC_UNMARKED_CAMERA_MOVER_H
#define TITANIC_UNMARKED_CAMERA_MOVER_H

#include "titanic/star_control/camera_mover.h"
#include "titanic/star_control/unmarked_auto_mover.h"

namespace Titanic {

class FMatrix;
class FVector;

class CUnmarkedCameraMover : public CCameraMover {
private:
	CUnmarkedAutoMover _autoMover;
public:
	CUnmarkedCameraMover(const CNavigationInfo *src);
	virtual ~CUnmarkedCameraMover() {}

	/**
	 * Start a movement to a given specified destination
	 */
	virtual void moveTo(const FVector &srcV, const FVector &destV, const FMatrix &orientation);

	virtual void transitionBetweenOrientations(const FVector &v1, const FVector &v2, const FVector &v3, const FMatrix &m);

	/**
	 * Update the passed position and orientation matrix
	 */
	virtual void updatePosition(CErrorCode &errorCode, FVector &pos, FMatrix &orientation);
};

} // End of namespace Titanic

#endif /* TITANIC_UNMARKED_CAMERA_MOVER_H */
