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

#ifndef NO_MP3_PLAYER

#include "media/audio/clip_mystaticmp3.h"
#include "media/audio/audiointernaldefs.h"
#include "media/audio/soundcache.h"
#include "util/mutex_lock.h"

#include "platform/base/agsplatformdriver.h"

extern int our_eip;

// ALMP3 functions are not reentrant! This mutex should be locked before calling any
// of the mp3 functions and unlocked afterwards.
AGS::Engine::Mutex _mp3_mutex;

void MYSTATICMP3::poll()
{
    if (state_ != SoundClipPlaying) { return; }

    int oldeip = our_eip;
    our_eip = 5997;

    AGS::Engine::MutexLock _lockMp3(_mp3_mutex);
    int result = almp3_poll_mp3(tune);

    if (result == ALMP3_POLL_PLAYJUSTFINISHED)
    {
        if (!repeat)
        {
            state_ = SoundClipStopped;
        }
    }
    our_eip = oldeip;
}

void MYSTATICMP3::adjust_stream()
{
    if (!is_playing()) { return; }
    AGS::Engine::MutexLock _lockMp3(_mp3_mutex);
    almp3_adjust_mp3(tune, get_final_volume(), panning, speed, repeat);
}

void MYSTATICMP3::adjust_volume()
{
    adjust_stream();
}

void MYSTATICMP3::set_volume(int newvol)
{
    vol = newvol;
    adjust_stream();
}

void MYSTATICMP3::set_speed(int new_speed)
{
    speed = new_speed;
    adjust_stream();
}

void MYSTATICMP3::destroy()
{
    if (tune) {
        AGS::Engine::MutexLock _lockMp3(_mp3_mutex);
        almp3_stop_mp3(tune);
        almp3_destroy_mp3(tune);
    }
    tune = nullptr;

    if (mp3buffer) {
        sound_cache_free(mp3buffer, false);
    }
    mp3buffer = nullptr;

    state_ = SoundClipStopped;
}

void MYSTATICMP3::seek(int pos)
{
    if (!is_playing()) { return; }
    AGS::Engine::MutexLock _lockMp3(_mp3_mutex);
    almp3_seek_abs_msecs_mp3(tune, pos);
}

int MYSTATICMP3::get_pos()
{
    if (!is_playing()) { return -1; }
    AGS::Engine::MutexLock _lockMp3(_mp3_mutex);
    return almp3_get_pos_msecs_mp3(tune);
}

int MYSTATICMP3::get_pos_ms()
{
    int result = get_pos();
    return result;
}

int MYSTATICMP3::get_length_ms()
{
    if (tune == nullptr) { return -1; }
    AGS::Engine::MutexLock _lockMp3(_mp3_mutex);
    return almp3_get_length_msecs_mp3(tune);
}

int MYSTATICMP3::get_voice()
{
    if (!is_playing()) { return -1; }
	AGS::Engine::MutexLock _lockMp3(_mp3_mutex);
    AUDIOSTREAM *ast = almp3_get_audiostream_mp3(tune);
	return (ast != nullptr ? ast->voice : -1);
}

int MYSTATICMP3::get_sound_type() {
    return MUS_MP3;
}

int MYSTATICMP3::play() {
    if (tune == nullptr) { return 0; }

    {
        AGS::Engine::MutexLock _lockMp3(_mp3_mutex);
        int result = almp3_play_ex_mp3(tune, 16384, vol, panning, 1000, repeat);
        if (result != ALMP3_OK) {
            return 0;
        }
    }

    state_ = SoundClipPlaying;

    if (!psp_audio_multithreaded)
      poll();

    return 1;
}

MYSTATICMP3::MYSTATICMP3() : SOUNDCLIP() {
    tune = nullptr;
    mp3buffer = nullptr;
}

#endif // !NO_MP3_PLAYER
