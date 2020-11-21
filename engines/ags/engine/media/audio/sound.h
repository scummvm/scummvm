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
// ACSOUND - AGS sound system wrapper
//
//=============================================================================

#ifndef __AC_SOUND_H
#define __AC_SOUND_H

#include "ac/asset_helper.h"
#include "media/audio/soundclip.h"

SOUNDCLIP *my_load_wave(const AssetPath &asset_name, int voll, int loop);
SOUNDCLIP *my_load_mp3(const AssetPath &asset_name, int voll);
SOUNDCLIP *my_load_static_mp3(const AssetPath &asset_name, int voll, bool loop);
SOUNDCLIP *my_load_static_ogg(const AssetPath &asset_name, int voll, bool loop);
SOUNDCLIP *my_load_ogg(const AssetPath &asset_name, int voll);
SOUNDCLIP *my_load_midi(const AssetPath &asset_name, int repet);
SOUNDCLIP *my_load_mod(const AssetPath &asset_name, int repet);

extern int use_extra_sound_offset;

#endif // __AC_SOUND_H
