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

#include "util/filestream.h"

#include <stdexcept>
#include "util/stdio_compat.h"
#include "util/string.h"

namespace AGS
{
namespace Common
{

FileStream::FileStream(const String &file_name, FileOpenMode open_mode, FileWorkMode work_mode,
            DataEndianess stream_endianess)
    : DataStream(stream_endianess)
    , _file(nullptr)
    , _openMode(open_mode)
    , _workMode(work_mode)
{
    Open(file_name, open_mode, work_mode);
}

FileStream::~FileStream()
{
    FileStream::Close();
}

bool FileStream::HasErrors() const
{
    return IsValid() && ferror(_file) != 0;
}

void FileStream::Close()
{
    if (_file)
    {
        fclose(_file);
    }
    _file = nullptr;
}

bool FileStream::Flush()
{
    if (_file)
    {
        return fflush(_file) == 0;
    }
    return false;
}

bool FileStream::IsValid() const
{
    return _file != nullptr;
}

bool FileStream::EOS() const
{
    return !IsValid() || feof(_file) != 0;
}

soff_t FileStream::GetLength() const
{
    if (IsValid())
    {
        soff_t pos = (soff_t)ags_ftell(_file);
        ags_fseek(_file, 0, SEEK_END);
        soff_t end = (soff_t)ags_ftell(_file);
        ags_fseek(_file, pos, SEEK_SET);
        return end;
    }

    return 0;
}

soff_t FileStream::GetPosition() const
{
    if (IsValid())
    {
        return (soff_t) ags_ftell(_file);
    }
    return -1;
}

bool FileStream::CanRead() const
{
    return IsValid() && _workMode != kFile_Write;
}

bool FileStream::CanWrite() const
{
    return IsValid() && _workMode != kFile_Read;
}

bool FileStream::CanSeek() const
{
    return IsValid();
}

size_t FileStream::Read(void *buffer, size_t size)
{
    if (_file && buffer)
    {
        return fread(buffer, sizeof(uint8_t), size, _file);
    }
    return 0;
}

int32_t FileStream::ReadByte()
{
    if (_file)
    {
        return fgetc(_file);
    }
    return -1;
}

size_t FileStream::Write(const void *buffer, size_t size)
{
    if (_file && buffer)
    {
        return fwrite(buffer, sizeof(uint8_t), size, _file);
    }
    return 0;
}

int32_t FileStream::WriteByte(uint8_t val)
{
    if (_file)
    {
        return fputc(val, _file);
    }
    return -1;
}

bool FileStream::Seek(soff_t offset, StreamSeek origin)
{
    if (!_file)
    {
        return false;
    }

    int stdclib_origin;
    switch (origin)
    {
    case kSeekBegin:    stdclib_origin = SEEK_SET; break;
    case kSeekCurrent:  stdclib_origin = SEEK_CUR; break;
    case kSeekEnd:      stdclib_origin = SEEK_END; break;
    default:
        // TODO: warning to the log
        return false;
    }

    return ags_fseek(_file, (file_off_t)offset, stdclib_origin) == 0;
}

void FileStream::Open(const String &file_name, FileOpenMode open_mode, FileWorkMode work_mode)
{
    String mode = File::GetCMode(open_mode, work_mode);
    if (mode.IsEmpty())
        throw std::runtime_error("Error determining open mode");
    _file = fopen(file_name, mode);
    if (_file == nullptr)
        throw std::runtime_error("Error opening file.");
}

} // namespace Common
} // namespace AGS
