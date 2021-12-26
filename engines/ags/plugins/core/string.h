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

#ifndef AGS_PLUGINS_CORE_STRING_H
#define AGS_PLUGINS_CORE_STRING_H

#include "ags/plugins/ags_plugin.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

class String : public ScriptContainer {
	BUILT_IN_HASH(String)
public:
	virtual ~String() {}
	void AGS_EngineStartup(IAGSEngine *engine) override;

	void IsNullOrEmpty(ScriptMethodParams &params);
	void Append(ScriptMethodParams &params);
	void AppendChar(ScriptMethodParams &params);
	void CompareTo(ScriptMethodParams &params);
	void StrContains(ScriptMethodParams &params);
	void Copy(ScriptMethodParams &params);
	void EndsWith(ScriptMethodParams &params);
	void ScPl_String_Format(ScriptMethodParams &params);
	void LowerCase(ScriptMethodParams &params);
	void Replace(ScriptMethodParams &params);
	void ReplaceCharAt(ScriptMethodParams &params);
	void StartsWith(ScriptMethodParams &params);
	void Substring(ScriptMethodParams &params);
	void Truncate(ScriptMethodParams &params);
	void UpperCase(ScriptMethodParams &params);
	void StringToFloat(ScriptMethodParams &params);
	void StringToInt(ScriptMethodParams &params);
	void GetChars(ScriptMethodParams &params);
	void strlen(ScriptMethodParams &params);
};

} // namespace Core
} // namespace Plugins
} // namespace AGS3

#endif
