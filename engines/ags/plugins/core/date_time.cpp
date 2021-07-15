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

#include "ags/plugins/core/date_time.h"
#include "ags/engine/ac/date_time.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void DateTime::AGS_EngineStartup(IAGSEngine *engine) {
	SCRIPT_METHOD_EXT(DateTime::get_Now, Now);
	SCRIPT_METHOD_EXT(DateTime::get_DayOfMonth, GetDayOfMonth);
	SCRIPT_METHOD_EXT(DateTime::get_Hour, GetHour);
	SCRIPT_METHOD_EXT(DateTime::get_Minute, GetMinute);
	SCRIPT_METHOD_EXT(DateTime::get_Month, GetMonth);
	SCRIPT_METHOD_EXT(DateTime::get_RawTime, GetRawTime);
	SCRIPT_METHOD_EXT(DateTime::get_Second, GetSecond);
	SCRIPT_METHOD_EXT(DateTime::get_Year, GetYear);
}

void DateTime::Now(ScriptMethodParams &params) {
	params._result = AGS3::DateTime_Now();
}

void DateTime::GetDayOfMonth(ScriptMethodParams &params) {
	PARAMS1(ScriptDateTime *, sdt);
	params._result = AGS3::DateTime_GetDayOfMonth(sdt);
}

void DateTime::GetHour(ScriptMethodParams &params) {
	PARAMS1(ScriptDateTime *, sdt);
	params._result = AGS3::DateTime_GetHour(sdt);
}

void DateTime::GetMinute(ScriptMethodParams &params) {
	PARAMS1(ScriptDateTime *, sdt);
	params._result = AGS3::DateTime_GetMinute(sdt);
}

void DateTime::GetMonth(ScriptMethodParams &params) {
	PARAMS1(ScriptDateTime *, sdt);
	params._result = AGS3::DateTime_GetMonth(sdt);
}

void DateTime::GetRawTime(ScriptMethodParams &params) {
	PARAMS1(ScriptDateTime *, sdt);
	params._result = AGS3::DateTime_GetRawTime(sdt);
}

void DateTime::GetSecond(ScriptMethodParams &params) {
	PARAMS1(ScriptDateTime *, sdt);
	params._result = AGS3::DateTime_GetSecond(sdt);
}

void DateTime::GetYear(ScriptMethodParams &params) {
	PARAMS1(ScriptDateTime *, sdt);
	params._result = AGS3::DateTime_GetYear(sdt);
}

} // namespace Core
} // namespace Plugins
} // namespace AGS3
