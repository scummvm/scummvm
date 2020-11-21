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
#ifndef __AGS_CN_UTIL__PROXYSTREAM_H
#define __AGS_CN_UTIL__PROXYSTREAM_H

#include "util/stream.h"

namespace AGS
{
namespace Common
{

// TODO: replace with std::shared_ptr!!!
enum ObjectOwnershipPolicy
{
    kReleaseAfterUse,
    kDisposeAfterUse
};

class ProxyStream : public Stream
{
public:
    ProxyStream(Stream *stream, ObjectOwnershipPolicy stream_ownership_policy = kReleaseAfterUse);
    ~ProxyStream() override;

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
    int16_t ReadInt16() override;
    int32_t ReadInt32() override;
    int64_t ReadInt64() override;
    size_t  ReadArray(void *buffer, size_t elem_size, size_t count) override;
    size_t  ReadArrayOfInt16(int16_t *buffer, size_t count) override;
    size_t  ReadArrayOfInt32(int32_t *buffer, size_t count) override;
    size_t  ReadArrayOfInt64(int64_t *buffer, size_t count) override;

    size_t  Write(const void *buffer, size_t size) override;
    int32_t WriteByte(uint8_t b) override;
    size_t  WriteInt16(int16_t val) override;
    size_t  WriteInt32(int32_t val) override;
    size_t  WriteInt64(int64_t val) override;
    size_t  WriteArray(const void *buffer, size_t elem_size, size_t count) override;
    size_t  WriteArrayOfInt16(const int16_t *buffer, size_t count) override;
    size_t  WriteArrayOfInt32(const int32_t *buffer, size_t count) override;
    size_t  WriteArrayOfInt64(const int64_t *buffer, size_t count) override;

    bool    Seek(soff_t offset, StreamSeek origin) override;

protected:
    Stream                  *_stream;
    ObjectOwnershipPolicy   _streamOwnershipPolicy;
};

} // namespace Common
} // namespace AGS

#endif // __AGS_CN_UTIL__PROXYSTREAM_H
