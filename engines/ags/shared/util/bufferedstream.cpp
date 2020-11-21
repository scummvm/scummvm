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
#include <algorithm>
#include <cstring>
#include <stdexcept>
#include "util/bufferedstream.h"
#include "util/stdio_compat.h"
#include "util/string.h"

namespace AGS
{
namespace Common
{

BufferedStream::BufferedStream(const String &file_name, FileOpenMode open_mode, FileWorkMode work_mode, DataEndianess stream_endianess)
        : FileStream(file_name, open_mode, work_mode, stream_endianess), _buffer(BufferStreamSize), _bufferPosition(0), _position(0)
{
    if (FileStream::Seek(0, kSeekEnd) == false)
        throw std::runtime_error("Error determining stream end.");

    _end = FileStream::GetPosition();
    if (_end == -1)
        throw std::runtime_error("Error determining stream end.");

    if (FileStream::Seek(0, kSeekBegin) == false)
        throw std::runtime_error("Error determining stream end.");

    _buffer.resize(0);

}

void BufferedStream::FillBufferFromPosition(soff_t position)
{
    FileStream::Seek(position, kSeekBegin);

    _buffer.resize(BufferStreamSize);
    auto sz = FileStream::Read(_buffer.data(), BufferStreamSize);
    _buffer.resize(sz);

    _bufferPosition = position;
}

bool BufferedStream::EOS() const
{
    return _position == _end;
}

soff_t BufferedStream::GetPosition() const
{
    return _position;
}

size_t BufferedStream::Read(void *toBuffer, size_t toSize)
{
    auto to = static_cast<char *>(toBuffer);

    while(toSize > 0)
    {
        if (_position < _bufferPosition || _position >= _bufferPosition + _buffer.size())
        {
            FillBufferFromPosition(_position);
        }
        if (_buffer.size() <= 0) { break; } // reached EOS
        assert(_position >= _bufferPosition && _position < _bufferPosition + _buffer.size());  // sanity check only, should be checked by above.

        soff_t bufferOffset = _position - _bufferPosition;
        assert(bufferOffset >= 0);
        size_t bytesLeft = _buffer.size() - (size_t)bufferOffset;
        size_t chunkSize = std::min<size_t>(bytesLeft, toSize);

        std::memcpy(to, _buffer.data()+bufferOffset, chunkSize);

        to += chunkSize;
        _position += chunkSize;
        toSize -= chunkSize;
    }

    return to - (char*)toBuffer;
}

int32_t BufferedStream::ReadByte()
{
    uint8_t ch;
    auto bytesRead = Read(&ch, 1);
    if (bytesRead != 1) { return EOF; }
    return ch;
}

size_t BufferedStream::Write(const void *buffer, size_t size)
{ 
    FileStream::Seek(_position, kSeekBegin);
    auto sz = FileStream::Write(buffer, size);
    if (_position == _end)
        _end += sz;
    _position += sz;
    return sz;
}

int32_t BufferedStream::WriteByte(uint8_t val)
{
    auto sz = Write(&val, 1);
    if (sz != 1) { return -1; }
    return sz;
}

bool BufferedStream::Seek(soff_t offset, StreamSeek origin)
{
    soff_t want_pos = -1;
    switch(origin)
    {
        case StreamSeek::kSeekCurrent:  want_pos = _position   + offset; break;
        case StreamSeek::kSeekBegin:    want_pos = 0           + offset; break;
        case StreamSeek::kSeekEnd:      want_pos = _end        + offset; break;
        break;
    }

    // clamp
    _position = std::min(std::max(want_pos, (soff_t)0), _end);
    return _position == want_pos;
}

} // namespace Common
} // namespace AGS
