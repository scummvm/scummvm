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

#include <allegro.h>
#include "ac/gamesetup.h"
#include "ac/gamestate.h"
#include "ac/global_audio.h"
#include "ac/global_game.h"
#include "ac/global_video.h"
#include "ac/path_helper.h"
#include "debug/debugger.h"
#include "media/video/video.h"
#include "media/audio/audio_system.h"
#include "platform/base/agsplatformdriver.h"
#include "util/string_compat.h"


void scrPlayVideo(const char* name, int skip, int flags) {
    EndSkippingUntilCharStops();

    if (play.fast_forward)
        return;
    if (debug_flags & DBG_NOVIDEO)
        return;

    if ((flags < 10) && (usetup.digicard == DIGI_NONE)) {
        // if game audio is disabled in Setup, then don't
        // play any sound on the video either
        flags += 10;
    }

    pause_sound_if_necessary_and_play_video(name, skip, flags);
}


#ifndef AGS_NO_VIDEO_PLAYER

void pause_sound_if_necessary_and_play_video(const char *name, int skip, int flags)
{
    int musplaying = play.cur_music_number, i;
    int ambientWas[MAX_SOUND_CHANNELS];
    for (i = 1; i < MAX_SOUND_CHANNELS; i++)
        ambientWas[i] = ambient[i].channel;

    if ((strlen(name) > 3) && (ags_stricmp(&name[strlen(name) - 3], "ogv") == 0))
    {
        play_theora_video(name, skip, flags);
    }
    else
    {
        char videoFilePath[MAX_PATH];
        get_install_dir_path(videoFilePath, name);

        platform->PlayVideo(videoFilePath, skip, flags);
    }

    if (flags < 10) 
    {
        update_music_volume();
        // restart the music
        if (musplaying >= 0)
            newmusic (musplaying);
        for (i = 1; i < MAX_SOUND_CHANNELS; i++) {
            if (ambientWas[i] > 0)
                PlayAmbientSound(ambientWas[i], ambient[i].num, ambient[i].vol, ambient[i].x, ambient[i].y);
        }
    }
}

#else

void pause_sound_if_necessary_and_play_video(const char *name, int skip, int flags) {}

#endif