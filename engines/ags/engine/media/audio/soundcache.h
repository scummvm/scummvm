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

#include "ags/engine/ac/asset_helper.h"
#include "common/stream.h"

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
} sound_cache_entry_t;

void clear_sound_cache();
void sound_cache_free(char *buffer);
char *get_cached_sound(const AssetPath &asset_name, size_t &size);
Common::SeekableReadStream *get_cached_sound(const AssetPath &asset_name);

} // namespace AGS3

#endif
