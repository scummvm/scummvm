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

#ifndef __AC_VIEW_H
#define __AC_VIEW_H

#include <vector>

namespace AGS { namespace Common { class Stream; } }
using namespace AGS; // FIXME later

#define VFLG_FLIPSPRITE 1

struct ViewFrame {
    int   pic;
    short xoffs, yoffs;
    short speed;
    int   flags;
    int   sound;  // play sound when this frame comes round
    int   reserved_for_future[2];
    ViewFrame();

    void ReadFromFile(Common::Stream *in);
    void WriteToFile(Common::Stream *out);
};

#define LOOPFLAG_RUNNEXTLOOP 1

struct ViewLoopNew
{
    short numFrames;
    int   flags;
    ViewFrame *frames;

    ViewLoopNew();
    void Initialize(int frameCount);
    void Dispose();
    bool RunNextLoop();
    void WriteToFile_v321(Common::Stream *out);
    void ReadFromFile_v321(Common::Stream *in);
    void WriteFrames_Aligned(Common::Stream *out);
    void ReadFrames_Aligned(Common::Stream *in);
};

struct ViewStruct
{
    short numLoops;
    ViewLoopNew *loops;

    ViewStruct();
    void Initialize(int loopCount);
    void Dispose();
    void WriteToFile(Common::Stream *out);
    void ReadFromFile(Common::Stream *in);
};

struct ViewStruct272 {
    short     numloops;
    short     numframes[16];
    int       loopflags[16];
    ViewFrame frames[16][20];

    ViewStruct272();
    void ReadFromFile(Common::Stream *in);
};

void Convert272ViewsToNew(const std::vector<ViewStruct272> &oldv, ViewStruct *newv);

#endif // __AC_VIEW_H