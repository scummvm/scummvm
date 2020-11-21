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

#include "debug/assert.h"
#include "util/alignedstream.h"
#include "util/math.h"

namespace AGS
{
namespace Common
{

AlignedStream::AlignedStream(Stream *stream, AlignedStreamMode mode, ObjectOwnershipPolicy stream_ownership_policy,
                             size_t base_alignment)
    : ProxyStream(stream, stream_ownership_policy)
    , _mode(mode)
    , _baseAlignment(base_alignment)
    , _maxAlignment(0)
    , _block(0)
{
}

AlignedStream::~AlignedStream()
{
    AlignedStream::Close();
}

void AlignedStream::Reset()
{
    if (!_stream)
    {
        return;
    }

    FinalizeBlock();
}

void AlignedStream::Close()
{
    if (!_stream)
    {
        return;
    }

    FinalizeBlock();
    ProxyStream::Close();
}

bool AlignedStream::CanRead() const
{
    return _stream ? (_mode == kAligned_Read && _stream->CanRead()) : false;
}

bool AlignedStream::CanWrite() const
{
    return _stream ? (_mode == kAligned_Write && _stream->CanWrite()) : false;
}

bool AlignedStream::CanSeek() const
{
    // Aligned stream does not support seeking, hence that will break padding count
    return false;
}

size_t AlignedStream::Read(void *buffer, size_t size)
{
    if (_stream)
    {
        ReadPadding(sizeof(int8_t));
        size = _stream->Read(buffer, size);
        _block += size;
        return size;
    }
    return 0;
}

int32_t AlignedStream::ReadByte()
{
    uint8_t b = 0;
    if (_stream)
    {
        ReadPadding(sizeof(uint8_t));
        b = _stream->ReadByte();
        _block += sizeof(uint8_t);
    }
    return b;
}

int16_t AlignedStream::ReadInt16()
{
    int16_t val = 0;
    if (_stream)
    {
        ReadPadding(sizeof(int16_t));
        val = _stream->ReadInt16();
        _block += sizeof(int16_t);
    }
    return val;
}

int32_t AlignedStream::ReadInt32()
{
    int32_t val = 0;
    if (_stream)
    {
        ReadPadding(sizeof(int32_t));
        val = _stream->ReadInt32();
        _block += sizeof(int32_t);
    }
    return val;
}

int64_t AlignedStream::ReadInt64()
{
    int64_t val = 0;
    if (_stream)
    {
        ReadPadding(sizeof(int64_t));
        val = _stream->ReadInt64();
        _block += sizeof(int64_t);
    }
    return val;
}

size_t AlignedStream::ReadArray(void *buffer, size_t elem_size, size_t count)
{
    if (_stream)
    {
        ReadPadding(elem_size);
        count = _stream->ReadArray(buffer, elem_size, count);
        _block += count * elem_size;
        return count;
    }
    return 0;
}

size_t AlignedStream::ReadArrayOfInt16(int16_t *buffer, size_t count)
{
    if (_stream)
    {
        ReadPadding(sizeof(int16_t));
        count = _stream->ReadArrayOfInt16(buffer, count);
        _block += count * sizeof(int16_t);
        return count;
    }
    return 0;
}

size_t AlignedStream::ReadArrayOfInt32(int32_t *buffer, size_t count)
{
    if (_stream)
    {
        ReadPadding(sizeof(int32_t));
        count = _stream->ReadArrayOfInt32(buffer, count);
        _block += count * sizeof(int32_t);
        return count;
    }
    return 0;
}

size_t AlignedStream::ReadArrayOfInt64(int64_t *buffer, size_t count)
{
    if (_stream)
    {
        ReadPadding(sizeof(int64_t));
        count = _stream->ReadArrayOfInt64(buffer, count);
        _block += count * sizeof(int64_t);
        return count;
    }
    return 0;
}

size_t AlignedStream::Write(const void *buffer, size_t size)
{
    if (_stream)
    {
        WritePadding(sizeof(int8_t));
        size = _stream->Write(buffer, size);
        _block += size;
        return size;
    }
    return 0;
}

int32_t AlignedStream::WriteByte(uint8_t b)
{
    if (_stream)
    {
        WritePadding(sizeof(uint8_t));
        b = _stream->WriteByte(b);
        _block += sizeof(uint8_t);
        return b;
    }
    return 0;
}

size_t AlignedStream::WriteInt16(int16_t val)
{
    if (_stream)
    {
        WritePadding(sizeof(int16_t));
        size_t size = _stream->WriteInt16(val);
        _block += sizeof(int16_t);
        return size;
    }
    return 0;
}

size_t AlignedStream::WriteInt32(int32_t val)
{
    if (_stream)
    {
        WritePadding(sizeof(int32_t));
        size_t size = _stream->WriteInt32(val);
        _block += sizeof(int32_t);
        return size;
    }
    return 0;
}

size_t AlignedStream::WriteInt64(int64_t val)
{
    if (_stream)
    {
        WritePadding(sizeof(int64_t));
        size_t size = _stream->WriteInt64(val);
        _block += sizeof(int64_t);
        return size;
    }
    return 0;
}

size_t AlignedStream::WriteArray(const void *buffer, size_t elem_size, size_t count)
{
    if (_stream)
    {
        WritePadding(elem_size);
        count = _stream->WriteArray(buffer, elem_size, count);
        _block += count * elem_size;
        return count;
    }
    return 0;
}

size_t AlignedStream::WriteArrayOfInt16(const int16_t *buffer, size_t count)
{
    if (_stream)
    {
        WritePadding(sizeof(int16_t));
        count = _stream->WriteArrayOfInt16(buffer, count);
        _block += count * sizeof(int16_t);
        return count;
    }
    return 0;
}

size_t AlignedStream::WriteArrayOfInt32(const int32_t *buffer, size_t count)
{
    if (_stream)
    {
        WritePadding(sizeof(int32_t));
        count = _stream->WriteArrayOfInt32(buffer, count);
        _block += count * sizeof(int32_t);
        return count;
    }
    return 0;
}

size_t AlignedStream::WriteArrayOfInt64(const int64_t *buffer, size_t count)
{
    if (_stream)
    {
        WritePadding(sizeof(int64_t));
        count = _stream->WriteArrayOfInt64(buffer, count);
        _block += count * sizeof(int64_t);
        return count;
    }
    return 0;
}

bool AlignedStream::Seek(soff_t offset, StreamSeek origin)
{
    // TODO: split out Seekable Stream interface
    return false;
}

void AlignedStream::ReadPadding(size_t next_type)
{
    if (!IsValid())
    {
        return;
    }

    if (next_type == 0)
    {
        return;
    }

    // The next is going to be evenly aligned data type,
    // therefore a padding check must be made
    if (next_type % _baseAlignment == 0)
    {
        int pad = _block % next_type;
        // Read padding only if have to
        if (pad)
        {
            // We do not know and should not care if the underlying stream
            // supports seek, so use read to skip the padding instead.
            for (size_t i = next_type - pad; i > 0; --i)
                _stream->ReadByte();
            _block += next_type - pad;
        }

        _maxAlignment = Math::Max(_maxAlignment, next_type);
        // Data is evenly aligned now
        if (_block % LargestPossibleType == 0)
        {
            _block = 0;
        }
    }
}

void AlignedStream::WritePadding(size_t next_type)
{
    if (!IsValid())
    {
        return;
    }

    if (next_type == 0)
    {
        return;
    }

    // The next is going to be evenly aligned data type,
    // therefore a padding check must be made
    if (next_type % _baseAlignment == 0)
    {
        int pad = _block % next_type;
        // Write padding only if have to
        if (pad)
        {
            _stream->WriteByteCount(0, next_type - pad);
            _block += next_type - pad;
        }

        _maxAlignment = Math::Max(_maxAlignment, next_type);
        // Data is evenly aligned now
        if (_block % LargestPossibleType == 0)
        {
            _block = 0;
        }
    }
}

void AlignedStream::FinalizeBlock()
{
    if (!IsValid())
    {
        return;
    }

    // Force the stream to read or write remaining padding to match the alignment
    if (_mode == kAligned_Read)
    {
        ReadPadding(_maxAlignment);
    }
    else if (_mode == kAligned_Write)
    {
        WritePadding(_maxAlignment);
    }

    _maxAlignment = 0;
    _block = 0;
}

} // namespace Common
} // namespace AGS
