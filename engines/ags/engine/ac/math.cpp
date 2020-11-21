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

#include <cmath>
#include "ac/math.h"
#include "ac/common.h" // quit
#include "util/math.h"

int FloatToInt(float value, int roundDirection)
{
    if (value >= 0.0) {
        if (roundDirection == eRoundDown)
            return static_cast<int>(value);
        else if (roundDirection == eRoundNearest)
            return static_cast<int>(value + 0.5);
        else if (roundDirection == eRoundUp)
            return static_cast<int>(value + 0.999999);
        else
            quit("!FloatToInt: invalid round direction");
    }
    else {
        // negative number
        if (roundDirection == eRoundUp)
            return static_cast<int>(value); // this just truncates
        else if (roundDirection == eRoundNearest)
            return static_cast<int>(value - 0.5);
        else if (roundDirection == eRoundDown)
            return static_cast<int>(value - 0.999999);
        else
            quit("!FloatToInt: invalid round direction");
    }
    return 0;
}

float IntToFloat(int value)
{
    return static_cast<float>(value);
}

float StringToFloat(const char *theString)
{
    return static_cast<float>(atof(theString));
}

float Math_Cos(float value)
{
    return cos(value);
}

float Math_Sin(float value)
{
    return sin(value);
}

float Math_Tan(float value)
{
    return tan(value);
}

float Math_ArcCos(float value)
{
    return acos(value);
}

float Math_ArcSin(float value)
{
    return asin(value);
}

float Math_ArcTan(float value)
{
    return atan(value);
}

float Math_ArcTan2(float yval, float xval)
{
    return atan2(yval, xval);
}

float Math_Log(float value)
{
    return log(value);
}

float Math_Log10(float value)
{
    return ::log10(value);
}

float Math_Exp(float value)
{
    return exp(value);
}

float Math_Cosh(float value)
{
    return cosh(value);
}

float Math_Sinh(float value)
{
    return sinh(value);
}

float Math_Tanh(float value)
{
    return tanh(value);
}

float Math_RaiseToPower(float base, float exp)
{
    return ::pow(base, exp);
}

float Math_DegreesToRadians(float value)
{
    return static_cast<float>(value * (M_PI / 180.0));
}

float Math_RadiansToDegrees(float value)
{
    return static_cast<float>(value * (180.0 / M_PI));
}

float Math_GetPi()
{
    return static_cast<float>(M_PI);
}

float Math_Sqrt(float value)
{
    if (value < 0.0)
        quit("!Sqrt: cannot perform square root of negative number");

    return ::sqrt(value);
}

int __Rand(int upto)
{
    upto++;
    if (upto < 1)
        quit("!Random: invalid parameter passed -- must be at least 0.");
    return rand()%upto;
}


//=============================================================================
//
// Script API Functions
//
//=============================================================================

#include "debug/out.h"
#include "script/script_api.h"
#include "script/script_runtime.h"

// float (float value)
RuntimeScriptValue Sc_Math_ArcCos(const RuntimeScriptValue *params, int32_t param_count)
{
    API_SCALL_FLOAT_PFLOAT(Math_ArcCos);
}

// float (float value)
RuntimeScriptValue Sc_Math_ArcSin(const RuntimeScriptValue *params, int32_t param_count)
{
    API_SCALL_FLOAT_PFLOAT(Math_ArcSin);
}

// float (float value)
RuntimeScriptValue Sc_Math_ArcTan(const RuntimeScriptValue *params, int32_t param_count)
{
    API_SCALL_FLOAT_PFLOAT(Math_ArcTan);
}

// float (SCRIPT_FLOAT(yval), SCRIPT_FLOAT(xval))
RuntimeScriptValue Sc_Math_ArcTan2(const RuntimeScriptValue *params, int32_t param_count)
{
    API_SCALL_FLOAT_PFLOAT2(Math_ArcTan2);
}

// float (float value)
RuntimeScriptValue Sc_Math_Cos(const RuntimeScriptValue *params, int32_t param_count)
{
    API_SCALL_FLOAT_PFLOAT(Math_Cos);
}

// float (float value)
RuntimeScriptValue Sc_Math_Cosh(const RuntimeScriptValue *params, int32_t param_count)
{
    API_SCALL_FLOAT_PFLOAT(Math_Cosh);
}

// float (float value)
RuntimeScriptValue Sc_Math_DegreesToRadians(const RuntimeScriptValue *params, int32_t param_count)
{
    API_SCALL_FLOAT_PFLOAT(Math_DegreesToRadians);
}

// float (float value)
RuntimeScriptValue Sc_Math_Exp(const RuntimeScriptValue *params, int32_t param_count)
{
    API_SCALL_FLOAT_PFLOAT(Math_Exp);
}

// float (float value)
RuntimeScriptValue Sc_Math_Log(const RuntimeScriptValue *params, int32_t param_count)
{
    API_SCALL_FLOAT_PFLOAT(Math_Log);
}

// float (float value)
RuntimeScriptValue Sc_Math_Log10(const RuntimeScriptValue *params, int32_t param_count)
{
    API_SCALL_FLOAT_PFLOAT(Math_Log10);
}

// float (float value)
RuntimeScriptValue Sc_Math_RadiansToDegrees(const RuntimeScriptValue *params, int32_t param_count)
{
    API_SCALL_FLOAT_PFLOAT(Math_RadiansToDegrees);
}

// float (SCRIPT_FLOAT(base), SCRIPT_FLOAT(exp))
RuntimeScriptValue Sc_Math_RaiseToPower(const RuntimeScriptValue *params, int32_t param_count)
{
    API_SCALL_FLOAT_PFLOAT2(Math_RaiseToPower);
}

// float (float value)
RuntimeScriptValue Sc_Math_Sin(const RuntimeScriptValue *params, int32_t param_count)
{
    API_SCALL_FLOAT_PFLOAT(Math_Sin);
}

// float (float value)
RuntimeScriptValue Sc_Math_Sinh(const RuntimeScriptValue *params, int32_t param_count)
{
    API_SCALL_FLOAT_PFLOAT(Math_Sinh);
}

// float (float value)
RuntimeScriptValue Sc_Math_Sqrt(const RuntimeScriptValue *params, int32_t param_count)
{
    API_SCALL_FLOAT_PFLOAT(Math_Sqrt);
}

// float (float value)
RuntimeScriptValue Sc_Math_Tan(const RuntimeScriptValue *params, int32_t param_count)
{
    API_SCALL_FLOAT_PFLOAT(Math_Tan);
}

// float (float value)
RuntimeScriptValue Sc_Math_Tanh(const RuntimeScriptValue *params, int32_t param_count)
{
    API_SCALL_FLOAT_PFLOAT(Math_Tanh);
}

// float ()
RuntimeScriptValue Sc_Math_GetPi(const RuntimeScriptValue *params, int32_t param_count)
{
    API_SCALL_FLOAT(Math_GetPi);
}


void RegisterMathAPI()
{
    ccAddExternalStaticFunction("Maths::ArcCos^1",              Sc_Math_ArcCos);
    ccAddExternalStaticFunction("Maths::ArcSin^1",              Sc_Math_ArcSin);
    ccAddExternalStaticFunction("Maths::ArcTan^1",              Sc_Math_ArcTan);
    ccAddExternalStaticFunction("Maths::ArcTan2^2",             Sc_Math_ArcTan2);
    ccAddExternalStaticFunction("Maths::Cos^1",                 Sc_Math_Cos);
    ccAddExternalStaticFunction("Maths::Cosh^1",                Sc_Math_Cosh);
    ccAddExternalStaticFunction("Maths::DegreesToRadians^1",    Sc_Math_DegreesToRadians);
    ccAddExternalStaticFunction("Maths::Exp^1",                 Sc_Math_Exp);
    ccAddExternalStaticFunction("Maths::Log^1",                 Sc_Math_Log);
    ccAddExternalStaticFunction("Maths::Log10^1",               Sc_Math_Log10);
    ccAddExternalStaticFunction("Maths::RadiansToDegrees^1",    Sc_Math_RadiansToDegrees);
    ccAddExternalStaticFunction("Maths::RaiseToPower^2",        Sc_Math_RaiseToPower);
    ccAddExternalStaticFunction("Maths::Sin^1",                 Sc_Math_Sin);
    ccAddExternalStaticFunction("Maths::Sinh^1",                Sc_Math_Sinh);
    ccAddExternalStaticFunction("Maths::Sqrt^1",                Sc_Math_Sqrt);
    ccAddExternalStaticFunction("Maths::Tan^1",                 Sc_Math_Tan);
    ccAddExternalStaticFunction("Maths::Tanh^1",                Sc_Math_Tanh);
    ccAddExternalStaticFunction("Maths::get_Pi",                Sc_Math_GetPi);

    /* ----------------------- Registering unsafe exports for plugins -----------------------*/

    ccAddExternalFunctionForPlugin("Maths::ArcCos^1",              (void*)Math_ArcCos);
    ccAddExternalFunctionForPlugin("Maths::ArcSin^1",              (void*)Math_ArcSin);
    ccAddExternalFunctionForPlugin("Maths::ArcTan^1",              (void*)Math_ArcTan);
    ccAddExternalFunctionForPlugin("Maths::ArcTan2^2",             (void*)Math_ArcTan2);
    ccAddExternalFunctionForPlugin("Maths::Cos^1",                 (void*)Math_Cos);
    ccAddExternalFunctionForPlugin("Maths::Cosh^1",                (void*)Math_Cosh);
    ccAddExternalFunctionForPlugin("Maths::DegreesToRadians^1",    (void*)Math_DegreesToRadians);
    ccAddExternalFunctionForPlugin("Maths::Exp^1",                 (void*)Math_Exp);
    ccAddExternalFunctionForPlugin("Maths::Log^1",                 (void*)Math_Log);
    ccAddExternalFunctionForPlugin("Maths::Log10^1",               (void*)Math_Log10);
    ccAddExternalFunctionForPlugin("Maths::RadiansToDegrees^1",    (void*)Math_RadiansToDegrees);
    ccAddExternalFunctionForPlugin("Maths::RaiseToPower^2",        (void*)Math_RaiseToPower);
    ccAddExternalFunctionForPlugin("Maths::Sin^1",                 (void*)Math_Sin);
    ccAddExternalFunctionForPlugin("Maths::Sinh^1",                (void*)Math_Sinh);
    ccAddExternalFunctionForPlugin("Maths::Sqrt^1",                (void*)Math_Sqrt);
    ccAddExternalFunctionForPlugin("Maths::Tan^1",                 (void*)Math_Tan);
    ccAddExternalFunctionForPlugin("Maths::Tanh^1",                (void*)Math_Tanh);
    ccAddExternalFunctionForPlugin("Maths::get_Pi",                (void*)Math_GetPi);
}
