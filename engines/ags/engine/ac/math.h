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

#ifndef AGS_ENGINE_AC_MATH_H
#define AGS_ENGINE_AC_MATH_H

#include "ags/shared/core/types.h"

namespace AGS3 {

enum RoundDirections {
	eRoundDown = 0,
	eRoundNearest = 1,
	eRoundUp = 2
};


int FloatToInt(float value, int roundDirection);
float IntToFloat(int value);
float StringToFloat(const char *theString);
float Math_Cos(float value);
float Math_Sin(float value);
float Math_Tan(float value);
float Math_ArcCos(float value);
float Math_ArcSin(float value);
float Math_ArcTan(float value);
float Math_ArcTan2(float yval, float xval);
float Math_Log(float value);
float Math_Log10(float value);
float Math_Exp(float value);
float Math_Cosh(float value);
float Math_Sinh(float value);
float Math_Tanh(float value);
float Math_RaiseToPower(float base, float exp);
float Math_DegreesToRadians(float value);
float Math_RadiansToDegrees(float value);
float Math_GetPi();
float Math_Sqrt(float value);

int __Rand(int upto);
#define Random __Rand

} // namespace AGS3

#endif
