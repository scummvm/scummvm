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

#include "util/stream.h"

namespace AGS
{
namespace Common
{

size_t Stream::WriteByteCount(uint8_t b, size_t count)
{
    if (!CanWrite())
        return 0;
    size_t size = 0;
    for (; count > 0; --count, ++size)
    {
        if (WriteByte(b) < 0)
            break;
    }
    return size;
}

} // namespace Common
} // namespace AGS
