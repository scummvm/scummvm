/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 */

#ifndef HPL_PID_CONTROLLER_H
#define HPL_PID_CONTROLLER_H

#include "hpl1/algorithms.h"
#include "hpl1/engine/math/MathTypes.h"

namespace hpl {

template<class T>
class cPidController {
public:
	float p, i, d;
	//////////////////////////////////////////
	// Constructors
	/////////////////////////////////////////
	cPidController() {
		Reset();
	}
	cPidController(float afP, float afI, float afD, int alErrorNum) {
		p = afP;
		i = afI;
		d = afD;
		SetErrorNum(alErrorNum);
		Reset();
	}

	//////////////////////////////////////////
	// Public
	/////////////////////////////////////////

	//------------------------------------
	T Output(T aError, float afTimeStep) {
		mvErrors[mlErrorNum] = aError;
		mvTimeSteps[mlErrorNum] = afTimeStep;

		integral = 0;
		size_t lCount = mvErrors.size();
		for (size_t error = 0; error < lCount; ++error) {
			integral += mvErrors[error] * mvTimeSteps[error];
		}

		derivative = 0.0f;
		if (mlLastNum >= 0) {
			derivative = (mvErrors[mlErrorNum] - mvErrors[mlLastNum]) / afTimeStep;
		}

		mlLastNum = mlErrorNum;
		mlErrorNum++;
		if (mlErrorNum >= (int)mvErrors.size())
			mlErrorNum = 0;

		return mvErrors[mlLastNum] * p + integral * i + derivative * d;
	}

	//------------------------------------

	void SetErrorNum(int alErrorNum) {
		mvErrors.resize(alErrorNum);
		Hpl1::resizeAndFill(mvTimeSteps, alErrorNum, 0.f);
	}

	//------------------------------------

	void Reset() {
		mlErrorNum = 0;
		mlLastNum = -1;
		Common::fill(mvTimeSteps.begin(), mvTimeSteps.end(), 0);

		integral = 0;
		derivative = 0;
	}

	//------------------------------------

	T GetLastError() {
		if (mlLastNum >= 0)
			return mvErrors[mlLastNum];
		return 0;
	}

	T GetLastDerivative() {
		return derivative;
	}

	T GetLastIntegral() {
		return integral;
	}

	//------------------------------------

private:
	Common::Array<T> mvErrors;
	Common::Array<float> mvTimeSteps;

	T integral, derivative;

	int mlErrorNum;
	int mlLastNum;
};

//---------------------------------

typedef cPidController<float> cPidControllerf;
typedef cPidController<cVector3f> cPidControllerVec3;

} // namespace hpl

#endif // HPL_PID_CONTROLLER_H
