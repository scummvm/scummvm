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

#include "titanic/star_control/error_code.h"
#include "titanic/star_control/fmatrix.h"
#include "titanic/star_control/fvector.h"
#include "titanic/star_control/orientation_changer.h"

namespace Titanic {

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
	Common::Array<double> _speeds;
	int _field54;
	double _transitionPercent;
	double _transitionPercentInc;
	COrientationChanger _orientationChanger;
public:
	CCameraAutoMover();
	virtual ~CCameraAutoMover() {}

	virtual void proc2(const FVector &oldPos, const FVector &newPos,
		const FMatrix &oldOrientation, const FMatrix &newOrientation);
	virtual void proc3(const FMatrix &srcOrient, const FMatrix &destOrient);
	virtual void setPath(const FVector &srcV, const FVector &destV, const FMatrix &orientation);
	virtual int proc5(CErrorCode &errorCode, FVector &pos, FMatrix &orientation) { return 2; }
	virtual void proc6(int val1, int val2, float val);

	bool isActive() const { return _active; }
};

} // End of namespace Titanic

#endif /* TITANIC_CAMERA_AUTO_MOVER_H */
