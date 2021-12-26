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

#include "ags/plugins/core/date_time.h"
#include "ags/engine/ac/date_time.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void DateTime::AGS_EngineStartup(IAGSEngine *engine) {
	ScriptContainer::AGS_EngineStartup(engine);

	SCRIPT_METHOD(DateTime::get_Now, DateTime::Now);
	SCRIPT_METHOD(DateTime::get_DayOfMonth, DateTime::GetDayOfMonth);
	SCRIPT_METHOD(DateTime::get_Hour, DateTime::GetHour);
	SCRIPT_METHOD(DateTime::get_Minute, DateTime::GetMinute);
	SCRIPT_METHOD(DateTime::get_Month, DateTime::GetMonth);
	SCRIPT_METHOD(DateTime::get_RawTime, DateTime::GetRawTime);
	SCRIPT_METHOD(DateTime::get_Second, DateTime::GetSecond);
	SCRIPT_METHOD(DateTime::get_Year, DateTime::GetYear);
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
