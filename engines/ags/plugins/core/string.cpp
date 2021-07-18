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

#include "ags/plugins/core/string.h"
#include "ags/engine/ac/string.h"
#include "ags/engine/ac/math.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void String::AGS_EngineStartup(IAGSEngine *engine) {
	SCRIPT_METHOD_EXT(String::IsNullOrEmpty^1, IsNullOrEmpty);
	SCRIPT_METHOD_EXT(String::Append^1, Append);
	SCRIPT_METHOD_EXT(String::AppendChar^1, AppendChar);
	SCRIPT_METHOD_EXT(String::CompareTo^2, CompareTo);
	SCRIPT_METHOD_EXT(String::Contains^1, StrContains);
	SCRIPT_METHOD_EXT(String::Copy^0, Copy);
	SCRIPT_METHOD_EXT(String::EndsWith^2, EndsWith);
	SCRIPT_METHOD_EXT(String::Format^101, ScPl_String_Format);
	SCRIPT_METHOD_EXT(String::IndexOf^1, StrContains);
	SCRIPT_METHOD_EXT(String::LowerCase^0, LowerCase);
	SCRIPT_METHOD_EXT(String::Replace^3, Replace);
	SCRIPT_METHOD_EXT(String::ReplaceCharAt^2, ReplaceCharAt);
	SCRIPT_METHOD_EXT(String::StartsWith^2, StartsWith);
	SCRIPT_METHOD_EXT(String::Substring^2, Substring);
	SCRIPT_METHOD_EXT(String::Truncate^1, Truncate);
	SCRIPT_METHOD_EXT(String::UpperCase^0, UpperCase);
	SCRIPT_METHOD_EXT(String::get_AsFloat, StringToFloat);
	SCRIPT_METHOD_EXT(String::get_AsInt, StringToInt);
	SCRIPT_METHOD_EXT(String::geti_Chars, GetChars);
	SCRIPT_METHOD_EXT(String::get_Length, strlen);
}

void String::IsNullOrEmpty(ScriptMethodParams &params) {
	PARAMS1(const char *, str);
	params._result = AGS3::String_IsNullOrEmpty(str);
}

void String::Append(ScriptMethodParams &params) {
	PARAMS2(const char *, str, const char *, extrabit);
	params._result = AGS3::String_Append(str, extrabit);
}

void String::AppendChar(ScriptMethodParams &params) {
	PARAMS2(const char *, str, char, extraOne);
	params._result = AGS3::String_AppendChar(str, extraOne);
}

void String::CompareTo(ScriptMethodParams &params) {
	PARAMS3(const char *, str1, const char *, str2, bool, caseSensitive);
	params._result = AGS3::String_CompareTo(str1, str2, caseSensitive);
}

void String::StrContains(ScriptMethodParams &params) {
	PARAMS2(const char *, s1, const char *, s2);
	params._result = AGS3::StrContains(s1, s2);
}

void String::Copy(ScriptMethodParams &params) {
	PARAMS1(const char *, str);
	params._result = AGS3::String_Copy(str);
}

void String::EndsWith(ScriptMethodParams &params) {
	PARAMS3(const char *, s1, const char *, s2, bool, caseSensitive);
	params._result = AGS3::String_EndsWith(s1, s2, caseSensitive);
}

void String::ScPl_String_Format(ScriptMethodParams &params) {
	Common::String text = params.format(0);
	params._result = AGS3::CreateNewScriptString(text.c_str());
}

void String::LowerCase(ScriptMethodParams &params) {
	PARAMS1(const char *, str);
	params._result = AGS3::String_LowerCase(str);
}

void String::Replace(ScriptMethodParams &params) {
	PARAMS4(const char *, thisString, const char *, lookForText, const char *, replaceWithText, bool, caseSensitive);
	params._result = AGS3::String_Replace(thisString, lookForText, replaceWithText, caseSensitive);
}

void String::ReplaceCharAt(ScriptMethodParams &params) {
	PARAMS3(const char *, thisString, int, index, char, newChar);
	params._result = AGS3::String_ReplaceCharAt(thisString, index, newChar);
}

void String::StartsWith(ScriptMethodParams &params) {
	PARAMS3(const char *, thisString, const char *, checkForString, bool, caseSensitive);
	params._result = AGS3::String_StartsWith(thisString, checkForString, caseSensitive);
}

void String::Substring(ScriptMethodParams &params) {
	PARAMS3(const char *, thisString, int, index, int, length);
	params._result = AGS3::String_Substring(thisString, index, length);
}

void String::Truncate(ScriptMethodParams &params) {
	PARAMS2(const char *, thisString, int, length);
	params._result = AGS3::String_Truncate(thisString, length);
}

void String::UpperCase(ScriptMethodParams &params) {
	PARAMS1(const char *, str);
	params._result = AGS3::String_UpperCase(str);
}

void String::StringToFloat(ScriptMethodParams &params) {
	PARAMS1(const char *, value);
	params._result = AGS3::StringToFloat(value);
}

void String::StringToInt(ScriptMethodParams &params) {
	PARAMS1(const char *, value);
	params._result = AGS3::StringToInt(value);
}

void String::GetChars(ScriptMethodParams &params) {
	PARAMS2(const char *, texx, int, index);
	params._result = AGS3::String_GetChars(texx, index);
}

void String::strlen(ScriptMethodParams &params) {
	PARAMS1(const char *, s);
	params._result = ::strlen(s);
}

} // namespace Core
} // namespace Plugins
} // namespace AGS3
