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

//include <time.h>
#include "ags/shared/ac/global_datetime.h"
#include "ags/shared/ac/datetime.h"
#include "ags/shared/ac/common.h"

namespace AGS3 {

int sc_GetTime(int whatti) {
	ScriptDateTime *sdt = DateTime_Now_Core();
	int returnVal = 0;

	if (whatti == 1) returnVal = sdt->hour;
	else if (whatti == 2) returnVal = sdt->minute;
	else if (whatti == 3) returnVal = sdt->second;
	else if (whatti == 4) returnVal = sdt->day;
	else if (whatti == 5) returnVal = sdt->month;
	else if (whatti == 6) returnVal = sdt->year;
	else quit("!GetTime: invalid parameter passed");

	delete sdt;

	return returnVal;
}

int GetRawTime() {
	// TODO: we might need to modify script API to support larger time type
	return static_cast<int>(time(nullptr));
}

} // namespace AGS3
