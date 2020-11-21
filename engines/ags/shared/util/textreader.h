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
// Specialized interface for reading plain text from the underlying source
//
//=============================================================================
#ifndef __AGS_CN_UTIL__TEXTREADER_H
#define __AGS_CN_UTIL__TEXTREADER_H

#include "util/string.h"

namespace AGS
{
namespace Common
{

class TextReader
{
public:
    virtual ~TextReader() = default;

    virtual bool IsValid() const            = 0;

    // Read single character
    virtual char    ReadChar()              = 0;
    // Read defined number of characters
    virtual String  ReadString(size_t length) = 0;
    // Read till line break
    virtual String  ReadLine()              = 0;
    // Read till end of available data
    virtual String  ReadAll()               = 0;
};

} // namespace Common
} // namespace AGS

#endif // __AGS_CN_UTIL__TEXTSTREAM_H
