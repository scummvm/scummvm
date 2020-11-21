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

#include "ac/cdaudio.h"
#include "platform/base/agsplatformdriver.h"

int use_cdplayer=0;
bool triedToUseCdAudioCommand = false;
int need_to_stop_cd=0;

int init_cd_player() 
{
    use_cdplayer=0;
    return platform->InitializeCDPlayer();
}

int cd_manager(int cmdd,int datt) 
{
    if (!triedToUseCdAudioCommand)
    {
        triedToUseCdAudioCommand = true;
        init_cd_player();
    }
    if (cmdd==0) return use_cdplayer;
    if (use_cdplayer==0) return 0;  // ignore other commands

    return platform->CDPlayerCommand(cmdd, datt);
}
