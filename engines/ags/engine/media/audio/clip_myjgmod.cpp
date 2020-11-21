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

#include "media/audio/audiodefines.h"

#ifdef JGMOD_MOD_PLAYER

#include "media/audio/clip_myjgmod.h"
#include "media/audio/audiointernaldefs.h"

int MYMOD::poll()
{
    if (done)
        return done;

    if (is_mod_playing() == 0)
        done = 1;

    return done;
}

void MYMOD::set_volume(int newvol)
{
    vol = newvol;
    if (!done)
        set_mod_volume(newvol);
}

void MYMOD::destroy()
{
    stop_mod();
    destroy_mod(tune);
    tune = NULL;
}

void MYMOD::seek(int patnum)
{
    if (is_mod_playing() != 0)
        goto_mod_track(patnum);
}

int MYMOD::get_pos()
{
    if (!is_mod_playing())
        return -1;
    return mi.trk;
}

int MYMOD::get_pos_ms()
{
    return 0;                   // we don't know ms offset
}

int MYMOD::get_length_ms()
{  // we don't know ms
    return 0;
}

int MYMOD::get_voice()
{
    // MOD uses so many different voices it's not practical to keep track
    return -1;
}

int MYMOD::get_sound_type() {
    return MUS_MOD;
}

int MYMOD::play() {
    play_mod(tune, repeat);

    return 1;
}

MYMOD::MYMOD() : SOUNDCLIP() {
}

#endif // #ifdef JGMOD_MOD_PLAYER