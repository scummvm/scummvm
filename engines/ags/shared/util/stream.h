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
// Base stream class.
//
// Provides default implementation for a few helper methods.
// 
// Only streams with uncommon behavior should be derived directly from Stream.
// Most I/O devices should inherit DataStream instead.
// Streams that wrap other streams should inherit ProxyStream.
//
//=============================================================================
#ifndef __AGS_CN_UTIL__STREAM_H
#define __AGS_CN_UTIL__STREAM_H

#include "api/stream_api.h"

namespace AGS
{
namespace Common
{

class Stream : public IAGSStream
{
public:
    // Tells if the stream has errors
    virtual bool HasErrors() const { return false; }
    // Flush stream buffer to the underlying device
    virtual bool Flush() = 0;

    //-----------------------------------------------------
    // Helper methods
    //-----------------------------------------------------
    inline int8_t ReadInt8() override
    {
        return ReadByte();
    }

    inline size_t WriteInt8(int8_t val) override
    {
        int32_t ival = WriteByte(val);
        return ival >= 0 ? ival : 0;
    }

    inline bool ReadBool() override
    {
        return ReadInt8() != 0;
    }

    inline size_t WriteBool(bool val) override
    {
        return WriteInt8(val ? 1 : 0);
    }

    // Practically identical to Read() and Write(), these two helpers' only
    // meaning is to underline the purpose of data being (de)serialized
    inline size_t ReadArrayOfInt8(int8_t *buffer, size_t count) override
    {
        return Read(buffer, count);
    }
    inline size_t WriteArrayOfInt8(const int8_t *buffer, size_t count) override
    {
        return Write(buffer, count);
    }

    // Fill the requested number of bytes with particular value
    size_t WriteByteCount(uint8_t b, size_t count);
};

} // namespace Common
} // namespace AGS

#endif // __AGS_CN_UTIL__STREAM_H
