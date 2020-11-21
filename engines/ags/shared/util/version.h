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
// Class, depicting version of the AGS engine
//
//=============================================================================
#ifndef __AGS_CN_MAIN__VERSION_H
#define __AGS_CN_MAIN__VERSION_H

#include "util/string.h"

namespace AGS
{
namespace Common
{

using Common::String;

struct Version
{
    int32_t Major;
    int32_t Minor;
    int32_t Release;
    int32_t Revision;
    String  Special;
    String  BuildInfo;

    String  LongString;
    String  ShortString;
    String  BackwardCompatibleString;

    // Last engine version, using different version format than AGS Editor (3.22.1120 / 3.2.2.1120)
    static const Version LastOldFormatVersion;

    Version();
    Version(int32_t major, int32_t minor, int32_t release);
    Version(int32_t major, int32_t minor, int32_t release, int32_t revision);
    Version(int32_t major, int32_t minor, int32_t release, int32_t revision, const String &special);
    Version(int32_t major, int32_t minor, int32_t release, int32_t revision, const String &special, const String &build_info);
    Version(const String &version_string);

    inline int32_t AsNumber() const
    {
        return Major * 10000 + Minor * 100 + Release;
    }

    inline int64_t AsLongNumber() const
    {
        return (int64_t)Major * 100000000L + (int64_t)Minor * 1000000L + (int64_t)Release * 10000L + Revision;
    }

    inline int32_t AsSmallNumber() const
    {
        return Major * 100 + Minor;
    }

    void SetFromString(const String &version_string);

    inline bool operator < (const Version &other) const
    {
        return AsLongNumber() < other.AsLongNumber();
    }

    inline bool operator <= (const Version &other) const
    {
        return AsLongNumber() <= other.AsLongNumber();
    }

    inline bool operator > (const Version &other) const
    {
        return AsLongNumber() > other.AsLongNumber();
    }

    inline bool operator >= (const Version &other) const
    {
        return AsLongNumber() >= other.AsLongNumber();
    }

    inline bool operator == (const Version &other) const
    {
        return AsLongNumber() == other.AsLongNumber();
    }

    inline bool operator != (const Version &other) const
    {
        return AsLongNumber() != other.AsLongNumber();
    }

private:
    void MakeString();
};

} // namespace Common
} // namespace AGS

#endif // __AGS_CN_MAIN__VERSION_H
