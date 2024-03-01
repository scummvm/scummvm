
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

#include "bagel/boflib/stdinc.h"
#include "bagel/boflib/bof_error.h"

namespace Bagel {

/**
 * Rename a file
 * @param pszOldName		Name of file to rename
 * @param pszNewName		New name for file
 * @return					Error return code
 */
extern ERROR_CODE FileRename(const CHAR *pszOldName, const CHAR *pszNewName);

/**
 * Delete specified file
 * @param fileName			File to delete
 * @return					Error return code
 */
extern ERROR_CODE FileDelete(const CHAR *fileName);

/**
 * Returns a unique file name
 * @param buf		Buffer to fill with new unique name
 * @return			Pointer to buffer containing new unique file name
 **/
extern CHAR *FileTempName(CHAR *buf);

/**
 * Checks if 'path' is a valid directory
 * @param path		Path name to test
 * @return			TRUE if specified entry is a directory, else FALSE
 **/
extern BOOL FileIsDirectory(const CHAR *path);

/**
 * Checks to see if specified file exists
 * @param pszFileName	Name of file to check existance
 * @return				TRUE if file exists, FALSE if file does not exist
 *
 **/
// for mac, use this routine to replace the diskid
extern BOOL FileExists(const CHAR *pszFileName);

/**
 * Gets length of file in bytes (via file name)
 * @param pszFileName		Name of file to get length for
 * @return					Size of specified file (or -1 if not exist or error)
 */
extern LONG FileLength(const CHAR *pszFileName);

extern CHAR *FileGetFullPath(CHAR *pszDstBuf, const CHAR *pszSrcBuf);

/**
 * Determines if current drive is write protected
 * @return		Returns TRUE if drive is write protected as in a CD-ROM drive.
 */
inline BOOL IsDriveWriteLocked() {
	return true;
}

/**
 * Gets number of free bytes on specified drive
 */
inline LONG GetFreeDiskSpace(const CHAR* pszDrive) {
	return 999999;
}

/**
 * Retrieves the full path for the current working directory
 * @param pszDirectory		Buffer to hold full path
 * @return					Error return code
 */
extern ERROR_CODE GetCurrentDir(CHAR *pszDirectory);

/**
 * Sets the current working directory to that specified
 * @param pszDirectory		New directory to switch to
 * @return					Error return code
 */
extern ERROR_CODE SetCurrentDir(CHAR *pszDirectory);

/**
 * Sets the current working directory to the System Dir
 * @return		Error return code
 */
extern ERROR_CODE GotoSystemDir();

/**
 * Retrieves the full path to the System Directory
 * @param pszDiretory		Buffer to hold name of system dir
 * @return					Error return code
 */
extern ERROR_CODE GetSystemDir(CHAR *pszDirectory);

extern VOID GetInstallPath(CHAR *pszDirectory);

extern VOID SetInstallPath(CHAR *pszDirectory);

} // namespace Bagel

#endif
