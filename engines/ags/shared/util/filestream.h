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

#ifndef __AGS_CN_UTIL__FILESTREAM_H
#define __AGS_CN_UTIL__FILESTREAM_H

#include <stdio.h>

#include "util/datastream.h"
#include "util/file.h" // TODO: extract filestream mode constants

namespace AGS
{
namespace Common
{

class FileStream : public DataStream
{
public:

    // Represents an open file object
    // The constructor may raise std::runtime_error if 
    // - there is an issue opening the file (does not exist, locked, permissions, etc)
    // - the open mode could not be determined
    FileStream(const String &file_name, FileOpenMode open_mode, FileWorkMode work_mode,
        DataEndianess stream_endianess = kLittleEndian);
    ~FileStream() override;

    bool    HasErrors() const override;
    void    Close() override;
    bool    Flush() override;

    // Is stream valid (underlying data initialized properly)
    bool    IsValid() const override;
    // Is end of stream
    bool    EOS() const override;
    // Total length of stream (if known)
    soff_t  GetLength() const override;
    // Current position (if known)
    soff_t  GetPosition() const override;
    bool    CanRead() const override;
    bool    CanWrite() const override;
    bool    CanSeek() const override;

    size_t  Read(void *buffer, size_t size) override;
    int32_t ReadByte() override;
    size_t  Write(const void *buffer, size_t size) override;
    int32_t WriteByte(uint8_t b) override;

    bool    Seek(soff_t offset, StreamSeek origin) override;

private:
    void            Open(const String &file_name, FileOpenMode open_mode, FileWorkMode work_mode);

    FILE                *_file;
    const FileOpenMode  _openMode;
    const FileWorkMode  _workMode;
};

} // namespace Common
} // namespace AGS

#endif // __AGS_CN_UTIL__FILESTREAM_H
