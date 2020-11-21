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

#include "util/proxystream.h"

namespace AGS
{
namespace Common
{

ProxyStream::ProxyStream(Stream *stream, ObjectOwnershipPolicy stream_ownership_policy)
    : _stream(stream)
    , _streamOwnershipPolicy(stream_ownership_policy)
{
}

ProxyStream::~ProxyStream()
{
    ProxyStream::Close();
}

void ProxyStream::Close()
{
    if (_stream && _streamOwnershipPolicy == kDisposeAfterUse)
    {
        delete _stream;
    }
    _stream = nullptr;
}

bool ProxyStream::Flush()
{
    if (_stream)
    {
        return _stream->Flush();
    }
    return false;
}

bool ProxyStream::IsValid() const
{
    return _stream && _stream->IsValid();
}

bool ProxyStream::EOS() const
{
    return _stream ? _stream->EOS() : true;
}

soff_t ProxyStream::GetLength() const
{
    return _stream ? _stream->GetLength() : 0;
}

soff_t ProxyStream::GetPosition() const
{
    return _stream ? _stream->GetPosition() : -1;
}

bool ProxyStream::CanRead() const
{
    return _stream ? _stream->CanRead() : false;
}

bool ProxyStream::CanWrite() const
{
    return _stream ? _stream->CanWrite() : false;
}

bool ProxyStream::CanSeek() const
{
    return _stream ? _stream->CanSeek() : false;
}

size_t ProxyStream::Read(void *buffer, size_t size)
{
    return _stream ? _stream->Read(buffer, size) : 0;
}

int32_t ProxyStream::ReadByte()
{
    return _stream ? _stream->ReadByte() : 0;
}

int16_t ProxyStream::ReadInt16()
{
    return _stream ? _stream->ReadInt16() : 0;
}

int32_t ProxyStream::ReadInt32()
{
    return _stream ? _stream->ReadInt32() : 0;
}

int64_t ProxyStream::ReadInt64()
{
    return _stream ? _stream->ReadInt64() : 0;
}

size_t ProxyStream::ReadArray(void *buffer, size_t elem_size, size_t count)
{
    return _stream ? _stream->ReadArray(buffer, elem_size, count) : 0;
}

size_t ProxyStream::ReadArrayOfInt16(int16_t *buffer, size_t count)
{
    return _stream ? _stream->ReadArrayOfInt16(buffer, count) : 0;
}

size_t ProxyStream::ReadArrayOfInt32(int32_t *buffer, size_t count)
{
    return _stream ? _stream->ReadArrayOfInt32(buffer, count) : 0;
}

size_t ProxyStream::ReadArrayOfInt64(int64_t *buffer, size_t count)
{
    return _stream ? _stream->ReadArrayOfInt64(buffer, count) : 0;
}

size_t ProxyStream::Write(const void *buffer, size_t size)
{
    return _stream ? _stream->Write(buffer, size) : 0;
}

int32_t ProxyStream::WriteByte(uint8_t b)
{
    return _stream ? _stream->WriteByte(b) : 0;
}

size_t ProxyStream::WriteInt16(int16_t val)
{
    return _stream ? _stream->WriteInt16(val) : 0;
}

size_t ProxyStream::WriteInt32(int32_t val)
{
    return _stream ? _stream->WriteInt32(val) : 0;
}

size_t ProxyStream::WriteInt64(int64_t val)
{
    return _stream ? _stream->WriteInt64(val) : 0;
}

size_t ProxyStream::WriteArray(const void *buffer, size_t elem_size, size_t count)
{
    return _stream ? _stream->WriteArray(buffer, elem_size, count) : 0;
}

size_t ProxyStream::WriteArrayOfInt16(const int16_t *buffer, size_t count)
{
    return _stream ? _stream->WriteArrayOfInt16(buffer, count) : 0;
}

size_t ProxyStream::WriteArrayOfInt32(const int32_t *buffer, size_t count)
{
    return _stream ? _stream->WriteArrayOfInt32(buffer, count) : 0;
}

size_t ProxyStream::WriteArrayOfInt64(const int64_t *buffer, size_t count)
{
    return _stream ? _stream->WriteArrayOfInt64(buffer, count) : 0;
}

bool ProxyStream::Seek(soff_t offset, StreamSeek origin)
{
    return _stream ? _stream->Seek(offset, origin) : false;
}

} // namespace Common
} // namespace AGS
