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

#include "ac/global_timer.h"
#include "ac/runtime_defines.h"
#include "ac/common.h"
#include "ac/gamestate.h"

extern GameState play;


void script_SetTimer(int tnum,int timeout) {
    if ((tnum < 1) || (tnum >= MAX_TIMERS))
        quit("!StartTimer: invalid timer number");
    play.script_timers[tnum] = timeout;
}

int IsTimerExpired(int tnum) {
    if ((tnum < 1) || (tnum >= MAX_TIMERS))
        quit("!IsTimerExpired: invalid timer number");
    if (play.script_timers[tnum] == 1) {
        play.script_timers[tnum] = 0;
        return 1;
    }
    return 0;
}
