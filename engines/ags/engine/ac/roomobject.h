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
#ifndef __AGS_EE_AC__ROOMOBJECT_H
#define __AGS_EE_AC__ROOMOBJECT_H

#include "ac/common_defines.h"

namespace AGS { namespace Common { class Stream; }}
using namespace AGS; // FIXME later

// IMPORTANT: this struct is restricted by plugin API!
struct RoomObject {
    int   x,y;
    int   transparent;    // current transparency setting
    short tint_r, tint_g;   // specific object tint
    short tint_b, tint_level;
    short tint_light;
    short zoom;           // zoom level, either manual or from the current area
    short last_width, last_height;   // width/height last time drawn
    short num;            // sprite slot number
    short baseline;       // <=0 to use Y co-ordinate; >0 for specific baseline
    short view,loop,frame; // only used to track animation - 'num' holds the current sprite
    short wait,moving;
    char  cycling;        // is it currently animating?
    char  overall_speed;
    char  on;
    char  flags;
    short blocking_width, blocking_height;

    RoomObject();

    int get_width();
    int get_height();
    int get_baseline();

    inline bool has_explicit_light() const { return (flags & OBJF_HASLIGHT) != 0; }
    inline bool has_explicit_tint()  const { return (flags & OBJF_HASTINT) != 0; }

	void UpdateCyclingView();
	void update_cycle_view_forwards();
	void update_cycle_view_backwards();

    void ReadFromFile(Common::Stream *in);
    void WriteToFile(Common::Stream *out) const;
};

#endif // __AGS_EE_AC__ROOMOBJECT_H
