
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
#include "bagel/boflib/error.h"

namespace Bagel {

/**
 * Rename a file
 * @param pszOldName        Name of file to rename
 * @param pszNewName        New name for file
 * @return                  Error return code
 */
extern ErrorCode FileRename(const char *pszOldName, const char *pszNewName);

/**
 * Delete specified file
 * @param fileName          File to delete
 * @return                  Error return code
 */
extern ErrorCode FileDelete(const char *fileName);

/**
 * Returns a unique file name
 * @param buf       Buffer to fill with new unique name
 * @return          Pointer to buffer containing new unique file name
 **/
extern char *FileTempName(char *buf);

/**
 * Checks if 'path' is a valid directory
 * @param path      Path name to test
 * @return          true if specified entry is a directory, else false
 **/
extern bool FileIsDirectory(const char *path);

/**
 * Checks to see if specified file exists
 * @param pszFileName   Name of file to check existance
 * @return              true if file exists, false if file does not exist
 *
 **/
// for mac, use this routine to replace the diskid
extern bool FileExists(const char *pszFileName);

/**
 * Gets length of file in bytes (via file name)
 * @param pszFileName       Name of file to get length for
 * @return                  Size of specified file (or -1 if not exist or error)
 */
extern int32 FileLength(const char *pszFileName);

extern char *FileGetFullPath(char *pszDstBuf, const char *pszSrcBuf);

/**
 * Determines if current drive is write protected
 * @return      Returns true if drive is write protected as in a CD-ROM drive.
 */
inline bool IsDriveWriteLocked() {
	return true;
}

/**
 * Gets number of free bytes on specified drive
 */
inline int32 GetFreeDiskSpace(const char *pszDrive) {
	return 999999;
}

/**
 * Retrieves the full path for the current working directory
 * @param pszDirectory      Buffer to hold full path
 * @return                  Error return code
 */
extern ErrorCode GetCurrentDir(char *pszDirectory);

/**
 * Sets the current working directory to that specified
 * @param pszDirectory      New directory to switch to
 * @return                  Error return code
 */
extern ErrorCode SetCurrentDir(char *pszDirectory);

/**
 * Sets the current working directory to the System Dir
 * @return      Error return code
 */
extern ErrorCode GotoSystemDir();

/**
 * Retrieves the full path to the System Directory
 * @param pszDiretory       Buffer to hold name of system dir
 * @return                  Error return code
 */
extern ErrorCode GetSystemDir(char *pszDirectory);

extern void GetInstallPath(char *pszDirectory);

extern void SetInstallPath(char *pszDirectory);

/**
 * Takes a directory and filename, merging them to form a
 * fully qualified filename. Also takes care of special folder
 * aliases at the start of the directory name
 */
extern const char *formPath(const char *dir, const char *pszFile);

} // namespace Bagel

#endif
