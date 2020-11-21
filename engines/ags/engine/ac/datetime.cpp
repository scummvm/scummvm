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

#include <time.h>
#include "ac/datetime.h"
#include "platform/base/agsplatformdriver.h"
#include "script/runtimescriptvalue.h"

ScriptDateTime* DateTime_Now_Core() {
    ScriptDateTime *sdt = new ScriptDateTime();

    platform->GetSystemTime(sdt);

    return sdt;
}

ScriptDateTime* DateTime_Now() {
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

#include "debug/out.h"
#include "script/script_api.h"
#include "script/script_runtime.h"

// ScriptDateTime* ()
RuntimeScriptValue Sc_DateTime_Now(const RuntimeScriptValue *params, int32_t param_count)
{
    API_SCALL_OBJAUTO(ScriptDateTime, DateTime_Now);
}

// int (ScriptDateTime *sdt)
RuntimeScriptValue Sc_DateTime_GetYear(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(ScriptDateTime, DateTime_GetYear);
}

// int (ScriptDateTime *sdt)
RuntimeScriptValue Sc_DateTime_GetMonth(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(ScriptDateTime, DateTime_GetMonth);
}

// int (ScriptDateTime *sdt)
RuntimeScriptValue Sc_DateTime_GetDayOfMonth(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(ScriptDateTime, DateTime_GetDayOfMonth);
}

// int (ScriptDateTime *sdt)
RuntimeScriptValue Sc_DateTime_GetHour(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(ScriptDateTime, DateTime_GetHour);
}

// int (ScriptDateTime *sdt)
RuntimeScriptValue Sc_DateTime_GetMinute(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(ScriptDateTime, DateTime_GetMinute);
}

// int (ScriptDateTime *sdt)
RuntimeScriptValue Sc_DateTime_GetSecond(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(ScriptDateTime, DateTime_GetSecond);
}

// int (ScriptDateTime *sdt)
RuntimeScriptValue Sc_DateTime_GetRawTime(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(ScriptDateTime, DateTime_GetRawTime);
}

void RegisterDateTimeAPI()
{
    ccAddExternalStaticFunction("DateTime::get_Now",        Sc_DateTime_Now);
    ccAddExternalObjectFunction("DateTime::get_DayOfMonth", Sc_DateTime_GetDayOfMonth);
    ccAddExternalObjectFunction("DateTime::get_Hour",       Sc_DateTime_GetHour);
    ccAddExternalObjectFunction("DateTime::get_Minute",     Sc_DateTime_GetMinute);
    ccAddExternalObjectFunction("DateTime::get_Month",      Sc_DateTime_GetMonth);
    ccAddExternalObjectFunction("DateTime::get_RawTime",    Sc_DateTime_GetRawTime);
    ccAddExternalObjectFunction("DateTime::get_Second",     Sc_DateTime_GetSecond);
    ccAddExternalObjectFunction("DateTime::get_Year",       Sc_DateTime_GetYear);

    /* ----------------------- Registering unsafe exports for plugins -----------------------*/

    ccAddExternalFunctionForPlugin("DateTime::get_Now",        (void*)DateTime_Now);
    ccAddExternalFunctionForPlugin("DateTime::get_DayOfMonth", (void*)DateTime_GetDayOfMonth);
    ccAddExternalFunctionForPlugin("DateTime::get_Hour",       (void*)DateTime_GetHour);
    ccAddExternalFunctionForPlugin("DateTime::get_Minute",     (void*)DateTime_GetMinute);
    ccAddExternalFunctionForPlugin("DateTime::get_Month",      (void*)DateTime_GetMonth);
    ccAddExternalFunctionForPlugin("DateTime::get_RawTime",    (void*)DateTime_GetRawTime);
    ccAddExternalFunctionForPlugin("DateTime::get_Second",     (void*)DateTime_GetSecond);
    ccAddExternalFunctionForPlugin("DateTime::get_Year",       (void*)DateTime_GetYear);
}
