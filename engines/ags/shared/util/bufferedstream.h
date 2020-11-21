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
//
//
//=============================================================================
#ifndef __AGS_CN_UTIL__BUFFEREDSTREAM_H
#define __AGS_CN_UTIL__BUFFEREDSTREAM_H

#include <vector>
#include "util/filestream.h"
#include "util/file.h" // TODO: extract filestream mode constants

namespace AGS
{
namespace Common
{

// Needs tuning depending on the platform.
const auto BufferStreamSize = 8*1024;

class BufferedStream : public FileStream
{
public:
    // Represents an open _buffered_ file object
    // The constructor may raise std::runtime_error if 
    // - there is an issue opening the file (does not exist, locked, permissions, etc)
    // - the open mode could not be determined
    // - could not determine the length of the stream
    // It is recommended to use File::OpenFile to safely construct this object.
    BufferedStream(const String &file_name, FileOpenMode open_mode, FileWorkMode work_mode, DataEndianess stream_endianess = kLittleEndian);

    bool    EOS() const override; ///< Is end of stream
    soff_t  GetPosition() const override; ///< Current position (if known)

    size_t  Read(void *buffer, size_t size) override;
    int32_t ReadByte() override;
    size_t  Write(const void *buffer, size_t size) override;
    int32_t WriteByte(uint8_t b) override;

    bool    Seek(soff_t offset, StreamSeek origin) override;


private:

    soff_t _bufferPosition;
    std::vector<char> _buffer;

    soff_t _position;
    soff_t _end;

    void FillBufferFromPosition(soff_t position);
};

} // namespace Common
} // namespace AGS

#endif // __AGS_CN_UTIL__BUFFEREDSTREAM_H
