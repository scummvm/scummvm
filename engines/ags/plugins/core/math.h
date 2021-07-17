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

#ifndef AGS_PLUGINS_CORE_MATH_H
#define AGS_PLUGINS_CORE_MATH_H

#include "ags/plugins/plugin_base.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

class Math : public ScriptContainer {
public:
	void AGS_EngineStartup(IAGSEngine *engine);

	static void ArcCos(ScriptMethodParams &params);
	static void ArcSin(ScriptMethodParams &params);
	static void ArcTan(ScriptMethodParams &params);
	static void ArcTan2(ScriptMethodParams &params);
	static void Cos(ScriptMethodParams &params);
	static void Cosh(ScriptMethodParams &params);
	static void DegreesToRadians(ScriptMethodParams &params);
	static void Exp(ScriptMethodParams &params);
	static void Log(ScriptMethodParams &params);
	static void Log10(ScriptMethodParams &params);
	static void RadiansToDegrees(ScriptMethodParams &params);
	static void RaiseToPower(ScriptMethodParams &params);
	static void Sin(ScriptMethodParams &params);
	static void Sinh(ScriptMethodParams &params);
	static void Sqrt(ScriptMethodParams &params);
	static void Tan(ScriptMethodParams &params);
	static void Tanh(ScriptMethodParams &params);
	static void GetPi(ScriptMethodParams &params);
};

} // namespace Core
} // namespace Plugins
} // namespace AGS3

#endif
