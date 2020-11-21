//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
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
