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

#ifndef AGS_PLUGINS_CORE_STRING_H
#define AGS_PLUGINS_CORE_STRING_H

#include "ags/plugins/plugin_base.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

class String : public ScriptContainer {
public:
	void AGS_EngineStartup(IAGSEngine *engine);

	static void IsNullOrEmpty(ScriptMethodParams &params);
	static void Append(ScriptMethodParams &params);
	static void AppendChar(ScriptMethodParams &params);
	static void CompareTo(ScriptMethodParams &params);
	static void StrContains(ScriptMethodParams &params);
	static void Copy(ScriptMethodParams &params);
	static void EndsWith(ScriptMethodParams &params);
	static void ScPl_String_Format(ScriptMethodParams &params);
	static void LowerCase(ScriptMethodParams &params);
	static void Replace(ScriptMethodParams &params);
	static void ReplaceCharAt(ScriptMethodParams &params);
	static void StartsWith(ScriptMethodParams &params);
	static void Substring(ScriptMethodParams &params);
	static void Truncate(ScriptMethodParams &params);
	static void UpperCase(ScriptMethodParams &params);
	static void StringToFloat(ScriptMethodParams &params);
	static void StringToInt(ScriptMethodParams &params);
	static void GetChars(ScriptMethodParams &params);
	static void strlen(ScriptMethodParams &params);
};

} // namespace Core
} // namespace Plugins
} // namespace AGS3

#endif
