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

#ifndef AGS_ENGINE_AC_DATETIME_H
#define AGS_ENGINE_AC_DATETIME_H

#include "ags/engine/ac/dynobj/script_date_time.h"

namespace AGS3 {

ScriptDateTime *DateTime_Now_Core();
ScriptDateTime *DateTime_Now();
int             DateTime_GetYear(ScriptDateTime *sdt);
int             DateTime_GetMonth(ScriptDateTime *sdt);
int             DateTime_GetDayOfMonth(ScriptDateTime *sdt);
int             DateTime_GetHour(ScriptDateTime *sdt);
int             DateTime_GetMinute(ScriptDateTime *sdt);
int             DateTime_GetSecond(ScriptDateTime *sdt);
int             DateTime_GetRawTime(ScriptDateTime *sdt);

} // namespace AGS3

#endif
