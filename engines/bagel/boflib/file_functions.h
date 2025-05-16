
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

#ifndef BAGEL_BOFLIB_FILE_FUNCTIONS_H
#define BAGEL_BOFLIB_FILE_FUNCTIONS_H

#include "common/scummsys.h"
#include "bagel/boflib/string.h"

namespace Bagel {

/**
 * Checks to see if specified file exists
 * @param pszFileName   Name of file to check existence
 * @return              true if file exists, false if file does not exist
 *
 **/
// for mac, use this routine to replace the diskid
extern bool fileExists(const char *pszFileName);

/**
 * Gets length of file in bytes (via file name)
 * @param pszFileName       Name of file to get length for
 * @return                  Size of specified file (or -1 if not exist or error)
 */
extern int32 fileLength(const char *pszFileName);

extern char *fileGetFullPath(char *pszDstBuf, const char *pszSrcBuf);


/**
 * Takes a directory and filename, merging them to form a
 * fully qualified filename. Also takes care of special folder
 * aliases at the start of the directory name
 */
extern const char *formPath(const char *dir, const char *pszFile);

extern void fixPathName(CBofString &s);

} // namespace Bagel

#endif
