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

#include "ags/engine/ac/date_time.h"
#include "ags/engine/ac/dynobj/dynobj_manager.h"
#include "ags/engine/platform/base/ags_platform_driver.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/script/script_api.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/globals.h"

namespace AGS3 {

ScriptDateTime *DateTime_Now_Core() {
	ScriptDateTime *sdt = new ScriptDateTime();

	_G(platform)->GetSystemTime(sdt);

	return sdt;
}

ScriptDateTime *DateTime_Now() {
	ScriptDateTime *sdt = DateTime_Now_Core();
	ccRegisterManagedObject(sdt, sdt);
	return sdt;
}

int DateTime_GetYear(ScriptDateTime *sdt) {
	return sdt->year;
}

int DateTime_GetMonth(ScriptDateTime *sdt) {
	return sdt->month;
}

int DateTime_GetDayOfMonth(ScriptDateTime *sdt) {
	return sdt->day;
}

int DateTime_GetHour(ScriptDateTime *sdt) {
	return sdt->hour;
}

int DateTime_GetMinute(ScriptDateTime *sdt) {
	return sdt->minute;
}

int DateTime_GetSecond(ScriptDateTime *sdt) {
	return sdt->second;
}

int DateTime_GetRawTime(ScriptDateTime *sdt) {
	return sdt->rawUnixTime;
}

//=============================================================================
//
// Script API Functions
//
//=============================================================================

// ScriptDateTime* ()
RuntimeScriptValue Sc_DateTime_Now(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJAUTO(ScriptDateTime, DateTime_Now);
}

// int (ScriptDateTime *sdt)
RuntimeScriptValue Sc_DateTime_GetYear(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptDateTime, DateTime_GetYear);
}

// int (ScriptDateTime *sdt)
RuntimeScriptValue Sc_DateTime_GetMonth(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptDateTime, DateTime_GetMonth);
}

// int (ScriptDateTime *sdt)
RuntimeScriptValue Sc_DateTime_GetDayOfMonth(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptDateTime, DateTime_GetDayOfMonth);
}

// int (ScriptDateTime *sdt)
RuntimeScriptValue Sc_DateTime_GetHour(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptDateTime, DateTime_GetHour);
}

// int (ScriptDateTime *sdt)
RuntimeScriptValue Sc_DateTime_GetMinute(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptDateTime, DateTime_GetMinute);
}

// int (ScriptDateTime *sdt)
RuntimeScriptValue Sc_DateTime_GetSecond(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptDateTime, DateTime_GetSecond);
}

// int (ScriptDateTime *sdt)
RuntimeScriptValue Sc_DateTime_GetRawTime(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptDateTime, DateTime_GetRawTime);
}

void RegisterDateTimeAPI() {
	ScFnRegister datetime_api[] = {
		{"DateTime::get_Now", API_FN_PAIR(DateTime_Now)},

		{"DateTime::get_DayOfMonth", API_FN_PAIR(DateTime_GetDayOfMonth)},
		{"DateTime::get_Hour", API_FN_PAIR(DateTime_GetHour)},
		{"DateTime::get_Minute", API_FN_PAIR(DateTime_GetMinute)},
		{"DateTime::get_Month", API_FN_PAIR(DateTime_GetMonth)},
		{"DateTime::get_RawTime", API_FN_PAIR(DateTime_GetRawTime)},
		{"DateTime::get_Second", API_FN_PAIR(DateTime_GetSecond)},
		{"DateTime::get_Year", API_FN_PAIR(DateTime_GetYear)},
	};

	ccAddExternalFunctions361(datetime_api);
}

} // namespace AGS3
