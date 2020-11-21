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

#ifdef DUMB_MOD_PLAYER

#include "media/audio/clip_mydumbmod.h"
#include "media/audio/audiointernaldefs.h"

void al_duh_set_loop(AL_DUH_PLAYER *dp, int loop) {
    DUH_SIGRENDERER *sr = al_duh_get_sigrenderer(dp);
    DUMB_IT_SIGRENDERER *itsr = duh_get_it_sigrenderer(sr);
    if (itsr == nullptr)
        return;

    if (loop)
        dumb_it_set_loop_callback(itsr, nullptr, nullptr);
    else
        dumb_it_set_loop_callback(itsr, dumb_it_callback_terminate, itsr);
}

void MYMOD::poll()
{
    if (state_ != SoundClipPlaying) { return; }

    if (al_poll_duh(duhPlayer)) {
        state_ = SoundClipStopped;
    }
}

void MYMOD::adjust_volume()
{
    if (!is_playing()) { return; }
    al_duh_set_volume(duhPlayer, VOLUME_TO_DUMB_VOL(get_final_volume()));
}

void MYMOD::set_volume(int newvol)
{
    vol = newvol;
    adjust_volume();
}

void MYMOD::destroy()
{
    if (duhPlayer) {
        al_stop_duh(duhPlayer);
    }
    duhPlayer = nullptr;

    if (tune) {
        unload_duh(tune);
    }
    tune = nullptr;

    state_ = SoundClipStopped;
}

void MYMOD::seek(int patnum)
{
    if (!is_playing()) { return; }

    al_stop_duh(duhPlayer);
    state_ = SoundClipInitial;

    DUH_SIGRENDERER *sr = dumb_it_start_at_order(tune, 2, patnum);
    duhPlayer = al_duh_encapsulate_sigrenderer(sr, VOLUME_TO_DUMB_VOL(vol), 8192, 22050);
    if (!duhPlayer) {
        duh_end_sigrenderer(sr);
        return;
    }

    al_duh_set_loop(duhPlayer, repeat);
    state_ = SoundClipPlaying;
}

int MYMOD::get_pos()
{
    if (!is_playing()) { return -1; }

    // determine the current track number (DUMB calls them 'orders')
    DUH_SIGRENDERER *sr = al_duh_get_sigrenderer(duhPlayer);
    DUMB_IT_SIGRENDERER *itsr = duh_get_it_sigrenderer(sr);
    if (itsr == nullptr)
        return -1;

    return dumb_it_sr_get_current_order(itsr);
}

int MYMOD::get_real_mod_pos()
{
    if (!is_playing()) { return -1; }
    return al_duh_get_position(duhPlayer);
}

int MYMOD::get_pos_ms()
{
    if (!is_playing()) { return -1; }
    return (get_real_mod_pos() * 10) / 655;
}

int MYMOD::get_length_ms()
{
    if (tune == nullptr)
        return 0;

    // duh_get_length represents time as 65536ths of a second
    return (duh_get_length(tune) * 10) / 655;
}

int MYMOD::get_voice()
{
    // MOD uses so many different voices it's not practical to keep track
    return -1;
}

void MYMOD::pause() {
    if (state_ != SoundClipPlaying) { return; }
    al_pause_duh(duhPlayer);
    state_ = SoundClipPaused;
}

void MYMOD::resume() {
    if (state_ != SoundClipPaused) { return; }
    al_resume_duh(duhPlayer);
    state_ = SoundClipPlaying;
}

int MYMOD::get_sound_type() {
    return MUS_MOD;
}

int MYMOD::play() {
    if (tune == nullptr) { return 0; }
    
    duhPlayer = al_start_duh(tune, 2, 0, 1.0, 8192, 22050);
    if (!duhPlayer) {
        return 0;
    }
    al_duh_set_loop(duhPlayer, repeat);
    set_volume(vol);

    state_ = SoundClipPlaying;
    return 1;
}  

MYMOD::MYMOD() : SOUNDCLIP() {
    tune = nullptr;
    duhPlayer = nullptr;
}

#endif // DUMB_MOD_PLAYER