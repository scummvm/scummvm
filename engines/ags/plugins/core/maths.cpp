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

#include "ags/plugins/core/maths.h"
#include "ags/engine/ac/math.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void Maths::AGS_EngineStartup(IAGSEngine *engine) {
	ScriptContainer::AGS_EngineStartup(engine);

	SCRIPT_METHOD(Maths::ArcCos^1, Maths::ArcCos);
	SCRIPT_METHOD(Maths::ArcSin^1, Maths::ArcSin);
	SCRIPT_METHOD(Maths::ArcTan^1, Maths::ArcTan);
	SCRIPT_METHOD(Maths::ArcTan2^2, Maths::ArcTan2);
	SCRIPT_METHOD(Maths::Cos^1, Maths::Cos);
	SCRIPT_METHOD(Maths::Cosh^1, Maths::Cosh);
	SCRIPT_METHOD(Maths::DegreesToRadians^1, Maths::DegreesToRadians);
	SCRIPT_METHOD(Maths::Exp^1, Maths::Exp);
	SCRIPT_METHOD(Maths::Log^1, Maths::Log);
	SCRIPT_METHOD(Maths::Log10^1, Maths::Log10);
	SCRIPT_METHOD(Maths::RadiansToDegrees^1, Maths::RadiansToDegrees);
	SCRIPT_METHOD(Maths::RaiseToPower^2, Maths::RaiseToPower);
	SCRIPT_METHOD(Maths::Sin^1, Maths::Sin);
	SCRIPT_METHOD(Maths::Sinh^1, Maths::Sinh);
	SCRIPT_METHOD(Maths::Sqrt^1, Maths::Sqrt);
	SCRIPT_METHOD(Maths::Tan^1, Maths::Tan);
	SCRIPT_METHOD(Maths::Tanh^1, Maths::Tanh);
	SCRIPT_METHOD(Maths::get_Pi, Maths::GetPi);
}

void Maths::ArcCos(ScriptMethodParams &params) {
	PARAMS1(float, value);
	params._result = AGS3::Math_ArcCos(value);
}

void Maths::ArcSin(ScriptMethodParams &params) {
	PARAMS1(float, value);
	params._result = AGS3::Math_ArcSin(value);
}

void Maths::ArcTan(ScriptMethodParams &params) {
	PARAMS1(float, value);
	params._result = AGS3::Math_ArcTan(value);
}

void Maths::ArcTan2(ScriptMethodParams &params) {
	PARAMS2(float, yval, float, xval);
	params._result = AGS3::Math_ArcTan2(yval, xval);
}

void Maths::Cos(ScriptMethodParams &params) {
	PARAMS1(float, value);
	params._result = AGS3::Math_Cos(value);
}

void Maths::Cosh(ScriptMethodParams &params) {
	PARAMS1(float, value);
	params._result = AGS3::Math_Cosh(value);
}

void Maths::DegreesToRadians(ScriptMethodParams &params) {
	PARAMS1(float, value);
	params._result = AGS3::Math_DegreesToRadians(value);
}

void Maths::Exp(ScriptMethodParams &params) {
	PARAMS1(float, value);
	params._result = AGS3::Math_Exp(value);
}

void Maths::Log(ScriptMethodParams &params) {
	PARAMS1(float, value);
	params._result = AGS3::Math_Log(value);
}

void Maths::Log10(ScriptMethodParams &params) {
	PARAMS1(float, value);
	params._result = AGS3::Math_Log10(value);
}

void Maths::RadiansToDegrees(ScriptMethodParams &params) {
	PARAMS1(float, value);
	params._result = AGS3::Math_RadiansToDegrees(value);
}

void Maths::RaiseToPower(ScriptMethodParams &params) {
	PARAMS2(float, base, float, exp);
	params._result = AGS3::Math_RaiseToPower(base, exp);
}

void Maths::Sin(ScriptMethodParams &params) {
	PARAMS1(float, value);
	params._result = AGS3::Math_Sin(value);
}

void Maths::Sinh(ScriptMethodParams &params) {
	PARAMS1(float, value);
	params._result = AGS3::Math_Sinh(value);
}

void Maths::Sqrt(ScriptMethodParams &params) {
	PARAMS1(float, value);
	params._result = AGS3::Math_Sqrt(value);
}

void Maths::Tan(ScriptMethodParams &params) {
	PARAMS1(float, value);
	params._result = AGS3::Math_Tan(value);
}

void Maths::Tanh(ScriptMethodParams &params) {
	PARAMS1(float, value);
	params._result = AGS3::Math_Tanh(value);
}

void Maths::GetPi(ScriptMethodParams &params) {
	params._result = AGS3::Math_GetPi();
}

} // namespace Core
} // namespace Plugins
} // namespace AGS3
