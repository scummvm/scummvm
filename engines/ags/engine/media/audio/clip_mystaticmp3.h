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

#ifndef __AC_MYSTATICMP3_H
#define __AC_MYSTATICMP3_H

#include "almp3.h"
#include "media/audio/soundclip.h"

extern AGS::Engine::Mutex _mp3_mutex;

// pre-loaded (non-streaming) MP3 file
struct MYSTATICMP3:public SOUNDCLIP
{
    ALMP3_MP3 *tune;
    char *mp3buffer;

    void poll() override;

    void set_volume(int newvol) override;
    void set_speed(int new_speed) override;

    void destroy() override;

    void seek(int pos) override;

    int get_pos() override;

    int get_pos_ms() override;

    int get_length_ms() override;

    int get_sound_type() override;

    int play() override;

    MYSTATICMP3();

protected:
    int get_voice() override;
    void adjust_volume() override;
private:
    void adjust_stream();
};

#endif // __AC_MYSTATICMP3_H
