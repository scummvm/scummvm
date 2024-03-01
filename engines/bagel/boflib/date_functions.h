
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

#ifndef BAGEL_BOFLIB_DATE_FUNCTIONS_H
#define BAGEL_BOFLIB_DATE_FUNCTIONS_H

#include "bagel/boflib/stdinc.h"

namespace Bagel {

/**
 * Fills specified buffer with the current time
 * @param pszBuf	Pointer to buffer to fill
 * @return			Pointer to filled buffer
 */
extern CHAR *strtime(CHAR *pszBuffer);

/**
 * Fills specified buffer with the current date
 * @param pszBuf	Pointer to buffer to fill
 * @return			Pointer to filled buffer
 */
extern CHAR *strdate(CHAR *pszBuffer);

/**
 * Reverses specified string
 * @param pStr		Description of param1
 * @return			Reversed string
 */
extern CHAR *strrev(CHAR *pszBuffer);

/**
 * Converts packed time into sec, min, hour
 */
extern VOID TimeUnpack(USHORT num, USHORT *hour, USHORT *min, USHORT *sec);

/**
 * Converts sec, min, hour into packed time
 */
extern USHORT TimePack(USHORT hour, USHORT min, USHORT sec);

/**
 * Converts packed date into day, month, year
 */
extern VOID DateUnpack(USHORT num, USHORT *year, USHORT *month, USHORT *day);

/**
 * Converts day, month, year into packed date
 */
extern USHORT DatePack(USHORT year, USHORT month, USHORT day);

} // namespace Bagel

#endif
