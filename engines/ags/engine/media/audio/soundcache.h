/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef AGS_ENGINE_MEDIA_AUDIO_SOUNDCACHE_H
#define AGS_ENGINE_MEDIA_AUDIO_SOUNDCACHE_H

#include "ac/asset_helper.h"

namespace AGS3 {

// PSP: A simple sound cache. The size can be configured in the config file.
// The data rate while reading from disk on the PSP is usually between 500 to 900 kiB/s,
// caching the last used sound files therefore improves game performance.

//#define SOUND_CACHE_DEBUG

typedef struct {
	char *file_name;
	int number;
	int free;
	unsigned int last_used;
	unsigned int size;
	char *data;
	int reference;
	bool is_wave;
} sound_cache_entry_t;

extern int psp_use_sound_cache;
extern int psp_sound_cache_max_size;
extern int psp_audio_cachesize;
extern int psp_midi_preload_patches;

void clear_sound_cache();
void sound_cache_free(char *buffer, bool is_wave);
char *get_cached_sound(const AssetPath &asset_name, bool is_wave, size_t &size);

} // namespace AGS3

#endif
