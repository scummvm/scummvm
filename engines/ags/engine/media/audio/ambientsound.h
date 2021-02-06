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

#ifndef __AC_AMBIENTSOUND_H
#define __AC_AMBIENTSOUND_H

// Forward declaration
namespace AGS { namespace Common { class Stream; } }
using namespace AGS; // FIXME later

#define AMBIENCE_FULL_DIST 25

struct AmbientSound {
    int  channel;  // channel number, 1 upwards
    int  x,y;
    int  vol;
    int  num;  // sound number, eg. 3 = sound3.wav
    int  maxdist;

    bool IsPlaying();

    void ReadFromFile(Common::Stream *in);
    void WriteToFile(Common::Stream *out);
};

#endif // __AC_AMBIENTSOUND_H
