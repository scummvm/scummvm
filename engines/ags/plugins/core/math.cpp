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

#include "ags/plugins/core/math.h"
#include "ags/engine/ac/math.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void Math::AGS_EngineStartup(IAGSEngine *engine) {
	SCRIPT_METHOD_EXT(Maths::ArcCos^1, ArcCos);
	SCRIPT_METHOD_EXT(Maths::ArcSin^1, ArcSin);
	SCRIPT_METHOD_EXT(Maths::ArcTan^1, ArcTan);
	SCRIPT_METHOD_EXT(Maths::ArcTan2^2, ArcTan2);
	SCRIPT_METHOD_EXT(Maths::Cos^1, Cos);
	SCRIPT_METHOD_EXT(Maths::Cosh^1, Cosh);
	SCRIPT_METHOD_EXT(Maths::DegreesToRadians^1, DegreesToRadians);
	SCRIPT_METHOD_EXT(Maths::Exp^1, Exp);
	SCRIPT_METHOD_EXT(Maths::Log^1, Log);
	SCRIPT_METHOD_EXT(Maths::Log10^1, Log10);
	SCRIPT_METHOD_EXT(Maths::RadiansToDegrees^1, RadiansToDegrees);
	SCRIPT_METHOD_EXT(Maths::RaiseToPower^2, RaiseToPower);
	SCRIPT_METHOD_EXT(Maths::Sin^1, Sin);
	SCRIPT_METHOD_EXT(Maths::Sinh^1, Sinh);
	SCRIPT_METHOD_EXT(Maths::Sqrt^1, Sqrt);
	SCRIPT_METHOD_EXT(Maths::Tan^1, Tan);
	SCRIPT_METHOD_EXT(Maths::Tanh^1, Tanh);
	SCRIPT_METHOD_EXT(Maths::get_Pi, GetPi);
}

void Math::ArcCos(ScriptMethodParams &params) {
	PARAMS1(float, value);
	params._result = AGS3::Math_ArcCos(value);
}

void Math::ArcSin(ScriptMethodParams &params) {
	PARAMS1(float, value);
	params._result = AGS3::Math_ArcSin(value);
}

void Math::ArcTan(ScriptMethodParams &params) {
	PARAMS1(float, value);
	params._result = AGS3::Math_ArcTan(value);
}

void Math::ArcTan2(ScriptMethodParams &params) {
	PARAMS2(float, yval, float, xval);
	params._result = AGS3::Math_ArcTan2(xval, yval);
}

void Math::Cos(ScriptMethodParams &params) {
	PARAMS1(float, value);
	params._result = AGS3::Math_Cos(value);
}

void Math::Cosh(ScriptMethodParams &params) {
	PARAMS1(float, value);
	params._result = AGS3::Math_Cosh(value);
}

void Math::DegreesToRadians(ScriptMethodParams &params) {
	PARAMS1(float, value);
	params._result = AGS3::Math_DegreesToRadians(value);
}

void Math::Exp(ScriptMethodParams &params) {
	PARAMS1(float, value);
	params._result = AGS3::Math_Exp(value);
}

void Math::Log(ScriptMethodParams &params) {
	PARAMS1(float, value);
	params._result = AGS3::Math_Log(value);
}

void Math::Log10(ScriptMethodParams &params) {
	PARAMS1(float, value);
	params._result = AGS3::Math_Log10(value);
}

void Math::RadiansToDegrees(ScriptMethodParams &params) {
	PARAMS1(float, value);
	params._result = AGS3::Math_RadiansToDegrees(value);
}

void Math::RaiseToPower(ScriptMethodParams &params) {
	PARAMS2(float, base, float, exp);
	params._result = AGS3::Math_RaiseToPower(base, exp);
}

void Math::Sin(ScriptMethodParams &params) {
	PARAMS1(float, value);
	params._result = AGS3::Math_Sin(value);
}

void Math::Sinh(ScriptMethodParams &params) {
	PARAMS1(float, value);
	params._result = AGS3::Math_Sinh(value);
}

void Math::Sqrt(ScriptMethodParams &params) {
	PARAMS1(float, value);
	params._result = AGS3::Math_Sqrt(value);
}

void Math::Tan(ScriptMethodParams &params) {
	PARAMS1(float, value);
	params._result = AGS3::Math_Tan(value);
}

void Math::Tanh(ScriptMethodParams &params) {
	PARAMS1(float, value);
	params._result = AGS3::Math_Tanh(value);
}

void Math::GetPi(ScriptMethodParams &params) {
	params._result = AGS3::Math_GetPi();
}

} // namespace Core
} // namespace Plugins
} // namespace AGS3
