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
// WFNFont - an immutable AGS font object.
//
//-----------------------------------------------------------------------------
//
// WFN format:
// - signature            ( 15 )
// - offsets table offset (  2 )
// - characters table (for unknown number of char items):
// -     width            (  2 )
// -     height           (  2 )
// -     pixel bits       ( (width / 8 + 1) * height )
// -     any unknown data
// - offsets table (for X chars):
// -     character offset (  2 )
//
// NOTE: unfortunately, at the moment the format does not provide means to
// know the number of supported characters for certain, and the size of the
// data (file) is used to determine that.
//
//=============================================================================

#ifndef __AGS_CN_FONT__WFNFONT_H
#define __AGS_CN_FONT__WFNFONT_H

#include <vector>
#include "core/types.h"

namespace AGS { namespace Common { class Stream; } }

enum WFNError
{
    kWFNErr_NoError,
    kWFNErr_BadSignature,
    kWFNErr_BadTableAddress,
    kWFNErr_HasBadCharacters
};

struct WFNChar
{
    uint16_t       Width;
    uint16_t       Height;
    const uint8_t *Data;

    WFNChar();

    inline size_t GetRowByteCount() const
    {
        return (Width + 7) / 8;
    }

    inline size_t GetRequiredPixelSize() const
    {
        return GetRowByteCount() * Height;
    }

    // Ensure character's width & height fit in given number of pixel bytes
    void RestrictToBytes(size_t bytes);
};


class WFNFont
{
public:
    inline uint16_t GetCharCount() const
    {
        return static_cast<uint16_t>(_refs.size());
    }

    // Get WFN character for the given code; if the character is missing, returns empty character
    inline const WFNChar &GetChar(uint8_t code) const
    {
        return code < _refs.size() ? *_refs[code] : _emptyChar;
    }

    void Clear();
    // Reads WFNFont object, using data_size bytes from stream; if data_size = 0,
    // the available stream's length is used instead. Returns error code.
    WFNError ReadFromFile(AGS::Common::Stream *in, const soff_t data_size = 0);

protected:
    std::vector<const WFNChar*> _refs;      // reference array, contains pointers to elements of _items
    std::vector<WFNChar>        _items;     // actual character items
    std::vector<uint8_t>        _pixelData; // pixel data array

    static const WFNChar        _emptyChar; // a dummy character to substitute bad symbols
};

#endif // __AGS_CN_FONT__WFNFONT_H
