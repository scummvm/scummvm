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

#ifndef __AC_SOUNDCACHE_H
#define __AC_SOUNDCACHE_H

#include "ac/asset_helper.h"

// PSP: A simple sound cache. The size can be configured in the config file.
// The data rate while reading from disk on the PSP is usually between 500 to 900 kiB/s,
// caching the last used sound files therefore improves game performance.

//#define SOUND_CACHE_DEBUG

typedef struct
{
    char* file_name;
    int number;
    int free;
    unsigned int last_used;
    unsigned int size;
    char* data;
    int reference;
    bool is_wave;
} sound_cache_entry_t;

extern int psp_use_sound_cache;
extern int psp_sound_cache_max_size;
extern int psp_audio_cachesize;
extern int psp_midi_preload_patches;

void clear_sound_cache();
void sound_cache_free(char* buffer, bool is_wave);
char* get_cached_sound(const AssetPath &asset_name, bool is_wave, size_t &size);


#endif // __AC_SOUNDCACHE_H
