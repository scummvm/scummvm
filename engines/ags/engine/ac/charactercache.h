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
#ifndef __AGS_EE_AC__CHARACTERCACHE_H
#define __AGS_EE_AC__CHARACTERCACHE_H

namespace AGS { namespace Common { class Bitmap; } }
using namespace AGS; // FIXME later

// stores cached info about the character
struct CharacterCache {
    Common::Bitmap *image;
    int sppic;
    int scaling;
    int inUse;
    short tintredwas, tintgrnwas, tintbluwas, tintamntwas;
    short lightlevwas, tintlightwas;
    // no mirroredWas is required, since the code inverts the sprite number
};

#endif // __AGS_EE_AC__CHARACTERCACHE_H
