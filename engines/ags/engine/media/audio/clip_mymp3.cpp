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

#include "media/audio/clip_mymp3.h"
#include "media/audio/audiointernaldefs.h"
#include "ac/common.h"               // quit()
#include "ac/asset_helper.h"
#include "util/mutex_lock.h"

#include "platform/base/agsplatformdriver.h"


void MYMP3::poll()
{
    if (state_ != SoundClipPlaying) { return; }

    // update the buffer
    char *tempbuf = nullptr;
    {
        AGS::Engine::MutexLock _lockMp3(_mp3_mutex);
        tempbuf = (char *)almp3_get_mp3stream_buffer(stream);
    }

    if (tempbuf != nullptr) {
        AGS_PACKFILE_OBJ* obj = (AGS_PACKFILE_OBJ*)in->userdata;
        int free_val = -1;
        if (chunksize >= obj->remains) {
            chunksize = obj->remains;
            free_val = chunksize;
        }
        pack_fread(tempbuf, chunksize, in);

        {
            AGS::Engine::MutexLock _lockMp3(_mp3_mutex);
            almp3_free_mp3stream_buffer(stream, free_val);
        }
    }

    {
        AGS::Engine::MutexLock _lockMp3(_mp3_mutex);
        if (almp3_poll_mp3stream(stream) == ALMP3_POLL_PLAYJUSTFINISHED) {
            state_ = SoundClipStopped;
        }
    }
}

void MYMP3::adjust_stream()
{
    if (!is_playing()) { return; }
    AGS::Engine::MutexLock _lockMp3(_mp3_mutex);
    almp3_adjust_mp3stream(stream, get_final_volume(), panning, speed);
}

void MYMP3::adjust_volume()
{
    adjust_stream();
}

void MYMP3::set_volume(int newvol)
{
    // boost MP3 volume
    newvol += 20;
    if (newvol > 255)
        newvol = 255;

    vol = newvol;
    adjust_stream();
}

void MYMP3::set_speed(int new_speed)
{
    speed = new_speed;
    adjust_stream();
}

void MYMP3::destroy()
{
    if (stream) {
        AGS::Engine::MutexLock _lockMp3(_mp3_mutex);
        almp3_stop_mp3stream(stream);
        almp3_destroy_mp3stream(stream);
    }
    stream = nullptr;

    if (buffer)
        free(buffer);
    buffer = nullptr;

    pack_fclose(in);

    state_ = SoundClipStopped;
}

void MYMP3::seek(int pos)
{
    if (!is_playing()) { return; }
    quit("Tried to seek an mp3stream");
}

int MYMP3::get_pos()
{
    return 0; // Return 0 to signify that Seek is not supported
    // return almp3_get_pos_msecs_mp3stream (stream);
}

int MYMP3::get_pos_ms()
{
    if (!is_playing()) { return -1; }
	AGS::Engine::MutexLock _lockMp3(_mp3_mutex);
    return almp3_get_pos_msecs_mp3stream(stream);
}

int MYMP3::get_length_ms()
{
    if (!is_playing()) { return -1; }
	AGS::Engine::MutexLock _lockMp3(_mp3_mutex);
    return almp3_get_length_msecs_mp3stream(stream, filesize);
}

int MYMP3::get_voice()
{
    if (!is_playing()) { return -1; }
	AGS::Engine::MutexLock _lockMp3(_mp3_mutex);
    AUDIOSTREAM *ast = almp3_get_audiostream_mp3stream(stream);
    return (ast != nullptr ? ast->voice : -1);
}

int MYMP3::get_sound_type() {
    return MUS_MP3;
}

int MYMP3::play() {
    if (in == nullptr) { return 0; }

    {
	    AGS::Engine::MutexLock _lockMp3(_mp3_mutex);
        if (almp3_play_mp3stream(stream, chunksize, (vol > 230) ? vol : vol + 20, panning) != ALMP3_OK) {
            return 0;
        }
    }

    state_ = SoundClipPlaying;

    if (!psp_audio_multithreaded)
      poll();

    return 1;
}

MYMP3::MYMP3() : SOUNDCLIP() {
    stream = nullptr;
    in = nullptr;
    filesize = 0;
    buffer = nullptr;
    chunksize = 0;
}

#endif // !NO_MP3_PLAYER