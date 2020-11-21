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

//=============================================================================
//
// Platform-independent File functions
//
//=============================================================================

#ifndef __AGS_CN_UTIL__FILE_H
#define __AGS_CN_UTIL__FILE_H

#include "core/platform.h"
#include "util/string.h"

namespace AGS
{
namespace Common
{

// Forward declarations
class Stream;

enum FileOpenMode
{
    kFile_Open,         // Open existing file
    kFile_Create,       // Create new file, or open existing one
    kFile_CreateAlways  // Always create a new file, replacing any existing one
};

enum FileWorkMode
{
    kFile_Read,
    kFile_Write,
    kFile_ReadWrite
};

namespace File
{
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

    // Sets FileOpenMode and FileWorkMode values corresponding to C-style file open mode string
    bool        GetFileModesFromCMode(const String &cmode, FileOpenMode &open_mode, FileWorkMode &work_mode);
    // Gets C-style file mode from FileOpenMode and FileWorkMode
    String      GetCMode(FileOpenMode open_mode, FileWorkMode work_mode);

    Stream      *OpenFile(const String &filename, FileOpenMode open_mode, FileWorkMode work_mode);
    // Convenience helpers
    // Create a totally new file, overwrite existing one
    inline Stream *CreateFile(const String &filename)
    {
        return OpenFile(filename, kFile_CreateAlways, kFile_Write);
    }
    // Open existing file for reading
    inline Stream *OpenFileRead(const String &filename)
    {
        return OpenFile(filename, kFile_Open, kFile_Read);
    }
    // Open existing file for writing (append) or create if it does not exist
    inline Stream *OpenFileWrite(const String &filename)
    {
        return OpenFile(filename, kFile_Create, kFile_Write);
    }
} // namespace File

} // namespace Common
} // namespace AGS

#endif // __AGS_CN_UTIL__FILE_H
