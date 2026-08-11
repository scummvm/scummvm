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

//=============================================================================
//
// Platform-independent File functions
//
//=============================================================================

#ifndef AGS_SHARED_UTIL_FILE_H
#define AGS_SHARED_UTIL_FILE_H

#include "ags/shared/core/platform.h"
#include "ags/shared/util/string.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

// Forward declarations
class Stream;

enum FileOpenMode {
	kFile_Open,         // Open existing file
	kFile_Create,       // Create new file, or open existing one
	kFile_CreateAlways  // Always create a new file, replacing any existing one
};

enum FileWorkMode {
	kFile_Read,
	kFile_Write,
	kFile_ReadWrite
};

namespace File {
// Tells if the given path is a directory
bool        IsDirectory(const String &directory);
// Tells if the given path is a file
bool        IsFile(const String &filename);
// Tells if the given path is file or directory;
// may be used to check if it's valid to use
bool        IsFileOrDir(const String &filename);
// Returns size of a file, or -1 if no such file found
soff_t      GetFileSize(const String &filename);
// Tests if file could be opened for reading
bool        TestReadFile(const String &filename);
// Opens a file for writing or creates new one if it does not exist; deletes file if it was created during test
bool        TestWriteFile(const String &filename);
// Create new empty file and deletes it; returns TRUE if was able to create file
bool        TestCreateFile(const String &filename);
// Deletes existing file; returns TRUE if was able to delete one
bool        DeleteFile(const String &filename);
// Renames existing file to the new name; returns TRUE on success
bool		RenameFile(const String &old_name, const String &new_name);
// Copies a file from src_path to dst_path; returns TRUE on success
bool		CopyFile(const String &src_path, const String &dst_path, bool overwrite);

// Sets FileOpenMode and FileWorkMode values corresponding to C-style file open mode string
bool        GetFileModesFromCMode(const String &cmode, FileOpenMode &open_mode, FileWorkMode &work_mode);
// Gets C-style file mode from FileOpenMode and FileWorkMode
String      GetCMode(FileOpenMode open_mode, FileWorkMode work_mode);

// Opens file in the given mode
Stream *OpenFile(const String &filename, FileOpenMode open_mode, FileWorkMode work_mode);
// Opens file for reading restricted to the arbitrary offset range
Stream *OpenFile(const String &filename, soff_t start_off, soff_t end_off);
// Convenience helpers
// Create a totally new file, overwrite existing one
inline Stream *CreateFile(const String &filename) {
	return OpenFile(filename, kFile_CreateAlways, kFile_Write);
}
// Open existing file for reading
inline Stream *OpenFileRead(const String &filename) {
	return OpenFile(filename, kFile_Open, kFile_Read);
}
// Open existing file for writing (append) or create if it does not exist
inline Stream *OpenFileWrite(const String &filename) {
	return OpenFile(filename, kFile_Create, kFile_Write);
}

// Opens stdin stream for reading
Stream *OpenStdin();
// Opens stdout stream for writing
Stream *OpenStdout();
// Opens stderr stream for writing
Stream *OpenStderr();

// Case insensitive find file
String FindFileCI(const String &dir_name, const String &file_name);
// Case insensitive file open: looks up for the file using FindFileCI
Stream *OpenFileCI(const String &file_name,
	FileOpenMode open_mode = kFile_Open,
	FileWorkMode work_mode = kFile_Read);

} // namespace File

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
