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
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_ERRORS_H
#define SAGA2_ERRORS_H

namespace Saga2 {

enum errorCodes {
	errOK = 0,                          // no error
	errUnknown,                         // unspecified error
	errNoMemory,                        // couldn't get memory
	errNoMouse,                         // couldn't find mouse driver
	errNoFont,                          // couldn't load a font
	errNoFile,                          // file or directory not found
	errNoAccess,                        // Access denied

	// hardware error codes
	errDeviceError,
	errWriteProtect,
	errDriveNotReady,
	errDiskError
};

const int errDosError = (1 << 15);      // an error from 'errno'

typedef int16       errorCode;          // typedef for errors

/* ===================================================================== *
                            Error Functions
 * ===================================================================== */

#ifdef __cplusplus
extern "C" {
#endif

void fatal(char *msg, ...);
void memerr(void);
void checkAlloc(void *ptr);
errorCode dosErrCode(void);

#ifdef __cplusplus
}
#endif

} // end of namespace Saga2

#endif
