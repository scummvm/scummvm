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

#include <string.h>
#include "debug/logfile.h"
#include "util/file.h"
#include "util/stream.h"


namespace AGS
{
namespace Engine
{

using namespace Common;

LogFile::LogFile()
    : _openMode(kLogFile_Overwrite)
{
}

void LogFile::PrintMessage(const DebugMessage &msg)
{
    if (!_file.get())
    {
        if (_filePath.IsEmpty())
            return;
        _file.reset(File::OpenFile(_filePath, _openMode == kLogFile_Append ? Common::kFile_Create : Common::kFile_CreateAlways,
            Common::kFile_Write));
        if (!_file)
        {
            Debug::Printf("Unable to write log to '%s'.", _filePath.GetCStr());
            _filePath = "";
            return;
        }
    }

    if (!msg.GroupName.IsEmpty())
    {
        _file->Write(msg.GroupName, msg.GroupName.GetLength());
        _file->Write(" : ", 3);
    }
    _file->Write(msg.Text, msg.Text.GetLength());
    _file->WriteInt8('\n');
    // We should flush after every write to the log; this will make writing
    // bit slower, but will increase the chances that all latest output
    // will get to the disk in case of program crash.
    _file->Flush();
}

bool LogFile::OpenFile(const String &file_path, OpenMode open_mode)
{
    CloseFile();

    _filePath = file_path;
    _openMode = open_mode;
    if (open_mode == OpenMode::kLogFile_OverwriteAtFirstMessage)
    {
        return File::TestWriteFile(_filePath);
    }
    else
    {
        _file.reset(File::OpenFile(file_path,
                           open_mode == kLogFile_Append ? Common::kFile_Create : Common::kFile_CreateAlways,
                           Common::kFile_Write));
        return _file.get() != nullptr;
    }
}

void LogFile::CloseFile()
{
    _file.reset();
    _filePath.Empty();
}

} // namespace Engine
} // namespace AGS
