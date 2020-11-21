//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================
//
//
//
//=============================================================================
#ifndef __AGS_EE_AC__MATH_H
#define __AGS_EE_AC__MATH_H

#include "core/types.h"

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

#endif // __AGS_EE_AC__MATH_H
