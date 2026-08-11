/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * of the License, or(at your option) any later version.
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

#include "ags/plugins/ags_maya/ags_maya.h"
#include "ags/engine/ac/math.h"

namespace AGS3 {
namespace Plugins {
namespace AGSMaya {

const char *AGSMaya::AGS_GetPluginName() {
	return "AGS Maya";
}

void AGSMaya::AGS_EngineStartup(IAGSEngine *engine) {
	PluginBase::AGS_EngineStartup(engine);

	SCRIPT_METHOD(sin, AGSMaya::Sin);
	SCRIPT_METHOD(cos, AGSMaya::Cos);
	SCRIPT_METHOD(fmul, AGSMaya::FMul);
	SCRIPT_METHOD(fdiv, AGSMaya::FDiv);
	SCRIPT_METHOD(int_to_float, AGSMaya::intToFloat);
	SCRIPT_METHOD(ints_to_float, AGSMaya::intsToFloat);
	SCRIPT_METHOD(float_to_int, AGSMaya::floatToInt);
}

void AGSMaya::Sin(ScriptMethodParams &params) {
	PARAMS1(int, angle);

	params._result = 1000 * Math_Sin(angle * 0.001f);
}

void AGSMaya::Cos(ScriptMethodParams &params) {
	PARAMS1(int, angle);

	params._result = 1000 * Math_Cos(angle * 0.001f);
}

void AGSMaya::FMul(ScriptMethodParams &params) {
	PARAMS2(int, value1, int, value2);

	int calculation = (value1 / 1000) * (value2 / 1000);
	calculation *= 1000;
	calculation += (((value1 % 1000) * (value2 % 1000)) / 1000);
	params._result = calculation;
}

void AGSMaya::FDiv(ScriptMethodParams &params) {
	PARAMS2(int, value1, int, value2);

	if (value2) {
		double division, decimals;
		division = value1 / (double)value2;
		modf(division, &decimals);
		params._result = floor(division) * 1000 + decimals * 1000;
	} else {
		params._result = 0;
		warning("AGSMaya::FDiv divide by zero!");
	}
}

void AGSMaya::intToFloat(ScriptMethodParams &params) {
	PARAMS1(int, value);

	params._result = value * 1000;
}

void AGSMaya::floatToInt(ScriptMethodParams &params) {
	PARAMS1(int, value);

	if (value % 1000 > 500)
		params._result = (value + 1000) / 1000; // round up
	else
		params._result = value / 1000;
}

void AGSMaya::intsToFloat(ScriptMethodParams &params) {
	PARAMS2(int, units, int, thousandths);

	params._result = units * 1000 + thousandths;
}

} // namespace AGSMaya
} // namespace Plugins
} // namespace AGS3
